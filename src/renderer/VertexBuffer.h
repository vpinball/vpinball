// license:GPLv3+

#pragma once

#include "core/stdafx.h"
#include "typedefs3D.h"
#include "IndexBuffer.h"


class SharedVertexBuffer;

class VertexBuffer final
{
public:
   VertexBuffer(RenderDevice* rd, const unsigned int vertexCount, const float* verts = nullptr, const bool isDynamic = false, const VertexFormat fmt = VertexFormat::VF_POS_NORMAL_TEX);
   ~VertexBuffer();

   // Position of buffer in a bigger shared data block
   unsigned int GetOffset() const { return m_offset; }
   unsigned int GetVertexOffset() const { return m_vertexOffset; }
   bool IsSharedBuffer() const;

   // Initial loading (before creation) or updating (for dynamic buffers only)
   template <typename T> void Lock(T*& data, const unsigned int offset = 0, const unsigned int size = 0) { LockUntyped((void*&) data, offset, size); }
   void Unlock();
   void Upload();

   RenderDevice* const m_rd;
   const bool m_isStatic; // True if this array is static and can not be modified after first upload
   const VertexFormat m_vertexFormat; // Vertex format (position of each attributes inside data stream)
   const unsigned int m_count; // Number of vertices
   const unsigned int m_sizePerVertex; // Size of each vertex
   const unsigned int m_size; // Size in bytes of the array

   #if defined(ENABLE_BGFX)
   bgfx::VertexBufferHandle GetStaticBuffer() const;
   bgfx::DynamicVertexBufferHandle GetDynamicBuffer() const;

   #elif defined(ENABLE_OPENGL)
   GLuint GetBuffer() const;
   void Bind() const;

   #elif defined(ENABLE_DX9)
   IDirect3DVertexBuffer9* GetBuffer() const;
   void Bind() const;
   #endif

private:
   unsigned int m_offset = 0; // Offset in bytes of the data inside the native GPU array
   unsigned int m_vertexOffset = 0; // Offset in vertices of the data inside the native GPU array
   SharedVertexBuffer* m_sharedBuffer = nullptr;
   void LockUntyped(void*& data, const unsigned int offset = 0, const unsigned int size = 0);
};
