#include "VPXFile.h"


VPXFile::VPXFile(const string& path)
   : m_path(path)
   , m_pStorage(new POLE::Storage(path.c_str()))
{
   if (m_pStorage->open() && m_pStorage->result() == POLE::Storage::Ok)
      for (const string& entry : m_pStorage->GetAllStreams("/GameStg/"))
         if (entry.starts_with("/GameStg/Image"))
            ReadImage(entry, true);
}

VPXFile::~VPXFile()
{
   delete m_pStorage;
}

class VPXReader
{
public:
   VPXReader(POLE::Storage* storage, const std::string& name)
      : m_stream(storage, name)
   {
   }

   POLE::uint64 GetPos() { return m_stream.tell(); }
   void Skip(int count) { m_stream.seek(m_stream.tell() + count); }
   void Seek(POLE::uint64 newPos) { m_stream.seek(newPos); }
   void Read(int length, uint8_t* buffer) { m_stream.read(buffer, length); }

   int ReadInt32()
   {
      int v;
      m_stream.read(reinterpret_cast<unsigned char*>(&v), 4);
      /* if (_reverse)
         v = BinaryPrimitives.ReverseEndianness(v);*/
      return v;
   }
   uint32_t ReadUInt32()
   {
      uint32_t v;
      m_stream.read(reinterpret_cast<unsigned char*>(&v), 4);
      /* if (_reverse)
         v = BinaryPrimitives.ReverseEndianness(v);*/
      return v;
   }

   string ReadLenPrefixedString()
   {
      uint32_t length = (int)ReadUInt32();
      char* buffer = new char[length + 1];
      m_stream.read(reinterpret_cast<unsigned char*>(buffer), length);
      buffer[length] = 0;
      string result(buffer);
      delete[] buffer;
      return result;
   }

private:
   POLE::Stream m_stream;
};

#define FID(A) (int)((unsigned int)(#A[0]) | ((unsigned int)(#A[1]) << 8) | ((unsigned int)(#A[2]) << 16) | ((unsigned int)(#A[3]) << 24))

std::pair<unsigned int, uint8_t*> VPXFile::ReadImage(const string& path, bool nameOnly)
{
   VPXReader reader(m_pStorage, path);
   bool done = false;
   bool inJpg = false;
   unsigned int size = 0;
   Entry entry;
   entry.path = path;
   while (!done)
   {
      uint32_t bytesInRecordRemaining = reader.ReadUInt32();
      POLE::uint64 fieldStart = reader.GetPos();
      uint32_t tag = reader.ReadUInt32();
      switch (tag)
      {
      case FID(ENDB):
         if (inJpg)
            inJpg = false;
         else
            done = true;
         break;
      case FID(NAME):
         if (!inJpg)
            entry.name = reader.ReadLenPrefixedString();
         break;
      case FID(PATH): entry.file = reader.ReadLenPrefixedString(); break;
      case FID(INME): reader.ReadLenPrefixedString(); break;
      case FID(WDTH): reader.ReadUInt32(); break;
      case FID(HGHT): reader.ReadUInt32(); break;
      case FID(SIZE): size = reader.ReadInt32(); break;
      case FID(ALTV):
         reader.Skip(4); // skip the float
         break;
      case FID(JPEG): inJpg = true; break;
      case FID(DATA):
         if (nameOnly)
            done = true;
         else
         {
            uint8_t* data = new uint8_t[size];
            reader.Read(size, data);
            if (data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4E && data[3] == 0x47 && data[4] == 0x0D && data[5] == 0x0A && data[6] == 0x1A && data[7] == 0x0A)
            {
               // PNG
               return { size, data };
            }
            else if (data[0] == 0xFF && data[1] == 0xD8)
            {
               // JPG
               return { size, data };
            }
            else if (data[0] == 0x23 && data[1] == 0x3F && data[2] == 0x52 && data[3] == 0x41 && data[4] == 0x44 && data[5] == 0x49 && data[6] == 0x41 && data[7] == 0x4E && data[8] == 0x43
               && data[9] == 0x45 && data[10] == 0x0A)
            {
               // HDR (unsupported)
               delete[] data;
               return { 0, nullptr };
            }
            else
            {
               delete[] data;
               return { 0, nullptr };
            }
         }
         break;
      }
      reader.Seek(fieldStart + bytesInRecordRemaining);
   }
   if (!entry.name.empty())
      m_images[string_to_lower(entry.name)] = entry;
   return { 0, nullptr };
}

string VPXFile::GetImportFile(const string& path)
{
   auto it = m_images.find(path);
   if (it != m_images.end())
      return it->second.file;
   return string();
}

std::pair<unsigned int, uint8_t*> VPXFile::GetImage(const string& path)
{
   auto it = m_images.find(path);
   if (it != m_images.end())
      return ReadImage(it->second.path, false);
   return { 0, nullptr };
}
