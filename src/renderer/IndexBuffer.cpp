// license:GPLv3+

#include "core/stdafx.h"
#include "IndexBuffer.h"
#include "RenderDevice.h"

class SharedIndexBuffer : public SharedBuffer<IndexBuffer::Format, IndexBuffer>
{
public:
   SharedIndexBuffer(IndexBuffer::Format fmt, bool stat) : SharedBuffer(fmt, fmt == IndexBuffer::Format::FMT_INDEX16 ? 2 : 4, stat) {}
   ~SharedIndexBuffer();
   void Upload() override;

   #if defined(ENABLE_BGFX)
   bgfx::IndexBufferHandle m_ib = BGFX_INVALID_HANDLE;
   bgfx::DynamicIndexBufferHandle m_dib = BGFX_INVALID_HANDLE;
   bool IsCreated() const override { return m_isStatic ? bgfx::isValid(m_ib) : bgfx::isValid(m_dib); }
   
   #elif defined(ENABLE_OPENGL)
   GLuint m_ib = 0;
   void Bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib); };
   bool IsCreated() const override { return m_ib; }

   #elif defined(ENABLE_DX9)
   IDirect3DIndexBuffer9* m_ib = nullptr;
   bool IsCreated() const override { return m_ib; }

   #endif
};

SharedIndexBuffer::~SharedIndexBuffer()
{
   if (IsCreated())
   {
      #if defined(ENABLE_BGFX)
      if (bgfx::isValid(m_ib))
         bgfx::destroy(m_ib);
      if (bgfx::isValid(m_dib))
         bgfx::destroy(m_dib);
      #elif defined(ENABLE_OPENGL)
      glDeleteBuffers(1, &m_ib);
      #elif defined(ENABLE_DX9)
      SAFE_RELEASE(m_ib);
      #endif
   }
   #if defined(ENABLE_BGFX)
   if (IsCreated())
      for (PendingUpload upload : m_pendingUploads)
         delete upload.mem;
   else
   #endif
   for (PendingUpload upload : m_pendingUploads)
      delete[] upload.data;
}

void SharedIndexBuffer::Upload()
{
   if (!IsCreated())
   {
      unsigned int size = m_count * m_bytePerElement;

      // Create data block
      #if defined(ENABLE_BGFX)
      const bgfx::Memory* mem = bgfx::alloc(size);
      UINT8* data = mem->data;

      #elif defined(ENABLE_OPENGL)
      UINT8* data = (UINT8*)malloc(size);

      #elif defined(ENABLE_DX9)
      CHECKD3D(m_buffers[0]->m_rd->GetCoreDevice()->CreateIndexBuffer(size, D3DUSAGE_WRITEONLY | (m_isStatic ? 0 : D3DUSAGE_DYNAMIC), m_format == IndexBuffer::FMT_INDEX16 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_ib, nullptr));
      UINT8* data;
      CHECKD3D(m_ib->Lock(0, size, (void**)&data, 0));

      #endif

      // Fill data block
      for (PendingUpload upload : m_pendingUploads)
      {
         //assert(upload.offset >= 0);
         assert(upload.offset + upload.size <= size);
         memcpy(data + upload.offset, upload.data, upload.size);
         delete[] upload.data;
      }
      m_pendingUploads.clear();

      // Upload data block
      #if defined(ENABLE_BGFX)
      if (m_isStatic)
         m_ib = bgfx::createIndexBuffer(mem, m_format == IndexBuffer::Format::FMT_INDEX16 ? BGFX_BUFFER_NONE : BGFX_BUFFER_INDEX32);
      else
         m_dib = bgfx::createDynamicIndexBuffer(mem, m_format == IndexBuffer::Format::FMT_INDEX16 ? BGFX_BUFFER_NONE : BGFX_BUFFER_INDEX32);

      #elif defined(ENABLE_OPENGL)
         #ifndef __OPENGLES__
         if (GLAD_GL_VERSION_4_5)
         {
            glCreateBuffers(1, &m_ib);
            glNamedBufferStorage(m_ib, size, data, m_isStatic ? 0 : GL_DYNAMIC_STORAGE_BIT);
         }
         else if (GLAD_GL_VERSION_4_4)
         {
            glGenBuffers(1, &m_ib);
            Bind();
            glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, size, data, m_isStatic ? 0 : GL_DYNAMIC_STORAGE_BIT);
         }
         else
         #endif
      {
         glGenBuffers(1, &m_ib);
         Bind();
         glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, m_isStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
      }
      free(data);

      #elif defined(ENABLE_DX9)
      CHECKD3D(m_ib->Unlock());
      #endif
   }
   else
   {
      for (PendingUpload upload : m_pendingUploads)
      {
         assert(!m_isStatic);
         #if defined(ENABLE_BGFX)
         bgfx::update(m_dib, upload.offset / m_bytePerElement, upload.mem);

         #elif defined(ENABLE_OPENGL)
            #ifndef __OPENGLES__
            if (GLAD_GL_VERSION_4_5)
               glNamedBufferSubData(m_ib, upload.offset, upload.size, upload.data);
            else
            #endif
         {
            Bind();
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, upload.offset, upload.size, upload.data);
         }
         delete[] upload.data;

         #elif defined(ENABLE_DX9)
         // It would be better to perform a single lock but in fact, I don't think there are situations where more than one update is pending
         UINT8* data;
         CHECKD3D(m_ib->Lock(upload.offset, upload.size, (void**)&data, 0));
         memcpy(data, upload.data, upload.size);
         CHECKD3D(m_ib->Unlock());
         delete[] upload.data;

         #endif
      }
      m_pendingUploads.clear();
   }
}


IndexBuffer::IndexBuffer(RenderDevice* rd, const unsigned int numIndices, const bool isDynamic, const IndexBuffer::Format format)
   : m_rd(rd)
   , m_count(numIndices)
   , m_indexFormat(format)
   , m_sizePerIndex(format == FMT_INDEX16 ? 2 : 4)
   , m_isStatic(!isDynamic)
   , m_size(numIndices * (format == FMT_INDEX16 ? 2 : 4))
{
   for (SharedIndexBuffer* block : m_rd->m_pendingSharedIndexBuffers)
   {
      if (block->m_format == m_indexFormat && block->m_isStatic == m_isStatic)
      {
         m_sharedBuffer = block;
         break;
      }
   }
   if (m_sharedBuffer == nullptr)
   {
      m_sharedBuffer = new SharedIndexBuffer(m_indexFormat, m_isStatic);
      m_rd->m_pendingSharedIndexBuffers.push_back(m_sharedBuffer);
   }
   m_indexOffset = m_sharedBuffer->Add(this);
   m_offset = m_indexOffset * m_sizePerIndex;
}

IndexBuffer::IndexBuffer(RenderDevice* rd, const unsigned int numIndices, const unsigned int* indices)
   : IndexBuffer(rd, numIndices, false, IndexBuffer::FMT_INDEX32)
{
   void* buf;
   Lock(buf);
   memcpy(buf, indices, numIndices * sizeof(indices[0]));
   Unlock();
}

IndexBuffer::IndexBuffer(RenderDevice* rd, const unsigned int numIndices, const WORD* indices)
   : IndexBuffer(rd, numIndices, false, IndexBuffer::FMT_INDEX16)
{
   void* buf;
   Lock(buf);
   memcpy(buf, indices, numIndices * sizeof(indices[0]));
   Unlock();
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
   if (m_sharedBuffer->Remove(this))
   {
      RemoveFromVectorSingle(m_rd->m_pendingSharedIndexBuffers, m_sharedBuffer);
      delete m_sharedBuffer;
   }
}

bool IndexBuffer::IsSharedBuffer() const { return m_sharedBuffer->IsShared(); }

#if defined(ENABLE_BGFX)
bgfx::IndexBufferHandle IndexBuffer::GetStaticBuffer() const { return m_sharedBuffer->m_ib; }
bgfx::DynamicIndexBufferHandle IndexBuffer::GetDynamicBuffer() const { return m_sharedBuffer->m_dib; }

#elif defined(ENABLE_OPENGL)
GLuint IndexBuffer::GetBuffer() const { return m_sharedBuffer->m_ib; }
void IndexBuffer::Bind() const { m_sharedBuffer->Bind(); }

#elif defined(ENABLE_DX9)
IDirect3DIndexBuffer9* IndexBuffer::GetBuffer() const { return m_sharedBuffer->m_ib; }
void IndexBuffer::Bind() const
{
   if (m_rd->m_curIndexBuffer != m_sharedBuffer->m_ib)
   {
      CHECKD3D(m_rd->GetCoreDevice()->SetIndices(m_sharedBuffer->m_ib));
      m_rd->m_curIndexBuffer = m_sharedBuffer->m_ib;
   }
}

#endif

void IndexBuffer::LockUntyped(void*& data, const unsigned int offset, const unsigned int size)
{
   m_rd->m_curLockCalls++;
   m_sharedBuffer->Lock(this, m_offset + offset, size == 0 ? m_size : size, data);
}

void IndexBuffer::Unlock()
{
   m_sharedBuffer->Unlock();
}

void IndexBuffer::ApplyOffset(VertexBuffer* vb)
{
   const unsigned int offset = vb->GetVertexOffset();
   if (offset == 0)
      return;
   for (SharedIndexBuffer::PendingUpload upload : m_sharedBuffer->m_pendingUploads)
   {
      if (upload.buffer == this)
      {
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
}

void IndexBuffer::Upload()
{
   if (!m_sharedBuffer->IsCreated())
      RemoveFromVectorSingle(m_rd->m_pendingSharedIndexBuffers, m_sharedBuffer);
   m_sharedBuffer->Upload();
}
