//-----------------------------------------------------------------------------
// File: WavRead.h
//
// Desc: Support for loading and playing Wave files using DirectSound sound
//       buffers.
//
// Copyright (c) 1999 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once

#include <mmreg.h>
#include <mmsystem.h>


//-----------------------------------------------------------------------------
// Name: class CWaveSoundRead
// Desc: A class to read in sound data from a Wave file
//-----------------------------------------------------------------------------
class CWaveSoundRead
{
public:
   WAVEFORMATEX* m_pwfx;        // Pointer to WAVEFORMATEX structure
   HMMIO         m_hmmioIn;     // MM I/O handle for the WAVE
   MMCKINFO      m_ckIn;        // Multimedia RIFF chunk
   MMCKINFO      m_ckInRiff;    // Use in opening a WAVE file

   CWaveSoundRead();
   ~CWaveSoundRead();

   HRESULT Open(const CHAR* const strFilename);
   HRESULT Reset();
   HRESULT Read(UINT nSizeToRead, BYTE* pbData, UINT* pnSizeRead);
   HRESULT Close();
};
