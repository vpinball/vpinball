// license:GPLv3+

#include "core/stdafx.h"

#include <iomanip>

#include "core/vpversion.h"

namespace VPX
{

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
   vector<uint8_t> file = read_file(filename);
   return file.empty() ? nullptr : new Sound(TitleFromFilename(filename), filename, file);
}

struct WaveHeader
{
   // [Master RIFF chunk]
   DWORD dwRiff; // "RIFF"
   DWORD dwSize; // Size
   DWORD dwWave; // "WAVE"
   // [Chunk describing the data format]
   DWORD dwFmt; // "fmt "
   DWORD dwFmtSize; // Wave Format Size
   WORD wFormatTag; // format type
   WORD wNChannels; // number of channels (i.e. mono, stereo...)
   DWORD dwNSamplesPerSec; // sample rate
   DWORD dwNAvgBytesPerSec; // for buffer estimation
   WORD wNBlockAlign; // block size of data
   WORD wBitsPerSample; // number of bits per sample of mono data
   // [Chunk containing the sampled data]
   DWORD dwData; // "data"
   DWORD dwDataSize; // Sampled data size
};

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) | ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24))
#endif

Sound* Sound::CreateFromStream(IStream* pstm, const int LoadFileVersion)
{
   int32_t len;
   ULONG read;

   // Name (length, then string)
   if (FAILED(pstm->Read(&len, sizeof(int32_t), &read)))
      return nullptr;
   string name(len, '\0');
   if (FAILED(pstm->Read(name.data(), len, &read)))
      return nullptr;

   // Filename (length, then string) including path (// full filename, incl. path)
   if (FAILED(pstm->Read(&len, sizeof(len), &read)))
      return nullptr;
   string path(len, '\0');
   if (FAILED(pstm->Read(path.data(), len, &read)))
      return nullptr;

   // Was the lower case name, but not used anymore since 10.7+, 10.8+ also only stores 1,'\0'
   if (FAILED(pstm->Read(&len, sizeof(len), &read)))
      return nullptr;
   string dummy(len, '\0');
   if (FAILED(pstm->Read(dummy.data(), len, &read)))
      return nullptr;

   // Since vpinball was originally only for windows, the microsoft library import was used, which stores/converts WAVs to the waveformatex.
   // This header is stored for WAV files, identified by their filename extension, instead of the regular WAV file format.
   const bool wav = isWav(path);
   WAVEFORMATEX wfx;
   if (wav && FAILED(pstm->Read(&wfx, sizeof(wfx), &read)))
      return nullptr;

   int32_t cdata = 0;
   if (FAILED(pstm->Read(&cdata, sizeof(int32_t), &read)))
      return nullptr;

   // WAV files are stored with a special format, while others are just the raw imported file.
   // We detect and (re)create the appropriate header for WAV files so that they can be treated as other sounds.
   vector<uint8_t> data;
   if (wav)
   {
      const size_t waveFileSize = sizeof(WaveHeader) + cdata;
      data.resize(waveFileSize);
      // [Master RIFF chunk]
      auto const waveHeader = reinterpret_cast<WaveHeader*>(data.data());
      waveHeader->dwRiff = MAKEFOURCC('R', 'I', 'F', 'F');
      waveHeader->dwSize = static_cast<DWORD>(waveFileSize - 8); // File size - 8
      waveHeader->dwWave = MAKEFOURCC('W', 'A', 'V', 'E');
      // [Chunk describing the data format]
      waveHeader->dwFmt = MAKEFOURCC('f', 'm', 't', ' ');
      waveHeader->dwFmtSize = 24 - 8; // Chunk size - 8
      waveHeader->wFormatTag = wfx.wFormatTag; // Format type
      waveHeader->wNChannels = wfx.nChannels; // Number of channels (i.e. mono, stereo...)
      waveHeader->dwNSamplesPerSec = wfx.nSamplesPerSec; // Sample rate
      waveHeader->dwNAvgBytesPerSec = wfx.nAvgBytesPerSec; // For buffer estimation
      waveHeader->wNBlockAlign = wfx.nBlockAlign; // Block size of data
      waveHeader->wBitsPerSample = wfx.wBitsPerSample; // Number of bits per sample of mono data
      // [Chunk containing the sampled data]
      waveHeader->dwData = MAKEFOURCC('d', 'a', 't', 'a');
      waveHeader->dwDataSize = static_cast<DWORD>(cdata); // Sampled data size
      if (FAILED(pstm->Read(data.data() + sizeof(WaveHeader), static_cast<ULONG>(cdata), &read)))
         return nullptr;
   }
   else
   {
      data.resize(cdata);
      if (FAILED(pstm->Read(data.data(), static_cast<ULONG>(data.size()), &read)))
         return nullptr;
   }

   // this reads in the settings that are used by the Windows UI in the Sound Manager and when PlaySound() is used.
   uint8_t outputTarget = static_cast<uint8_t>(SoundOutTypes::SNDOUT_TABLE);
   int32_t volume = 100;
   int32_t pan = 100;
   int32_t frontRearFade = 100;
   if (LoadFileVersion >= NEW_SOUND_FORMAT_VERSION)
   {
      if (FAILED(pstm->Read(&outputTarget, sizeof(char), &read)))
         return nullptr;
      if (outputTarget < 0 || outputTarget > SoundOutTypes::SNDOUT_BACKGLASS)
         outputTarget = static_cast<uint8_t>(SoundOutTypes::SNDOUT_TABLE);
      if (FAILED(pstm->Read(&volume, sizeof(int32_t), &read)))
         return nullptr;
      if (FAILED(pstm->Read(&pan, sizeof(int32_t), &read)))
         return nullptr;
      if (FAILED(pstm->Read(&frontRearFade, sizeof(int32_t), &read)))
         return nullptr;
      if (FAILED(pstm->Read(&volume, sizeof(int32_t), &read)))
         return nullptr;
   }
   else
   {
      bool toBackglassOutput = false; // false: for pre-VPX tables
      if (FAILED(pstm->Read(&toBackglassOutput, sizeof(bool), &read)))
         return nullptr;
      outputTarget = (StrFindNoCase(name, "bgout_"s) != string::npos) // legacy behavior, where the BG selection was encoded into the strings directly
               || StrCompareNoCase(path, "* Backglass Output *"s) 
               || toBackglassOutput
            ? SNDOUT_BACKGLASS
            : SNDOUT_TABLE;
   }

   Sound* const pps = new Sound(name, path, data);
   pps->SetOutputTarget(static_cast<SoundOutTypes>(outputTarget));
   pps->SetVolume(volume);
   pps->SetPan(pan);
   pps->SetFrontRearFade(frontRearFade);
   return pps;
}

void Sound::SetFromFileData(string filename, vector<uint8_t> filedata)
{
   m_path = std::move(filename);
   m_data = std::move(filedata);
}

bool Sound::SaveToFile(const string& filename) const
{
   FILE* f;
   if ((fopen_s(&f, filename.c_str(), "wb") == 0) && f)
   {
      fwrite(m_data.data(), 1, m_data.size(), f);
      fclose(f);
      return true;
   }
   return false;
}

void Sound::SaveToStream(IStream* pstm) const
{
   ULONG writ = 0;
   int32_t nameLen = static_cast<int32_t>(m_name.length());
   int32_t pathLen = static_cast<int32_t>(m_path.length());
   int32_t dummyLen = 1;
   constexpr char dummyPath = '\0';
   pstm->Write(&nameLen, sizeof(int32_t), &writ);
   pstm->Write(m_name.c_str(), nameLen, &writ);
   pstm->Write(&pathLen, sizeof(int32_t), &writ);
   pstm->Write(m_path.c_str(), pathLen, &writ);
   pstm->Write(&dummyLen, sizeof(int32_t), &writ); // Used to have the same name again in lower case, now just save an empty string for backward compatibility
   pstm->Write(&dummyPath, dummyLen, &writ);
   if (isWav(m_path))
   {
      auto const waveHeader = reinterpret_cast<const WaveHeader*>(m_data.data());
      WAVEFORMATEX wfx;
      wfx.wFormatTag = waveHeader->wFormatTag;
      wfx.nChannels = waveHeader->wNChannels;
      wfx.nSamplesPerSec = waveHeader->dwNSamplesPerSec;
      wfx.nAvgBytesPerSec = waveHeader->dwNAvgBytesPerSec;
      wfx.nBlockAlign = waveHeader->wNBlockAlign;
      wfx.wBitsPerSample = waveHeader->wBitsPerSample;
      wfx.cbSize = 0;
      pstm->Write(&wfx, sizeof(WAVEFORMATEX), &writ);
      const int32_t sampleDataLength = static_cast<int32_t>(m_data.size() - sizeof(WaveHeader));
      pstm->Write(&sampleDataLength, sizeof(int32_t), &writ);
      pstm->Write(m_data.data() + sizeof(WaveHeader), static_cast<ULONG>(sampleDataLength), &writ);
   }
   else
   {
      const int32_t dataLength = static_cast<int32_t>(m_data.size());
      pstm->Write(&dataLength, sizeof(int32_t), &writ);
      pstm->Write(m_data.data(), static_cast<ULONG>(dataLength), &writ);
   }

   // Begin NEW_SOUND_FORMAT_VERSION data
   const uint8_t outputTarget = static_cast<uint8_t>(GetOutputTarget());
   pstm->Write(&outputTarget, sizeof(uint8_t), &writ);
   const int32_t volume = GetVolume();
   pstm->Write(&volume, sizeof(int32_t), &writ);
   const int32_t pan = GetPan();
   pstm->Write(&pan, sizeof(int32_t), &writ);
   const int32_t frontRearFade = GetFrontRearFade();
   pstm->Write(&frontRearFade, sizeof(int32_t), &writ);
   pstm->Write(&volume, sizeof(int32_t), &writ);
}

}
