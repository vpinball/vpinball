#include "stdafx.h"
#include "VertexBuffer.h"
#include "RenderDevice.h"
#include "Shader.h"

extern unsigned m_curLockCalls;

static unsigned int fvfToSize(const DWORD fvf)
{
   switch (fvf)
   {
   case MY_D3DFVF_NOTEX2_VERTEX:
   case MY_D3DTRANSFORMED_NOTEX2_VERTEX:
      return sizeof(Vertex3D_NoTex2);
   case MY_D3DFVF_TEX:
      return sizeof(Vertex3D_TexelOnly);
   default:
      assert(false); // Unsupported FVF
      return 0;
   }
}

vector<VertexBuffer*> VertexBuffer::pendingSharedBuffers;

VertexBuffer::VertexBuffer(RenderDevice* rd, const unsigned int vertexCount, const DWORD usage, const DWORD fvf)
   : m_rd(rd)
   , m_vertexCount(vertexCount)
   , m_fvf(fvf)
   , m_sizePerVertex(fvfToSize(fvf))
   , m_isStatic(usage == 0 || usage == USAGE_STATIC)
   , m_size(fvfToSize(fvf) * vertexCount)
{
   #ifndef __OPENGLES__
   // Disabled since OpenGL ES does not support glDrawElementsBaseVertex and we need it unless we remap the indices when creating the index buffer (and we should)
   if (m_isStatic)
   {
      if (pendingSharedBuffers.size() > 0 && pendingSharedBuffers[0]->m_fvf != m_fvf)
         CreatePendingSharedBuffer();
      pendingSharedBuffers.push_back(this);
   }
   else
   #endif
   {
      #if defined(ENABLE_SDL) // OpenGL
      glGenBuffers(1, &m_vb);
      glBindBuffer(GL_ARRAY_BUFFER, m_vb);
      glBufferData(GL_ARRAY_BUFFER, m_size, nullptr, m_isStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);

      #else // DirectX 9
      // NB: We always specify WRITEONLY since MSDN states,
      // "Buffers created with D3DPOOL_DEFAULT that do not specify D3DUSAGE_WRITEONLY may suffer a severe performance penalty."
      // This means we cannot read from vertex buffers, but I don't think we need to.
      CHECKD3D(rd->GetCoreDevice()->CreateVertexBuffer(m_size, D3DUSAGE_WRITEONLY | (m_isStatic ? 0 : D3DUSAGE_DYNAMIC), 0, D3DPOOL_DEFAULT, &m_vb, nullptr));

      #endif
   }
}

VertexBuffer::VertexBuffer(RenderDevice* rd, const unsigned int vertexCount, const DWORD usage, const DWORD fvf, const float* verts)
   : VertexBuffer(rd, vertexCount, usage, fvf)
{
   BYTE* const data = new BYTE[vertexCount * m_sizePerVertex];
   memcpy(data, verts, vertexCount * m_sizePerVertex);
   PendingUpload pending { 0, vertexCount * m_sizePerVertex, data };
   m_pendingUploads.push_back(pending);
}

VertexBuffer::~VertexBuffer()
{
   for (size_t j = 0; j < m_pendingUploads.size(); j++)
   {
      delete[] m_pendingUploads[j].data;
   }
#if defined(ENABLE_SDL) // OpenGL
   if (m_sharedBufferRefCount != nullptr)
   {
      (*m_sharedBufferRefCount)--;
      if ((*m_sharedBufferRefCount) == 0)
      {
         delete m_sharedBufferRefCount;
         if (m_vb != 0)
            glDeleteBuffers(1, &m_vb);
      }
   }
   else if (m_vb != 0)
      glDeleteBuffers(1, &m_vb);
#else // DirectX 9
   SAFE_RELEASE(m_vb);
#endif
}

void VertexBuffer::lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void **dataBuffer, const DWORD flags)
{
   assert(m_lock.data == nullptr); // Lock is not reentrant
   m_curLockCalls++;
   m_lock.offset = offsetToLock;
   m_lock.size = sizeToLock == 0 ? m_size : sizeToLock;
   m_lock.data = new BYTE[m_lock.size];
   *dataBuffer = m_lock.data;
}

void VertexBuffer::unlock()
{
   PendingUpload upload = m_lock;
   m_pendingUploads.push_back(upload);
   m_lock.data = nullptr;
}

void VertexBuffer::CreatePendingSharedBuffer()
{
   UINT size = 0;
   for (size_t i = 0; i < pendingSharedBuffers.size(); i++)
   {
      size += pendingSharedBuffers[i]->m_size;
   }
   #if defined(ENABLE_SDL) // OpenGL
   GLuint vb = 0;
   glGenBuffers(1, &vb);
   glBindBuffer(GL_ARRAY_BUFFER, vb);
   glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
   int* refCount = new int();
   (*refCount) = (int) pendingSharedBuffers.size();
   #else // DirectX 9
   IDirect3DVertexBuffer9* vb = nullptr;
   CHECKD3D(m_rd->GetCoreDevice()->CreateVertexBuffer(size, D3DUSAGE_WRITEONLY, 0 /* pendingSharedBuffers[0]->m_fvf */, D3DPOOL_DEFAULT, &vb, nullptr));
   UINT8* data;
   CHECKD3D(vb->Lock(0, size, (void**)&data, 0));
   #endif
   UINT offset = 0;
   for (size_t i = 0; i < pendingSharedBuffers.size(); i++)
   {
      VertexBuffer* buffer = pendingSharedBuffers[i];
      assert(buffer->m_fvf == pendingSharedBuffers[0]->m_fvf);
      buffer->m_vb = vb;
      buffer->m_offset = offset;
      for (size_t j = 0; j < buffer->m_pendingUploads.size(); j++)
      {
         PendingUpload& upload = buffer->m_pendingUploads[j];
         #ifdef ENABLE_SDL // OpenGL
         glBufferSubData(GL_ARRAY_BUFFER, offset + upload.offset, upload.size, upload.data);
         #else // DirectX 9
         memcpy(data + offset + upload.offset, upload.data, upload.size);
         #endif
         delete[] upload.data;
      }
      buffer->m_pendingUploads.clear();
      offset += buffer->m_size;
      #if defined(ENABLE_SDL) // OpenGL
      buffer->m_sharedBufferRefCount = refCount;
      #else // DirectX 9
      vb->AddRef();
      #endif
   }
   pendingSharedBuffers.clear();
   #ifndef ENABLE_SDL // DirectX 9
   CHECKD3D(vb->Unlock());
   vb->Release();
   #endif
}

void VertexBuffer::Upload()
{
   if (!m_vb)
      CreatePendingSharedBuffer();

   if (m_pendingUploads.size() > 0)
   {
      for (size_t j = 0; j < m_pendingUploads.size(); j++)
      {
         PendingUpload& upload = m_pendingUploads[j];

         #if defined(ENABLE_SDL) // OpenGL
         glBindBuffer(GL_ARRAY_BUFFER, m_vb);
         glBufferSubData(GL_ARRAY_BUFFER, m_offset + upload.offset, upload.size, upload.data);

         #else // DirectX 9
         // It would be better to perform a single lock but in fact, I don't think there are situations where more than one update is pending
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
