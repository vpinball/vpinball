// license:GPLv3+

#include "common.h"

#include <algorithm>
#include <filesystem>
#include <charconv>
#include <cmath>
#include <climits>
#include <bit>

#include <cstddef> // for size_t, ptrdiff_t
// Define ssize_t for Windows
#if defined(_WIN32) && !defined(__SSIZE_T_DEFINED)
#if defined(_WIN64)
typedef __int64 ssize_t;
#else
typedef int ssize_t;
#endif
#define __SSIZE_T_DEFINED
#endif

#include "base64.h"

#include "plugins/PluginStrings.h"

namespace B2S
{

#ifndef __clang__
#define double_as_int64(x) std::bit_cast<int64_t>(x)
#else // for whatever reason apple/clang is special again
#define double_as_int64(x) __builtin_bit_cast(int64_t, x)
#endif
static constexpr bool infNaN(const double a) { return ((double_as_int64(a) & 0x7FF0000000000000ULL) == 0x7FF0000000000000ULL); }

string trim_string(const string& str)
{
   size_t start = 0;
   size_t end = str.length();
   while (start < end && (str[start] == ' ' || str[start] == '\t' || str[start] == '\r' || str[start] == '\n'))
      ++start;
   while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t' || str[end - 1] == '\r' || str[end - 1] == '\n'))
      --end;
   return str.substr(start, end - start);
}

string string_to_lower(string str)
{
   std::ranges::transform(str.begin(), str.end(), str.begin(), cLower);
   return str;
}

std::filesystem::path find_case_insensitive_file_path(const std::filesystem::path& searchedFile)
{
   const std::filesystem::path found = PluginStrings::FindPathNoCase(searchedFile, false);
   if (std::error_code ec; !found.empty() && !std::filesystem::exists(searchedFile, ec))
   {
      LOGI(std::format("Case insensitive file match: requested \"{}\", actual \"{}\"", PluginStrings::PathToUTF8(searchedFile), PluginStrings::PathToUTF8(found)));
   }
   return found;
}

// Wraps up https://github.com/czkz/base64 public domain decoder (plus extensions/optimizations)
vector<uint8_t> base64_decode(const char * const __restrict value, const size_t size_bytes)
{
   vector<uint8_t> ret(size_bytes);

   // First remove any newlines or carriage returns from the input
   uint8_t* __restrict dst = ret.data();
   for (size_t i = 0; i < size_bytes; ++i)
   {
      const char c = value[i];
      if (c != '\r' && c != '\n')
         *dst++ = c;
   }

   const size_t newLen = from_base64_inplace(ret.data(), dst - ret.data());
   ret.resize(newLen);
   return ret; // will be moved
}

// trims leading whitespace or similar, this is needed as e.g. B2S reels feature leading whitespace(s)
int string_to_int(const string& str, int defaultValue)
{
   int result;
   return is_string_numeric(str, &result) ? result : defaultValue;
}

bool is_string_numeric(const string& str, int* const __restrict result)
{
   const string tmp = trim_string(str);
   if (tmp.empty())
      return false;
   char* end = nullptr;
   const double valued = std::nearbyint(std::strtod(tmp.c_str(), &end));
   if (infNaN(valued) || valued < static_cast<double>(INT_MIN) || valued > static_cast<double>(INT_MAX))
      return false;
   const int valuei = static_cast<int>(valued);
   if (result)
      *result = valuei;
   return end == tmp.c_str() + tmp.length();
}

bool DecodeWav(const vector<uint8_t>& wav, WavData& out)
{
   if (wav.size() < 12 || memcmp(wav.data(), "RIFF", 4) != 0 || memcmp(wav.data() + 8, "WAVE", 4) != 0)
      return false;

   const auto read16 = [&wav](const size_t pos) { return static_cast<uint16_t>(wav[pos] | (wav[pos + 1] << 8)); };
   const auto read32 = [&wav](const size_t pos) { return static_cast<uint32_t>(wav[pos] | (wav[pos + 1] << 8) | (wav[pos + 2] << 16) | (wav[pos + 3] << 24)); };

   uint16_t format = 0, channels = 0, bitsPerSample = 0;
   uint32_t sampleRate = 0;
   const uint8_t* data = nullptr;
   size_t dataSize = 0;
   for (size_t pos = 12; pos + 8 <= wav.size();)
   {
      const uint32_t chunkSize = read32(pos + 4);
      const size_t payload = pos + 8;
      if (payload + chunkSize > wav.size())
         return false;
      if (memcmp(wav.data() + pos, "fmt ", 4) == 0 && chunkSize >= 16)
      {
         format = read16(payload);
         channels = read16(payload + 2);
         sampleRate = read32(payload + 4);
         bitsPerSample = read16(payload + 14);
      }
      else if (memcmp(wav.data() + pos, "data", 4) == 0)
      {
         data = wav.data() + payload;
         dataSize = chunkSize;
      }
      pos = payload + chunkSize + (chunkSize & 1);
   }
   if (data == nullptr || format == 0 || channels == 0 || channels > 2 || sampleRate == 0)
      return false;

   const size_t bytesPerSample = (bitsPerSample + 7) / 8;
   const size_t nSamples = dataSize / bytesPerSample;
   if (format == 1 && bitsPerSample == 16)
   {
      out.isFloat = false;
      out.pcm.assign(data, data + dataSize - (dataSize % 2));
   }
   else if (format == 1 && bitsPerSample == 8)
   {
      out.isFloat = false;
      out.pcm.resize(nSamples * 2);
      int16_t* const dst = reinterpret_cast<int16_t*>(out.pcm.data());
      for (size_t i = 0; i < nSamples; i++)
         dst[i] = static_cast<int16_t>((static_cast<int>(data[i]) - 128) << 8);
   }
   else if (format == 1 && bitsPerSample == 24)
   {
      out.isFloat = false;
      out.pcm.resize(nSamples * 2);
      int16_t* const dst = reinterpret_cast<int16_t*>(out.pcm.data());
      for (size_t i = 0; i < nSamples; i++)
         dst[i] = static_cast<int16_t>(data[i * 3 + 1] | (data[i * 3 + 2] << 8));
   }
   else if (format == 1 && bitsPerSample == 32)
   {
      out.isFloat = false;
      out.pcm.resize(nSamples * 2);
      int16_t* const dst = reinterpret_cast<int16_t*>(out.pcm.data());
      for (size_t i = 0; i < nSamples; i++)
         dst[i] = static_cast<int16_t>(data[i * 4 + 2] | (data[i * 4 + 3] << 8));
   }
   else if (format == 3 && bitsPerSample == 32)
   {
      out.isFloat = true;
      out.pcm.assign(data, data + dataSize - (dataSize % 4));
   }
   else
      return false;

   out.channels = channels;
   out.sampleRate = static_cast<double>(sampleRate);
   return true;
}

int B2SAnimationSlowDown(const string& list, const string& name)
{
   size_t pos = 0;
   while (pos < list.size())
   {
      const size_t end = list.find(';', pos);
      const string entry = trim_string(list.substr(pos, end == string::npos ? string::npos : end - pos));
      if (const size_t eq = entry.find('='); eq != string::npos && trim_string(entry.substr(0, eq)) == name)
         return std::max(1, string_to_int(trim_string(entry.substr(eq + 1)), 1));
      pos = end == string::npos ? list.size() : end + 1;
   }
   return 1;
}
}
