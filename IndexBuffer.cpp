#include "stdafx.h"
#include "IndexBuffer.h"
#include "RenderDevice.h"

vector<IndexBuffer::SharedBuffer*> IndexBuffer::pendingSharedBuffers;

IndexBuffer::IndexBuffer(RenderDevice* rd, const unsigned int indexCount, const bool isDynamic, const IndexBuffer::Format format)
   : m_rd(rd)
   , m_indexCount(indexCount)
   , m_indexFormat(format)
   , m_sizePerIndex(format == FMT_INDEX16 ? 2 : 4)
   , m_isStatic(!isDynamic)
   , m_size(indexCount * (format == FMT_INDEX16 ? 2 : 4))
{
   for (SharedBuffer* block : pendingSharedBuffers)
   {
      if (block->format == m_indexFormat && block->isStatic == m_isStatic)
      {
         m_sharedBuffer = block;
         break;
      }
   }
   if (m_sharedBuffer == nullptr)
   {
      m_sharedBuffer = new SharedBuffer();
      m_sharedBuffer->format = m_indexFormat;
      m_sharedBuffer->isStatic = m_isStatic;
      pendingSharedBuffers.push_back(m_sharedBuffer);
   }
   m_indexOffset = m_sharedBuffer->count;
   m_offset = m_indexOffset * m_sizePerIndex;
   m_sharedBuffer->buffers.push_back(this);
   m_sharedBuffer->count += indexCount;
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
   for (PendingUpload upload : m_pendingUploads)
      delete[] upload.data;
   RemoveFromVectorSingle(m_sharedBuffer->buffers, this);
   if (m_sharedBuffer->buffers.size() == 0)
   {
      RemoveFromVectorSingle(pendingSharedBuffers, m_sharedBuffer);
      if (IsCreated())
      #if defined(ENABLE_SDL) // OpenGL
         glDeleteBuffers(1, &m_ib);
      #else // DirectX 9
         SAFE_RELEASE(m_ib);
      #endif
      delete m_sharedBuffer;
   }
}

void IndexBuffer::lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void **dataBuffer, const DWORD flags)
{
   assert(!m_isStatic || !IsCreated()); // Static buffers can't be locked after first upload
   assert(m_lock.data == nullptr); // Lock is not reentrant
   m_rd->m_curLockCalls++;
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
   if (vb->GetOffset() == 0)
      return;
   for (PendingUpload upload : m_pendingUploads)
   {
      const unsigned int offset = vb->GetVertexOffset();
      const unsigned int count = upload.size / m_sizePerIndex;
      if (m_indexFormat == FMT_INDEX16)
      {
         U16* const __restrict indices = (U16*) upload.data;
         for (unsigned int i = 0; i < count; i++)
            indices[i] += offset;
      }
      else // FMT_INDEX32
      {
         assert(m_indexFormat == FMT_INDEX32);
         U32* const __restrict indices = (U32*) upload.data;
         for (unsigned int i = 0; i < count; i++)
            indices[i] += offset;
      }
   }
}

void IndexBuffer::CreateSharedBuffer(SharedBuffer* sharedBuffer)
{
   RemoveFromVectorSingle(pendingSharedBuffers, sharedBuffer);
   unsigned int size = sharedBuffer->count * (sharedBuffer->format == FMT_INDEX16 ? 2 : 4);
   
   #if defined(ENABLE_SDL) // OpenGL
   UINT8* data = (UINT8*)malloc(size);

   #else // DirectX 9
   IDirect3DIndexBuffer9* ib = nullptr;
   CHECKD3D(sharedBuffer->buffers[0]->m_rd->GetCoreDevice()->CreateIndexBuffer(size, D3DUSAGE_WRITEONLY | (sharedBuffer->isStatic ? 0 : D3DUSAGE_DYNAMIC), sharedBuffer->format == FMT_INDEX16 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &ib, nullptr));
   UINT8* data;
   CHECKD3D(ib->Lock(0, size, (void**)&data, 0));
   #endif

   for (IndexBuffer* buffer : sharedBuffer->buffers)
   {
      for (PendingUpload upload : buffer->m_pendingUploads)
      {
         assert(buffer->m_offset + upload.offset >= 0);
         assert(buffer->m_offset + upload.offset + upload.size <= sharedBuffer->count * (sharedBuffer->format == FMT_INDEX16 ? 2 : 4));
         memcpy(data + buffer->m_offset + upload.offset, upload.data, upload.size);
         delete[] upload.data;
      }
      buffer->m_pendingUploads.clear();
   }

   #if defined(ENABLE_SDL) // OpenGL && OpenGL ES
   GLuint ib = 0;
   #ifndef __OPENGLES__
   if (GLAD_GL_VERSION_4_5)
   {
      glCreateBuffers(1, &ib);
      glNamedBufferStorage(ib, size, data, sharedBuffer->isStatic ? 0 : GL_DYNAMIC_STORAGE_BIT);
   }
   else if (GLAD_GL_VERSION_4_4)
   {
      glGenBuffers(1, &ib);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
      glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, size, data, sharedBuffer->isStatic ? 0 : GL_DYNAMIC_STORAGE_BIT);
   }
   else
   #endif
   {
      glGenBuffers(1, &ib);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, sharedBuffer->isStatic ? 0 : GL_DYNAMIC_STORAGE_BIT);
   }
   free(data);

   #else // DirectX 9
   CHECKD3D(ib->Unlock());
   #endif

   for (IndexBuffer* buffer : sharedBuffer->buffers)
      buffer->m_ib = ib;
}

void IndexBuffer::Upload()
{
   if (!IsCreated())
      CreateSharedBuffer(m_sharedBuffer);
   else if (m_pendingUploads.size() > 0)
   {
      for (PendingUpload upload : m_pendingUploads)
      {
         #if defined(ENABLE_SDL) // OpenGL
         #ifndef __OPENGLES__
         if (GLAD_GL_VERSION_4_5)
            glNamedBufferSubData(m_ib, m_offset + upload.offset, upload.size, upload.data);
         else
         #endif
         {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, m_offset + upload.offset, upload.size, upload.data);
         }

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
