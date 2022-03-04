#pragma once

#include "stdafx.h"
#include "typedefs3D.h"

#ifdef ENABLE_SDL

class VertexBuffer
{
public:
   enum LockFlags //!! not handled
   {
      WRITEONLY,
      NOOVERWRITE,
      DISCARDCONTENTS
   };

   void lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void **dataBuffer, const DWORD flags);
   void unlock();
   void release();
   void bind();

   static void bindNull() { m_curVertexBuffer = nullptr; }
   static void CreateVertexBuffer(const unsigned int vertexCount, const DWORD usage, const DWORD fvf, VertexBuffer **vBuffer, const deviceNumber dN);
   static void UploadBuffers();

   GLuint getOffset() const { return offset; }

private:
   GLuint count;
   GLuint size;
   GLuint sizePerVertex;
   DWORD fvf;
   DWORD usage;
   bool isUploaded;
   bool sharedBuffer;

   // CPU memory management
   unsigned int offsetToLock;
   unsigned int sizeToLock;
   void *dataBuffer = nullptr;

   //GPU memory management
   GLuint Buffer = 0;
   GLuint Array = 0;
   GLuint offset = 0;//unused ATM, but if we want to group multiple IndexBuffers later in one buffer we might need it

   static VertexBuffer* m_curVertexBuffer; // for caching
   static std::vector<VertexBuffer*> notUploadedBuffers;

   void UploadData();
   void addToNotUploadedBuffers();
};

#else

class VertexBuffer
{
public:
   enum LockFlags
   {
      WRITEONLY = 0,                        // in DX9, this is specified during VB creation
      NOOVERWRITE = D3DLOCK_NOOVERWRITE,    // meaning: no recently drawn vertices are overwritten. only works with dynamic VBs.
                                            // it's only needed for VBs which are locked several times per frame
      DISCARDCONTENTS = D3DLOCK_DISCARD     // discard previous contents; only works with dynamic VBs
   };

   void lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void **dataBuffer, const DWORD flags);
   void unlock();
   void release();
   void bind();

   static void bindNull() { m_curVertexBuffer = nullptr; }
   static void setD3DDevice(IDirect3DDevice9* primary, IDirect3DDevice9* secondary) { m_pd3dPrimaryDevice = primary; m_pd3dSecondaryDevice = secondary; }

   static void CreateVertexBuffer(const unsigned int vertexCount, const DWORD usage, const DWORD fvf, VertexBuffer **vBuffer, const deviceNumber dN);

   static VertexBuffer* m_curVertexBuffer; // for caching

   IDirect3DVertexBuffer9* m_vb = nullptr;

private:
   //VertexBuffer();     // disable default constructor

   DWORD m_fvf;
   deviceNumber m_dN;

   static IDirect3DDevice9* m_pd3dPrimaryDevice;
   static IDirect3DDevice9* m_pd3dSecondaryDevice;
};

#endif
