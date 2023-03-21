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

   VertexBuffer(RenderDevice* rd, const unsigned int vertexCount, const DWORD usage, const DWORD fvf);
   VertexBuffer(RenderDevice* rd, const unsigned int vertexCount, const DWORD usage, const DWORD fvf, const float* verts);
   ~VertexBuffer();

   unsigned int GetVertexOffset() const { return m_offset / m_sizePerVertex; }

   void lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void **dataBuffer, const DWORD flags);
   void unlock();
   void bind();

   RenderDevice* const m_rd;
   const unsigned int m_vertexCount;
   const unsigned int m_sizePerVertex;
   const unsigned int m_size;
   const bool m_isStatic;
   const DWORD m_fvf;

private:
   void CreatePendingSharedBuffer();

   struct PendingUpload
   {
      unsigned int offset;
      unsigned int size;
      BYTE* data;
   };
   vector<PendingUpload> m_pendingUploads;
   PendingUpload m_lock = { 0, 0, nullptr };
   int m_offset = 0;
   static vector<VertexBuffer*> pendingSharedBuffers;

#ifdef ENABLE_SDL
   GLuint m_vb = 0;
   int* m_sharedBufferRefCount = nullptr;

public:
   GLuint GetBuffer() const { return m_vb; }
   bool IsSharedBuffer() const { return m_sharedBufferRefCount != nullptr; }
   bool HasPendingUploads() const { return m_pendingUploads.size() > 0; }

#else
   IDirect3DVertexBuffer9* m_vb = nullptr;
#endif
};
