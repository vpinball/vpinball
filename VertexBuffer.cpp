#include "stdafx.h"
#include "VertexBuffer.h"
#include "RenderDevice.h"
#include "Shader.h"

static unsigned int fvfToSize(const DWORD fvf)
{
   switch (fvf)
   {
   case VertexFormat::VF_POS_NORMAL_TEX: return sizeof(Vertex3D_NoTex2);
   case VertexFormat::VF_POS_TEX: return sizeof(Vertex3D_TexelOnly);
   default: assert(false); return 0; // Unsupported Vertex Format
   }
}

vector<VertexBuffer::SharedBuffer*> VertexBuffer::pendingSharedBuffers;

VertexBuffer::VertexBuffer(RenderDevice* rd, const unsigned int vertexCount, const float* verts, const bool isDynamic, const VertexFormat fvf)
   : m_rd(rd)
   , m_vertexCount(vertexCount)
   , m_vertexFormat(fvf)
   , m_sizePerVertex(fvfToSize(fvf))
   , m_isStatic(!isDynamic)
   , m_size(fvfToSize(fvf) * vertexCount)
{
   #ifndef __OPENGLES__
   // Disabled since OpenGL ES does not support glDrawElementsBaseVertex and we need it unless we remap the indices when creating the index buffer (and we should)
   for (SharedBuffer* block : pendingSharedBuffers)
   {
      if (block->format == fvf && block->isStatic == m_isStatic && block->count + vertexCount <= 65535)
      {
         m_sharedBuffer = block;
         break;
      }
   }
   #endif
   if (m_sharedBuffer == nullptr)
   {
      m_sharedBuffer = new SharedBuffer();
      m_sharedBuffer->format = fvf;
      m_sharedBuffer->isStatic = m_isStatic;
      pendingSharedBuffers.push_back(m_sharedBuffer);
   }
   m_vertexOffset = m_sharedBuffer->count;
   m_offset = m_vertexOffset * m_sizePerVertex;
   m_sharedBuffer->buffers.push_back(this);
   m_sharedBuffer->count += vertexCount;
   if (verts != nullptr)
   {
      void* data;
      lock(0, 0, &data, 0);
      memcpy(data, verts, m_size);
      unlock();
   }
}

VertexBuffer::~VertexBuffer()
{
   for (PendingUpload upload : m_pendingUploads)
      delete[] upload.data;
   RemoveFromVectorSingle(m_sharedBuffer->buffers, this);
   if (m_sharedBuffer->buffers.size() == 0)
   {
      RemoveFromVectorSingle(pendingSharedBuffers, m_sharedBuffer);
      if (IsCreated())
      #if defined(ENABLE_SDL) // OpenGL
         glDeleteBuffers(1, &m_vb);
      #else // DirectX 9
         SAFE_RELEASE(m_vb);
      #endif
      delete m_sharedBuffer;
   }
}

void VertexBuffer::lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void **dataBuffer, const DWORD flags)
{
   assert(!m_isStatic || !IsCreated()); // Static buffers can't be locked after first upload
   assert(m_lock.data == nullptr); // Lock is not reentrant
   m_rd->m_curLockCalls++;
   m_lock.offset = offsetToLock;
   m_lock.size = sizeToLock == 0 ? m_size : sizeToLock;
   assert(m_lock.offset + m_lock.size <= m_size);
   m_lock.data = new BYTE[m_lock.size];
   *dataBuffer = m_lock.data;
}

void VertexBuffer::unlock()
{
   assert(m_lock.data != nullptr);
   PendingUpload upload = m_lock;
   m_pendingUploads.push_back(upload);
   m_lock.data = nullptr;
}

void VertexBuffer::CreateSharedBuffer(SharedBuffer* sharedBuffer)
{
   RemoveFromVectorSingle(pendingSharedBuffers, sharedBuffer);
   unsigned int size = sharedBuffer->count * fvfToSize(sharedBuffer->format);

   #if defined(ENABLE_SDL) // OpenGL
   UINT8* data = (UINT8*)malloc(size);

   #else // DirectX 9
   // NB: We always specify WRITEONLY since MSDN states,
   // "Buffers created with D3DPOOL_DEFAULT that do not specify D3DUSAGE_WRITEONLY may suffer a severe performance penalty."
   // This means we cannot read from vertex buffers, but I don't think we need to.
   IDirect3DVertexBuffer9* vb = nullptr;
   CHECKD3D(sharedBuffer->buffers[0]->m_rd->GetCoreDevice()->CreateVertexBuffer(size, D3DUSAGE_WRITEONLY | (sharedBuffer->isStatic ? 0 : D3DUSAGE_DYNAMIC), 0 /* sharedBuffer->format */, D3DPOOL_DEFAULT, &vb, nullptr));
   UINT8* data;
   CHECKD3D(vb->Lock(0, size, (void**)&data, 0));
   #endif

   for (VertexBuffer* buffer : sharedBuffer->buffers)
   {
      for (PendingUpload upload : buffer->m_pendingUploads)
      {
         assert(buffer->m_offset + upload.offset >= 0);
         assert(buffer->m_offset + upload.offset + upload.size <= sharedBuffer->count * buffer->m_sizePerVertex);
         memcpy(data + buffer->m_offset + upload.offset, upload.data, upload.size);
         delete[] upload.data;
      }
      buffer->m_pendingUploads.clear();
   }

   #if defined(ENABLE_SDL) // OpenGL
   GLuint vb = 0;
   #ifndef __OPENGLES__
   if (GLAD_GL_VERSION_4_5)
   {
      glCreateBuffers(1, &vb);
      glNamedBufferStorage(vb, size, data, sharedBuffer->isStatic ? 0 : GL_DYNAMIC_STORAGE_BIT);
   }
   else if (GLAD_GL_VERSION_4_4)
   {
      glGenBuffers(1, &vb);
      glBindBuffer(GL_ARRAY_BUFFER, vb);
      glBufferStorage(GL_ARRAY_BUFFER, size, data, sharedBuffer->isStatic ? 0 : GL_DYNAMIC_STORAGE_BIT);
   }
   else
   #endif
   {
      glGenBuffers(1, &vb);
      glBindBuffer(GL_ARRAY_BUFFER, vb);
      glBufferData(GL_ARRAY_BUFFER, size, data, sharedBuffer->isStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
   }
   free(data);

   #else ENABLE_SDL // DirectX 9
   CHECKD3D(vb->Unlock());
   #endif

   for (VertexBuffer* buffer : sharedBuffer->buffers)
      buffer->m_vb = vb;
}

void VertexBuffer::Upload()
{
   if (!IsCreated())
      CreateSharedBuffer(m_sharedBuffer);
   else if (m_pendingUploads.size() > 0)
   {
      for (PendingUpload upload : m_pendingUploads)
      {
         #if defined(ENABLE_SDL) // OpenGL & OpenGL ES
         #ifndef __OPENGLES__
         if (GLAD_GL_VERSION_4_5)
            glNamedBufferSubData(m_vb, m_offset + upload.offset, upload.size, upload.data);
         else
         #endif
         {
            glBindBuffer(GL_ARRAY_BUFFER, m_vb);
            glBufferSubData(GL_ARRAY_BUFFER, m_offset + upload.offset, upload.size, upload.data);
         }

         #else // DirectX 9
         UINT8* data;
         CHECKD3D(m_vb->Lock(m_offset + upload.offset, upload.size, (void**)&data, 0));
         memcpy(data, upload.data, upload.size);
         CHECKD3D(m_vb->Unlock());

         #endif
         delete[] upload.data;
      }
      m_pendingUploads.clear();
   }
}
