#pragma once

#include "stdafx.h"
#include "typedefs3D.h"

template<class Fmt, class Buf>
class SharedBuffer
{
protected:
   SharedBuffer(Fmt fmt, unsigned int bytePerElem, bool stat): m_format(fmt), m_bytePerElement(bytePerElem), m_isStatic(stat) { }

public:
   unsigned int GetCount() const { return m_count; }
   bool IsShared() const { return m_buffers.size() > 1; }
   bool IsEmpty() const { return m_buffers.size() == 0; }

   unsigned int Add(Buf* buffer)
   {
      assert(!IsUploaded());
      unsigned int pos = m_count;
      m_buffers.push_back(buffer);
      m_count += buffer->m_count;
      return pos;
   }

   bool Remove(Buf* buffer)
   {
      RemoveFromVectorSingle(m_buffers, buffer);
      return IsEmpty();
   }

   void Lock(Buf* buffer, const unsigned int offsetToLock, const unsigned int sizeToLock, void** dataBuffer)
   {
      assert(!m_isStatic || !IsUploaded()); // Static buffers can't be locked after first upload
      assert(m_lock.data == nullptr); // Lock is not reentrant
      m_lock.buffer = buffer;
      m_lock.offset = offsetToLock;
      m_lock.size = sizeToLock;
      m_lock.data = new BYTE[m_lock.size];
      *dataBuffer = m_lock.data;
   }

   void Unlock()
   {
      PendingUpload upload = m_lock;
      m_pendingUploads.push_back(upload);
      m_lock.data = nullptr;
   }

   virtual void Upload() = 0;

   virtual bool IsUploaded() const = 0;

   const Fmt m_format;
   const unsigned int m_bytePerElement;
   const bool m_isStatic;

   struct PendingUpload
   {
      Buf* buffer;
      unsigned int offset;
      unsigned int size;
      BYTE* data;
   };
   vector<PendingUpload> m_pendingUploads;

protected:
   PendingUpload m_lock = { nullptr, 0, 0, nullptr };
   vector<Buf*> m_buffers;
   unsigned int m_count = 0;
};



class SharedIndexBuffer;
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
   bool IsSharedBuffer() const;

   void lock(const unsigned int offsetToLock, const unsigned int sizeToLock, void** dataBuffer, const DWORD flags);
   void unlock();
   void ApplyOffset(VertexBuffer* vb);
   void Upload();

   RenderDevice* const m_rd;
   const unsigned int m_count;
   const Format m_indexFormat;
   const unsigned int m_sizePerIndex;
   const bool m_isStatic;
   const unsigned int m_size;

   #ifdef ENABLE_SDL
   GLuint GetBuffer() const;
   void Bind() const;
   #else
   IDirect3DIndexBuffer9* GetBuffer() const;
   #endif

private:
   unsigned int m_offset = 0; // Offset in bytes of the data inside the native GPU array
   unsigned int m_indexOffset = 0; // Offset in indices of the data inside the native GPU array
   SharedIndexBuffer* m_sharedBuffer = nullptr;
};


class SharedIndexBuffer : public SharedBuffer<IndexBuffer::Format, IndexBuffer>
{
public:
   SharedIndexBuffer(IndexBuffer::Format fmt, bool stat) : SharedBuffer(fmt, fmt == IndexBuffer::Format::FMT_INDEX16 ? 2 : 4, stat) {}
   ~SharedIndexBuffer();
   void Upload() override;
   bool IsUploaded() const override { return m_ib; }

   #ifdef ENABLE_SDL
   GLuint m_ib = 0;
   void Bind() const;
   #else
   IDirect3DIndexBuffer9* m_ib = nullptr;
   #endif
};

