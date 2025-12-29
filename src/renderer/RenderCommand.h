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

   void Clear();

   bool IsFullClear(const bool hasDepth) const;
   bool IsTransparent() const { return m_isTransparent; }
   bool IsDrawCommand() const { return m_command == RC_DRAW_MESH || m_command == RC_DRAW_QUAD_PT || m_command == RC_DRAW_QUAD_PNT; }
   bool IsDrawMeshCommand() const { return m_command == RC_DRAW_MESH; }
   RenderState GetRenderState() const { return m_renderState; }
   ShaderState* GetShaderState() const { return m_shaderState; }
   void* GetQuadVertices() { return m_vertices; }
   std::shared_ptr<MeshBuffer> GetMeshBuffer() const { return m_mb; }
   float GetDepth() const { return m_depth; }
   void SetTransparent(bool t) { m_isTransparent = t; }
   void SetDepth(float d) { m_depth = d; }

   void Execute(const int nInstances, const bool log);

   // Build from render device live state
   void SetClear(DWORD clearFlags, DWORD clearARGB);
   void SetCopy(RenderTarget* from, RenderTarget* to, bool color, bool depth,
                const int x1 = -1, const int y1 = -1, const int w1 = -1, const int h1 = -1,
                const int x2 = -1, const int y2 = -1, const int w2 = -1, const int h2 = -1, const int srcLayer = -1, const int dstLayer = -1);
   void SetSubmitVR(RenderTarget* from);
   void SetDrawMesh(Shader* shader, std::shared_ptr<MeshBuffer> mb, const RenderDevice::PrimitiveTypes type, const uint32_t startIndex, const uint32_t indexCount, const bool isTransparent, const float depth);
   void SetDrawTexturedQuad(Shader* shader, const Vertex3D_TexelOnly* vertices, const bool isTransparent = false, const float depth = 0.f);
   void SetDrawTexturedQuad(Shader* shader, const Vertex3D_NoTex2* vertices, const bool isTransparent = false, const float depth = 0.f);

   RenderPass* m_dependency;

private:
   enum Command
   {
      RC_CLEAR,
      RC_COPY,
      RC_DRAW_MESH,
      RC_DRAW_QUAD_PT,
      RC_DRAW_QUAD_PNT,
      RC_SUBMIT_VR
   };

   RenderDevice* const m_rd;

   Command m_command;
   Shader* m_shader = nullptr;
   ShaderState* m_shaderState = nullptr;
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
   uint8_t m_vertices[4 * sizeof(Vertex3D_NoTex2)]; 

   // For RC_DRAW_MESH
   std::shared_ptr<MeshBuffer> m_mb;
   RenderDevice::PrimitiveTypes m_primitiveType;
   unsigned int m_indicesCount = 0;
   unsigned int m_startIndex = 0;
   float m_depth = 0.f;
};
