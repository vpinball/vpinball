#pragma once

#include "stdafx.h"
#include "typedefs3D.h"

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

   void lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void **dataBuffer, const DWORD flags);
   void unlock();
   void Upload();

   RenderDevice* const m_rd;
   const bool m_isStatic; // True if this array is static and can not be modified after first upload
   const VertexFormat m_vertexFormat; // Vertex format (position of each attributes inside data stream)
   const unsigned int m_vertexCount; // Number of vertices
   const unsigned int m_sizePerVertex; // Size of each vertex
   const unsigned int m_size; // Size in bytes of the array
   const unsigned int m_offset; // Offset in bytes of the data inside the native GPU array
   const unsigned int m_vertexOffset; // Offset in vertices of the data inside the native GPU array

private:
   void CreatePendingSharedBuffer();
   static vector<VertexBuffer*> pendingSharedBuffers;
   static unsigned int getPendingSharedBufferSize();

   struct PendingUpload
   {
      unsigned int offset;
      unsigned int size;
      BYTE* data;
   };
   vector<PendingUpload> m_pendingUploads;
   PendingUpload m_lock = { 0, 0, nullptr };
   
#ifdef ENABLE_SDL
   GLuint m_vb = 0;
   int* m_sharedBufferRefCount = nullptr;

public:
   GLuint GetBuffer() const { return m_vb; }
   bool IsSharedBuffer() const { return m_sharedBufferRefCount != nullptr; }

#else
   IDirect3DVertexBuffer9* m_vb = nullptr;

public:
   IDirect3DVertexBuffer9* GetBuffer() const { return m_vb; }
#endif
};
