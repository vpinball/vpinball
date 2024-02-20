#pragma once

#include "core/stdafx.h"
#include "typedefs3D.h"
#include "IndexBuffer.h"


class SharedVertexBuffer;

class VertexBuffer final
{
public:
   enum LockFlags
   {
      #if defined(ENABLE_OPENGL) || defined(ENABLE_BGFX)
      WRITEONLY,
      DISCARDCONTENTS
      #elif defined(ENABLE_DX9)
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

   #if defined(ENABLE_BGFX)

   #elif defined(ENABLE_OPENGL)
   GLuint GetBuffer() const;
   void Bind() const;
   
   #elif defined(ENABLE_DX9)
   IDirect3DVertexBuffer9* GetBuffer() const;
   #endif

   SharedVertexBuffer* GetSharedBuffer() const { return m_sharedBuffer;  }

private:
   unsigned int m_offset = 0; // Offset in bytes of the data inside the native GPU array
   unsigned int m_vertexOffset = 0; // Offset in vertices of the data inside the native GPU array
   SharedVertexBuffer* m_sharedBuffer = nullptr;
};



class SharedVertexBuffer : public SharedBuffer<VertexFormat, VertexBuffer>
{
public:
   SharedVertexBuffer(VertexFormat fmt, bool stat) : SharedBuffer(fmt, fmt ==  VertexFormat::VF_POS_NORMAL_TEX ? sizeof(Vertex3D_NoTex2) : sizeof(Vertex3D_TexelOnly), stat) {}
   ~SharedVertexBuffer();
   void Upload() override;

   #if defined(ENABLE_BGFX)
   bgfx::VertexBufferHandle m_vb = BGFX_INVALID_HANDLE;
   bgfx::DynamicVertexBufferHandle m_dvb = BGFX_INVALID_HANDLE;
   bool IsUploaded() const override { return m_isStatic ? bgfx::isValid(m_vb) : bgfx::isValid(m_dvb); }
   
   #elif defined(ENABLE_OPENGL)
   GLuint m_vb = 0;
   void Bind() const;
   bool IsUploaded() const override { return m_vb; }

   #elif defined(ENABLE_DX9)
   IDirect3DVertexBuffer9* m_vb = nullptr;
   bool IsUploaded() const override { return m_vb; }
   #endif
};



