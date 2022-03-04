#pragma once

#include "stdafx.h"
#include "typedefs3D.h"

#ifdef ENABLE_SDL

class IndexBuffer
{
public:
   enum Format {
      FMT_INDEX16 = 16,
      FMT_INDEX32 = 32
   };

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

   static void bindNull() { m_curIndexBuffer = nullptr; }
   static void CreateIndexBuffer(const unsigned int numIndices, const DWORD usage, const IndexBuffer::Format format, IndexBuffer **idxBuffer, const deviceNumber dN);

   static IndexBuffer* CreateAndFillIndexBuffer(const unsigned int numIndices, const unsigned int * indices, const deviceNumber dN);
   static IndexBuffer* CreateAndFillIndexBuffer(const unsigned int numIndices, const WORD * indices, const deviceNumber dN);
   static IndexBuffer* CreateAndFillIndexBuffer(const std::vector<unsigned int>& indices, const deviceNumber dN);
   static IndexBuffer* CreateAndFillIndexBuffer(const std::vector<WORD>& indices, const deviceNumber dN);

   static void UploadBuffers();

   GLuint getOffset() const { return offset; }
   Format getIndexFormat() const { return indexFormat; }

private:
   GLuint count;
   GLuint size;
   GLuint offset;
   DWORD usage;
   bool isUploaded;
   bool sharedBuffer;

   // CPU memory management
   unsigned int offsetToLock;
   unsigned int sizeToLock;
   void *dataBuffer = nullptr;

   //GPU memory management
   GLuint Buffer = 0;
   Format indexFormat;

   static IndexBuffer* m_curIndexBuffer; // for caching
   static std::vector<IndexBuffer*> notUploadedBuffers;

   void UploadData(bool freeData);
   void addToNotUploadedBuffers(const void* indices = nullptr);
};

#else

class IndexBuffer
{
public:
   enum Format {
      FMT_INDEX16 = D3DFMT_INDEX16,
      FMT_INDEX32 = D3DFMT_INDEX32
   };
   enum LockFlags
   {
      WRITEONLY = 0,                      // in DX9, this is specified during VB creation
      NOOVERWRITE = D3DLOCK_NOOVERWRITE,  // meaning: no recently drawn vertices are overwritten. only works with dynamic VBs.
                                          // it's only needed for VBs which are locked several times per frame
      DISCARDCONTENTS = D3DLOCK_DISCARD   // discard previous contents; only works with dynamic VBs
   };

   void lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void **dataBuffer, const DWORD flags);
   void unlock();
   void release();
   void bind();

   static void bindNull() { m_curIndexBuffer = nullptr; }
   static void setD3DDevice(IDirect3DDevice9* primary, IDirect3DDevice9* secondary) { m_pd3dPrimaryDevice = primary; m_pd3dSecondaryDevice = secondary; }

   static void CreateIndexBuffer(const unsigned int numIndices, const DWORD usage, const IndexBuffer::Format format, IndexBuffer** idxBuffer, const deviceNumber dN);

   static IndexBuffer* CreateAndFillIndexBuffer(const unsigned int numIndices, const unsigned int* indices, const deviceNumber dN);
   static IndexBuffer* CreateAndFillIndexBuffer(const unsigned int numIndices, const WORD* indices, const deviceNumber dN);
   static IndexBuffer* CreateAndFillIndexBuffer(const std::vector<unsigned int>& indices, const deviceNumber dN);
   static IndexBuffer* CreateAndFillIndexBuffer(const std::vector<WORD>& indices, const deviceNumber dN);

   static IndexBuffer* m_curIndexBuffer; // for caching

   IDirect3DIndexBuffer9* m_ib = nullptr;

private:
   //IndexBuffer();      // disable default constructor

   deviceNumber m_dN;

   static IDirect3DDevice9* m_pd3dPrimaryDevice;
   static IDirect3DDevice9* m_pd3dSecondaryDevice;
};

#endif
