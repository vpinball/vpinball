#pragma once

#include "stdafx.h"
#include "typedefs3D.h"

class VertexBuffer;
class IndexBuffer;

class MeshBuffer final
{
public:
   MeshBuffer(VertexBuffer* vb, IndexBuffer* ib = nullptr);
   ~MeshBuffer();
   void bind();

   VertexBuffer* const m_vb;
   IndexBuffer* const m_ib;

#ifdef ENABLE_SDL
public:
   static void ClearSharedBuffers();

private:
   GLuint m_vao = 0;
   bool m_isSharedVAO = false;
   struct SharedVAO
   {
      GLuint vb, ib, vao, ref_count;
   };
   static vector<SharedVAO> sharedVAOs;
#else
   VertexDeclaration* const m_vertexDeclaration;
#endif
};

class IndexBuffer final
{
public:
   enum Format {
      FMT_INDEX16, FMT_INDEX32
   };

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

   IndexBuffer(RenderDevice* rd, const unsigned int numIndices, const DWORD usage, const IndexBuffer::Format format);
   IndexBuffer(RenderDevice* rd, const unsigned int numIndices, const unsigned int* indices);
   IndexBuffer(RenderDevice* rd, const unsigned int numIndices, const WORD* indices);
   IndexBuffer(RenderDevice* rd, const vector<unsigned int>& indices);
   IndexBuffer(RenderDevice* rd, const vector<WORD>& indices);
   ~IndexBuffer();

   unsigned int GetOffset() const { return m_offset; }
   unsigned int GetIndexOffset() const { return m_offset / m_sizePerIndex; }

   void lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void** dataBuffer, const DWORD flags);
   void unlock();
   void bind();

   RenderDevice* const m_rd;
   const unsigned int m_indexCount;
   const unsigned int m_sizePerIndex;
   const unsigned int m_size;
   const bool m_isStatic;
   const Format m_indexFormat;

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
   static vector<IndexBuffer*> pendingSharedBuffers;

#ifdef ENABLE_SDL
   GLuint m_ib = 0;
   int* m_sharedBufferRefCount = nullptr;
#else
   IDirect3DIndexBuffer9* m_ib = nullptr;
#endif
};
