// license:GPLv3+

#pragma once

#include <mmreg.h>
#include <mmsystem.h>

// A class to read in sound data from a Wave file
class CWaveSoundRead final
{
public:
   WAVEFORMATEX* m_pwfx;        // Pointer to WAVEFORMATEX structure
   HMMIO         m_hmmioIn;     // MM I/O handle for the WAVE
   MMCKINFO      m_ckIn;        // Multimedia RIFF chunk
   MMCKINFO      m_ckInRiff;    // Use in opening a WAVE file

   CWaveSoundRead();
   ~CWaveSoundRead();

   HRESULT Open(const string& strFilename);
   HRESULT Reset();
   HRESULT Read(UINT nSizeToRead, BYTE* pbData, UINT* pnSizeRead);
   HRESULT Close();
};
