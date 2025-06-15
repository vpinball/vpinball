// license:GPLv3+

#include "core/stdafx.h"

#include <iomanip>

#include "core/vpversion.h"


PinSound::~PinSound()
{
   delete[] m_pdata;
}

/**
 * @brief Loads a sound file and initializes a PinSound object with its data.
 * 
 * This function attempts to load a sound file specified by the given file path (`filename`), 
 * reads its content into memory, and creates a `PinSound` object with the loaded data. The sound 
 * file's name is extracted from the file path and stored, and the file's data is read into dynamically 
 * allocated memory. The function also reinitializes the sound object, and if the initialization succeeds, 
 * it returns a pointer to the new `PinSound` object. If any part of the loading or initialization process fails, 
 * it returns `nullptr`.
 * 
 * @param filename The file path to the sound file to load.
 * 
 * @return PinSound* A pointer to the newly loaded and initialized `PinSound` object, or `nullptr` if 
 *         loading or initialization fails.
 * 
 * @note Windows UI?  Load sound into Sound Resource Manager?
 * 
 * @note The function uses raw file handling (`fopen_s`, `fseek`, `fread_s`, and `fclose`), and expects
 *       the sound file to be in a valid format that can be processed by `ReInitialize()`.
 */
PinSound* PinSound::CreateFromFile(const string& filename)
{
   PinSound * const pps = new PinSound();
   pps->m_path = filename;
   pps->m_name = TitleFromFilename(filename);
   FILE *f;
   if (fopen_s(&f, filename.c_str(), "rb") != 0 || !f) {
      ShowError("Could not open sound file.");
      return nullptr;
   }
   fseek(f, 0, SEEK_END);
   pps->m_cdata = (int)ftell(f);
   fseek(f, 0, SEEK_SET);
   pps->m_pdata = new uint8_t[pps->m_cdata];
   if (fread_s(pps->m_pdata, pps->m_cdata, 1, pps->m_cdata, f) < 1) {
      fclose(f);
      ShowError("Could not read from sound file.");
      return nullptr;
   }
   fclose(f);
   return pps;
}

PinSound* PinSound::CreateFromStream(IStream* pstm, const int LoadFileVersion)
{
   int len;
   ULONG read;

   PinSound * const pps = new PinSound();

   // get length of filename
   if (FAILED(pstm->Read(&len, sizeof(len), &read)))
      return nullptr;

   // read in filename (only filename, no ext)
   char* tmp = new char[len+1];
   if (FAILED(pstm->Read(tmp, len, &read)))
   {
      delete pps;
      return nullptr;
   }
   tmp[len] = '\0';
   pps->m_name = tmp;
   delete[] tmp;

   // get length of filename including path (// full filename, incl. path)
   if (FAILED(pstm->Read(&len, sizeof(len), &read)))
   {
      delete pps;
      return nullptr;
   }

   //read in filename including path (// full filename, incl. path)
   tmp = new char[len+1];
   if (FAILED(pstm->Read(tmp, len, &read)))
   {
      delete pps;
      return nullptr;
   }
   tmp[len] = '\0';
   pps->m_path = tmp;
   delete[] tmp;

   // was the lower case name, but not used anymore since 10.7+, 10.8+ also only stores 1,'\0'
   if (FAILED(pstm->Read(&len, sizeof(len), &read)))
   {
      delete pps;
      return nullptr;
   }
   tmp = new char[len];
   if (FAILED(pstm->Read(tmp, len, &read)))
   {
      delete[] tmp;
      delete pps;
      return nullptr;
   }
   delete[] tmp;

   const bool wav = isWav(pps->m_path);

   if (wav && FAILED(pstm->Read(&pps->m_wfx, sizeof(pps->m_wfx), &read)))
   {
      delete pps;
      return nullptr;
   }

   if (FAILED(pstm->Read(&pps->m_cdata_org, sizeof(int), &read)))
   {
      delete pps;
      return nullptr;
   }
   pps->m_cdata = pps->m_cdata_org;

   // Since vpinball was originally only for windows, the microsoft library import was used, which stores/converts WAVs
   // to the waveformatex.  OGG files will still have their original header.  For WAVs
   // we put the regular WAV header back on for SDL to process the file
   if (wav)
   {
      struct WAVEHEADER
      {
         DWORD   dwRiff;    // "RIFF"
         DWORD   dwSize;    // Size
         DWORD   dwWave;    // "WAVE"
         DWORD   dwFmt;     // "fmt "
         DWORD   dwFmtSize; // Wave Format Size
      };
      // Static RIFF header
      static constexpr BYTE WaveHeader[] =
      {
         'R','I','F','F',0x00,0x00,0x00,0x00,'W','A','V','E','f','m','t',' ',0x00,0x00,0x00,0x00
      };
      // Static wave DATA tag
      static constexpr BYTE WaveData[] = { 'd','a','t','a' };

      DWORD waveFileSize;
      uint8_t* waveFilePointer;

      waveFileSize = sizeof(WAVEHEADER) + sizeof(WAVEFORMATEX) + pps->m_wfx.cbSize + sizeof(WaveData) + sizeof(DWORD) + pps->m_cdata_org;
      pps->m_pdata = new uint8_t[waveFileSize];
      waveFilePointer = pps->m_pdata;
      WAVEHEADER * const waveHeader = reinterpret_cast<WAVEHEADER *>(pps->m_pdata);

      // Wave header
      memcpy(waveFilePointer, WaveHeader, sizeof(WaveHeader));
      waveFilePointer += sizeof(WaveHeader);

      // Update sizes in wave header
      waveHeader->dwSize = waveFileSize - sizeof(DWORD) * 2;
      waveHeader->dwFmtSize = sizeof(WAVEFORMATEX) + pps->m_wfx.cbSize;

      // WAVEFORMATEX
      memcpy(waveFilePointer, &pps->m_wfx, sizeof(WAVEFORMATEX) + pps->m_wfx.cbSize);
      waveFilePointer += sizeof(WAVEFORMATEX) + pps->m_wfx.cbSize;

      // Data header
      memcpy(waveFilePointer, WaveData, sizeof(WaveData));
      waveFilePointer += sizeof(WaveData);
      *(reinterpret_cast<DWORD *>(waveFilePointer)) = pps->m_cdata_org;
      waveFilePointer += sizeof(DWORD);

      pps->m_pdata_org = waveFilePointer;
      pps->m_cdata = waveFileSize;
   }
   else
      pps->m_pdata = pps->m_pdata_org = new uint8_t[pps->m_cdata];

   if (FAILED(pstm->Read(pps->m_pdata_org, pps->m_cdata_org, &read)))
   {
      delete pps;
      return nullptr;
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
                        || toBackglassOutput ? SNDOUT_BACKGLASS : SNDOUT_TABLE);
   }
   return pps;
}

void PinSound::SetFromFileData(const string& filename, uint8_t* data, size_t size)
{
   m_path = filename;
   delete[] m_pdata;
   m_pdata = data;
   m_cdata = size;
}

bool PinSound::SaveToFile(const string& filename) const
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

void PinSound::SaveToStream(IStream* pstm) const
{
   ULONG writ = 0;
   int nameLen = (int)m_name.length();
   int pathLen = (int)m_path.length();
   int dummyLen = 1;
   constexpr char dummyPath = '\0';
   pstm->Write(&nameLen, sizeof(int), &writ);
   pstm->Write(m_name.c_str(), nameLen, &writ);
   pstm->Write(&pathLen, sizeof(int), &writ);
   pstm->Write(m_path.c_str(), pathLen, &writ);
   pstm->Write(&dummyLen, sizeof(int), &writ); // Used to have the same name again in lower case, now just save an empty string for backward compatibility
   pstm->Write(&dummyPath, dummyLen, &writ);
   if (isWav(m_path))
      pstm->Write(&m_wfx, sizeof(m_wfx), &writ);
   pstm->Write(&m_cdata_org, sizeof(int), &writ);
   pstm->Write(m_pdata_org, m_cdata_org, &writ);
   const SoundOutTypes outputTarget = GetOutputTarget();
   pstm->Write(&outputTarget, sizeof(bool), &writ);

   // Begin NEW_SOUND_FORMAT_VERSION data
   const int volume = GetVolume();
   pstm->Write(&volume, sizeof(int), &writ);
   const int pan = GetPan();
   pstm->Write(&pan, sizeof(int), &writ);
   const int frontRearFade = GetFrontRearFade();
   pstm->Write(&frontRearFade, sizeof(int), &writ);
   pstm->Write(&volume, sizeof(int), &writ);
}
