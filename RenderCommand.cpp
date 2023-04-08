#include "stdafx.h"
#include "RenderCommand.h"

// Enable frame logging
//#define WIN_LOG

RenderCommand::RenderCommand(RenderDevice* rd)
   : m_rd(rd)
{
   memset(m_uniformState, 0, SHADER_UNIFORM_COUNT * sizeof(Shader::UniformCache));
}

RenderCommand::~RenderCommand()
{
}

bool RenderCommand::IsFullClear(const bool hasDepth) const
{
   if (m_command == RC_CLEAR)
      return hasDepth ? m_clearFlags == (clearType::TARGET | clearType::ZBUFFER) : (m_clearFlags & clearType::TARGET) != 0;
   else if (m_command == RC_COPY)
      return hasDepth ? m_copyColor && m_copyDepth : m_copyColor;
   else
      return false;
}

void RenderCommand::Execute()
{
   switch (m_command)
   {
   case RC_CLEAR:
   {
      #ifdef WIN_LOG
      OutputDebugString("> Clear\n");
      #endif
      m_renderState.Apply(m_rd);
      const D3DVALUE z = 1.0f;
      const DWORD stencil = 0L;
      #ifdef ENABLE_SDL
         // Default OpenGL Values
         static GLfloat clear_z = 1.f;
         static GLint clear_s = 0;
         static D3DCOLOR clear_color = 0;
         if (clear_s != stencil)
         {
            clear_s = stencil;
            glClearStencil(stencil);
         }
         if (clear_z != z)
         {
            clear_z = z;
            glClearDepthf(z);
         }
         if (clear_color != m_clearARGB)
         {
            clear_color = m_clearARGB;
            const float r = (float)(m_clearARGB & 0xff) / 255.0f;
            const float g = (float)((m_clearARGB & 0xff00) >> 8) / 255.0f;
            const float b = (float)((m_clearARGB & 0xff0000) >> 16) / 255.0f;
            const float a = (float)((m_clearARGB & 0xff000000) >> 24) / 255.0f;
            glClearColor(r, g, b, a);
         }
         glClear(m_clearFlags);
      #else
      CHECKD3D(m_rd->GetCoreDevice()->Clear(0, nullptr, m_clearFlags, m_clearARGB, z, stencil));
      #endif
      break;
   }

   case RC_COPY:
   {
      #ifdef WIN_LOG
      OutputDebugString("> Copy ");
      OutputDebugString(m_copyFrom->m_name.c_str());
      OutputDebugString(" => ");
      OutputDebugString(m_copyTo->m_name.c_str());
      OutputDebugString("\n");
      #endif

      // Original VPX code state that on DirectX 9 StretchRect must not be called between BeginScene/EndScene.
      // This does not seem to appear in Microsoft's docs and I could not find any glitch.
      #ifndef ENABLE_SDL
      //CHECKD3D(m_rd->GetCoreDevice()->EndScene());
      #endif
      m_copyFrom->CopyTo(m_copyTo, m_copyColor, m_copyDepth);
      #ifndef ENABLE_SDL
      //CHECKD3D(m_rd->GetCoreDevice()->BeginScene());
      #endif
      
      break;
   }

   case RC_DRAW_QUAD_PT:
   case RC_DRAW_QUAD_PNT:
   case RC_DRAW_MESH:
   {
      m_renderState.Apply(m_rd);
      m_shader->SetTechnique(m_shaderTechnique);
      m_shader->CopyUniformCache(false, m_shaderTechnique, m_uniformState, m_textureState);
      m_shader->Begin();
      m_rd->m_curDrawCalls++;
      switch (m_command)
      {
      case RC_DRAW_QUAD_PT:
      {
         #ifdef WIN_LOG
         OutputDebugString("> Draw Quad PT  ");
         #endif
         m_rd->m_stats_drawn_triangles += 2;
         #ifdef ENABLE_SDL
            void* bufvb;
            m_rd->m_quadPTDynMeshBuffer->m_vb->lock(0, 0, &bufvb, VertexBuffer::DISCARDCONTENTS);
            memcpy(bufvb, m_vertices, 4 * sizeof(Vertex3D_TexelOnly));
            m_rd->m_quadPTDynMeshBuffer->m_vb->unlock();
            m_rd->m_quadPTDynMeshBuffer->bind();
            glDrawArrays(RenderDevice::PrimitiveTypes::TRIANGLESTRIP, m_rd->m_quadPTDynMeshBuffer->m_vb->GetVertexOffset(), 4);
         #else
            // having a VB and lock/copying stuff each time is slower on DX9 :/ (is it still true ? looks overly complicated for a very marginal benefit)
            if (m_rd->m_currentVertexDeclaration != m_rd->m_pVertexTexelDeclaration)
            {
               CHECKD3D(m_rd->GetCoreDevice()->SetVertexDeclaration(m_rd->m_pVertexTexelDeclaration));
               m_rd->m_currentVertexDeclaration = m_rd->m_pVertexTexelDeclaration;
               m_rd->m_curStateChanges++;
            }
            CHECKD3D(m_rd->GetCoreDevice()->DrawPrimitiveUP((D3DPRIMITIVETYPE)RenderDevice::TRIANGLESTRIP, 2, m_vertices, sizeof(Vertex3D_TexelOnly)));
            m_rd->m_curVertexBuffer = nullptr; // DrawPrimitiveUP sets the VB to nullptr
         #endif
         break;
      }

      case RC_DRAW_QUAD_PNT:
      {
         #ifdef WIN_LOG
         OutputDebugString("> Draw Quad PNT ");
         #endif
         m_rd->m_stats_drawn_triangles += 2;
         #ifdef ENABLE_SDL
            void* bufvb;
            m_rd->m_quadPNTDynMeshBuffer->m_vb->lock(0, 0, &bufvb, VertexBuffer::DISCARDCONTENTS);
            memcpy(bufvb, m_vertices, 4 * sizeof(Vertex3D_NoTex2));
            m_rd->m_quadPNTDynMeshBuffer->m_vb->unlock();
            m_rd->m_quadPNTDynMeshBuffer->bind();
            glDrawArrays(RenderDevice::PrimitiveTypes::TRIANGLESTRIP, m_rd->m_quadPNTDynMeshBuffer->m_vb->GetVertexOffset(), 4);
         #else
            // having a VB and lock/copying stuff each time is slower on DX9 :/ (is it still true ? looks overly complicated for a very marginal benefit)
         if (m_rd->m_currentVertexDeclaration != m_rd->m_pVertexNormalTexelDeclaration)
            {
               CHECKD3D(m_rd->GetCoreDevice()->SetVertexDeclaration(m_rd->m_pVertexNormalTexelDeclaration));
               m_rd->m_currentVertexDeclaration = m_rd->m_pVertexNormalTexelDeclaration;
               m_rd->m_curStateChanges++;
            }
            CHECKD3D(m_rd->GetCoreDevice()->DrawPrimitiveUP((D3DPRIMITIVETYPE)RenderDevice::TRIANGLESTRIP, 2, m_vertices, sizeof(Vertex3D_NoTex2)));
            m_rd->m_curVertexBuffer = nullptr; // DrawPrimitiveUP sets the VB to nullptr
         #endif
         break;
      }

      case RC_DRAW_MESH:
      {
         #ifdef WIN_LOG
         OutputDebugString("> Draw Mesh     ");
         #endif

         unsigned int np; 
         switch (m_primitiveType)
         {
         case RenderDevice::POINTLIST: np = m_indicesCount; break;
         case RenderDevice::LINELIST: np = m_indicesCount / 2; break;
         case RenderDevice::LINESTRIP: np = std::max(0u, m_indicesCount - 1); break;
         case RenderDevice::TRIANGLELIST: np = m_indicesCount / 3; break;
         case RenderDevice::TRIANGLESTRIP:
         case RenderDevice::TRIANGLEFAN: np = std::max(0u, m_indicesCount - 2); break;
         default: assert(false);
         }
         m_rd->m_stats_drawn_triangles += np;

         // m_stats_drawn_triangles += np;

         m_mb->bind();

         if (m_mb->m_ib == nullptr)
         {
            #ifdef ENABLE_SDL
            glDrawArrays(m_primitiveType, m_mb->m_vb->GetVertexOffset(), m_indicesCount);
            #else
            CHECKD3D(m_rd->GetCoreDevice()->DrawPrimitive((D3DPRIMITIVETYPE)m_primitiveType, 0, np));
            #endif
         }
         else
         {
            #ifdef ENABLE_SDL
            const int indexOffset = m_mb->m_ib->GetOffset() + m_startIndice * m_mb->m_ib->m_sizePerIndex;
            const GLenum indexType = m_mb->m_ib->m_indexFormat == IndexBuffer::FMT_INDEX16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
            if (m_mb->m_isVBOffsetApplied || m_mb->m_vb->m_offset == 0)
            {
               //glDrawElements(type, indexCount, indexType, (void*)(intptr_t)indexOffset);
               glDrawRangeElements(m_primitiveType, 
                  m_mb->m_vb->GetVertexOffset(), m_mb->m_vb->GetVertexOffset() + m_mb->m_vb->m_vertexCount, 
                  m_indicesCount, indexType, (void*)(intptr_t)indexOffset);
            }
            else
            {
               #if defined(__OPENGLES__)
               assert(false); // OpenGL ES does not support offseted vertices. The buffers must be built accordingly
               #else
               //glDrawElementsBaseVertex(type, indexCount, indexType, (void*)(intptr_t)indexOffset, mb->m_vb->GetVertexOffset());
               glDrawRangeElementsBaseVertex(m_primitiveType, 
                  m_mb->m_vb->GetVertexOffset(), m_mb->m_vb->GetVertexOffset() + m_mb->m_vb->m_vertexCount, 
                  m_indicesCount, indexType, (void*)(intptr_t)indexOffset, 
                  m_mb->m_vb->GetVertexOffset());
               #endif
            }
            #else
            CHECKD3D(m_rd->GetCoreDevice()->DrawIndexedPrimitive((D3DPRIMITIVETYPE)m_primitiveType, 
               m_mb->m_isVBOffsetApplied ? 0 : m_mb->m_vb->GetVertexOffset(), 
               0, m_mb->m_vb->m_vertexCount, 
               m_mb->m_ib->GetIndexOffset() + m_startIndice, np));
            #endif
         }
         break;
      }
      }
      m_shader->End();

      #ifdef WIN_LOG
      std::stringstream ss;
      ss << std::setw(40) << Shader::GetTechniqueName(m_shaderTechnique) << std::setw(0) << " " << m_renderState.GetLog();
      if (m_command == RC_DRAW_MESH)
      {
         ss << " MB:" << std::setw(4) << std::hex << m_mb->GetSortKey() << std::dec << " IndCount: " << std::setw(8) << m_indicesCount << " " << m_mb->m_name;
      }
      ss << std::endl;
      OutputDebugString(ss.str().c_str());
      #endif
      break;
   }
   }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Default build from live render device state
//
//  Setup a render command from the live state. The aim would be to not use
//  this in the end (instead, prepare render command and lazily update them),
//  but this will help transitionning from the current implementation.

void RenderCommand::SetClear(DWORD clearFlags, DWORD clearARGB)
{
   m_command = Command::RC_CLEAR;
   m_clearFlags = clearFlags;
   m_clearARGB = clearARGB;
   m_rd->CopyRenderStates(true, m_renderState);
   if (clearFlags & clearType::TARGET)
      m_renderState.SetRenderState(RenderState::COLORWRITEENABLE, RenderState::RGBMASK_RGBA);
   if (clearFlags & clearType::ZBUFFER)
      m_renderState.SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_TRUE);
}

void RenderCommand::SetCopy(RenderTarget* from, RenderTarget* to, bool color, bool depth)
{
   m_command = Command::RC_COPY;
   m_copyFrom = from;
   m_copyTo = to;
   m_copyColor = color;
   m_copyDepth = depth;
}

void RenderCommand::SetDrawMesh(MeshBuffer* mb, const RenderDevice::PrimitiveTypes type, const DWORD startIndice, const DWORD indexCount)
{
   m_command = Command::RC_DRAW_MESH;
   m_mb = mb;
   m_primitiveType = type;
   m_startIndice = startIndice;
   m_indicesCount = indexCount;
   m_rd->CopyRenderStates(true, m_renderState);
   m_shader = Shader::GetCurrentShader();
   m_shaderTechnique = m_shader->GetCurrentTechnique();
   Shader::GetCurrentShader()->CopyUniformCache(true, m_shaderTechnique, m_uniformState, m_textureState);
}

void RenderCommand::SetDrawTexturedQuad(const Vertex3D_TexelOnly* vertices)
{
   m_command = Command::RC_DRAW_QUAD_PT;
   memcpy(m_vertices, vertices, 4 * sizeof(Vertex3D_TexelOnly));
   m_rd->CopyRenderStates(true, m_renderState);
   m_shader = Shader::GetCurrentShader();
   m_shaderTechnique = m_shader->GetCurrentTechnique();
   Shader::GetCurrentShader()->CopyUniformCache(true, m_shaderTechnique, m_uniformState, m_textureState);
}

void RenderCommand::SetDrawTexturedQuad(const Vertex3D_NoTex2* vertices)
{
   m_command = Command::RC_DRAW_QUAD_PNT;
   memcpy(m_vertices, vertices, 4 * sizeof(Vertex3D_NoTex2));
   m_rd->CopyRenderStates(true, m_renderState);
   m_shader = Shader::GetCurrentShader();
   m_shaderTechnique = m_shader->GetCurrentTechnique();
   Shader::GetCurrentShader()->CopyUniformCache(true, m_shaderTechnique, m_uniformState, m_textureState);
}

