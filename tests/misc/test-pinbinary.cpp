// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pinbinary.h"
#include "utils/BiffReader.h"
#include "utils/BiffWriter.h"
#include "core/vpversion.h"

#include "doctest.h"

TEST_CASE("PinBinary")
{
   SUBCASE("BIFF save/load round-trip")
   {
      PinBinary bin;
      bin.m_name = "testblob";
      bin.m_path = "images/testblob.bin";
      bin.m_buffer = { 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x42 };

      InMemStream stream;
      {
         BiffWriter writer(&stream, nullptr);
         bin.Save(writer);
         CHECK_FALSE(writer.HasError());
      }

      PinBinary loaded;
      BiffReader reader(stream.Data(), static_cast<uint32_t>(stream.Size()), CURRENT_FILE_FORMAT_VERSION, nullptr, 0);
      loaded.Load(reader);
      CHECK_FALSE(reader.HasError());

      CHECK(loaded.m_name == "testblob");
      CHECK(loaded.m_path == std::filesystem::path("images/testblob.bin"));
      CHECK(loaded.m_buffer == bin.m_buffer);
   }

   SUBCASE("BIFF round-trip of an empty buffer")
   {
      PinBinary bin;
      bin.m_name = "empty";
      bin.m_path = "empty.bin";

      InMemStream stream;
      {
         BiffWriter writer(&stream, nullptr);
         bin.Save(writer);
         CHECK_FALSE(writer.HasError());
      }

      PinBinary loaded;
      BiffReader reader(stream.Data(), static_cast<uint32_t>(stream.Size()), CURRENT_FILE_FORMAT_VERSION, nullptr, 0);
      loaded.Load(reader);
      CHECK_FALSE(reader.HasError());

      CHECK(loaded.m_name == "empty");
      CHECK(loaded.m_buffer.empty());
   }

   SUBCASE("file write/read round-trip")
   {
      const std::filesystem::path file = GetTestTmpDir() / "pinbinary-blob.bin";

      PinBinary bin;
      bin.m_buffer = { 1, 2, 3, 4, 5, 250, 251, 252 };
      REQUIRE(bin.WriteToFile(file.string()));

      PinBinary loaded;
      CHECK(loaded.ReadFromFile(file));
      CHECK(loaded.m_buffer == bin.m_buffer);
      // ReadFromFile derives the name from the file name (without extension)
      CHECK(loaded.m_name == "pinbinary-blob");
      CHECK(loaded.m_path == file);

      std::error_code ec;
      std::filesystem::remove(file, ec);
   }
}
