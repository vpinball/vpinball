#pragma once

#include "stdafx.h"
#include "typedefs3D.h"
#include "IndexBuffer.h"


class SharedVertexBuffer;

class VertexBuffer final
{
public:
   enum LockFlags
   {
#ifdef ENABLE_SDL
      WRITEONLY,
      DISCARDCONTENTS
#else
      WRITEONLY = 0,                        // in DX9, this is specified during VB creation
      DISCARDCONTENTS = D3DLOCK_DISCARD     // discard previous contents; only works with dynamic VBs
#endif
   };

   VertexBuffer(RenderDevice* rd, const unsigned int vertexCount, const float* verts = nullptr, const bool isDynamic = false, const VertexFormat fvf = VertexFormat::VF_POS_NORMAL_TEX);
   ~VertexBuffer();

   unsigned int GetOffset() const { return m_offset; }
   unsigned int GetVertexOffset() const { return m_vertexOffset; }
   bool IsSharedBuffer() const;

   void lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void **dataBuffer, const DWORD flags);
   void unlock();
   void Upload();

   RenderDevice* const m_rd;
   const bool m_isStatic; // True if this array is static and can not be modified after first upload
   const VertexFormat m_vertexFormat; // Vertex format (position of each attributes inside data stream)
   const unsigned int m_count; // Number of vertices
   const unsigned int m_sizePerVertex; // Size of each vertex
   const unsigned int m_size; // Size in bytes of the array

   #ifdef ENABLE_SDL
   GLuint GetBuffer() const;
   void Bind() const;
   #else
   IDirect3DVertexBuffer9* GetBuffer() const;
   #endif

   SharedVertexBuffer* GetSharedBuffer() const { return m_sharedBuffer;  }

private:
   unsigned int m_offset = 0; // Offset in bytes of the data inside the native GPU array
   unsigned int m_vertexOffset = 0; // Offset in vertices of the data inside the native GPU array
   SharedVertexBuffer* m_sharedBuffer = nullptr;

   static vector<SharedVertexBuffer*> pendingSharedBuffers;
};



class SharedVertexBuffer : public SharedBuffer<VertexFormat, VertexBuffer>
{
public:
   SharedVertexBuffer(VertexFormat fmt, bool stat) : SharedBuffer(fmt, fmt ==  VertexFormat::VF_POS_NORMAL_TEX ? sizeof(Vertex3D_NoTex2) : sizeof(Vertex3D_TexelOnly), stat) {}
   ~SharedVertexBuffer();
   void Upload() override;
   bool IsUploaded() const override { return m_vb; }

   #ifdef ENABLE_SDL
   GLuint m_vb = 0;
   void Bind() const;
   #else
   IDirect3DVertexBuffer9* m_vb = nullptr;
   #endif
};



