#include "stdafx.h"
#include "IndexBuffer.h"
#include "RenderDevice.h"

extern unsigned m_curLockCalls, m_frameLockCalls;

#define COMBINE_BUFFERS 1

#ifdef ENABLE_SDL
vector<IndexBuffer*> IndexBuffer::notUploadedBuffers;
vector<MeshBuffer::SharedVAO> MeshBuffer::sharedVAOs;
#endif

#include "Shader.h"
#include "VertexBuffer.h"

MeshBuffer::MeshBuffer(const DWORD fvf, VertexBuffer* vb, const bool ownBuffers)
   : MeshBuffer(fvf, vb, nullptr, ownBuffers)
{
}

MeshBuffer::MeshBuffer(const DWORD fvf, VertexBuffer* vb, IndexBuffer* ib, const bool ownBuffers)
   : m_vertexFormat(fvf)
   , m_vb(vb)
   , m_ib(ib)
   , m_ownBuffers(ownBuffers)
{
}

MeshBuffer::~MeshBuffer()
{
#ifdef ENABLE_SDL
   if (m_isSharedVAO)
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
   else if (m_vao != 0)
   {
      glDeleteVertexArrays(1, &m_vao);
      m_vao = 0;
   }
   #endif
   if (m_ownBuffers)
   {
      delete m_vb;
      delete m_ib;
   }
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
      if (m_ib)
         m_ib->bind();
      // If index & vertex buffer are using shared buffers (for static objects), then this buffer should use a shared VAO
      m_isSharedVAO = m_vb->useSharedBuffer() && (m_ib == nullptr || m_ib->useSharedBuffer());
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
      }
      // FIXME this supposes that this mesh buffer is always used with the same attribute layout.
      // This happens to be true but it would be more clean to fix the attribute layout in the shaders
      Shader::GetCurrentShader()->setAttributeFormat(m_vertexFormat);
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
      if (!m_vb->isUploaded())
         m_vb->bind();
      if (m_ib && !m_vb->isUploaded())
         m_ib->bind();
   }
#else
   m_vb->bind();
   if (m_ib)
      m_ib->bind();
#endif
}

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
   m_buffer = 0;
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
   release();
}

#ifdef ENABLE_SDL
void IndexBuffer::ClearSharedBuffers()
{
   // FIXME add some debug logging since a well behaving application should not have any pending upload here
   notUploadedBuffers.clear();
}
#endif

void IndexBuffer::lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void **dataBuffer, const DWORD flags)
{
   m_curLockCalls++;
#ifdef ENABLE_SDL
   assert(m_dataBuffer == nullptr);
   m_sizeToLock = sizeToLock == 0 ? m_size : sizeToLock;
   if (offsetToLock < m_size)
   {
      *dataBuffer = malloc(m_sizeToLock); //!! does not init the buffer from the IBuffer data if flags is set accordingly (i.e. WRITEONLY, or better: create a new flag like 'PARTIALUPDATE'?)
      m_dataBuffer = *dataBuffer;
      m_offsetToLock = offsetToLock;
   }
   else
   {
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
   if (!m_sharedBuffer && (m_buffer != 0))
   {
      glDeleteBuffers(1, &m_buffer);
      m_buffer = 0;
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
         UploadBuffers(m_rd);
      else
         UploadData();
   }
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer);
#else
   if (/*m_curIndexBuffer == nullptr ||*/ m_rd->m_curIndexBuffer != this)
   {
      CHECKD3D(m_rd->GetCoreDevice()->SetIndices(m_ib));
      m_rd->m_curIndexBuffer = this;
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
   if (m_buffer == 0)
   {
      glGenBuffers(1, &m_buffer);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_size, nullptr, m_usage);
   }
   else
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer);
   if (m_size - m_offsetToLock > 0)
      glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, m_offset + m_offsetToLock, min(m_sizeToLock, m_size - m_offsetToLock), m_dataBuffer);
   m_rd->m_curIndexBuffer = this;
   m_isUploaded = true;
   free(m_dataBuffer);
   m_dataBuffer = nullptr;
}

void IndexBuffer::UploadBuffers(RenderDevice* rd)
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
            (*it)->m_buffer = Buffer16;
         }
         else
         {
            (*it)->m_offset = size32;
            size32 += (*it)->m_size;
            (*it)->m_buffer = Buffer32;
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
   rd->m_curIndexBuffer = nullptr;
   for (auto it = notUploadedBuffers.begin(); it != notUploadedBuffers.end(); ++it)
      (*it)->UploadData();
   notUploadedBuffers.clear();
   // OutputDebugString(">>>> Index Buffer uploaded\n");
}
#endif
