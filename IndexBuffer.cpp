#include "stdafx.h"
#include "IndexBuffer.h"
#include "RenderDevice.h"

extern unsigned m_curLockCalls, m_frameLockCalls;

//!! Disabled since it still has some bugs
#define COMBINE_BUFFERS 0

IndexBuffer* IndexBuffer::m_curIndexBuffer = nullptr; // is also reset before each Player start

#ifndef ENABLE_SDL
IDirect3DDevice9* IndexBuffer::m_pd3dPrimaryDevice = nullptr; // is set before each Player start
IDirect3DDevice9* IndexBuffer::m_pd3dSecondaryDevice = nullptr; // is set before each Player start
#endif

#ifdef ENABLE_SDL
std::vector<IndexBuffer*> IndexBuffer::notUploadedBuffers;
#endif

void IndexBuffer::CreateIndexBuffer(const unsigned int numIndices, const DWORD usage, const IndexBuffer::Format format, IndexBuffer **idxBuffer, const deviceNumber dN)
{
#ifdef ENABLE_SDL
   IndexBuffer* const ib = new IndexBuffer();
   ib->count = numIndices;
   ib->indexFormat = format;
   ib->size = numIndices * (ib->indexFormat == FMT_INDEX16 ? 2 : 4);
   ib->usage = usage ? usage : GL_STATIC_DRAW;
   *idxBuffer = ib;
   ib->isUploaded = false;
   ib->dataBuffer = nullptr;
#else
   // NB: We always specify WRITEONLY since MSDN states,
   // "Buffers created with D3DPOOL_DEFAULT that do not specify D3DUSAGE_WRITEONLY may suffer a severe performance penalty."
   const unsigned idxSize = (format == IndexBuffer::FMT_INDEX16) ? 2 : 4;
   const HRESULT hr = (dN == PRIMARY_DEVICE ? m_pd3dPrimaryDevice : m_pd3dSecondaryDevice)->CreateIndexBuffer(idxSize * numIndices, usage | USAGE_STATIC, (D3DFORMAT)format,
      (D3DPOOL)memoryPool::DEFAULT, (IDirect3DIndexBuffer9**)idxBuffer, nullptr);
   if (FAILED(hr))
      ReportError("Fatal Error: unable to create index buffer!", hr, __FILE__, __LINE__);
#endif
}

IndexBuffer* IndexBuffer::CreateAndFillIndexBuffer(const unsigned int numIndices, const WORD *indices, const deviceNumber dN)
{
#ifdef ENABLE_SDL
   IndexBuffer* const ib = new IndexBuffer();
   ib->count = numIndices;
   ib->indexFormat = IndexBuffer::FMT_INDEX16;
   ib->usage = GL_STATIC_DRAW;
   ib->size = numIndices * (ib->indexFormat == FMT_INDEX16 ? 2 : 4);
   ib->sizeToLock = ib->size;
   ib->Buffer = 0;
   if (COMBINE_BUFFERS == 0 || ib->usage != GL_STATIC_DRAW) {
      ib->dataBuffer = (void*)indices;
      ib->UploadData(false);
   }
   else {
      ib->offsetToLock = 0;
      ib->dataBuffer = nullptr;
      ib->addToNotUploadedBuffers(indices);
   }
#else
   IndexBuffer* ib;
   CreateIndexBuffer(numIndices, 0, IndexBuffer::FMT_INDEX16, &ib, dN);

   void* buf;
   ib->lock(0, 0, &buf, 0);
   memcpy(buf, indices, numIndices * sizeof(indices[0]));
   ib->unlock();
#endif
   return ib;
}

IndexBuffer* IndexBuffer::CreateAndFillIndexBuffer(const unsigned int numIndices, const unsigned int *indices, const deviceNumber dN)
{
#ifdef ENABLE_SDL
   IndexBuffer* const ib = new IndexBuffer();
   ib->count = numIndices;
   ib->indexFormat = IndexBuffer::FMT_INDEX32;
   ib->usage = GL_STATIC_DRAW;
   ib->size = numIndices * (ib->indexFormat == FMT_INDEX16 ? 2 : 4);
   ib->sizeToLock = ib->size;
   ib->Buffer = 0;
   if (COMBINE_BUFFERS == 0 || ib->usage != GL_STATIC_DRAW) {
      ib->dataBuffer = (void*)indices;
      ib->UploadData(false);
   }
   else {
      ib->offsetToLock = 0;
      ib->dataBuffer = nullptr;
      ib->addToNotUploadedBuffers(indices);
   }
#else
   IndexBuffer* ib;
   CreateIndexBuffer(numIndices, 0, IndexBuffer::FMT_INDEX32, &ib, dN);

   void* buf;
   ib->lock(0, 0, &buf, 0);
   memcpy(buf, indices, numIndices * sizeof(indices[0]));
   ib->unlock();
#endif
   return ib;
}

IndexBuffer* IndexBuffer::CreateAndFillIndexBuffer(const std::vector<WORD>& indices, const deviceNumber dN)
{
   return CreateAndFillIndexBuffer((unsigned int)indices.size(), indices.data(), dN);
}

IndexBuffer* IndexBuffer::CreateAndFillIndexBuffer(const std::vector<unsigned int>& indices, const deviceNumber dN)
{
   return CreateAndFillIndexBuffer((unsigned int)indices.size(), indices.data(), dN);
}

void IndexBuffer::lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void **dataBuffer, const DWORD flags)
{
   m_curLockCalls++;
#ifdef ENABLE_SDL
   if (sizeToLock == 0)
      this->sizeToLock = size;
   else
      this->sizeToLock = sizeToLock;

   if (offsetToLock < size) {
      *dataBuffer = malloc(this->sizeToLock);
      this->dataBuffer = *dataBuffer;
      this->offsetToLock = offsetToLock;
   }
   else {
      *dataBuffer = nullptr;
      this->dataBuffer = nullptr;
      this->sizeToLock = 0;
   }
#else
   CHECKD3D(this->Lock(offsetToLock, sizeToLock, dataBuffer, flags));
#endif
}

void IndexBuffer::unlock()
{
#ifdef ENABLE_SDL
   if (COMBINE_BUFFERS == 0 || usage != GL_STATIC_DRAW || Buffer>0)
      UploadData(true);
   else
      addToNotUploadedBuffers();
#else
   CHECKD3D(this->Unlock());
#endif
}

void IndexBuffer::release(void)
{
#ifdef ENABLE_SDL
   if (!sharedBuffer)
      CHECKD3D(glDeleteBuffers(1, &this->Buffer));
   this->Buffer = 0;
#else
   SAFE_RELEASE_NO_CHECK_NO_SET(this);
#endif
}

void IndexBuffer::bind(const deviceNumber dN)
{
#ifdef ENABLE_SDL
   if (!isUploaded)
      IndexBuffer::UploadBuffers();//Should never happen...
   if (m_curIndexBuffer == nullptr || this->Buffer != m_curIndexBuffer->Buffer)
   {
      CHECKD3D(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->Buffer));
      m_curIndexBuffer = this;
   }
#else
   if (m_curIndexBuffer == nullptr || m_curIndexBuffer != this)
   {
      CHECKD3D((dN == PRIMARY_DEVICE ? m_pd3dPrimaryDevice : m_pd3dSecondaryDevice)->SetIndices(this));
      m_curIndexBuffer = this;
   }
#endif
}

#ifdef ENABLE_SDL
void IndexBuffer::UploadData(bool freeData)
{
   if (isUploaded || !dataBuffer) return;

   if (Buffer == 0) {
      sharedBuffer = false;
      sizeToLock = size;
      CHECKD3D(glGenBuffers(1, &(Buffer)));
      CHECKD3D(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffer));
      CHECKD3D(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, nullptr, usage));
      offset = 0;
   }
   else
      CHECKD3D(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffer));
   CHECKD3D(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset + offsetToLock, min(sizeToLock, size - offsetToLock), dataBuffer));
   CHECKD3D(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
   if (freeData)
      free(dataBuffer);
   dataBuffer = nullptr;
   isUploaded = true;
}

void IndexBuffer::addToNotUploadedBuffers(const void* indices)
{
   if (std::find(notUploadedBuffers.begin(), notUploadedBuffers.end(), this) == notUploadedBuffers.end())
      notUploadedBuffers.push_back(this);
   else if (indices) {
      //merge dataBuffer and indices...
   }
   offsetToLock = 0;
   if (indices && indices != dataBuffer) {
      if (dataBuffer)
         free(dataBuffer);
      dataBuffer = malloc(size);
      memcpy(dataBuffer, indices, size);
   }
   Buffer = 0;
   isUploaded = false;
}

void IndexBuffer::UploadBuffers()
{
   if (notUploadedBuffers.size() == 0) return;

   int size16 = 0;
   int size32 = 0;
   GLuint Buffer16;
   GLuint Buffer32;
   glGenBuffers(1, &Buffer16);
   CHECKD3D(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffer16));
   glGenBuffers(1, &Buffer32);
   CHECKD3D(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffer32));
   //Find out how much data needs to be uploaded
   for (auto it = notUploadedBuffers.begin(); it != notUploadedBuffers.end(); it++) {
      if (!(*it)->isUploaded && (*it)->usage == GL_STATIC_DRAW) {
         if ((*it)->Buffer>0 && !(*it)->sharedBuffer)
            CHECKD3D(glDeleteBuffers(1, &(*it)->Buffer));
         if ((*it)->indexFormat == FMT_INDEX16) {
            (*it)->offset = size16;
            size16 += (*it)->size;
            (*it)->Buffer = Buffer16;
         }
         else {
            (*it)->offset = size32;
            size32 += (*it)->size;
            (*it)->Buffer = Buffer32;
         }
         (*it)->sharedBuffer = true;
      }
   }
   //Allocate BufferData on GPU
   if (size16 > 0) {
      CHECKD3D(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffer16));
      CHECKD3D(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size16, nullptr, GL_STATIC_DRAW));
   }
   if (size32 > 0) {
      CHECKD3D(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffer32));
      CHECKD3D(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size32, nullptr, GL_STATIC_DRAW));
   }
   //Upload all Buffers
   for (auto it = notUploadedBuffers.begin(); it != notUploadedBuffers.end(); it++)
      (*it)->UploadData(true);
   CHECKD3D(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
   notUploadedBuffers.clear();
}
#endif
