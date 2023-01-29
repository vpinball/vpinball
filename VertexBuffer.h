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
      NOOVERWRITE,
      DISCARDCONTENTS
#else
      WRITEONLY = 0,                        // in DX9, this is specified during VB creation
      NOOVERWRITE = D3DLOCK_NOOVERWRITE,    // meaning: no recently drawn vertices are overwritten. only works with dynamic VBs.
                                            // it's only needed for VBs which are locked several times per frame
      DISCARDCONTENTS = D3DLOCK_DISCARD     // discard previous contents; only works with dynamic VBs
#endif
   };

   VertexBuffer(RenderDevice* rd, const unsigned int vertexCount, const DWORD usage, const DWORD fvf);
   VertexBuffer(RenderDevice* rd, const unsigned int vertexCount, const DWORD usage, const DWORD fvf, const float* verts);
   ~VertexBuffer();

   void lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void **dataBuffer, const DWORD flags);
   void unlock();
   void release();
   void bind();

private:
   RenderDevice* m_rd;
   DWORD m_usage;
   DWORD m_fvf;
   unsigned int m_sizePerVertex;

#ifdef ENABLE_SDL
public:
   GLuint getOffset() const { return m_offset; }
   GLuint getBuffer() const { return m_buffer; }
   bool isUploaded() const { return m_isUploaded; }
   bool useSharedBuffer() const { return m_sharedBuffer; }

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
   static vector<VertexBuffer*> notUploadedBuffers;
   static void UploadBuffers(RenderDevice* rd);
#else
   IDirect3DVertexBuffer9* m_vb = nullptr;
#endif
};
