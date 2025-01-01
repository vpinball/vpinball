// license:GPLv3+

#include "core/stdafx.h"
#include "RenderCommand.h"
#include "VRDevice.h"

RenderCommand::RenderCommand(RenderDevice* rd)
   : m_rd(rd)
{
}

RenderCommand::~RenderCommand()
{
   delete m_shaderState;
}

bool RenderCommand::IsFullClear(const bool hasDepth) const
{
   if (m_command == RC_CLEAR)
      return hasDepth ? m_clearFlags == (clearType::TARGET | clearType::ZBUFFER) : (m_clearFlags & clearType::TARGET) != 0;
   //else if (m_command == RC_COPY)
   //   return hasDepth ? m_copyColor && m_copyDepth : m_copyColor;
   else
      return false;
}

void RenderCommand::Execute(const int nInstances, const bool log)
{
   switch (m_command)
   {
   case RC_CLEAR:
   {
      if (log) {
         PLOGI << "> Clear";
      }
      m_renderState.Apply(m_rd);
      constexpr D3DVALUE z = 1.0f;
      constexpr DWORD stencil = 0L;

      #if defined(ENABLE_BGFX)
      const uint32_t r = (m_clearARGB & 0x000000ff);
      const uint32_t g = (m_clearARGB & 0x0000ff00) >> 8;
      const uint32_t b = (m_clearARGB & 0x00ff0000) >> 16;
      const uint32_t a = (m_clearARGB & 0xff000000) >> 24;
      const uint32_t rgba = (r << 24) | (g << 16) | (b << 8) | a;
      bgfx::setViewClear(m_rd->m_activeViewId, (uint16_t) m_clearFlags, rgba);
      bgfx::touch(m_rd->m_activeViewId);

      #elif defined(ENABLE_OPENGL)
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
         const float b = (float)(m_clearARGB & 0xff) / 255.0f;
         const float g = (float)((m_clearARGB & 0xff00) >> 8) / 255.0f;
         const float r = (float)((m_clearARGB & 0xff0000) >> 16) / 255.0f;
         const float a = (float)((m_clearARGB & 0xff000000) >> 24) / 255.0f;
         glClearColor(r, g, b, a);
      }
      glClear(m_clearFlags);

      #elif defined(ENABLE_DX9)
      CHECKD3D(m_rd->GetCoreDevice()->Clear(0, nullptr, m_clearFlags, m_clearARGB, z, stencil));

      #endif
      break;
   }

   case RC_COPY:
   {
      if (log) {
         PLOGI << "> Copy " << m_copyFrom->m_name << " => " << m_copyTo->m_name;
      }

      // Original VPX code state that on DirectX 9 StretchRect must not be called between BeginScene/EndScene.
      // This does not seem to appear in Microsoft's docs and I could not find any glitch.
      #if defined(ENABLE_DX9)
      //CHECKD3D(m_rd->GetCoreDevice()->EndScene());
      #endif

      m_copyFrom->CopyTo(m_copyTo, m_copyColor, m_copyDepth,
         (int) m_copySrcRect.x, (int) m_copySrcRect.y, (int) m_copySrcRect.z, (int) m_copySrcRect.w, 
         (int) m_copyDstRect.x, (int) m_copyDstRect.y, (int) m_copyDstRect.z, (int) m_copyDstRect.w, m_copySrcLayer);

      #if defined(ENABLE_DX9)
      //CHECKD3D(m_rd->GetCoreDevice()->BeginScene());
      #endif
      break;
   }

   case RC_SUBMIT_VR:
   {
      if (log) {
         PLOGI << "> Submit VR";
      }
      #if defined(ENABLE_VR)
         if (g_pplayer->m_vrDevice && g_pplayer->m_vrDevice->IsVRReady())
         {
            g_pplayer->m_logicProfiler.EnterProfileSection(FrameProfiler::PROFILE_RENDER_FLIP); 
            g_pplayer->m_vrDevice->SubmitFrame(g_pplayer->m_renderer->GetOffscreenVR(0)->GetColorSampler(), g_pplayer->m_renderer->GetOffscreenVR(1)->GetColorSampler());
            g_pplayer->m_logicProfiler.OnPresented(usec());
            g_pplayer->m_logicProfiler.ExitProfileSection();
         }
      #endif
      break;
   }

   case RC_DRAW_LIVEUI:
   {
      if (g_pplayer->m_liveUI)
      {
         if (log) {
            PLOGI << "> Draw LiveUI";
         }
         g_pplayer->m_liveUI->Render();
      }
      break;
   }

   case RC_DRAW_QUAD_PT:
   case RC_DRAW_QUAD_PNT:
   case RC_DRAW_MESH:
   {
      m_renderState.Apply(m_rd);
      m_shader->SetTechnique(m_shaderTechnique);
      m_shaderState->SetInt(SHADER_layer, RenderTarget::GetCurrentRenderLayer() < 0 ? 0 : RenderTarget::GetCurrentRenderLayer());
      m_shader->m_state->CopyTo(false, m_shaderState, m_shaderTechnique);
      m_shader->Begin();
      m_rd->m_curDrawCalls++;
      switch (m_command)
      {
      case RC_DRAW_QUAD_PT:
      {
         m_rd->m_curDrawnTriangles += 2;
         
         #if defined(ENABLE_BGFX)
         bgfx::TransientVertexBuffer tvb; // TODO only allocate one per frame instead of one per render command
         bgfx::allocTransientVertexBuffer(&tvb, 4, *m_rd->m_pVertexTexelDeclaration);
         memcpy(tvb.data, m_vertices, 4 * sizeof(Vertex3D_TexelOnly));
         bgfx::setVertexBuffer(0, &tvb);
         bgfx::setInstanceCount(nInstances);
         bgfx::setState(m_rd->m_bgfxState | BGFX_STATE_PT_TRISTRIP);
         bgfx::submit(m_rd->m_activeViewId, m_shader->GetCore());         

         #elif defined(ENABLE_OPENGL)
         void* bufvb;
         m_rd->m_quadPTDynMeshBuffer->m_vb->Lock(bufvb);
         memcpy(bufvb, m_vertices, 4 * sizeof(Vertex3D_TexelOnly));
         m_rd->m_quadPTDynMeshBuffer->m_vb->Unlock();
         m_rd->m_quadPTDynMeshBuffer->bind();
         if (nInstances > 1)
            glDrawArraysInstanced(RenderDevice::PrimitiveTypes::TRIANGLESTRIP, m_rd->m_quadPTDynMeshBuffer->m_vb->GetVertexOffset(), 4, nInstances);
         else
            glDrawArrays(RenderDevice::PrimitiveTypes::TRIANGLESTRIP, m_rd->m_quadPTDynMeshBuffer->m_vb->GetVertexOffset(), 4);

         #elif defined(ENABLE_DX9)
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
         m_rd->m_curDrawnTriangles += 2;

         #if defined(ENABLE_BGFX)
         bgfx::TransientVertexBuffer tvb; // TODO only allocate one per frame instead of one per render command
         bgfx::allocTransientVertexBuffer(&tvb, 4, *m_rd->m_pVertexNormalTexelDeclaration);
         memcpy(tvb.data, m_vertices, 4 * sizeof(Vertex3D_NoTex2));
         bgfx::setVertexBuffer(0, &tvb);
         bgfx::setInstanceCount(nInstances);
         bgfx::setState(m_rd->m_bgfxState | BGFX_STATE_PT_TRISTRIP);
         bgfx::submit(m_rd->m_activeViewId, m_shader->GetCore());

         #elif defined(ENABLE_OPENGL)
         void* bufvb;
         m_rd->m_quadPNTDynMeshBuffer->m_vb->Lock(bufvb);
         memcpy(bufvb, m_vertices, 4 * sizeof(Vertex3D_NoTex2));
         m_rd->m_quadPNTDynMeshBuffer->m_vb->Unlock();
         m_rd->m_quadPNTDynMeshBuffer->bind();
         if (nInstances > 1)
            glDrawArraysInstanced(RenderDevice::PrimitiveTypes::TRIANGLESTRIP, m_rd->m_quadPNTDynMeshBuffer->m_vb->GetVertexOffset(), 4, nInstances);
         else
            glDrawArrays(RenderDevice::PrimitiveTypes::TRIANGLESTRIP, m_rd->m_quadPNTDynMeshBuffer->m_vb->GetVertexOffset(), 4);

         #elif defined(ENABLE_DX9)
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
         m_rd->m_curDrawnTriangles += np;

         m_mb->bind();

         if (m_mb->m_ib == nullptr)
         {
            #if defined(ENABLE_BGFX)
            if (m_mb->m_vb->m_isStatic)
               bgfx::setVertexBuffer(0, m_mb->m_vb->GetStaticBuffer(), m_mb->m_vb->GetVertexOffset(), m_indicesCount);
            else
               bgfx::setVertexBuffer(0, m_mb->m_vb->GetDynamicBuffer(), m_mb->m_vb->GetVertexOffset(), m_indicesCount);
            bgfx::setInstanceCount(nInstances);
            if (m_primitiveType == RenderDevice::TRIANGLELIST)
               bgfx::setState(m_rd->m_bgfxState);
            else if (m_primitiveType == RenderDevice::TRIANGLESTRIP)
               bgfx::setState(m_rd->m_bgfxState | BGFX_STATE_PT_TRISTRIP);
            else if (m_primitiveType == RenderDevice::POINTLIST)
               bgfx::setState(m_rd->m_bgfxState | BGFX_STATE_PT_POINTS);
            else
               assert(false); // Unsupported primitive type
            bgfx::submit(m_rd->m_activeViewId, m_shader->GetCore());

            #elif defined(ENABLE_OPENGL)
            //assert(0 <= m_mb->m_vb->GetVertexOffset() && m_mb->m_vb->GetVertexOffset() + m_indicesCount <= m_mb->m_vb->GetSharedBuffer()->GetCount());
            if (nInstances > 1)
               glDrawArraysInstanced(m_primitiveType, m_mb->m_vb->GetVertexOffset() + m_startIndex, m_indicesCount, nInstances);
            else
               glDrawArrays(m_primitiveType, m_mb->m_vb->GetVertexOffset() + m_startIndex, m_indicesCount);

            #elif defined(ENABLE_DX9)
            CHECKD3D(m_rd->GetCoreDevice()->DrawPrimitive((D3DPRIMITIVETYPE)m_primitiveType, m_mb->m_vb->GetVertexOffset() + m_startIndex, np));
            #endif
         }
         else
         {
            const int vertexOffset = m_mb->m_isVBOffsetApplied ? 0 : m_mb->m_vb->GetOffset();

            #if defined(ENABLE_BGFX)
            assert(vertexOffset == 0); // BGFX does not support offseted vertices. The buffers must be built accordingly
            if (m_mb->m_vb->m_isStatic)
               bgfx::setVertexBuffer(0, m_mb->m_vb->GetStaticBuffer());
            else
               bgfx::setVertexBuffer(0, m_mb->m_vb->GetDynamicBuffer());
            if (m_mb->m_ib->m_isStatic)
               bgfx::setIndexBuffer(m_mb->m_ib->GetStaticBuffer(), m_mb->m_ib->GetIndexOffset() + m_startIndex, m_indicesCount);
            else
               bgfx::setIndexBuffer(m_mb->m_ib->GetDynamicBuffer(), m_mb->m_ib->GetIndexOffset() + m_startIndex, m_indicesCount);
            bgfx::setInstanceCount(nInstances);
            if (m_primitiveType == RenderDevice::TRIANGLELIST)
               bgfx::setState(m_rd->m_bgfxState);
            else if (m_primitiveType == RenderDevice::TRIANGLESTRIP)
               bgfx::setState(m_rd->m_bgfxState | BGFX_STATE_PT_TRISTRIP);
            else
               assert(false); // Unsupported primitive type
            bgfx::submit(m_rd->m_activeViewId, m_shader->GetCore());

            #elif defined(ENABLE_OPENGL)
            const int indexOffset = m_mb->m_ib->GetOffset() + m_startIndex * m_mb->m_ib->m_sizePerIndex;
            const GLenum indexType = m_mb->m_ib->m_indexFormat == IndexBuffer::FMT_INDEX16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
            #if defined(DEBUG) && 0
            // Track invalid vertex memory reference. Very slow, only for debugging memory access exception in OpenGL
            BYTE* tmp = new BYTE[m_indicesCount * m_mb->m_ib->m_sizePerIndex];
            U16* tmp16 = (U16*)tmp;
            U32* tmp32 = (U32*)tmp;
            glGetNamedBufferSubData(m_mb->m_ib->GetBuffer(), indexOffset, m_indicesCount * m_mb->m_ib->m_sizePerIndex, tmp);
            //assert(m_mb->m_vb->GetVertexOffset() + m_mb->m_vb->m_count <= m_mb->m_vb->GetSharedBuffer()->GetCount());
            for (unsigned int i = 0; i < m_indicesCount; i++)
            {
               unsigned int idx = m_mb->m_ib->m_indexFormat == IndexBuffer::FMT_INDEX16 ? tmp16[i] : tmp32[i];
               assert((m_mb->m_vb->GetVertexOffset() <= vertexOffset + idx) && (vertexOffset + idx <= m_mb->m_vb->GetVertexOffset() + m_mb->m_vb->m_count));
            }
            delete[] tmp;
            #endif
            if (vertexOffset == 0)
            {
               if (nInstances > 1)
                  glDrawElementsInstanced(m_primitiveType, m_indicesCount, indexType, (void*)(intptr_t)indexOffset, nInstances);
               else
                  glDrawRangeElements(m_primitiveType, 
                     m_mb->m_vb->GetVertexOffset(), m_mb->m_vb->GetVertexOffset() + m_mb->m_vb->m_count, 
                     m_indicesCount, indexType, (void*)(intptr_t)indexOffset);
            }
            else
            {
               #if defined(__OPENGLES__)
               assert(false); // OpenGL ES does not support offseted vertices. The buffers must be built accordingly
               #else
               if (nInstances > 1)
                  glDrawElementsInstancedBaseVertex(m_primitiveType, m_indicesCount, indexType, (void*)(intptr_t)indexOffset, vertexOffset, nInstances);
               else
                  glDrawRangeElementsBaseVertex(m_primitiveType, 
                     m_mb->m_vb->GetVertexOffset(), m_mb->m_vb->GetVertexOffset() + m_mb->m_vb->m_count, 
                     m_indicesCount, indexType, (void*)(intptr_t)indexOffset, vertexOffset);
               #endif
            }

            #elif defined(ENABLE_DX9)
            CHECKD3D(m_rd->GetCoreDevice()->DrawIndexedPrimitive((D3DPRIMITIVETYPE)m_primitiveType, 
               vertexOffset, 0, m_mb->m_vb->m_count, m_mb->m_ib->GetIndexOffset() + m_startIndex, np));
            #endif
         }
         break;
      }
      default: break;
      }
      m_shader->End();

      if (log)
      {
         std::stringstream ss;
         if (m_command == RC_DRAW_QUAD_PT)
            ss << "> Draw Quad PT  ";
         else if (m_command == RC_DRAW_QUAD_PNT)
            ss << "> Draw Quad PNT ";
         else if (m_command == RC_DRAW_MESH)
            ss << "> Draw Mesh     ";
         ss << (m_isTransparent ? "T "s : "O "s);
         ss << std::setw(40) << Shader::GetTechniqueName(m_shaderTechnique) << std::setw(0) << " " << m_renderState.GetLog();
         ss << " Depth: " << std::fixed << std::setw(8) << std::setprecision(2) << m_depth;
         if (m_command == RC_DRAW_MESH)
         {
            ss << " MB:" << std::setw(4) << std::hex << m_mb->GetSortKey() << std::dec;
            ss << " IndCount: " << std::setw(8) << m_indicesCount << " " << m_mb->m_name;
         }
         PLOGI << ss.str();
      }
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

void RenderCommand::SetCopy(RenderTarget* from, RenderTarget* to, bool color, bool depth, const int x1, const int y1, const int w1, const int h1, const int x2, const int y2, const int w2,
   const int h2, const int srcLayer, const int dstLayer)
{
   m_command = Command::RC_COPY;
   m_copyFrom = from;
   m_copyTo = to;
   m_copyColor = color;
   m_copyDepth = depth;
   m_copySrcRect = vec4((const float)x1, (const float)y1, (const float)w1, (const float)h1);
   m_copyDstRect = vec4((const float)x2, (const float)y2, (const float)w2, (const float)h2);
   m_copySrcLayer = srcLayer;
   m_copyDstLayer = dstLayer;
}

void RenderCommand::SetSubmitVR(RenderTarget* from)
{
   m_command = Command::RC_SUBMIT_VR;
   m_copyFrom = from;
}

void RenderCommand::SetRenderLiveUI()
{
   m_command = Command::RC_DRAW_LIVEUI;
}

void RenderCommand::SetDrawMesh(
   Shader* shader, MeshBuffer* mb, const RenderDevice::PrimitiveTypes type, 
   const DWORD startIndex, const DWORD indexCount, const bool isTransparent, const float depth)
{
   assert(mb != nullptr);
   m_command = Command::RC_DRAW_MESH;
   m_mb = mb;
   m_primitiveType = type;
   m_startIndex = startIndex;
   m_indicesCount = indexCount;
   m_rd->CopyRenderStates(true, m_renderState);
   m_depth = depth;
   m_isTransparent = isTransparent;
   m_shader = shader;
   m_shaderTechnique = m_shader->GetCurrentTechnique();
   assert(m_shaderTechnique < SHADER_TECHNIQUE_INVALID);
   if (m_shaderState == nullptr || m_shader->GetStateSize() > m_shaderState->m_stateSize)
   {
      delete m_shaderState;
      m_shaderState = new Shader::ShaderState(m_shader, m_rd->UseLowPrecision());
   }
   else
      m_shaderState->Reset(m_shader);
   m_shader->m_state->CopyTo(true, m_shaderState, m_shaderTechnique);
}

void RenderCommand::SetDrawTexturedQuad(Shader* shader, const Vertex3D_TexelOnly* vertices, const bool isTransparent, const float depth)
{
   m_command = Command::RC_DRAW_QUAD_PT;
   memcpy(m_vertices, vertices, 4 * sizeof(Vertex3D_TexelOnly));
   m_rd->CopyRenderStates(true, m_renderState);
   m_depth = depth;
   m_isTransparent = isTransparent;
   m_shader = shader;
   m_shaderTechnique = m_shader->GetCurrentTechnique();
   if (m_shaderState == nullptr || m_shader->GetStateSize() > m_shaderState->m_stateSize)
   {
      delete m_shaderState;
      m_shaderState = new Shader::ShaderState(m_shader, m_rd->UseLowPrecision());
   }
   else
      m_shaderState->Reset(m_shader);
   m_shader->m_state->CopyTo(true, m_shaderState, m_shaderTechnique);
}

void RenderCommand::SetDrawTexturedQuad(Shader* shader, const Vertex3D_NoTex2* vertices, const bool isTransparent, const float depth)
{
   m_command = Command::RC_DRAW_QUAD_PNT;
   memcpy(m_vertices, vertices, 4 * sizeof(Vertex3D_NoTex2));
   m_rd->CopyRenderStates(true, m_renderState);
   m_depth = depth;
   m_isTransparent = isTransparent;
   m_shader = shader;
   m_shaderTechnique = m_shader->GetCurrentTechnique();
   if (m_shaderState == nullptr || m_shader->GetStateSize() > m_shaderState->m_stateSize)
   {
      delete m_shaderState;
      m_shaderState = new Shader::ShaderState(m_shader, m_rd->UseLowPrecision());
   }
   else
      m_shaderState->Reset(m_shader);
   m_shader->m_state->CopyTo(true, m_shaderState, m_shaderTechnique);
}
