#pragma once

#include "stdafx.h"
#include "typedefs3D.h"

class VertexBuffer;
class IndexBuffer;

class MeshBuffer final
{
public:
   MeshBuffer(VertexBuffer* vb, IndexBuffer* ib = nullptr, const bool applyVertexBufferOffsetToIndexBuffer = false);
   ~MeshBuffer();
   void bind();
   unsigned int GetSortKey() const;

   VertexBuffer* const m_vb;
   IndexBuffer* const m_ib;
   const bool m_isVBOffsetApplied; // True if vertex buffer offset is already applied to index buffer

#if defined(ENABLE_SDL) // OpenGL
private:
   GLuint m_vao = 0;
   struct SharedVAO
   {
      GLuint vb, ib, vao, ref_count;
   };
   SharedVAO* m_sharedVAO = nullptr;
   static vector<SharedVAO*> sharedVAOs;
#else // DirectX 9
   IDirect3DVertexDeclaration9* const m_vertexDeclaration;
#endif
};
