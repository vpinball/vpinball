// license:GPLv3+

#pragma once

#include "RenderDevice.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "RenderState.h"
#include "Shader.h"


class RenderCommand final
{
public:
   RenderCommand(RenderDevice* const m_rd);
   ~RenderCommand();

   bool IsFullClear(const bool hasDepth) const;
   bool IsTransparent() const { return m_isTransparent; }
   bool IsDrawCommand() const { return m_command == RC_DRAW_MESH || m_command == RC_DRAW_QUAD_PT || m_command == RC_DRAW_QUAD_PNT || m_command == RC_DRAW_LIVEUI; }
   bool IsDrawMeshCommand() const { return m_command == RC_DRAW_MESH; }
   bool IsDrawLiveUICommand() const { return m_command == RC_DRAW_LIVEUI; }
   inline RenderState GetRenderState() const { return m_renderState; }
   inline Shader::ShaderState* GetShaderState() const { return m_shaderState; }
   inline ShaderTechniques GetShaderTechnique() const { return m_shaderTechnique; }
   inline MeshBuffer* GetMeshBuffer() const { return m_mb; }
   inline float GetDepth() const { return m_depth; }
   inline void SetTransparent(bool t) { m_isTransparent = t; }
   inline void SetDepth(float d) { m_depth = d; }

   void Execute(const int nInstances, const bool log);

   // Build from render device live state
   void SetClear(DWORD clearFlags, DWORD clearARGB);
   void SetCopy(RenderTarget* from, RenderTarget* to, bool color, bool depth,  
      const int x1 = -1, const int y1 = -1, const int w1 = -1, const int h1 = -1,
      const int x2 = -1, const int y2 = -1, const int w2 = -1, const int h2 = -1, const int srcLayer = -1, const int dstLayer = -1);
   void SetSubmitVR(RenderTarget* from);
   void SetRenderLiveUI();
   void SetDrawMesh(Shader* shader, MeshBuffer* mb, const RenderDevice::PrimitiveTypes type, const DWORD startIndex, const DWORD indexCount, const bool isTransparent, const float depth);
   void SetDrawTexturedQuad(Shader* shader, const Vertex3D_TexelOnly* vertices);
   void SetDrawTexturedQuad(Shader* shader, const Vertex3D_NoTex2* vertices);

   RenderPass* m_dependency;

private:
   enum Command
   {
      RC_CLEAR,
      RC_COPY,
      RC_DRAW_MESH,
      RC_DRAW_QUAD_PT,
      RC_DRAW_QUAD_PNT,
      RC_DRAW_LIVEUI,
      RC_SUBMIT_VR
   };

   RenderDevice* const m_rd;

   Command m_command;
   Shader* m_shader = nullptr;
   ShaderTechniques m_shaderTechnique = ShaderTechniques::SHADER_TECHNIQUE_INVALID;
   Shader::ShaderState* m_shaderState = nullptr;
   RenderState m_renderState;
   bool m_isTransparent;

   // For RC_CLEAR
   DWORD m_clearARGB = 0;
   DWORD m_clearFlags = 0;

   // For RC_COPY
   RenderTarget* m_copyFrom = nullptr;
   RenderTarget* m_copyTo = nullptr;
   bool m_copyColor = true;
   bool m_copyDepth = true;
   vec4 m_copySrcRect;
   vec4 m_copyDstRect;
   int m_copySrcLayer;
   int m_copyDstLayer;

   // For RC_DRAW_QUAD_PT / RC_DRAW_QUAD_PNT
   BYTE m_vertices[4 * sizeof(Vertex3D_NoTex2)]; 

   // For RC_DRAW_MESH
   MeshBuffer* m_mb = nullptr;
   RenderDevice::PrimitiveTypes m_primitiveType;
   unsigned int m_indicesCount = 0;
   unsigned int m_startIndex = 0;
   float m_depth = 0.f;
};
