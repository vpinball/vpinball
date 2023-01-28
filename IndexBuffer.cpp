#include "stdafx.h"
#include "IndexBuffer.h"
#include "RenderDevice.h"

extern unsigned m_curLockCalls, m_frameLockCalls;

//!! Disabled since it still has some bugs
#define COMBINE_BUFFERS 0

IndexBuffer* IndexBuffer::m_curIndexBuffer = nullptr; // is also reset before each Player start

#ifdef ENABLE_SDL
vector<IndexBuffer*> IndexBuffer::notUploadedBuffers;
#endif

IndexBuffer::IndexBuffer(RenderDevice* rd, const unsigned int numIndices, const DWORD usage, const IndexBuffer::Format format)
{
   m_rd = rd;
   m_usage = usage ? usage : USAGE_STATIC;
   m_indexFormat = format;
   m_sizePerIndex = m_indexFormat == FMT_INDEX16 ? 2 : 4;
#ifdef ENABLE_SDL
   m_count = numIndices;
   m_size = numIndices * m_sizePerIndex;
   m_isUploaded = false;
   m_dataBuffer = nullptr;
   m_Buffer = 0;
   m_offset = 0;
   m_offsetToLock = 0;
   m_sizeToLock = 0;
   m_sharedBuffer = false;
#else
   // NB: We always specify WRITEONLY since MSDN states,
   // "Buffers created with D3DPOOL_DEFAULT that do not specify D3DUSAGE_WRITEONLY may suffer a severe performance penalty."
   const HRESULT hr = m_rd->GetCoreDevice()->CreateIndexBuffer(m_sizePerIndex * numIndices, usage | USAGE_STATIC, (D3DFORMAT)format, (D3DPOOL)memoryPool::DEFAULT, &m_ib, nullptr);
   if (FAILED(hr))
      ReportError("Fatal Error: unable to create index buffer!", hr, __FILE__, __LINE__);
#endif
}

IndexBuffer* IndexBuffer::CreateAndFillIndexBuffer(RenderDevice* rd, const unsigned int numIndices, const WORD* indices)
{
   IndexBuffer* ib = new IndexBuffer(rd, numIndices, 0, IndexBuffer::FMT_INDEX16);
   void* buf;
   ib->lock(0, 0, &buf, WRITEONLY);
   memcpy(buf, indices, numIndices * sizeof(indices[0]));
   ib->unlock();
   return ib;
}

IndexBuffer* IndexBuffer::CreateAndFillIndexBuffer(RenderDevice* rd, const unsigned int numIndices, const unsigned int* indices)
{
   IndexBuffer* ib = new IndexBuffer(rd, numIndices, 0, IndexBuffer::FMT_INDEX32);
   void* buf;
   ib->lock(0, 0, &buf, WRITEONLY);
   memcpy(buf, indices, numIndices * sizeof(indices[0]));
   ib->unlock();
   return ib;
}

IndexBuffer* IndexBuffer::CreateAndFillIndexBuffer(RenderDevice* rd, const vector<WORD>& indices)
{
   return CreateAndFillIndexBuffer(rd, (unsigned int)indices.size(), indices.data());
}

IndexBuffer* IndexBuffer::CreateAndFillIndexBuffer(RenderDevice* rd, const vector<unsigned int>& indices)
{
   return CreateAndFillIndexBuffer(rd, (unsigned int)indices.size(), indices.data());
}

void IndexBuffer::lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void **dataBuffer, const DWORD flags)
{
   m_curLockCalls++;
#ifdef ENABLE_SDL
   assert(m_dataBuffer == nullptr);
   m_sizeToLock = sizeToLock == 0 ? m_size : sizeToLock;
   if (offsetToLock < m_size) {
      *dataBuffer = malloc(m_sizeToLock); //!! does not init the buffer from the IBuffer data if flags is set accordingly (i.e. WRITEONLY, or better: create a new flag like 'PARTIALUPDATE'?)
      m_dataBuffer = *dataBuffer;
      m_offsetToLock = offsetToLock;
   }
   else {
      *dataBuffer = nullptr;
      m_dataBuffer = nullptr;
      m_sizeToLock = 0;
   }
#else
   CHECKD3D(m_ib->Lock(offsetToLock, sizeToLock, dataBuffer, flags));
#endif
}

void IndexBuffer::unlock()
{
#ifdef ENABLE_SDL
   if (!m_dataBuffer)
      return;
   addToNotUploadedBuffers();
#else
   CHECKD3D(m_ib->Unlock());
#endif
}

void IndexBuffer::release()
{
#ifdef ENABLE_SDL
   if (!m_sharedBuffer && (m_Buffer != 0))
   {
      glDeleteBuffers(1, &m_Buffer);
      m_Buffer = 0;
      m_offset = 0;
      m_count = 0;
      m_size = 0;
   }
#else
   SAFE_RELEASE(m_ib);
#endif
}

void IndexBuffer::bind()
{
#ifdef ENABLE_SDL
   if (!m_isUploaded)
   {
      if (m_sharedBuffer)
         UploadBuffers();
      else
         UploadData();
   }
   if (m_curIndexBuffer == nullptr || m_Buffer != m_curIndexBuffer->m_Buffer)
   {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffer);
      m_curIndexBuffer = this;
   }
#else
   if (/*m_curIndexBuffer == nullptr ||*/ m_curIndexBuffer != this)
   {
      CHECKD3D(m_rd->GetCoreDevice()->SetIndices(m_ib));
      m_curIndexBuffer = this;
   }
#endif
}

#ifdef ENABLE_SDL
void IndexBuffer::addToNotUploadedBuffers()
{
   if (COMBINE_BUFFERS == 0 || m_usage != USAGE_STATIC)
      UploadData();
   else
   {
      m_sharedBuffer = true;
      if (std::find(notUploadedBuffers.begin(), notUploadedBuffers.end(), this) == notUploadedBuffers.end())
         notUploadedBuffers.push_back(this);
   }
}

void IndexBuffer::UploadData()
{
   if (m_Buffer == 0)
   {
      glGenBuffers(1, &m_Buffer);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffer);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_size, nullptr, m_usage);
   }
   else
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffer);
   if (m_size - m_offsetToLock > 0)
      glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, m_offset + m_offsetToLock, min(m_sizeToLock, m_size - m_offsetToLock), m_dataBuffer);
   m_curIndexBuffer = this;
   m_isUploaded = true;
   free(m_dataBuffer);
   m_dataBuffer = nullptr;
}

void IndexBuffer::UploadBuffers()
{
   if (notUploadedBuffers.empty()) return;

   int size16 = 0;
   int size32 = 0;
   GLuint Buffer16;
   GLuint Buffer32;
   glGenBuffers(1, &Buffer16);
   glGenBuffers(1, &Buffer32);
   //Find out how much data needs to be uploaded
   for (auto it = notUploadedBuffers.begin(); it != notUploadedBuffers.end(); ++it) {
      assert((*it)->m_sharedBuffer == true);
      if (!(*it)->m_isUploaded && (*it)->m_usage == USAGE_STATIC)
      {
         if ((*it)->m_indexFormat == FMT_INDEX16)
         {
            (*it)->m_offset = size16;
            size16 += (*it)->m_size;
            (*it)->m_Buffer = Buffer16;
         }
         else
         {
            (*it)->m_offset = size32;
            size32 += (*it)->m_size;
            (*it)->m_Buffer = Buffer32;
         }
      }
   }
   //Allocate BufferData on GPU
   if (size16 > 0) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffer16);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, size16 * 2, nullptr, GL_STATIC_DRAW);
   }
   if (size32 > 0) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffer32);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, size32 * 4, nullptr, GL_STATIC_DRAW);
   }
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   m_curIndexBuffer = nullptr;
   for (auto it = notUploadedBuffers.begin(); it != notUploadedBuffers.end(); ++it)
      (*it)->UploadData();
   notUploadedBuffers.clear();
   // OutputDebugString(">>>> Index Buffer uploaded\n");
}
#endif
