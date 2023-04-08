#include "stdafx.h"
#include "MeshBuffer.h"
#include "RenderDevice.h"

#ifdef ENABLE_SDL
vector<MeshBuffer::SharedVAO*> MeshBuffer::sharedVAOs;
#endif

#include "Shader.h"
#include "VertexBuffer.h"

#include <locale>
#include <codecvt>

MeshBuffer::MeshBuffer(VertexBuffer* vb, IndexBuffer* ib, const bool applyVertexBufferOffsetToIndexBuffer)
   : MeshBuffer(L""s, vb, ib, applyVertexBufferOffsetToIndexBuffer)
{
}

MeshBuffer::MeshBuffer(const wstring& name, VertexBuffer* vb, IndexBuffer* ib, const bool applyVertexBufferOffsetToIndexBuffer)
   : m_name(std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(name))
   , m_vb(vb)
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

unsigned int MeshBuffer::GetSortKey() const
{
   #ifdef ENABLE_SDL // OpenGL
   return m_vao;
   #else // DirectX 9
   return ((unsigned int)m_vb) ^ ((unsigned int)m_ib);
   #endif
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

