#include "stdafx.h"
#include "VertexBuffer.h"
#include "RenderDevice.h"
#include "Shader.h"

extern unsigned m_curLockCalls, m_frameLockCalls;

#ifdef __OPENGLES__ 
// Disabled since OpenGL ES does not support glDrawElementsBaseVertex and we need it unless we remap the indices when creating the index buffer (and we should)
#define COMBINE_BUFFERS 0
#else
#define COMBINE_BUFFERS 0
#endif

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

#ifdef ENABLE_SDL
vector<VertexBuffer*> VertexBuffer::notUploadedBuffers;
#endif

VertexBuffer::VertexBuffer(RenderDevice* rd, const unsigned int vertexCount, const DWORD usage, const DWORD fvf)
   : m_rd(rd)
   , m_vertexCount(vertexCount)
   , m_fvf(fvf)
   , m_sizePerVertex(fvfToSize(m_fvf))
   , m_usage(usage ? usage : USAGE_STATIC)
   , m_size(fvfToSize(m_fvf) * vertexCount)
{
#ifndef ENABLE_SDL
   // NB: We always specify WRITEONLY since MSDN states,
   // "Buffers created with D3DPOOL_DEFAULT that do not specify D3DUSAGE_WRITEONLY may suffer a severe performance penalty."
   // This means we cannot read from vertex buffers, but I don't think we need to.
   CHECKD3D(rd->GetCoreDevice()->CreateVertexBuffer(vertexCount * m_sizePerVertex, USAGE_STATIC | usage, 0, (D3DPOOL)memoryPool::DEFAULT, &m_vb, nullptr));
#endif
}

VertexBuffer::VertexBuffer(RenderDevice* rd, const unsigned int vertexCount, const DWORD usage, const DWORD fvf, const float* verts)
   : VertexBuffer(rd, vertexCount, usage, fvf)
{
   void* bufvb;
   lock(0, 0, (void**)&bufvb, VertexBuffer::WRITEONLY);
   memcpy(bufvb, verts, vertexCount * m_sizePerVertex);
   unlock();
}

VertexBuffer::~VertexBuffer()
{
   release();
}

#ifdef ENABLE_SDL
void VertexBuffer::ClearSharedBuffers()
{
   // FIXME add some debug logging since a well behaving application should not have any pending upload here
   notUploadedBuffers.clear(); 
}
#endif

void VertexBuffer::lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void **dataBuffer, const DWORD flags)
{
   m_curLockCalls++;
#ifdef ENABLE_SDL
   assert(m_dataBuffer == nullptr);
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
   if (m_sharedBuffer)
   {
      if (m_buffer == 0)
      {
         RemoveFromVectorSingle(notUploadedBuffers, this);
      }
      // FIXME we should ref count the shared buffers and release them
   }
   else if (m_buffer != 0)
   {
      glDeleteBuffers(1, &m_buffer);
      m_buffer = 0;
      m_offset = 0;
   }
#else
   SAFE_RELEASE(m_vb);
#endif
}

void VertexBuffer::bind()
{
#ifdef ENABLE_SDL
   if (!m_isUploaded)
   {
      if (m_sharedBuffer)
         UploadBuffers(m_rd);
      else
         UploadData();
   }
   glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
#else
   if (/*m_curVertexBuffer == nullptr ||*/ m_rd->m_curVertexBuffer != this)
   {
      CHECKD3D(m_rd->GetCoreDevice()->SetStreamSource(0, m_vb, 0, m_sizePerVertex));
      m_rd->m_curVertexBuffer = this;
   }
#endif
}

#ifdef ENABLE_SDL
void VertexBuffer::addToNotUploadedBuffers()
{
   assert(m_rd == g_pplayer->m_pin3d.m_pd3dPrimaryDevice);
   if (COMBINE_BUFFERS == 0 || m_usage != USAGE_STATIC)
      UploadData();
   else
   {
      m_sharedBuffer = true;
      if (std::find(notUploadedBuffers.begin(), notUploadedBuffers.end(), this) == notUploadedBuffers.end())
         notUploadedBuffers.push_back(this);
   }
}

void VertexBuffer::UploadData()
{
   if (m_buffer == 0)
   {
      glGenBuffers(1, &m_buffer);
      glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
      glBufferData(GL_ARRAY_BUFFER, m_size, nullptr, m_usage);
   }
   else
      glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
   if (m_size - m_offsetToLock > 0)
      glBufferSubData(GL_ARRAY_BUFFER, m_offset * m_sizePerVertex + m_offsetToLock, min(m_sizeToLock, m_size - m_offsetToLock), m_dataBuffer);
   m_rd->m_curVertexBuffer = this;
   m_isUploaded = true;
   free(m_dataBuffer);
   m_dataBuffer = nullptr;
}

void VertexBuffer::UploadBuffers(RenderDevice* rd)
{
   if (notUploadedBuffers.empty()) return;

   int countNT = 0;
   int countT = 0;
   GLuint BufferNT;
   GLuint BufferT;
   glGenBuffers(1, &BufferNT);
   glGenBuffers(1, &BufferT);
   for (auto it = notUploadedBuffers.begin(); it != notUploadedBuffers.end(); ++it) {
      assert(rd == (*it)->m_rd);
      if (!(*it)->m_isUploaded && (*it)->m_usage == GL_STATIC_DRAW)
      {
         if ((*it)->m_fvf == MY_D3DFVF_TEX)
         {
            (*it)->m_offset = countT;
            countT += (*it)->m_vertexCount;
            (*it)->m_buffer = BufferT;
         }
         else {
            (*it)->m_offset = countNT;
            countNT += (*it)->m_vertexCount;
            (*it)->m_buffer = BufferNT;
         }
         (*it)->m_sharedBuffer = true;
      }
   }
   //Allocate BufferData on GPU
   if (countNT > 0) {
      glBindBuffer(GL_ARRAY_BUFFER, BufferNT);
      glBufferData(GL_ARRAY_BUFFER, countNT * fvfToSize(MY_D3DFVF_NOTEX2_VERTEX), nullptr, GL_STATIC_DRAW);
   }
   if (countT > 0) {
      glBindBuffer(GL_ARRAY_BUFFER, BufferT);
      glBufferData(GL_ARRAY_BUFFER, countT * fvfToSize(MY_D3DFVF_TEX), nullptr, GL_STATIC_DRAW);
   }
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   rd->m_curVertexBuffer = nullptr;
   for (auto it = notUploadedBuffers.begin(); it != notUploadedBuffers.end(); ++it)
      (*it)->UploadData();
   notUploadedBuffers.clear();
   // OutputDebugString(">>>> Vertex Buffer uploaded\n");
}
#endif
