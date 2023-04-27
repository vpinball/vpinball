#pragma once

#include "stdafx.h"
#include "typedefs3D.h"

class VertexBuffer;

class IndexBuffer final
{
public:
   enum Format
   {
      FMT_INDEX16,
      FMT_INDEX32
   };

   enum LockFlags
   {
#ifdef ENABLE_SDL
      WRITEONLY,
      DISCARDCONTENTS
#else
      WRITEONLY = 0, // in DX9, this is specified during VB creation
      DISCARDCONTENTS = D3DLOCK_DISCARD // discard previous contents; only works with dynamic VBs
#endif
   };

   IndexBuffer(RenderDevice* rd, const unsigned int numIndices, const bool isDynamic = false, const IndexBuffer::Format format = IndexBuffer::Format::FMT_INDEX16);
   IndexBuffer(RenderDevice* rd, const unsigned int numIndices, const unsigned int* indices);
   IndexBuffer(RenderDevice* rd, const unsigned int numIndices, const WORD* indices);
   IndexBuffer(RenderDevice* rd, const vector<unsigned int>& indices);
   IndexBuffer(RenderDevice* rd, const vector<WORD>& indices);
   ~IndexBuffer();

   unsigned int GetOffset() const { return m_offset; }
   unsigned int GetIndexOffset() const { return m_indexOffset; }
   bool IsSharedBuffer() const { return m_sharedBuffer->buffers.size() > 1; }

   void lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void** dataBuffer, const DWORD flags);
   void unlock();
   void ApplyOffset(VertexBuffer* vb);
   void Upload();

   RenderDevice* const m_rd;
   const unsigned int m_indexCount;
   const unsigned int m_sizePerIndex;
   const unsigned int m_size;
   const bool m_isStatic;
   const Format m_indexFormat;

private:
   struct SharedBuffer
   {
      vector<IndexBuffer*> buffers;
      unsigned int count = 0;
      Format format;
      bool isStatic;
   };

   bool IsCreated() const { return m_ib; }

   SharedBuffer* m_sharedBuffer = nullptr;
   unsigned int m_offset = 0; // Offset in bytes of the data inside the native GPU array
   unsigned int m_indexOffset = 0; // Offset in indices of the data inside the native GPU array

   static void CreateSharedBuffer(SharedBuffer* sharedBuffer);
   static vector<SharedBuffer*> pendingSharedBuffers;

   struct PendingUpload
   {
      unsigned int offset;
      unsigned int size;
      BYTE* data;
   };
   vector<PendingUpload> m_pendingUploads;
   PendingUpload m_lock = { 0, 0, nullptr };

#ifdef ENABLE_SDL
   GLuint m_ib = 0;

public:
   GLuint GetBuffer() const { return m_ib; }

#else
   IDirect3DIndexBuffer9* m_ib = nullptr;

public:
   IDirect3DIndexBuffer9* GetBuffer() const { return m_ib; }
#endif
};
