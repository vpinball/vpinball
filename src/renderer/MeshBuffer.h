// license:GPLv3+

#pragma once

#include "typedefs3D.h"

class VertexBuffer;
class IndexBuffer;

class MeshBuffer final
{
public:
   MeshBuffer(const wstring& name, std::shared_ptr<VertexBuffer> vb, IndexBuffer* ib = nullptr, const bool applyVertexBufferOffsetToIndexBuffer = false);
   MeshBuffer(std::shared_ptr<VertexBuffer> vb, IndexBuffer* ib = nullptr, const bool applyVertexBufferOffsetToIndexBuffer = false);
   ~MeshBuffer();
   void bind();
   unsigned int GetSortKey() const;

   std::unique_ptr<MeshBuffer> CreateEdgeMeshBuffer(const vector<unsigned int>& indices) const;
   std::unique_ptr<MeshBuffer> CreateEdgeMeshBuffer(const vector<unsigned int>& indices, const vector<Vertex3D_NoTex2>& vertices) const;
   std::unique_ptr<MeshBuffer> CreateSharedVertexMeshBuffer(IndexBuffer* ib) const;

   const wstring m_wname;
   const std::shared_ptr<VertexBuffer> m_vb;
   IndexBuffer* const m_ib;
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

private:
   bool m_sharedVB = false;
};
