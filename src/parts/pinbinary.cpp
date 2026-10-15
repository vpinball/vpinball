// license:GPLv3+

#include "core/stdafx.h"
#include "parts/pinbinary.h"

bool PinBinary::ReadFromFile(const std::filesystem::path& filename)
{
   m_buffer = read_file(filename);
   m_path = filename;
   m_name = TitleFromFilename(filename);
   return true;
}

bool PinBinary::WriteToFile(const std::filesystem::path& filename) const
{
   write_file(filename, m_buffer);
   return true;
}

void PinBinary::Save(IObjectWriter& writer) const
{
   writer.WriteString(FID(NAME), m_name);
   writer.WriteString(FID(PATH), PathToUTF8(m_path));
   writer.WriteInt(FID(SIZE), static_cast<int>(m_buffer.size()));
   writer.WriteRaw(FID(DATA), m_buffer.data(), static_cast<int>(m_buffer.size()));
   writer.EndObject();
}

void PinBinary::Load(IObjectReader& reader)
{
   reader.AsObject(
      [this](int tag, IObjectReader& reader)
      {
         switch (tag)
         {
         case FID(NAME): m_name = reader.AsString(); break;
         case FID(PATH): m_path = PathFromUTF8(reader.AsString()); break;
         case FID(SIZE): m_buffer.resize(reader.AsInt()); break;
         // Size must come before data, otherwise our structure won't be allocated
         case FID(DATA): reader.AsRaw(m_buffer.data(), static_cast<int>(m_buffer.size())); break;
         }
         return true;
      });
}


