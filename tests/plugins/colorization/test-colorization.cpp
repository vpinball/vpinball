// license:GPLv3+

// VNI and Serum colorization at every DMD size standalone supports natively:
// 128x32, 128x16 and 192x64. The colorizations are synthetic one-frame files
// built in this file, no shipped colorization data is used.

#include "core/stdafx.h"
#include "../../vpx-test.h"
#include "doctest.h"

#include "serum-decode.h"
#include "vni.h"

namespace
{

// zlib-style CRC32 (reflected, poly 0xEDB88320), the same variant libserum's
// crc32_fast uses for frame hashcodes.
uint32_t Crc32(const uint8_t* data, size_t size)
{
   uint32_t crc = 0xffffffffu;
   for (size_t i = 0; i < size; i++)
   {
      crc ^= data[i];
      for (int k = 0; k < 8; k++)
         crc = (crc & 1) ? (crc >> 1) ^ 0xedb88320u : crc >> 1;
   }
   return ~crc;
}

// --- Minimal Serum cROM writers -------------------------------------------
//
// The layouts below mirror what libserum's Serum_LoadFilev1Stream /
// Serum_LoadFilev2Stream read, with all optional blocks empty and a header
// size that ends before any extended block. One frame is stored, colorized to
// a single flat color, identified by the CRC32 of the ROM frame (no mask, no
// shape mode).

void P8(std::vector<uint8_t>& out, uint8_t v) { out.push_back(v); }
void P16(std::vector<uint8_t>& out, uint16_t v)
{
   out.push_back(static_cast<uint8_t>(v));
   out.push_back(static_cast<uint8_t>(v >> 8));
}
void P32(std::vector<uint8_t>& out, uint32_t v)
{
   out.push_back(static_cast<uint8_t>(v));
   out.push_back(static_cast<uint8_t>(v >> 8));
   out.push_back(static_cast<uint8_t>(v >> 16));
   out.push_back(static_cast<uint8_t>(v >> 24));
}
void Fill8(std::vector<uint8_t>& out, size_t count, uint8_t v) { out.insert(out.end(), count, v); }
void Fill16(std::vector<uint8_t>& out, size_t count, uint16_t v)
{
   for (size_t i = 0; i < count; i++)
      P16(out, v);
}

constexpr uint16_t kSerumColor = 0xf81f; // magenta, RGB565

// Serum v1: palette-indexed output, no height restriction. cframes holds
// palette index 1 pixels, cpal colors index 1 with a known RGB.
std::vector<uint8_t> MakeSerumV1(uint32_t w, uint32_t h, const std::vector<uint8_t>& romFrame, uint8_t r, uint8_t g, uint8_t b)
{
   const size_t px = static_cast<size_t>(w) * h;
   std::vector<uint8_t> out;
   Fill8(out, 64, 0); // name
   P32(out, 13 * sizeof(uint32_t)); // sizeheader: includes trigger/spriteBB/background blocks
   P32(out, w);
   P32(out, h);
   P32(out, 1); // nframes
   P32(out, 4); // nocolors
   P32(out, 4); // nccolors
   P32(out, 0);
   P32(out, 0);
   P32(out, 0); // ncompmasks, nmovmasks, nsprites
   P16(out, 0); // nbackgrounds (uint16)
   P32(out, Crc32(romFrame.data(), romFrame.size())); // hashcodes
   P8(out, 0); // shapecompmode
   P8(out, 255); // compmaskID: no mask
   P8(out, 255); // movrctID
   P8(out, 0);
   P8(out, 0);
   P8(out, 0);
   P8(out, r);
   P8(out, g);
   P8(out, b); // cpal: index 0..3
   P8(out, 0);
   P8(out, 0);
   P8(out, 0);
   P8(out, 0);
   P8(out, 0);
   P8(out, 0);
   Fill8(out, px, 1); // cframes: index 1
   Fill8(out, px, 255); // dynamasks: none
   Fill8(out, 16 * 4, 0); // dyna4cols
   Fill8(out, 32, 255); // framesprites
   P8(out, 1); // activeframes
   Fill8(out, 24, 255); // colorrotations: all inactive
   P32(out, 0xffffffff); // triggerIDs
   Fill16(out, 32 * 4, 0xffff); // framespriteBB
   P16(out, 0xffff); // backgroundIDs (uint16)
   Fill16(out, 4, 0xffff); // backgroundBB (uint16 x4)
   return out;
}

// Serum v2: RGB565 planes, base height must be 32 or 64. cframes_v2 holds
// `color` pixels everywhere.
std::vector<uint8_t> MakeSerumV2(uint32_t w, uint32_t h, const std::vector<uint8_t>& romFrame, uint16_t color = kSerumColor)
{
   const size_t px = static_cast<size_t>(w) * h;
   std::vector<uint8_t> out;
   Fill8(out, 64, 0); // name
   P32(out, 14 * sizeof(uint32_t)); // sizeheader: smallest value that selects v2
   P32(out, w);
   P32(out, h);
   P32(out, 0);
   P32(out, 0); // fwidth, fheight, no extra plane
   P32(out, 1); // nframes
   P32(out, 4); // nocolors
   P32(out, 0);
   P32(out, 0);
   P16(out, 0); // ncompmasks, nsprites (uint32), nbackgrounds (uint16)
   P32(out, Crc32(romFrame.data(), romFrame.size())); // hashcodes
   P8(out, 0); // shapecompmode
   P8(out, 255); // compmaskID: no mask
   P8(out, 0); // isextraframe
   Fill16(out, px, color); // cframes_v2
   Fill8(out, px, 255); // dynamasks: none
   Fill16(out, 32 * 4, 0); // dyna4cols_v2
   Fill16(out, 32 * 4, 0); // dyna4cols_v2_extra (read unconditionally)
   Fill8(out, 32, 255); // framesprites
   P8(out, 1); // activeframes
   Fill16(out, 64 * 4, 0); // colorrotations_v2: empty
   Fill16(out, 64 * 4, 0); // colorrotations_v2_extra: empty
   P32(out, 0xffffffff); // triggerIDs
   Fill16(out, 32 * 4, 0xffff); // framespriteBB
   P16(out, 0xffff); // backgroundIDs (uint16)
   Fill8(out, px, 0); // backgroundmask
   return out;
}

std::vector<uint8_t> Centered128x32(const std::vector<uint8_t>& frame128x16)
{
   std::vector<uint8_t> out(128 * 32, 0);
   memcpy(out.data() + 128 * 8, frame128x16.data(), 128 * 16);
   return out;
}

// --- Minimal pin2dmd .pal writer ------------------------------------------
//
// One palette, no mappings, no masks: libvni colorizes every frame with it.

void PB16(std::vector<uint8_t>& out, uint16_t v)
{
   out.push_back(static_cast<uint8_t>(v >> 8));
   out.push_back(static_cast<uint8_t>(v));
}

std::vector<uint8_t> MakePal()
{
   std::vector<uint8_t> out;
   P8(out, 4); // version
   PB16(out, 1); // one palette
   PB16(out, 0); // palette index 0
   PB16(out, 4); // four colors
   P8(out, 1); // type 1: default palette
   P8(out, 0);
   P8(out, 0);
   P8(out, 0);
   P8(out, 10);
   P8(out, 20);
   P8(out, 30);
   P8(out, 40);
   P8(out, 50);
   P8(out, 60);
   P8(out, 70);
   P8(out, 80);
   P8(out, 90);
   PB16(out, 0); // no mappings
   P8(out, 0); // no masks
   return out;
}

std::string g_serumLog;

void SerumLog(const char* format, va_list args, const void*)
{
   char buf[1024];
   vsnprintf(buf, sizeof(buf), format, args);
   g_serumLog += buf;
   g_serumLog += '\n';
}

} // namespace

void SerumTestSetup()
{
   static bool once = false;
   if (!once)
   {
      once = true;
      Serum_SetLogCallback(SerumLog, nullptr);
   }
   g_serumLog.clear();
}

TEST_CASE("Serum colorization: 128x32")
{
   SerumTestSetup();
   std::vector<uint8_t> rom(128 * 32, 3);
   std::vector<uint8_t> crom = MakeSerumV2(128, 32, rom);
   Serum_Frame_Struc* serum = Serum_LoadFromMemory(crom.data(), crom.size(), FLAG_REQUEST_32P_FRAMES | FLAG_REQUEST_64P_FRAMES);
   INFO("load error: ", std::string(Serum_GetLastErrorMessage()));
   INFO("serum log:\n", g_serumLog);
   REQUIRE(serum != nullptr);
   CHECK(serum->SerumVersion == SERUM_V2);

   std::vector<uint8_t> frame = rom;
   CHECK(Serum_Colorize(frame.data()) != IDENTIFY_NO_FRAME);
   CHECK((serum->flags & FLAG_RETURNED_32P_FRAME_OK) != 0);
   REQUIRE(serum->frame32 != nullptr);
   CHECK(serum->width32 == 128);
   CHECK(serum->frame32[0] == kSerumColor);
   CHECK(serum->frame32[128 * 32 - 1] == kSerumColor);
   Serum_Dispose();
}

TEST_CASE("Serum colorization: 128x16")
{
   SerumTestSetup();
   // A v2 colorization for a 16-row ROM is authored on 32-row frames: the 16
   // rows are stored centered between 8 blank rows top and bottom, and the
   // stored hash is over that 128x32 frame.
   std::vector<uint8_t> rom16(128 * 16, 3);
   std::vector<uint8_t> rom32 = Centered128x32(rom16);
   std::vector<uint8_t> crom = MakeSerumV2(128, 32, rom32);
   Serum_Frame_Struc* serum = Serum_LoadFromMemory(crom.data(), crom.size(), FLAG_REQUEST_32P_FRAMES | FLAG_REQUEST_64P_FRAMES);
   INFO("load error: ", std::string(Serum_GetLastErrorMessage()));
   INFO("serum log:\n", g_serumLog);
   REQUIRE(serum != nullptr);
   CHECK(serum->SerumVersion == SERUM_V2);

   // The raw 128x16 frame cannot be identified; the frame the colorization was
   // authored on can be. libserum always reads a full 128x32 frame, so the raw
   // rows are passed at the top of a zeroed 128x32 buffer, not centered.
   std::vector<uint8_t> frame = rom16;
   frame.resize(128 * 32, 0);
   CHECK(Serum_Colorize(frame.data()) == IDENTIFY_NO_FRAME);
   frame = Centered128x32(rom16);
   CHECK(Serum_Colorize(frame.data()) != IDENTIFY_NO_FRAME);
   CHECK((serum->flags & FLAG_RETURNED_32P_FRAME_OK) != 0);
   REQUIRE(serum->frame32 != nullptr);
   CHECK(serum->frame32[0] == kSerumColor);
   Serum_Dispose();
}

TEST_CASE("Serum colorization: 192x64")
{
   SerumTestSetup();
   std::vector<uint8_t> rom(192 * 64, 3);
   std::vector<uint8_t> crom = MakeSerumV2(192, 64, rom);
   Serum_Frame_Struc* serum = Serum_LoadFromMemory(crom.data(), crom.size(), FLAG_REQUEST_32P_FRAMES | FLAG_REQUEST_64P_FRAMES);
   INFO("load error: ", std::string(Serum_GetLastErrorMessage()));
   REQUIRE(serum != nullptr);
   CHECK(serum->SerumVersion == SERUM_V2);

   std::vector<uint8_t> frame = rom;
   CHECK(Serum_Colorize(frame.data()) != IDENTIFY_NO_FRAME);
   CHECK((serum->flags & FLAG_RETURNED_64P_FRAME_OK) != 0);
   REQUIRE(serum->frame64 != nullptr);
   CHECK(serum->width64 == 192);
   CHECK(serum->frame64[0] == kSerumColor);
   Serum_Dispose();
}

TEST_CASE("Serum v1 colorization: 128x16 native")
{
   SerumTestSetup();
   std::vector<uint8_t> rom(128 * 16, 3);
   std::vector<uint8_t> crom = MakeSerumV1(128, 16, rom, 10, 20, 30);
   Serum_Frame_Struc* serum = Serum_LoadFromMemory(crom.data(), crom.size(), FLAG_REQUEST_32P_FRAMES | FLAG_REQUEST_64P_FRAMES);
   INFO("load error: ", std::string(Serum_GetLastErrorMessage()));
   REQUIRE(serum != nullptr);
   CHECK(serum->SerumVersion == SERUM_V1);

   std::vector<uint8_t> frame = rom;
   CHECK(Serum_Colorize(frame.data()) != IDENTIFY_NO_FRAME);
   REQUIRE(serum->frame != nullptr);
   REQUIRE(serum->palette != nullptr);
   CHECK(serum->frame[0] == 1);
   CHECK(serum->palette[3] == 10);
   CHECK(serum->palette[4] == 20);
   CHECK(serum->palette[5] == 30);
   Serum_Dispose();
}

TEST_CASE("VNI colorization")
{
   const std::vector<uint8_t> pal = MakePal();
   const std::filesystem::path palPath = std::filesystem::temp_directory_path() / "vpx_test_pin2dmd.pal";
   {
      std::ofstream f(palPath, std::ios::binary | std::ios::trunc);
      f.write(reinterpret_cast<const char*>(pal.data()), static_cast<std::streamsize>(pal.size()));
   }
   Vni_Context* vni = Vni_LoadFromPaths(palPath.string().c_str(), nullptr, nullptr, nullptr);
   REQUIRE(vni != nullptr);

   struct Case
   {
      uint32_t w, h, outW, outH;
   };
   const Case cases[] = {
      { 128, 32, 128, 32 }, // native
      { 128, 16, 128, 32 }, // centered into a 128x32 frame by libvni
      { 192, 64, 192, 64 }, // native
   };
   for (const Case& c : cases)
   {
      std::vector<uint8_t> frame(static_cast<size_t>(c.w) * c.h, 3);
      CHECK(Vni_Colorize(vni, frame.data(), c.w, c.h, 2) != 0);
      const Vni_Frame_Struc* out = Vni_GetFrame(vni);
      REQUIRE(out != nullptr);
      REQUIRE(out->has_frame != 0);
      CHECK(out->width == c.outW);
      CHECK(out->height == c.outH);
      // first content pixel: for a 128x16 input centered in 128x32 the top 8
      // rows are padding
      CHECK(out->frame[static_cast<size_t>((c.outH - c.h) / 2) * c.outW] == 3);
      CHECK(out->frame[static_cast<size_t>(c.outW) * c.outH - 1] == (c.outH == c.h ? 3 : 0));
      // Palette index 3 is RGB(70,80,90)
      CHECK(out->palette[9] == 70);
      CHECK(out->palette[10] == 80);
      CHECK(out->palette[11] == 90);
   }

   Vni_Dispose(vni);
   std::filesystem::remove(palPath);
}
