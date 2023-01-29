#pragma once

#include "stdafx.h"
#include "typedefs3D.h"

class VertexBuffer;
class IndexBuffer;

class MeshBuffer final
{
public:
   MeshBuffer(VertexBuffer* vb, const bool ownBuffers);
   MeshBuffer(VertexBuffer* vb, IndexBuffer* ib, const bool ownBuffers);
   ~MeshBuffer();
   void bind();

   const bool m_ownBuffers;
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
#ifdef ENABLE_SDL
      FMT_INDEX16 = 16,
      FMT_INDEX32 = 32
#else
      FMT_INDEX16 = D3DFMT_INDEX16,
      FMT_INDEX32 = D3DFMT_INDEX32
#endif
   };

   enum LockFlags
   {
#ifdef ENABLE_SDL
      WRITEONLY,
      NOOVERWRITE,
      DISCARDCONTENTS
#else
      WRITEONLY = 0,                        // in DX9, this is specified during VB creation
      NOOVERWRITE = D3DLOCK_NOOVERWRITE,    // meaning: no recently drawn vertices are overwritten. only works with dynamic VBs.
                                            // it's only needed for VBs which are locked several times per frame
      DISCARDCONTENTS = D3DLOCK_DISCARD     // discard previous contents; only works with dynamic VBs
#endif
   };

   IndexBuffer(RenderDevice* rd, const unsigned int numIndices, const DWORD usage, const IndexBuffer::Format format);
   IndexBuffer(RenderDevice* rd, const unsigned int numIndices, const unsigned int* indices);
   IndexBuffer(RenderDevice* rd, const unsigned int numIndices, const WORD* indices);
   IndexBuffer(RenderDevice* rd, const vector<unsigned int>& indices);
   IndexBuffer(RenderDevice* rd, const vector<WORD>& indices);
   ~IndexBuffer();

   void lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void** dataBuffer, const DWORD flags);
   void unlock();
   void release();
   void bind();

private:
   RenderDevice* m_rd;
   DWORD m_usage;
   unsigned int m_sizePerIndex;
   Format m_indexFormat;

#ifdef ENABLE_SDL
public:
   GLuint getOffset() const { return m_offset; }
   GLuint getBuffer() const { return m_buffer; }
   bool useSharedBuffer() const { return m_sharedBuffer; }
   Format getIndexFormat() const { return m_indexFormat; }

   static void ClearSharedBuffers();

private:
   GLuint m_count;
   GLuint m_size;
   bool m_isUploaded;
   bool m_sharedBuffer;

   // CPU memory management
   unsigned int m_offsetToLock;
   unsigned int m_sizeToLock;
   void* m_dataBuffer = nullptr;

   //GPU memory management
   GLuint m_buffer = 0;
   GLuint m_offset = 0; // Offset if stored in a shared GPU buffer

   void UploadData();
   void addToNotUploadedBuffers();
   static vector<IndexBuffer*> notUploadedBuffers;
   static void UploadBuffers(RenderDevice* rd);
#else
   IDirect3DIndexBuffer9* m_ib = nullptr;
#endif
};
