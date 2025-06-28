// license:GPLv3+

#include "core/stdafx.h"

#include <iomanip>

#include "core/vpversion.h"

namespace VPX
{

struct WAVEHEADER
{
   DWORD dwRiff;    // "RIFF"
   DWORD dwSize;    // Size
   DWORD dwWave;    // "WAVE"
   DWORD dwFmt;     // "fmt "
   DWORD dwFmtSize; // Wave Format Size
};


Sound::~Sound()
{
   delete[] m_pdata;
}

/**
 * @brief Loads a sound file and initializes a Sound object with its data.
 * 
 * This function attempts to load a sound file specified by the given file path (`filename`), 
 * reads its content into memory, and creates a `Sound` object with the loaded data. The sound 
 * file's name is extracted from the file path and stored, and the file's data is read into dynamically 
 * allocated memory. The function also reinitializes the sound object, and if the initialization succeeds, 
 * it returns a pointer to the new `Sound` object. If any part of the loading or initialization process fails, 
 * it returns `nullptr`.
 * 
 * @param filename The file path to the sound file to load.
 * 
 * @return Sound* A pointer to the newly loaded and initialized `Sound` object, or `nullptr` if 
 *         loading or initialization fails.
 * 
 * @note The function uses raw file handling (`fopen_s`, `fseek`, `fread_s`, and `fclose`), and expects
 *       the sound file to be in a valid format.
 */
Sound* Sound::CreateFromFile(const string& filename)
{
   FILE* f;
   if (fopen_s(&f, filename.c_str(), "rb") != 0 || !f)
   {
      ShowError("Could not open sound file.");
      return nullptr;
   }
   fseek(f, 0, SEEK_END);
   Sound* const pps = new Sound();
   pps->m_path = filename;
   pps->m_name = TitleFromFilename(filename);
   pps->m_cdata = (int)ftell(f);
   fseek(f, 0, SEEK_SET);
   pps->m_pdata = new uint8_t[pps->m_cdata];
   if (fread_s(pps->m_pdata, pps->m_cdata, 1, pps->m_cdata, f) < 1)
   {
      fclose(f);
      delete pps;
      ShowError("Could not read from sound file.");
      return nullptr;
   }
   fclose(f);
   return pps;
}

Sound* Sound::CreateFromStream(IStream* pstm, const int LoadFileVersion)
{
   int32_t len;
   ULONG read;

   // Name (length, then string)
   if (FAILED(pstm->Read(&len, sizeof(int32_t), &read)))
      return nullptr;
   Sound* const pps = new Sound();
   char* tmp = new char[len + 1];
   if (FAILED(pstm->Read(tmp, len, &read)))
   {
      delete pps;
      delete[] tmp;
      return nullptr;
   }
   tmp[len] = '\0';
   pps->m_name = tmp;
   delete[] tmp;

   // Filename (length, then string) including path (// full filename, incl. path)
   if (FAILED(pstm->Read(&len, sizeof(len), &read)))
   {
      delete pps;
      return nullptr;
   }
   tmp = new char[len + 1];
   if (FAILED(pstm->Read(tmp, len, &read)))
   {
      delete pps;
      delete[] tmp;
      return nullptr;
   }
   tmp[len] = '\0';
   pps->m_path = tmp;
   delete[] tmp;

   // Was the lower case name, but not used anymore since 10.7+, 10.8+ also only stores 1,'\0'
   if (FAILED(pstm->Read(&len, sizeof(len), &read)))
   {
      delete pps;
      return nullptr;
   }
   tmp = new char[len];
   if (FAILED(pstm->Read(tmp, len, &read)))
   {
      delete pps;
      delete[] tmp;
      return nullptr;
   }
   delete[] tmp;

   // Since vpinball was originally only for windows, the microsoft library import was used, which stores/converts WAVs
   // to the waveformatex. This header is stored for WAV files, identified by their filename extension, instead of the regular WAV file format
   const bool wav = isWav(pps->m_path);
   WAVEFORMATEX wfx;
   if (wav && FAILED(pstm->Read(&wfx, sizeof(wfx), &read)))
   {
      delete pps;
      return nullptr;
   }

   // 
   if (FAILED(pstm->Read(&pps->m_cdata, sizeof(int), &read)))
   {
      delete pps;
      return nullptr;
   }

   // Since vpinball was originally only for windows, the microsoft library import was used, which stores/converts WAVs
   // to the waveformatex.  OGG files will still have their original header.  For WAVs
   // we put the regular WAV header back on for SDL to process the file
   if (wav)
   {
      DWORD waveFileSize = sizeof(WAVEHEADER) + sizeof(WAVEFORMATEX) + wfx.cbSize + sizeof(DWORD) + sizeof(DWORD) + static_cast<DWORD>(pps->m_cdata);
      pps->m_pdata = new uint8_t[waveFileSize];
      uint8_t* waveFilePointer = pps->m_pdata;

      // Wave header (Static RIFF header)
      WAVEHEADER* const waveHeader = reinterpret_cast<WAVEHEADER*>(pps->m_pdata);
      static constexpr BYTE WaveHeader[] = { 'R', 'I', 'F', 'F', 0x00, 0x00, 0x00, 0x00, 'W', 'A', 'V', 'E', 'f', 'm', 't', ' ', 0x00, 0x00, 0x00, 0x00 };
      memcpy(waveFilePointer, WaveHeader, sizeof(WaveHeader));
      waveFilePointer += sizeof(WaveHeader);

      // Update sizes in wave header
      waveHeader->dwSize = waveFileSize - sizeof(DWORD) * 2;
      waveHeader->dwFmtSize = sizeof(WAVEFORMATEX) + wfx.cbSize;

      // WAVEFORMATEX
      memcpy(waveFilePointer, &wfx, sizeof(WAVEFORMATEX) + wfx.cbSize);
      waveFilePointer += sizeof(WAVEFORMATEX) + wfx.cbSize;

      // Data header (Static wave DATA tag followed by data size)
      static constexpr BYTE WaveData[] = { 'd', 'a', 't', 'a' };
      memcpy(waveFilePointer, WaveData, sizeof(WaveData));
      waveFilePointer += sizeof(WaveData);
      *(reinterpret_cast<DWORD*>(waveFilePointer)) = static_cast<DWORD>(pps->m_cdata);
      waveFilePointer += sizeof(DWORD);

      // Sample data
      if (FAILED(pstm->Read(waveFilePointer, static_cast<ULONG>(pps->m_cdata), &read)))
      {
         delete pps;
         return nullptr;
      }

      pps->m_cdata = waveFileSize;
   }
   else
   {
      pps->m_pdata = new uint8_t[pps->m_cdata];
      if (FAILED(pstm->Read(pps->m_pdata, static_cast<ULONG>(pps->m_cdata), &read)))
      {
         delete pps;
         return nullptr;
      }
   }

   // this reads in the settings that are used by the Windows UI in the Sound Manager and when PlaySound() is used.
   if (LoadFileVersion >= NEW_SOUND_FORMAT_VERSION)
   {
      SoundOutTypes outputTarget = SoundOutTypes::SNDOUT_TABLE;
      if (FAILED(pstm->Read(&outputTarget, sizeof(char), &read)))
      {
         delete pps;
         return nullptr;
      }
      if (outputTarget < 0 || outputTarget > SoundOutTypes::SNDOUT_BACKGLASS)
         outputTarget = SoundOutTypes::SNDOUT_TABLE;
      pps->SetOutputTarget(outputTarget);
      int volume;
      if (FAILED(pstm->Read(&volume, sizeof(int), &read)))
      {
         delete pps;
         return nullptr;
      }
      pps->SetVolume(volume);
      int pan;
      if (FAILED(pstm->Read(&pan, sizeof(int), &read)))
      {
         delete pps;
         return nullptr;
      }
      pps->SetPan(pan);
      int frontRearFade;
      if (FAILED(pstm->Read(&frontRearFade, sizeof(int), &read)))
      {
         delete pps;
         return nullptr;
      }
      pps->SetFrontRearFade(frontRearFade);
      if (FAILED(pstm->Read(&volume, sizeof(int), &read)))
      {
         delete pps;
         return nullptr;
      }
      pps->SetVolume(volume);
   }
   else
   {
      bool toBackglassOutput = false; // false: for pre-VPX tables
      if (FAILED(pstm->Read(&toBackglassOutput, sizeof(bool), &read)))
      {
         delete pps;
         return nullptr;
      }

      pps->SetOutputTarget((StrStrI(pps->m_name.c_str(), "bgout_") != nullptr)
               || StrCompareNoCase(pps->m_path, "* Backglass Output *"s) // legacy behavior, where the BG selection was encoded into the strings directly
               || toBackglassOutput
            ? SNDOUT_BACKGLASS
            : SNDOUT_TABLE);
   }
   return pps;
}

void Sound::SetFromFileData(const string& filename, uint8_t* data, size_t size)
{
   m_path = filename;
   delete[] m_pdata;
   m_pdata = data;
   m_cdata = size;
}

bool Sound::SaveToFile(const string& filename) const
{
   FILE* f;
   if ((fopen_s(&f, filename.c_str(), "wb") == 0) && f)
   {
      fwrite(m_pdata, 1, m_cdata, f);
      fclose(f);
      return true;
   }
   return false;
}

void Sound::SaveToStream(IStream* pstm) const
{
   ULONG writ = 0;
   int32_t nameLen = (int32_t)m_name.length();
   int32_t pathLen = (int32_t)m_path.length();
   int32_t dummyLen = 1;
   constexpr char dummyPath = '\0';
   pstm->Write(&nameLen, sizeof(int32_t), &writ);
   pstm->Write(m_name.c_str(), nameLen, &writ);
   pstm->Write(&pathLen, sizeof(int32_t), &writ);
   pstm->Write(m_path.c_str(), pathLen, &writ);
   pstm->Write(&dummyLen, sizeof(int32_t), &writ); // Used to have the same name again in lower case, now just save an empty string for backward compatibility
   pstm->Write(&dummyPath, dummyLen, &writ);
   int32_t dataLength = static_cast<int32_t>(m_cdata);
   if (isWav(m_path))
   {
      uint8_t* pData = m_pdata;
      pData += sizeof(WAVEHEADER); // skip wave header
      WAVEFORMATEX* wfx = (WAVEFORMATEX*)pData;
      pstm->Write(wfx, sizeof(WAVEFORMATEX) + wfx->cbSize, &writ); // Save WAVEFORMATEX with its optional data block
      pData += 2 * sizeof(DWORD); // skip wave data and length
      dataLength -= static_cast<int32_t>(pData - m_pdata); // Data block does not include the WAV header
      pstm->Write(&dataLength, sizeof(int32_t), &writ);
      pstm->Write(m_pdata, static_cast<ULONG>(dataLength), &writ);
   }
   else
   {
      pstm->Write(&dataLength, sizeof(int32_t), &writ);
      pstm->Write(m_pdata, static_cast<ULONG>(dataLength), &writ);
   }
   const char outputTarget = (char)GetOutputTarget();
   pstm->Write(&outputTarget, sizeof(char), &writ);

   // Begin NEW_SOUND_FORMAT_VERSION data
   const int32_t volume = GetVolume();
   pstm->Write(&volume, sizeof(int32_t), &writ);
   const int32_t pan = GetPan();
   pstm->Write(&pan, sizeof(int32_t), &writ);
   const int32_t frontRearFade = GetFrontRearFade();
   pstm->Write(&frontRearFade, sizeof(int32_t), &writ);
   pstm->Write(&volume, sizeof(int32_t), &writ);
}

}