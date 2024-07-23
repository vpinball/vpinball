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


void SharedVertexBuffer::Upload()
{
   if (!IsUploaded())
   {
      unsigned int size = m_count * m_bytePerElement;

      // Create data block
      #if defined(ENABLE_SDL) // OpenGL
      UINT8* data = (UINT8*)malloc(size);
      #else // DirectX 9
      // NB: We always specify WRITEONLY since MSDN states,
      // "Buffers created with D3DPOOL_DEFAULT that do not specify D3DUSAGE_WRITEONLY may suffer a severe performance penalty."
      // This means we cannot read from vertex buffers, but I don't think we need to.
      CHECKD3D(m_buffers[0]->m_rd->GetCoreDevice()->CreateVertexBuffer(size, D3DUSAGE_WRITEONLY | (m_isStatic ? 0 : D3DUSAGE_DYNAMIC), 0 /* sharedBuffer->format */, D3DPOOL_DEFAULT, &m_vb, nullptr));
      UINT8* data;
      CHECKD3D(m_vb->Lock(0, size, (void**)&data, 0));
      #endif

      // Fill data block
      for (PendingUpload upload : m_pendingUploads)
      {
         assert(upload.offset >= 0);
         assert(upload.offset + upload.size <= size);
         memcpy(data + upload.offset, upload.data, upload.size);
         delete[] upload.data;
      }
      m_pendingUploads.clear();

      // Upload data block
      #if defined(ENABLE_SDL) // OpenGL
      #ifndef __OPENGLES__
      if (GLAD_GL_VERSION_4_5)
      {
         glCreateBuffers(1, &m_vb);
         glNamedBufferStorage(m_vb, size, data, m_isStatic ? 0 : GL_DYNAMIC_STORAGE_BIT);
      }
      else if (GLAD_GL_VERSION_4_4)
      {
         glGenBuffers(1, &m_vb);
         Bind();
         glBufferStorage(GL_ARRAY_BUFFER, size, data, m_isStatic ? 0 : GL_DYNAMIC_STORAGE_BIT);
      }
      else
      #endif
      {
         glGenBuffers(1, &m_vb);
         Bind();
         glBufferData(GL_ARRAY_BUFFER, size, data, m_isStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
      }
      free(data);
      #else // DirectX 9
      CHECKD3D(m_vb->Unlock());
      #endif
   }
   else
   {
      for (PendingUpload upload : m_pendingUploads)
      {
         #if defined(ENABLE_SDL) // OpenGL
         #ifndef __OPENGLES__
         if (GLAD_GL_VERSION_4_5)
            glNamedBufferSubData(m_vb, upload.offset, upload.size, upload.data);
         else
         #endif
         {
            Bind();
            glBufferSubData(GL_ARRAY_BUFFER, upload.offset, upload.size, upload.data);
         }
         #else // DirectX 9
         // It would be better to perform a single lock but in fact, I don't think there are situations where more than one update is pending
         UINT8* data;
         CHECKD3D(m_vb->Lock(upload.offset, upload.size, (void**)&data, 0));
         memcpy(data, upload.data, upload.size);
         CHECKD3D(m_vb->Unlock());
         #endif
         delete[] upload.data;
      }
      m_pendingUploads.clear();
   }
}

#ifdef ENABLE_SDL
void SharedVertexBuffer::Bind() const
{
   glBindBuffer(GL_ARRAY_BUFFER, m_vb);
}
#endif

SharedVertexBuffer::~SharedVertexBuffer()
{
   if (IsUploaded())
   {
      #if defined(ENABLE_SDL) // OpenGL
      glDeleteBuffers(1, &m_vb);
      #else // DirectX 9
      SAFE_RELEASE(m_vb);
      #endif
   }
   for (PendingUpload upload : m_pendingUploads)
      delete[] upload.data;
}




VertexBuffer::VertexBuffer(RenderDevice* rd, const unsigned int vertexCount, const float* verts, const bool isDynamic, const VertexFormat fvf)
   : m_rd(rd)
   , m_count(vertexCount)
   , m_vertexFormat(fvf)
   , m_sizePerVertex(fvfToSize(fvf))
   , m_isStatic(!isDynamic)
   , m_size(fvfToSize(fvf) * vertexCount)
{
   for (SharedVertexBuffer* block : m_rd->m_pendingSharedVertexBuffers)
   {
      if (block->m_format == fvf && block->m_isStatic == m_isStatic && block->GetCount() + vertexCount <= 65535)
      {
         m_sharedBuffer = block;
         break;
      }
   }
   if (m_sharedBuffer == nullptr)
   {
      m_sharedBuffer = new SharedVertexBuffer(fvf, m_isStatic);
      m_rd->m_pendingSharedVertexBuffers.push_back(m_sharedBuffer);
   }
   m_vertexOffset = m_sharedBuffer->Add(this);
   m_offset = m_vertexOffset * m_sizePerVertex;
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
   if (m_sharedBuffer->Remove(this))
   {
      RemoveFromVectorSingle(m_rd->m_pendingSharedVertexBuffers, m_sharedBuffer);
      delete m_sharedBuffer;
   }
}

bool VertexBuffer::IsSharedBuffer() const { return m_sharedBuffer->IsShared(); }

#ifdef ENABLE_SDL
GLuint VertexBuffer::GetBuffer() const { return m_sharedBuffer->m_vb; }
void VertexBuffer::Bind() const { m_sharedBuffer->Bind(); }
#else
IDirect3DVertexBuffer9* VertexBuffer::GetBuffer() const { return m_sharedBuffer->m_vb; }
#endif

void VertexBuffer::lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void** dataBuffer, const DWORD flags)
{
   m_rd->m_curLockCalls++;
   m_sharedBuffer->Lock(this, m_offset + offsetToLock, sizeToLock == 0 ? m_size : sizeToLock, dataBuffer);
}

void VertexBuffer::unlock()
{
   m_sharedBuffer->Unlock();
}

void VertexBuffer::Upload()
{
   if (!m_sharedBuffer->IsUploaded())
      RemoveFromVectorSingle(m_rd->m_pendingSharedVertexBuffers, m_sharedBuffer);
   m_sharedBuffer->Upload();
}
