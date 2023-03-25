#include "stdafx.h"
#include "IndexBuffer.h"
#include "RenderDevice.h"

extern unsigned m_curLockCalls;

#ifdef ENABLE_SDL
vector<MeshBuffer::SharedVAO*> MeshBuffer::sharedVAOs;
#endif

#include "Shader.h"
#include "VertexBuffer.h"

MeshBuffer::MeshBuffer(VertexBuffer* vb, IndexBuffer* ib, const bool applyVertexBufferOffsetToIndexBuffer)
   : m_vb(vb)
   , m_ib(ib)
   , m_isVBOffsetApplied(applyVertexBufferOffsetToIndexBuffer)
#ifndef ENABLE_SDL
   , m_vertexDeclaration(
      vb->m_vertexFormat == VertexFormat::VF_POS_NORMAL_TEX ? m_vb->m_rd->m_pVertexNormalTexelDeclaration :
      vb->m_vertexFormat == VertexFormat::VF_POS_TEX ? m_vb->m_rd->m_pVertexTexelDeclaration : nullptr)
#endif
{
   if (m_ib != nullptr && applyVertexBufferOffsetToIndexBuffer)
      m_ib->ApplyOffset(m_vb);
}

MeshBuffer::~MeshBuffer()
{
#ifdef ENABLE_SDL
   if (m_sharedVAO != nullptr)
   {
      m_sharedVAO->ref_count--;
      if (m_sharedVAO->ref_count == 0)
      {
         glDeleteVertexArrays(1, &m_sharedVAO->vao);
         RemoveFromVectorSingle(sharedVAOs, m_sharedVAO);
      }
   }
   else if (m_vao != 0)
   {
      glDeleteVertexArrays(1, &m_vao);
   }
#endif
   delete m_vb;
   delete m_ib;
}

void MeshBuffer::bind()
{
   RenderDevice* rd = m_vb->m_rd;
#ifdef ENABLE_SDL
   if (m_vao == 0)
   {
      // If index & vertex buffer are using shared buffers (for static objects), then we can also use a shared VAO
      bool isShared = m_vb->IsSharedBuffer() && (m_ib == nullptr || m_ib->IsSharedBuffer());
      if (isShared)
      {
         GLuint vb = m_vb->GetBuffer(), ib = m_ib == nullptr ? 0 : m_ib->GetBuffer();
         std::vector<SharedVAO*>::iterator existing = std::find_if(sharedVAOs.begin(), sharedVAOs.end(), [vb, ib](SharedVAO* v) { return v->vb == vb && v->ib == ib; });
         if (existing != sharedVAOs.end())
         {
            m_sharedVAO = *existing;
            m_sharedVAO->ref_count++;
            m_vao = m_sharedVAO->vao;
         }
      }
      if (m_vao == 0)
      {
         glGenVertexArrays(1, &m_vao);
         glBindVertexArray(m_vao);
         rd->m_curVAO = m_vao;
         m_vb->Upload();
         glBindBuffer(GL_ARRAY_BUFFER, m_vb->GetBuffer());
         if (m_ib != nullptr)
         {
            m_ib->Upload();
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib->GetBuffer());
         }
         // this needs the attribute layout to be enforced in the shaders using layout(location=...)
         switch (m_vb->m_vertexFormat)
         {
         case VertexFormat::VF_POS_NORMAL_TEX:
            glEnableVertexAttribArray(0); // Position
            glEnableVertexAttribArray(1); // Normal
            glEnableVertexAttribArray(2); // Texture Coordinate
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, (void*)0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, (void*)12);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, (void*)24);
            break;
         case VertexFormat::VF_POS_TEX:
            glEnableVertexAttribArray(0); // Position
            glEnableVertexAttribArray(1); // Texture Coordinate
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 20, (void*)0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 20, (void*)12);
            break;
         default: assert(false); // Unsupported FVF
         }
         if (isShared)
         {
            m_sharedVAO = new SharedVAO { m_vb->GetBuffer(), m_ib == nullptr ? 0 : m_ib->GetBuffer(), m_vao, 1 };
            sharedVAOs.push_back(m_sharedVAO);
         }
      }
   }
   if (rd->m_curVAO != m_vao)
   {
      glBindVertexArray(m_vao);
      rd->m_curVAO = m_vao;
   }
   m_vb->Upload();
   if (m_ib != nullptr)
      m_ib->Upload();

#else
   m_vb->Upload();
   IDirect3DVertexBuffer9* vb = m_vb->GetBuffer();
   if (rd->m_curVertexBuffer != vb)
   {
      CHECKD3D(rd->GetCoreDevice()->SetStreamSource(0, vb, 0, m_vb->m_sizePerVertex));
      rd->m_curVertexBuffer = vb;
   }
   if (rd->m_currentVertexDeclaration != m_vertexDeclaration)
   {
      CHECKD3D(rd->GetCoreDevice()->SetVertexDeclaration(m_vertexDeclaration));
      rd->m_currentVertexDeclaration = m_vertexDeclaration;
      rd->m_curStateChanges++;
   }
   if (m_ib != nullptr)
   {
      m_ib->Upload();
      IDirect3DIndexBuffer9 * ib = m_ib->GetBuffer();
      if (rd->m_curIndexBuffer != ib)
      {
         CHECKD3D(rd->GetCoreDevice()->SetIndices(ib));
         rd->m_curIndexBuffer = ib;
      }
   }
#endif
}




vector<IndexBuffer*> IndexBuffer::pendingSharedBuffers;

IndexBuffer::IndexBuffer(RenderDevice* rd, const unsigned int numIndices, const bool isDynamic, const IndexBuffer::Format format)
   : m_rd(rd)
   , m_indexCount(numIndices)
   , m_indexFormat(format)
   , m_sizePerIndex(format == FMT_INDEX16 ? 2 : 4)
   , m_isStatic(!isDynamic)
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
   : IndexBuffer(rd, numIndices, false, IndexBuffer::FMT_INDEX32)
{
   void* buf;
   lock(0, 0, &buf, WRITEONLY);
   memcpy(buf, indices, numIndices * sizeof(indices[0]));
   unlock();
}

IndexBuffer::IndexBuffer(RenderDevice* rd, const unsigned int numIndices, const WORD* indices)
   : IndexBuffer(rd, numIndices, false, IndexBuffer::FMT_INDEX16)
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
   if (!m_ib)
      RemoveFromVectorSingle(pendingSharedBuffers, this);
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

void IndexBuffer::ApplyOffset(VertexBuffer* vb)
{
   if (vb->m_offset == 0)
      return;
   for (size_t j = 0; j < m_pendingUploads.size(); j++)
   {
      PendingUpload& upload = m_pendingUploads[j];
      unsigned int offset = vb->GetVertexOffset();
      unsigned int count = upload.size / m_sizePerIndex;
      if (m_indexFormat == FMT_INDEX16)
      {
         U16* indices = (U16*) upload.data;
         for (unsigned int i = 0; i < count; i++)
            indices[i] += offset;
      }
      else // FMT_INDEX32
      {
         U32* indices = (U32*) upload.data;
         for (unsigned int i = 0; i < count; i++)
            indices[i] += offset;
      }
   }
}

void IndexBuffer::CreatePendingSharedBuffer()
{
   UINT size = 0;
   for (size_t i = 0; i < pendingSharedBuffers.size(); i++)
   {
      size += pendingSharedBuffers[i]->m_size;
   }
   #if defined(ENABLE_SDL) // OpenGL
   GLuint ib = 0;
   glGenBuffers(1, &ib);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
   int* refCount = new int();
   (*refCount) = (int) pendingSharedBuffers.size();
   #else // DirectX 9
   IDirect3DIndexBuffer9* ib = nullptr;
   CHECKD3D(m_rd->GetCoreDevice()->CreateIndexBuffer(
      size, D3DUSAGE_WRITEONLY, pendingSharedBuffers[0]->m_indexFormat == FMT_INDEX16 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &ib, nullptr));
   UINT8* data;
   CHECKD3D(ib->Lock(0, size, (void**)&data, 0));
   #endif
   UINT offset = 0;
   for (size_t i = 0; i < pendingSharedBuffers.size(); i++)
   {
      IndexBuffer* buffer = pendingSharedBuffers[i];
      assert(buffer->m_indexFormat == pendingSharedBuffers[0]->m_indexFormat);
      buffer->m_ib = ib;
      buffer->m_offset = offset;
      for (size_t j = 0; j < buffer->m_pendingUploads.size(); j++)
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

void IndexBuffer::Upload()
{
   if (!m_ib)
      CreatePendingSharedBuffer();

   if (m_pendingUploads.size() > 0)
   {
      for (size_t j = 0; j < m_pendingUploads.size(); j++)
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
}
