#include "stdafx.h"
#include "VertexBuffer.h"
#include "RenderDevice.h"
#include "Shader.h"

extern unsigned m_curLockCalls, m_frameLockCalls;

//!! Disabled since it still has some bugs
#define COMBINE_BUFFERS 0

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
      return 0;
   }
}

VertexBuffer* VertexBuffer::m_curVertexBuffer = nullptr; // is reset before each Player start

#ifdef ENABLE_SDL
vector<VertexBuffer*> VertexBuffer::notUploadedBuffers;
#endif

VertexBuffer::VertexBuffer(RenderDevice* rd, const unsigned int vertexCount, const DWORD usage, const DWORD fvf)
{
   m_rd = rd;
   m_fvf = fvf;
   m_sizePerVertex = fvfToSize(m_fvf);
   m_usage = usage ? usage : USAGE_STATIC;
#ifdef ENABLE_SDL
   m_count = vertexCount;
   m_size = m_sizePerVertex * m_count;
   m_isUploaded = false;
   m_Array = 0;
   m_Buffer = 0;
#else
   // NB: We always specify WRITEONLY since MSDN states,
   // "Buffers created with D3DPOOL_DEFAULT that do not specify D3DUSAGE_WRITEONLY may suffer a severe performance penalty."
   // This means we cannot read from vertex buffers, but I don't think we need to.
   const HRESULT hr = rd->GetCoreDevice()->CreateVertexBuffer(vertexCount * m_sizePerVertex, USAGE_STATIC | usage, 0, (D3DPOOL)memoryPool::DEFAULT, &m_vb, nullptr);
   if (FAILED(hr))
      ReportError("Fatal Error: unable to create vertex buffer!", hr, __FILE__, __LINE__);
#endif
}

void VertexBuffer::lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void **dataBuffer, const DWORD flags)
{
   m_curLockCalls++;
#ifdef ENABLE_SDL
   m_sizeToLock = sizeToLock == 0 ? m_size : sizeToLock;
   if (offsetToLock < m_size)
   {
      *dataBuffer = malloc(m_sizeToLock); //!! does not init the buffer from the VBuffer data if flags is set accordingly (i.e. WRITEONLY, or better: create a new flag like 'PARTIALUPDATE'?)
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
   CHECKD3D(m_vb->Lock(offsetToLock, sizeToLock, dataBuffer, flags));
#endif
}

void VertexBuffer::unlock()
{
#ifdef ENABLE_SDL
   if (!m_dataBuffer)
      return;
   addToNotUploadedBuffers();
#else
   CHECKD3D(m_vb->Unlock());
#endif
}

void VertexBuffer::release()
{
#ifdef ENABLE_SDL
   if (!m_sharedBuffer && (m_Buffer != 0))
   {
      glDeleteBuffers(1, &m_Buffer);
      glDeleteVertexArrays(1, &m_Array);
      m_Buffer = 0;
      m_Array = 0;
      m_sizePerVertex = 0;
      m_offset = 0;
      m_m_count = 0;
      m_size = 0;
   }
#else
   SAFE_RELEASE(m_vb);
#endif
}

void VertexBuffer::bind()
{
#ifdef ENABLE_SDL
   if (!m_isUploaded) {
      if (m_sharedBuffer)
         UploadBuffers();
      else
         UploadData();
   }
   if (m_curVertexBuffer == nullptr || m_Array != m_curVertexBuffer->m_Array || m_Buffer != m_curVertexBuffer->m_Buffer)
   {
      glBindVertexArray(m_Array);
      glBindBuffer(GL_ARRAY_BUFFER, m_Buffer);
      m_curVertexBuffer = this;
      IndexBuffer::m_curIndexBuffer = nullptr; // glBindVertexArray also can/will change the GL_ELEMENT_ARRAY_BUFFER binding!
   }
#else
   if (/*m_curVertexBuffer == nullptr ||*/ m_curVertexBuffer != this)
   {
      CHECKD3D(m_rd->GetCoreDevice()->SetStreamSource(0, m_vb, 0, m_sizePerVertex));
      m_curVertexBuffer = this;
   }
#endif
}

#ifdef ENABLE_SDL
void VertexBuffer::UploadData()
{
   if (m_Array == 0)
      glGenVertexArrays(1, &m_Array);
   glBindVertexArray(m_Array);
   if (m_Buffer == 0)
   {
      glGenBuffers(1, &m_Buffer);
      glBindBuffer(GL_ARRAY_BUFFER, m_Buffer);
      glBufferData(GL_ARRAY_BUFFER, m_size, nullptr, m_usage);
   }
   else
      glBindBuffer(GL_ARRAY_BUFFER, m_Buffer);
   if (m_size - m_offsetToLock > 0)
      glBufferSubData(GL_ARRAY_BUFFER, m_offset * m_sizePerVertex + m_offsetToLock, min(m_sizeToLock, m_size - m_offsetToLock), m_dataBuffer);
   m_curVertexBuffer = this;
   m_isUploaded = true;
   free(m_dataBuffer);
   m_dataBuffer = nullptr;
}

void VertexBuffer::addToNotUploadedBuffers()
{
   if (COMBINE_BUFFERS == 0 || m_usage != USAGE_STATIC)
      UploadData();
   else if (std::find(notUploadedBuffers.begin(), notUploadedBuffers.end(), this) == notUploadedBuffers.end())
      notUploadedBuffers.push_back(this);
}

void VertexBuffer::UploadBuffers()
{
   if (notUploadedBuffers.empty()) return;

   int countNT = 0;
   int countT = 0;
   GLuint BufferNT;
   GLuint BufferT;
   GLuint ArrayNT;
   GLuint ArrayT;
   glGenVertexArrays(1, &ArrayNT);
   glGenVertexArrays(1, &ArrayT);
   glGenBuffers(1, &BufferNT);
   glGenBuffers(1, &BufferT);
   for (auto it = notUploadedBuffers.begin(); it != notUploadedBuffers.end(); ++it) {
      if (!(*it)->m_isUploaded && (*it)->m_usage == GL_STATIC_DRAW)
      {
         if ((*it)->m_fvf == MY_D3DFVF_TEX)
         {
            (*it)->m_offset = countT;
            countT += (*it)->m_count;
            (*it)->m_Buffer = BufferT;
            (*it)->m_Array = ArrayT;
         }
         else {
            (*it)->m_offset = countNT;
            countNT += (*it)->m_size;
            (*it)->m_Buffer = BufferNT;
            (*it)->m_Array = ArrayNT;
         }
         (*it)->m_sharedBuffer = true;
      }
   }
   //Allocate BufferData on GPU
   if (countNT > 0) {
      glBindBuffer(GL_ARRAY_BUFFER, BufferNT);
      glBindVertexArray(ArrayNT);
      glBufferData(GL_ARRAY_BUFFER, countNT * fvfToSize(MY_D3DFVF_NOTEX2_VERTEX), nullptr, GL_STATIC_DRAW);
   }
   if (countT > 0) {
      glBindBuffer(GL_ARRAY_BUFFER, BufferT);
      glBindVertexArray(ArrayT);
      glBufferData(GL_ARRAY_BUFFER, countT * fvfToSize(MY_D3DFVF_TEX), nullptr, GL_STATIC_DRAW);
   }
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   m_curVertexBuffer = nullptr;
   for (auto it = notUploadedBuffers.begin(); it != notUploadedBuffers.end(); ++it)
      (*it)->UploadData();
   notUploadedBuffers.clear();
}
#endif
