// license:GPLv3+

#include "core/stdafx.h"

#include "TournamentFile.h"

#ifndef __STANDALONE__
#include "FreeImage.h"
#else
#include "standalone/FreeImage.h"
#endif

namespace VPX::TournamentFile
{

static constexpr uint8_t lookupRev[16] = { 0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe, 0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf };

constexpr inline uint8_t reverse(const uint8_t n) { return (lookupRev[n & 0x0f] << 4) | lookupRev[n >> 4]; }

static unsigned int GenerateTournamentFileInternal(PinTable* table, uint8_t *const dmd_data, const unsigned int dmd_size, const std::filesystem::path &tablefile, unsigned int &tablefileChecksum, unsigned int &vpxChecksum, unsigned int &scriptsChecksum)
{
   tablefileChecksum = vpxChecksum = scriptsChecksum = 0;
   unsigned int dmd_data_c = 0;

   FILE *f;
   if (fopen_s(&f, tablefile.string().c_str(), "rb") == 0 && f)
   {
      uint8_t tmp[4096];
      size_t r;
      while ((r = fread(tmp, 1, sizeof(tmp), f))) //!! also include MD5 at end?
      {
         for (unsigned int i = 0; i < r; ++i)
         {
            dmd_data[dmd_data_c++] ^= reverse(tmp[i]);
            if (dmd_data_c == dmd_size)
               dmd_data_c = 0;
         }

         uint32_t md5[4];
         generateMD5(tmp, r, (uint8_t *)md5);
         for (unsigned int i = 0; i < 4; ++i)
            tablefileChecksum ^= md5[i];
      }
      fclose(f);
   }
   else
   {
      if (g_pplayer && g_pplayer->m_liveUI)
         g_pplayer->m_liveUI->PushNotification("Cannot open Table"s, 4000);
      else
         ShowError("Cannot open Table");
      return ~0u;
   }

   //

   const size_t cchar = table->m_original_table_script.size();
   char *const szText = new char[cchar + 1];
   strncpy_s(szText, cchar + 1, table->m_original_table_script.data());

   for (size_t i = 0; i < cchar; ++i)
      szText[i] = cLower(szText[i]);

   const char *textPos = szText;
   const char *const textEnd = szText + cchar;
   vector<string> vbsFiles;
   while (textPos < textEnd)
   {
      const char *const textFound = strstr(textPos, ".vbs\"");
      if (textFound == nullptr)
         break;
      textPos = textFound + 1;

      const char *textFoundStart = textFound;
      while (*textFoundStart != '"')
         --textFoundStart;

      vbsFiles.emplace_back(textFoundStart + 1, textFound + 3 - textFoundStart);
   }

   delete[] szText;

   vbsFiles.push_back("core.vbs"s);

   for (const auto &i3 : vbsFiles)
      if (fopen_s(&f, g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Scripts, i3).string().c_str(), "rb") == 0 && f)
      {
         uint8_t tmp[4096];
         size_t r;
         while ((r = fread(tmp, 1, sizeof(tmp), f))) //!! also include MD5 at end?
         {
            for (unsigned int i = 0; i < r; ++i)
            {
               dmd_data[dmd_data_c++] ^= reverse(tmp[i]);
               if (dmd_data_c == dmd_size)
                  dmd_data_c = 0;
            }

            uint32_t md5[4];
            generateMD5(tmp, r, (uint8_t *)md5);
            for (unsigned int i = 0; i < 4; ++i)
               scriptsChecksum ^= md5[i];
         }
         fclose(f);
      }

   //

#ifdef _MSC_VER
   const string strpath = GetExecutablePath();
   const char *const path = strpath.c_str();
#elif defined(__APPLE__) //!! ??
   const char *const path = SDL_GetBasePath();
#else
   char path[MAXSTRING];
   const ssize_t len = ::readlink("/proc/self/exe", path, sizeof(path) - 1);
   if (len != -1)
      path[len] = '\0';
#endif
   if (fopen_s(&f, path, "rb") == 0 && f)
   {
      uint8_t tmp[4096];
      size_t r;
      while ((r = fread(tmp, 1, sizeof(tmp), f))) //!! also include MD5 at end?
      {
         for (unsigned int i = 0; i < r; ++i)
         {
            dmd_data[dmd_data_c++] ^= reverse(tmp[i]);
            if (dmd_data_c == dmd_size)
               dmd_data_c = 0;
         }

         uint32_t md5[4];
         generateMD5(tmp, r, (uint8_t *)md5);
         for (unsigned int i = 0; i < 4; ++i)
            vpxChecksum ^= md5[i];
      }
      fclose(f);
   }
   else
   {
      if (g_pplayer && g_pplayer->m_liveUI)
         g_pplayer->m_liveUI->PushNotification("Cannot open Executable"s, 4000);
      else
         ShowError("Cannot open Executable");
      return ~0u;
   }

#if defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__)           \
   || defined(__x86_64__)
   _mm_sfence();
#else // for now arm only
   __atomic_thread_fence(__ATOMIC_SEQ_CST);
#endif

   return dmd_data_c;
}

static void GenerateTournamentFileInternal2(uint8_t *const dmd_data, const unsigned int dmd_size, unsigned int dmd_data_c)
{
   uint8_t *fs = (uint8_t *)&GenerateTournamentFileInternal;
   uint8_t *fe = fs;
   while (true)
   {
#if defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__)           \
   || defined(__x86_64__)
      if (fe[0] == 0x0F && fe[1] == 0xAE && fe[2] == 0xF8)
#else // for now arm only
      if (fe[0] == 0xD5 && fe[1] == 0x03 && fe[2] == 0x3B && fe[3] == 0xBF)
#endif
         break;
      fe++;
   } //!! also include MD5 ?

   while (fs < fe)
   {
      dmd_data[dmd_data_c++] ^= reverse(*fs);
      fs++;
      if (dmd_data_c == dmd_size)
         dmd_data_c = 0;
   }
}

void GenerateTournamentFile()
{
   assert(g_pplayer);
   unsigned int dmd_size = g_pplayer->m_dmdSize.x * g_pplayer->m_dmdSize.y;
   if (dmd_size == 0)
   {
      g_pplayer->m_liveUI->PushNotification("Tournament file export requires a valid DMD script connection to PinMAME via 'UseVPM(Colored)DMD = True'"s, 4000);
      return;
   }

   uint8_t *const dmd_data = new uint8_t[dmd_size + 16];
   if (g_pplayer->m_dmdFrame->m_format == BaseTexture::BW)
      memcpy(dmd_data, g_pplayer->m_dmdFrame->data(), dmd_size);
   else if (g_pplayer->m_dmdFrame->m_format == BaseTexture::RGBA)
   {
      const uint32_t *const __restrict data = (uint32_t *)g_pplayer->m_dmdFrame->data();
      for (unsigned int i = 0; i < dmd_size; ++i)
         dmd_data[i] = ((data[i] & 0xFF) + ((data[i] >> 8) & 0xFF) + ((data[i] >> 16) & 0xFF)) / 3;
   }
   generateMD5(dmd_data, dmd_size, dmd_data + dmd_size);
   dmd_size += 16;
   unsigned int tablefileChecksum, vpxChecksum, scriptsChecksum;
   const unsigned int res = GenerateTournamentFileInternal(g_pplayer->m_ptable, dmd_data, dmd_size, g_pplayer->m_ptable->m_filename, tablefileChecksum, vpxChecksum, scriptsChecksum);
   if (res == ~0u)
      return;
   GenerateTournamentFileInternal2(dmd_data, dmd_size, res);

   FILE *f;
   if (fopen_s(&f, (g_pplayer->m_ptable->m_filename.string() + ".txt").c_str(), "w") == 0 && f)
   {
      fprintf(f, "%03X", g_pplayer->m_dmdSize.x);
      fprintf(f, "%03X", g_pplayer->m_dmdSize.y);
      fprintf(f, "%01X", GET_PLATFORM_CPU_ENUM);
      fprintf(f, "%01X", GET_PLATFORM_BITS_ENUM);
      fprintf(f, "%01X", GET_PLATFORM_OS_ENUM);
      fprintf(f, "%01X", GET_PLATFORM_RENDERER_ENUM);
      fprintf(f, "%01X", VP_VERSION_MAJOR);
      fprintf(f, "%01X", VP_VERSION_MINOR);
      fprintf(f, "%01X", VP_VERSION_REV);
      fprintf(f, "%04X", GIT_REVISION);
      fprintf(f, "%08X", tablefileChecksum);
      fprintf(f, "%08X", vpxChecksum);
      fprintf(f, "%08X", scriptsChecksum);
      for (unsigned int i = 0; i < dmd_size; ++i)
         fprintf(f, "%02X", dmd_data[i]);
      fclose(f);

      g_pplayer->m_liveUI->PushNotification("Tournament file saved as " + g_pplayer->m_ptable->m_filename.string() + ".txt", 4000);
   }
   else
      g_pplayer->m_liveUI->PushNotification("Cannot save Tournament file"s, 4000);

   delete[] dmd_data;
}

void GenerateImageFromTournamentFile(PinTable* table, const std::filesystem::path &txtfile)
{
   unsigned int x = 0, y = 0, dmd_size = 0, cpu = 0, bits = 0, os = 0, renderer = 0, major = 0, minor = 0, rev = 0, git_rev = 0;
   unsigned int tablefileChecksum_in = 0, vpxChecksum_in = 0, scriptsChecksum_in = 0;
   vector<uint8_t> dmd_data;
   FILE *f;
   if (fopen_s(&f, txtfile.string().c_str(), "r") == 0 && f)
   {
      bool error = false;
      error |= fscanf_s(f, "%03X", &x) != 1;
      error |= fscanf_s(f, "%03X", &y) != 1;
      error |= fscanf_s(f, "%01X", &cpu) != 1;
      error |= fscanf_s(f, "%01X", &bits) != 1;
      error |= fscanf_s(f, "%01X", &os) != 1;
      error |= fscanf_s(f, "%01X", &renderer) != 1;
      error |= fscanf_s(f, "%01X", &major) != 1;
      error |= fscanf_s(f, "%01X", &minor) != 1;
      error |= fscanf_s(f, "%01X", &rev) != 1;
      error |= fscanf_s(f, "%04X", &git_rev) != 1;
      error |= fscanf_s(f, "%08X", &tablefileChecksum_in) != 1;
      error |= fscanf_s(f, "%08X", &vpxChecksum_in) != 1;
      error |= fscanf_s(f, "%08X", &scriptsChecksum_in) != 1;
      dmd_size = x * y + 16;
      dmd_data.resize(dmd_size);
      for (unsigned int i = 0; i < dmd_size; ++i)
      {
         unsigned int v;
         error |= fscanf_s(f, "%02X", &v) != 1;
         dmd_data[i] = v;
      }
      fclose(f);

      if (error)
      {
         ShowError("Error parsing Tournament file");
         return;
      }
   }
   else
   {
      ShowError("Cannot open Tournament file");
      return;
   }

   if (cpu != GET_PLATFORM_CPU_ENUM || bits != GET_PLATFORM_BITS_ENUM || os != GET_PLATFORM_OS_ENUM || renderer != GET_PLATFORM_RENDERER_ENUM)
   {
      ShowError("Cannot decode Tournament file\nas the setup differs:\nEncoder: " + platform_cpu[cpu] + ' ' + platform_bits[bits] + "bits " + platform_os[os] + ' '
         + platform_renderer[renderer] + "\nDecoder: " + platform_cpu[GET_PLATFORM_CPU_ENUM] + ' ' + platform_bits[GET_PLATFORM_BITS_ENUM] + "bits " + platform_os[GET_PLATFORM_OS_ENUM] + ' '
         + platform_renderer[GET_PLATFORM_RENDERER_ENUM]);
      return;
   }

   if (major != VP_VERSION_MAJOR || minor != VP_VERSION_MINOR || rev != VP_VERSION_REV || git_rev != GIT_REVISION)
   {
      ShowError("Cannot decode Tournament file\nas the VP version differs:\nEncoder: " + std::to_string(major) + '.' + std::to_string(minor) + '.' + std::to_string(rev) + " rev. "
         + std::to_string(git_rev) + "\nDecoder: " + std::to_string(VP_VERSION_MAJOR) + '.' + std::to_string(VP_VERSION_MINOR) + '.' + std::to_string(VP_VERSION_REV) + " rev. "
         + std::to_string(GIT_REVISION));
      return;
   }

   unsigned int tablefileChecksum, vpxChecksum, scriptsChecksum;
   const unsigned int res = GenerateTournamentFileInternal(table, dmd_data.data(), dmd_size, table->m_filename, tablefileChecksum, vpxChecksum, scriptsChecksum);
   if (res == ~0u)
   {
      return;
   }
   if (tablefileChecksum != tablefileChecksum_in)
   {
      ShowError("Cannot decode Tournament file\nas the table version differs"s);
      return;
   }
   if (vpxChecksum != vpxChecksum_in)
   {
      ShowError("Cannot decode Tournament file\nas VP was modified"s);
      return;
   }
   if (scriptsChecksum != scriptsChecksum_in)
   {
      ShowError("Cannot decode Tournament file\nas scripts version differs"s);
      return;
   }
   GenerateTournamentFileInternal2(dmd_data.data(), dmd_size, res);
   uint8_t md5[16];
   generateMD5(dmd_data.data(), dmd_size - 16, md5);
   if (memcmp(dmd_data.data() + (dmd_size - 16), md5, 16) != 0)
   {
      ShowError("Corrupt Tournament file or non-matching table-version or modified VP used to encode");
      return;
   }

   FIBITMAP *dib = FreeImage_Allocate(x, y, 8);
   BYTE *const pdst = FreeImage_GetBits(dib);
   //const unsigned int pitch_dst = FreeImage_GetPitch(dib); //!! needed?
   for (unsigned int j = 0; j < y; j++)
      for (unsigned int i = 0; i < x; i++)
         pdst[i + (y - 1 - j) * x] = dmd_data[i + j * x]; // flip y-axis for image output
   if (!FreeImage_Save(FIF_PNG, dib, (txtfile.string() + ".png").c_str(), PNG_Z_BEST_COMPRESSION))
      ShowError("Tournament file converted image could not be saved");
   FreeImage_Unload(dib);
}

};