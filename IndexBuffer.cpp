#include "stdafx.h"
#include "IndexBuffer.h"
#include "RenderDevice.h"

extern unsigned m_curLockCalls;

#ifdef ENABLE_SDL
vector<MeshBuffer::SharedVAO> MeshBuffer::sharedVAOs;
#endif

#include "Shader.h"
#include "VertexBuffer.h"

MeshBuffer::MeshBuffer(VertexBuffer* vb, IndexBuffer* ib) : m_vb(vb), m_ib(ib)
#ifndef ENABLE_SDL
   , m_vertexDeclaration(
      vb->m_fvf == MY_D3DFVF_NOTEX2_VERTEX ? m_vb->m_rd->m_pVertexNormalTexelDeclaration :
      vb->m_fvf == MY_D3DTRANSFORMED_NOTEX2_VERTEX ? m_vb->m_rd->m_pVertexTrafoTexelDeclaration :
      vb->m_fvf == MY_D3DFVF_TEX ? m_vb->m_rd->m_pVertexTexelDeclaration : nullptr)
#endif
{
}

MeshBuffer::~MeshBuffer()
{
#ifdef ENABLE_SDL
   /* if (m_isSharedVAO)
   {
      // Shared VAO are ref counted
      GLuint vb = m_vb->getBuffer();
      GLuint ib = m_ib == nullptr ? 0 : m_ib->getBuffer();
      std::vector<SharedVAO>::iterator existing = std::find_if(sharedVAOs.begin(), sharedVAOs.end(), [vb, ib](SharedVAO v) { return v.vb == vb && v.ib == ib; });
      if (existing != sharedVAOs.end())
      {
         existing->ref_count--;
         if (existing->ref_count == 0)
         {
            glDeleteVertexArrays(1, &existing->vao);
            sharedVAOs.erase(existing);
         }
      }
   }
   else*/ if (m_vao != 0)
   {
      glDeleteVertexArrays(1, &m_vao);
      m_vao = 0;
   }
#endif
   delete m_vb;
   delete m_ib;
}

#ifdef ENABLE_SDL
void MeshBuffer::ClearSharedBuffers()
{
   // FIXME add some debug logging since a well behaving application should not have any shared VAO still alive at this point
   sharedVAOs.clear();
}
#endif

void MeshBuffer::bind()
{
#ifdef ENABLE_SDL
   static GLuint curVAO = 0;
   // Create or reuse VAO
   if (m_vao == 0)
   {
      glGenVertexArrays(1, &m_vao);
      glBindVertexArray(m_vao);
      m_vb->bind();
      // this needs that the attribute layout is enforced in the shaders using layout(location=...)
      Shader::GetCurrentShader()->setAttributeFormat(m_vb->m_fvf);
      if (m_ib)
         m_ib->bind();
      // If index & vertex buffer are using shared buffers (for static objects), then this buffer should use a shared VAO
      m_isSharedVAO = false;
      
      /* m_vb->useSharedBuffer() && (m_ib == nullptr || m_ib->useSharedBuffer());
      if (m_isSharedVAO)
      {
         GLuint vb = m_vb->getBuffer();
         GLuint ib = m_ib == nullptr ? 0 : m_ib->getBuffer();
         std::vector<SharedVAO>::iterator existing = std::find_if(sharedVAOs.begin(), sharedVAOs.end(), [vb, ib](SharedVAO v) { return v.vb == vb && v.ib == ib; });
         if (existing == sharedVAOs.end())
         {
            SharedVAO vao = { vb, ib, m_vao, 1 };
            sharedVAOs.push_back(vao);
         }
         else
         {
            existing->ref_count++;
            glDeleteVertexArrays(1, &m_vao);
            m_vao = existing->vao;
            glBindVertexArray(m_vao);
         }
      }*/
      curVAO = m_vao;
   }
   else 
   {
      if (curVAO != m_vao)
      {
         glBindVertexArray(m_vao);
         curVAO = m_vao;
      }
      // Upload any pending data to GPU buffer
      // FIXME this is broken, so force binding
      if (true) // || !m_vb->isUploaded() || (m_ib && !m_vb->isUploaded()))
      {
         m_vb->bind();
         Shader::GetCurrentShader()->setAttributeFormat(m_vb->m_fvf);
         if (m_ib)
            m_ib->bind();
      }
   }
#else
   m_vb->bind();
   if (m_ib)
      m_ib->bind();
   m_vb->m_rd->SetVertexDeclaration(m_vertexDeclaration);
#endif
}

vector<IndexBuffer*> IndexBuffer::pendingSharedBuffers;

IndexBuffer::IndexBuffer(RenderDevice* rd, const unsigned int numIndices, const DWORD usage, const IndexBuffer::Format format)
   : m_rd(rd)
   , m_indexCount(numIndices)
   , m_indexFormat(format)
   , m_sizePerIndex(format == FMT_INDEX16 ? 2 : 4)
   , m_isStatic(usage == 0 || usage == USAGE_STATIC)
   , m_size(numIndices * (format == FMT_INDEX16 ? 2 : 4))
{
   if (m_isStatic)
   {
      if (pendingSharedBuffers.size() > 0 && pendingSharedBuffers[0]->m_indexFormat != m_indexFormat)
         CreatePendingSharedBuffer();
      pendingSharedBuffers.push_back(this);
   }
   else
   {
      #if defined(ENABLE_SDL) // OpenGL
      glGenBuffers(1, &m_ib);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_size, nullptr, m_isStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
      m_rd->m_curIndexBuffer = this;

      #else // DirectX 9
      // NB: We always specify WRITEONLY since MSDN states,
      // "Buffers created with D3DPOOL_DEFAULT that do not specify D3DUSAGE_WRITEONLY may suffer a severe performance penalty."
      // This means we cannot read from index buffers, but I don't think we need to.
      CHECKD3D(rd->GetCoreDevice()->CreateIndexBuffer(m_size, D3DUSAGE_WRITEONLY | (m_isStatic ? 0 : D3DUSAGE_DYNAMIC),
         m_indexFormat == FMT_INDEX16 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_ib, nullptr));

      #endif
   }
}

IndexBuffer::IndexBuffer(RenderDevice* rd, const unsigned int numIndices, const unsigned int* indices)
   : IndexBuffer(rd, numIndices, 0, IndexBuffer::FMT_INDEX32)
{
   void* buf;
   lock(0, 0, &buf, WRITEONLY);
   memcpy(buf, indices, numIndices * sizeof(indices[0]));
   unlock();
}

IndexBuffer::IndexBuffer(RenderDevice* rd, const unsigned int numIndices, const WORD* indices)
   : IndexBuffer(rd, numIndices, 0, IndexBuffer::FMT_INDEX16)
{
   void* buf;
   lock(0, 0, &buf, WRITEONLY);
   memcpy(buf, indices, numIndices * sizeof(indices[0]));
   unlock();
}

IndexBuffer::IndexBuffer(RenderDevice* rd, const vector<WORD>& indices)
   : IndexBuffer(rd, (unsigned int)indices.size(), indices.data())
{
}

IndexBuffer::IndexBuffer(RenderDevice* rd, const vector<unsigned int>& indices)
   : IndexBuffer(rd, (unsigned int)indices.size(), indices.data())
{
}

IndexBuffer::~IndexBuffer()
{
   for (int j = 0; j < m_pendingUploads.size(); j++)
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
         if (m_ib != 0)
            glDeleteBuffers(1, &m_ib);
      }
   }
   else if (m_ib != 0)
      glDeleteBuffers(1, &m_ib);
#else // DirectX 9
   SAFE_RELEASE(m_ib);
#endif
}

void IndexBuffer::lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void **dataBuffer, const DWORD flags)
{
   assert(m_lock.data == nullptr); // Lock is not reentrant
   m_curLockCalls++;
   m_lock.offset = offsetToLock;
   m_lock.size = sizeToLock == 0 ? m_size : sizeToLock;
   m_lock.data = new BYTE[m_lock.size];
   *dataBuffer = m_lock.data;
}

void IndexBuffer::unlock()
{
   PendingUpload upload = m_lock;
   m_pendingUploads.push_back(upload);
   m_lock.data = nullptr;
}

void IndexBuffer::CreatePendingSharedBuffer()
{
   UINT size = 0;
   for (int i = 0; i < pendingSharedBuffers.size(); i++)
   {
      size += pendingSharedBuffers[i]->m_size;
   }
   #if defined(ENABLE_SDL) // OpenGL
   GLuint ib = 0;
   glGenBuffers(1, &ib);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
   int* refCount = new int();
   (*refCount) = pendingSharedBuffers.size();
   #else // DirectX 9
   IDirect3DIndexBuffer9* ib = nullptr;
   CHECKD3D(m_rd->GetCoreDevice()->CreateIndexBuffer(
      size, D3DUSAGE_WRITEONLY, pendingSharedBuffers[0]->m_indexFormat == FMT_INDEX16 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &ib, nullptr));
   UINT8* data;
   CHECKD3D(ib->Lock(0, size, (void**)&data, 0));
   #endif
   UINT offset = 0;
   for (int i = 0; i < pendingSharedBuffers.size(); i++)
   {
      IndexBuffer* buffer = pendingSharedBuffers[i];
      assert(buffer->m_indexFormat == pendingSharedBuffers[0]->m_indexFormat);
      buffer->m_ib = ib;
      buffer->m_offset = offset;
      for (int j = 0; j < buffer->m_pendingUploads.size(); j++)
      {
         PendingUpload& upload = buffer->m_pendingUploads[j];
         #ifdef ENABLE_SDL // OpenGL
         glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset + upload.offset, upload.size, upload.data);
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
      ib->AddRef();
      #endif
   }
   pendingSharedBuffers.clear();
   #ifndef ENABLE_SDL // DirectX 9
   CHECKD3D(ib->Unlock());
   ib->Release();
   #endif
}

void IndexBuffer::bind()
{
   if (!m_ib)
      CreatePendingSharedBuffer();

   if (m_pendingUploads.size() > 0)
   {
      for (int j = 0; j < m_pendingUploads.size(); j++)
      {
         PendingUpload& upload = m_pendingUploads[j];

         #if defined(ENABLE_SDL) // OpenGL
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);
         glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, m_offset + upload.offset, upload.size, upload.data);

         #else // DirectX 9
         // It would be better to perform a single lock but in fact, I don't think there are situations where more than one update is pending
         UINT8* data;
         CHECKD3D(m_ib->Lock(m_offset + upload.offset, upload.size, (void**)&data, 0));
         memcpy(data, upload.data, upload.size);
         CHECKD3D(m_ib->Unlock());

         #endif
         delete[] upload.data;
      }
      m_pendingUploads.clear();
   }

   if (m_rd->m_curIndexBuffer != this)
   {
      #if defined(ENABLE_SDL) // OpenGL
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);

      #else // DirectX 9
      CHECKD3D(m_rd->GetCoreDevice()->SetIndices(m_ib));

      #endif
      m_rd->m_curIndexBuffer = this;
   }
}
