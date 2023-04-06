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
   bool IsDrawCommand() const { return m_command != RC_CLEAR && m_command != RC_COPY; }
   inline RenderState GetRenderState() const { return m_renderState; }
   inline ShaderTechniques GetShaderTechnique() const { return m_shaderTechnique; }
   inline MeshBuffer* GetMeshBuffer() const { return m_mb; }

   void Execute();

   // Build from render device live state
   void SetClear(DWORD clearFlags, DWORD clearARGB);
   void SetCopy(RenderTarget* from, RenderTarget* to, bool color, bool depth);
   void SetDrawMesh(MeshBuffer* mb, const RenderDevice::PrimitiveTypes type, const DWORD startIndice, const DWORD indexCount);
   void SetDrawTexturedQuad(const Vertex3D_TexelOnly* vertices);
   void SetDrawTexturedQuad(const Vertex3D_NoTex2* vertices);

private:
   enum Command
   {
      RC_CLEAR,
      RC_COPY,
      RC_DRAW_MESH,
      RC_DRAW_QUAD_PT,
      RC_DRAW_QUAD_PNT,
   };

   RenderDevice* const m_rd;

   Command m_command;
   Shader* m_shader = nullptr;
   ShaderTechniques m_shaderTechnique = ShaderTechniques::SHADER_TECHNIQUE_INVALID;
   Shader::UniformCache m_uniformState[SHADER_UNIFORM_COUNT];
   Sampler* m_textureState[Shader::TEXTURESET_STATE_CACHE_SIZE];
   RenderState m_renderState;

   // For RC_CLEAR
   DWORD m_clearARGB = 0;
   DWORD m_clearFlags = 0;

   // For RC_COPY
   RenderTarget* m_copyFrom = nullptr;
   RenderTarget* m_copyTo = nullptr;
   bool m_copyColor = true;
   bool m_copyDepth = true;

   // For RC_DRAW_QUAD_PT / RC_DRAW_QUAD_PNT
   BYTE m_vertices[4 * sizeof(Vertex3D_NoTex2)]; 

   // For RC_DRAW_MESH
   MeshBuffer* m_mb = nullptr;
   RenderDevice::PrimitiveTypes m_primitiveType;
   unsigned int m_indicesCount = 0;
   unsigned int m_startIndice = 0;
};
