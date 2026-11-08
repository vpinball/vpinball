// license:GPLv3+

#pragma once

#include "RenderDevice.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "RenderState.h"
#include "Shader.h"


// Snapshot of a UI command list's geometry, produced on the game thread while recording a frame and
// consumed on the render thread at submit time (BGFX transient buffers, or the dynamic MeshBuffer
// of the same command as a fallback when the transient arena lacks room for it)
struct UIListGeometry
{
   vector<uint8_t> vtx; // numVertices * sizeof(ImDrawVert)
   vector<uint8_t> idx; // numIndices * sizeof(ImDrawIdx)
   uint32_t numVertices = 0;
   uint32_t numIndices = 0;
   bool uploaded = false; // Set once the snapshot has been pushed to the fallback dynamic buffers
};


class RenderCommand final
{
public:
   RenderCommand(RenderDevice* const m_rd);
   ~RenderCommand();

   void Clear();

   bool IsFullClear(const bool hasDepth) const;
   bool IsTransparent() const { return m_isTransparent; }
   bool IsDrawCommand() const { return m_command == RC_DRAW_MESH || m_command == RC_DRAW_TRANSIENT || m_command == RC_DRAW_QUAD_PT || m_command == RC_DRAW_QUAD_PNT; }
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
   void SetDrawMesh(Shader* shader, std::shared_ptr<MeshBuffer> mb, const RenderDevice::PrimitiveTypes type, const uint32_t startIndex, const uint32_t indexCount, const bool isTransparent, const float depth);
#if defined(ENABLE_BGFX)
   void SetDrawTransientMesh(Shader* shader, std::shared_ptr<UIListGeometry> geo, std::shared_ptr<MeshBuffer> mb, const RenderDevice::PrimitiveTypes type, const uint32_t startIndex,
      const uint32_t indexCount, const bool isTransparent, const float depth);
#endif
   void SetDrawTexturedQuad(Shader* shader, const Vertex3D_TexelOnly* vertices, const bool isTransparent = false, const float depth = 0.f);
   void SetDrawTexturedQuad(Shader* shader, const Vertex3D_NoTex2* vertices, const bool isTransparent = false, const float depth = 0.f);

   string ToString(bool detailled) const;

   RenderPass* m_dependency = nullptr;

private:
   enum Command
   {
      RC_CLEAR,
      RC_COPY,
      RC_DRAW_MESH,
      RC_DRAW_TRANSIENT,
      RC_DRAW_QUAD_PT,
      RC_DRAW_QUAD_PNT,
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

// For RC_DRAW_TRANSIENT (LiveUI uploads: transient arena when it has room, m_mb as fallback)
#if defined(ENABLE_BGFX)
   std::shared_ptr<UIListGeometry> m_uiGeo;
#endif
};
