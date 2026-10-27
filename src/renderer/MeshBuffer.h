// license:GPLv3+

#pragma once

#include "typedefs3D.h"

class VertexBuffer;
class IndexBuffer;

class MeshBuffer final
{
public:
   MeshBuffer(const string& name, std::shared_ptr<VertexBuffer> vb, std::shared_ptr<IndexBuffer> ib = nullptr, const bool applyVertexBufferOffsetToIndexBuffer = false);
   MeshBuffer(std::shared_ptr<VertexBuffer> vb, std::shared_ptr<IndexBuffer> ib = nullptr, const bool applyVertexBufferOffsetToIndexBuffer = false);
   ~MeshBuffer();
   void bind();
   unsigned int GetSortKey() const;

   std::unique_ptr<MeshBuffer> CreateEdgeMeshBuffer(const vector<unsigned int>& indices) const;
   std::unique_ptr<MeshBuffer> CreateEdgeMeshBuffer(const vector<unsigned int>& indices, const vector<Vertex3D_NoTex2>& vertices) const;
   // Expands each triangle edge as a line (face diagonals included)
   std::unique_ptr<MeshBuffer> CreateEdgeMeshBuffer(const unsigned int* indices, size_t numIndices) const;
   std::unique_ptr<MeshBuffer> CreateEdgeMeshBuffer(const WORD* indices, size_t numIndices) const;
   // Expands only silhouette and crease edges (edges shared by coplanar triangles are dropped)
   std::unique_ptr<MeshBuffer> CreateEdgeMeshBuffer(const unsigned int* indices, size_t numIndices, const Vertex3D_NoTex2* vertices) const;
   std::unique_ptr<MeshBuffer> CreateEdgeMeshBuffer(const WORD* indices, size_t numIndices, const Vertex3D_NoTex2* vertices) const;
   // Expands each consecutive pair of triangles as a quad outline (the shared diagonal is dropped, no cross-quad merging)
   std::unique_ptr<MeshBuffer> CreateQuadOutlineEdgeMeshBuffer(const unsigned int* indices, size_t numIndices) const;
   std::unique_ptr<MeshBuffer> CreateQuadOutlineEdgeMeshBuffer(const WORD* indices, size_t numIndices) const;
   // Expands a ring through consecutive vertices (i, i+1) wrapping around, derived from the vertex buffer size
   std::unique_ptr<MeshBuffer> CreateVertexRingEdgeMeshBuffer() const;

   const string m_name;
   const std::shared_ptr<VertexBuffer> m_vb;
   const std::shared_ptr<IndexBuffer> m_ib;
   const bool m_isVBOffsetApplied; // True if vertex buffer offset is already applied to index buffer

#if defined(ENABLE_OPENGL)
   struct SharedVAO
   {
      GLuint vb, ib, vao, ref_count;
   };
private:
   GLuint m_vao = 0;
   SharedVAO* m_sharedVAO = nullptr;
#endif
};
