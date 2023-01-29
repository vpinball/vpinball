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

   const unsigned int m_vertexCount;

   RenderDevice* const m_rd;
   const DWORD m_usage;
   const DWORD m_fvf;
   const unsigned int m_sizePerVertex;
   const unsigned int m_size;

#ifdef ENABLE_SDL
public:
   GLuint getOffset() const { return m_offset; }
   GLuint getBuffer() const { return m_buffer; }
   bool isUploaded() const { return m_isUploaded; }
   bool useSharedBuffer() const { return m_sharedBuffer; }

   static void ClearSharedBuffers();

private:
   bool m_isUploaded = false;
   bool m_sharedBuffer = false;

   // CPU memory management
   unsigned int m_offsetToLock = 0;
   unsigned int m_sizeToLock = 0;
   void* m_dataBuffer = nullptr;

   //GPU memory management
   GLuint m_buffer = 0;
   GLuint m_offset = 0; // Offset if stored in a shared GPU buffer

   void UploadData();
   void addToNotUploadedBuffers();
   static vector<VertexBuffer*> notUploadedBuffers;
   static void UploadBuffers(RenderDevice* rd);
#else
private:
   IDirect3DVertexBuffer9* m_vb = nullptr;
#endif
};
