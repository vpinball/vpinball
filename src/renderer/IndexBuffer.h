// license:GPLv3+

#pragma once

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
      assert(!IsCreated());
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

   void Lock(Buf* buffer, const unsigned int offset, const unsigned int size, void*& data)
   {
      assert(!m_isStatic || !IsCreated()); // Static buffers can't be locked after first upload
      assert(m_lock.data == nullptr); // Lock is not reentrant
      m_lock.buffer = buffer;
      m_lock.offset = offset;
      m_lock.size = size;
      #if defined(ENABLE_BGFX)
      if (IsCreated()) {
         m_lock.mem = bgfx::alloc(m_lock.size);
         m_lock.data = m_lock.mem->data;
      }
      else
      #endif
      {
         m_lock.data = new BYTE[m_lock.size];
      }
      data = m_lock.data;
   }

   void Unlock()
   {
      PendingUpload upload = m_lock;
      m_pendingUploads.push_back(upload);
      m_lock.data = nullptr;
   }

   virtual void Upload() = 0;

   virtual bool IsCreated() const = 0;

   const Fmt m_format;
   const unsigned int m_bytePerElement;
   const bool m_isStatic;

   struct PendingUpload
   {
      Buf* buffer;
      unsigned int offset;
      unsigned int size;
      BYTE* data;
      #if defined(ENABLE_BGFX)
      const bgfx::Memory* mem;
      #endif
   };
   vector<PendingUpload> m_pendingUploads;

protected:
   PendingUpload m_lock = { nullptr, 0, 0, nullptr
#if defined(ENABLE_BGFX)
      ,nullptr
#endif
   };
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

   IndexBuffer(RenderDevice* rd, const unsigned int numIndices, const bool isDynamic = false, const IndexBuffer::Format format = IndexBuffer::Format::FMT_INDEX16);
   IndexBuffer(RenderDevice* rd, const unsigned int numIndices, const unsigned int* indices);
   IndexBuffer(RenderDevice* rd, const unsigned int numIndices, const WORD* indices);
   IndexBuffer(RenderDevice* rd, const vector<unsigned int>& indices);
   IndexBuffer(RenderDevice* rd, const vector<WORD>& indices);
   ~IndexBuffer();

   // Position of buffer in a bigger shared data block
   unsigned int GetOffset() const { return m_offset; }
   unsigned int GetIndexOffset() const { return m_indexOffset; }
   bool IsSharedBuffer() const;

   // Initial loading (before creation) or updating (for dynamic buffers only)
   template <typename T> void Lock(T*& data, const unsigned int offset = 0, const unsigned int size = 0) { LockUntyped((void*&)data, offset, size); }
   void Unlock();
   void Upload();

   void ApplyOffset(VertexBuffer* vb);

   RenderDevice* const m_rd;
   const unsigned int m_count;
   const Format m_indexFormat;
   const unsigned int m_sizePerIndex;
   const bool m_isStatic;
   const unsigned int m_size;

   #if defined(ENABLE_BGFX)
   bgfx::IndexBufferHandle GetStaticBuffer() const;
   bgfx::DynamicIndexBufferHandle GetDynamicBuffer() const;

   #elif defined(ENABLE_OPENGL)
   GLuint GetBuffer() const;
   void Bind() const;

   #elif defined(ENABLE_DX9)
   IDirect3DIndexBuffer9* GetBuffer() const;
   void Bind() const;
   #endif

private:
   unsigned int m_offset = 0; // Offset in bytes of the data inside the native GPU array
   unsigned int m_indexOffset = 0; // Offset in indices of the data inside the native GPU array
   SharedIndexBuffer* m_sharedBuffer = nullptr;
   void LockUntyped(void*& data, const unsigned int offset = 0, const unsigned int size = 0);
};
