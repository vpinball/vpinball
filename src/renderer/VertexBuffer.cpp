// license:GPLv3+

#include "core/stdafx.h"
#include "VertexBuffer.h"
#include "RenderDevice.h"
#include "Shader.h"


class SharedVertexBuffer : public SharedBuffer<VertexFormat, VertexBuffer>
{
public:
   SharedVertexBuffer(RenderDevice* const rd, VertexFormat fmt, bool stat);
   ~SharedVertexBuffer();
   void Upload() override;

   #if defined(ENABLE_BGFX)
   bgfx::VertexBufferHandle m_vb = BGFX_INVALID_HANDLE;
   bgfx::DynamicVertexBufferHandle m_dvb = BGFX_INVALID_HANDLE;
   bool IsCreated() const override { return m_isStatic ? bgfx::isValid(m_vb) : bgfx::isValid(m_dvb); }
   const bgfx::VertexLayout* const m_vertexDeclaration;

   #elif defined(ENABLE_OPENGL)
   GLuint m_vb = 0;
   void Bind() const { glBindBuffer(GL_ARRAY_BUFFER, m_vb); }
   bool IsCreated() const override { return m_vb; }

   #elif defined(ENABLE_DX9)
   IDirect3DVertexBuffer9* m_vb = nullptr;
   IDirect3DVertexDeclaration9* const m_vertexDeclaration;
   bool IsCreated() const override { return m_vb; }
   #endif
};

SharedVertexBuffer::SharedVertexBuffer(RenderDevice* const rd, VertexFormat fmt, bool stat)
   : SharedBuffer(fmt, fmt ==  VertexFormat::VF_POS_NORMAL_TEX ? sizeof(Vertex3D_NoTex2) : sizeof(Vertex3D_TexelOnly), stat)
#if defined(ENABLE_DX9) || defined(ENABLE_BGFX)
   , m_vertexDeclaration(
      fmt == VertexFormat::VF_POS_NORMAL_TEX ? rd->m_pVertexNormalTexelDeclaration :
      fmt == VertexFormat::VF_POS_TEX        ? rd->m_pVertexTexelDeclaration : nullptr)
#endif
{
}

SharedVertexBuffer::~SharedVertexBuffer()
{
   if (IsCreated())
   {
      #if defined(ENABLE_BGFX)
      if (bgfx::isValid(m_vb))
         bgfx::destroy(m_vb);
      if (bgfx::isValid(m_dvb))
         bgfx::destroy(m_dvb);
      #elif defined(ENABLE_OPENGL)
      glDeleteBuffers(1, &m_vb);
      #elif defined(ENABLE_DX9)
      SAFE_RELEASE(m_vb);
      #endif
   }
   for (PendingUpload upload : m_pendingUploads)
      delete[] upload.data;
}

void SharedVertexBuffer::Upload()
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
      // We always specify WRITEONLY since MSDN states,
      // "Buffers created with D3DPOOL_DEFAULT that do not specify D3DUSAGE_WRITEONLY may suffer a severe performance penalty."
      // This means we cannot read from vertex buffers, but I don't think we need to.
      CHECKD3D(m_buffers[0]->m_rd->GetCoreDevice()->CreateVertexBuffer(size, D3DUSAGE_WRITEONLY | (m_isStatic ? 0 : D3DUSAGE_DYNAMIC), 0 /* sharedBuffer->format */, D3DPOOL_DEFAULT, &m_vb, nullptr));
      UINT8* data;
      CHECKD3D(m_vb->Lock(0, size, (void**)&data, 0));

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
         m_vb = bgfx::createVertexBuffer(mem, *m_vertexDeclaration, BGFX_BUFFER_NONE);
      else
         m_dvb = bgfx::createDynamicVertexBuffer(mem, *m_vertexDeclaration, BGFX_BUFFER_NONE);

      #elif defined(ENABLE_OPENGL)
         #ifndef __OPENGLES__
         if (GLAD_GL_VERSION_4_5)
         {
            glCreateBuffers(1, &m_vb);
            glNamedBufferStorage(m_vb, size, data, m_isStatic ? 0 : GL_DYNAMIC_STORAGE_BIT);
         }
         else if (GLAD_GL_VERSION_4_4)
         {
            glGenBuffers(1, &m_vb);
            Bind();
            glBufferStorage(GL_ARRAY_BUFFER, size, data, m_isStatic ? 0 : GL_DYNAMIC_STORAGE_BIT);
         }
         else
         #endif
      {
         glGenBuffers(1, &m_vb);
         Bind();
         glBufferData(GL_ARRAY_BUFFER, size, data, m_isStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
      }
      free(data);

      #elif defined(ENABLE_DX9)
      CHECKD3D(m_vb->Unlock());

      #endif
   }
   else
   {
      for (PendingUpload upload : m_pendingUploads)
      {
         assert(!m_isStatic);
         #if defined(ENABLE_BGFX)
         bgfx::update(m_dvb, upload.offset / m_bytePerElement, upload.mem);

         #elif defined(ENABLE_OPENGL)
         #ifndef __OPENGLES__
         if (GLAD_GL_VERSION_4_5)
            glNamedBufferSubData(m_vb, upload.offset, upload.size, upload.data);
         else
         #endif
         {
            Bind();
            glBufferSubData(GL_ARRAY_BUFFER, upload.offset, upload.size, upload.data);
         }
         delete[] upload.data;

         #elif defined(ENABLE_DX9)
         // It would be better to perform a single lock but in fact, I don't think there are situations where more than one update is pending
         UINT8* data;
         CHECKD3D(m_vb->Lock(upload.offset, upload.size, (void**)&data, 0));
         memcpy(data, upload.data, upload.size);
         CHECKD3D(m_vb->Unlock());
         delete[] upload.data;

         #endif
      }
      m_pendingUploads.clear();
   }
}



VertexBuffer::VertexBuffer(RenderDevice* rd, const unsigned int vertexCount, const float* verts, const bool isDynamic, const VertexFormat fmt)
   : m_rd(rd)
   , m_count(vertexCount)
   , m_vertexFormat(fmt)
   , m_sizePerVertex(fmt ==  VertexFormat::VF_POS_NORMAL_TEX ? sizeof(Vertex3D_NoTex2) : sizeof(Vertex3D_TexelOnly))
   , m_isStatic(!isDynamic)
   , m_size(vertexCount * (fmt ==  VertexFormat::VF_POS_NORMAL_TEX ? sizeof(Vertex3D_NoTex2) : sizeof(Vertex3D_TexelOnly)))
{
   // Disabled since OpenGL ES does not support glDrawElementsBaseVertex, but now that we remap the indices when creating the index buffer it should be good
   for (SharedVertexBuffer* block : m_rd->m_pendingSharedVertexBuffers)
   {
      if (block->m_format == fmt && block->m_isStatic == m_isStatic && block->GetCount() + vertexCount <= 65535)
      {
         m_sharedBuffer = block;
         break;
      }
   }
   if (m_sharedBuffer == nullptr)
   {
      m_sharedBuffer = new SharedVertexBuffer(rd, fmt, m_isStatic);
      m_rd->m_pendingSharedVertexBuffers.push_back(m_sharedBuffer);
   }
   m_vertexOffset = m_sharedBuffer->Add(this);
   m_offset = m_vertexOffset * m_sizePerVertex;
   if (verts != nullptr)
   {
      void* data;
      Lock(data);
      memcpy(data, verts, m_size);
      Unlock();
   }
}

VertexBuffer::~VertexBuffer()
{
   if (m_sharedBuffer->Remove(this))
   {
      RemoveFromVectorSingle(m_rd->m_pendingSharedVertexBuffers, m_sharedBuffer);
      delete m_sharedBuffer;
   }
}

bool VertexBuffer::IsSharedBuffer() const { return m_sharedBuffer->IsShared(); }

#if defined(ENABLE_BGFX)
bgfx::VertexBufferHandle VertexBuffer::GetStaticBuffer() const { return m_sharedBuffer->m_vb; }
bgfx::DynamicVertexBufferHandle VertexBuffer::GetDynamicBuffer() const { return m_sharedBuffer->m_dvb; }

#elif defined(ENABLE_OPENGL)
GLuint VertexBuffer::GetBuffer() const { return m_sharedBuffer->m_vb; }
void VertexBuffer::Bind() const { m_sharedBuffer->Bind(); }

#elif defined(ENABLE_DX9)
IDirect3DVertexBuffer9* VertexBuffer::GetBuffer() const { return m_sharedBuffer->m_vb; }
void VertexBuffer::Bind() const
{
   if (m_rd->m_curVertexBuffer != m_sharedBuffer->m_vb)
   {
      CHECKD3D(m_rd->GetCoreDevice()->SetStreamSource(0, m_sharedBuffer->m_vb, 0, m_sizePerVertex));
      m_rd->m_curVertexBuffer = m_sharedBuffer->m_vb;
   }
   if (m_rd->m_currentVertexDeclaration != m_sharedBuffer->m_vertexDeclaration)
   {
      CHECKD3D(m_rd->GetCoreDevice()->SetVertexDeclaration(m_sharedBuffer->m_vertexDeclaration));
      m_rd->m_currentVertexDeclaration = m_sharedBuffer->m_vertexDeclaration;
      m_rd->m_curStateChanges++;
   }
}

#endif

void VertexBuffer::LockUntyped(void*& data, const unsigned int offset, const unsigned int size)
{
   m_rd->m_curLockCalls++;
   m_sharedBuffer->Lock(this, m_offset + offset, size == 0 ? m_size : size, data);
}

void VertexBuffer::Unlock()
{
   m_sharedBuffer->Unlock();
}

void VertexBuffer::Upload()
{
   if (!m_sharedBuffer->IsCreated())
      RemoveFromVectorSingle(m_rd->m_pendingSharedVertexBuffers, m_sharedBuffer);
   m_sharedBuffer->Upload();
}
