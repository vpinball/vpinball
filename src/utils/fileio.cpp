// license:GPLv3+

#include "core/stdafx.h"

#include "pole/pole.h"


void InMemStream::Write(const void *pv, const size_t count)
{
   if (count == 0)
      return;

   if (m_cSeek + count > m_data.size())
   {
      m_data.reserve(max(m_cSeek * 2, m_cSeek + count));
      m_data.resize(m_cSeek + count);
   }

   memcpy(m_data.data() + m_cSeek, pv, count);
   m_cSeek += count;
}


InMemStructuredStorage::~InMemStructuredStorage()
{
   for (const auto &stream : m_streams)
      delete stream.second;
}

InMemStream *InMemStructuredStorage::CreateStream(const string &name)
{
   // Same semantics as STGM_CREATE: creating over an existing name replaces it
   for (auto &entry : m_streams)
      if (entry.first == name)
      {
         delete entry.second;
         entry.second = new InMemStream();
         return entry.second;
      }
   InMemStream *const stream = new InMemStream();
   m_streams.emplace_back(name, stream);
   return stream;
}

bool InMemStructuredStorage::WriteToStorage(POLE::Storage &storage) const
{
   std::unique_ptr<POLE::Stream> last;
   for (const auto &entry : m_streams)
   {
      auto stream = std::make_unique<POLE::Stream>(&storage, entry.first, true);
      if (stream->fail() || stream->write(const_cast<unsigned char *>(entry.second->Data()), entry.second->Size()) != entry.second->Size())
         return false;
      last = std::move(stream);
   }
   if (last)
      last->flush(); // flushes the storage header, allocation tables and directory tree
   return true;
}
