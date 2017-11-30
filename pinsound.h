// PinSound.h: interface for the PinSound class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_PINSOUND_H__61491D0B_9950_480C_B453_911B3A2CDB8E__INCLUDED_)
#define AFX_PINSOUND_H__61491D0B_9950_480C_B453_911B3A2CDB8E__INCLUDED_

struct DSAudioDevice
{
   LPGUID guid;
   string description;
   string module;
   DSAudioDevice() {
      guid = NULL;
   }
   ~DSAudioDevice() {
      if (guid)
         delete guid;
   }
};

typedef vector<DSAudioDevice*> DSAudioDevices;

BOOL CALLBACK DSEnumCallBack(LPGUID guid, LPCSTR desc, LPCSTR mod, LPVOID list);

enum SoundOutTypes : char { SNDOUT_TABLE = 0, SNDOUT_BACKGLASS = 1 };
enum SoundConfigTypes : int { SNDCFG_SND3D2CH = 0, SNDCFG_SND3DALLREAR = 1, SNDCFG_SND3DFRONTISREAR = 2, 
                              SNDCFG_SND3DFRONTISFRONT = 3, SNDCFG_SND3D6CH = 4};

// Surround modes
// ==============
//
// 2CH:  Standard stereo output
//
// ALLREAR: All table effects shifted to rear channels.   This can replace the need to use two sound cards to move table audio
// inside the cab.  Default backglass audio and VPinMame audio plays from front speakers.
//
// FRONTISFRONT: Recommended mapping for a dedicated sound card attached to the playfield.   Front channel maps to the front
// of the cab.   We "flip" the rear to the standard 2 channels, so older versions of VP still play sounds on the front most
// channels of the cab.    This mapping could also be used to place 6 channels on the playfield. 
//
// FRONTISREAR: Table effects are mapped such that the front of the cab is the rear surround channels.   If you were to play
// VPX in a home theater system with the TV in front of you, this would produce an appropriate result with the ball coming 
// from the rear channels as it get closer to you.  
//
// 6CH: Rear of playfield shifted to the sides, and front of playfield shifted to the far rear.   Leaves front channels open
// for default backglass and VPinMame. 

class PinSoundCopy
{
public:
	PinSoundCopy(class PinSound *pOriginal);

	void Play(const float volume, const float randompitch, const int pitch, const float pan, const float front_rear_fade, const int flags, const bool restart);
	void Stop();
	HRESULT Get3DBuffer();

	LPDIRECTSOUNDBUFFER m_pDSBuffer;
	LPDIRECTSOUND3DBUFFER m_pDS3DBuffer;
	class PinSound *m_ppsOriginal;
};

class PinSound : public PinSoundCopy
{
public:
   PinSound();
   ~PinSound();

   class PinDirectSound *m_pPinDirectSound;

   char m_szName[MAXTOKEN];
   char m_szInternalName[MAXTOKEN];
   char m_szPath[MAX_PATH];

   char *m_pdata; // Copy of the buffer data so we can save it out
   WAVEFORMATEX m_wfx;

   int m_cdata;
   char m_iOutputTarget;
   int m_iBalance;
   int m_iFade;
   int m_iVolume;

   class PinDirectSound *GetPinDirectSound();
   void UnInitialize();
   void ReInitialize();
};



class PinDirectSound
{
public:
   PinDirectSound();
   ~PinDirectSound();

   void InitDirectSound(HWND hwnd, bool IsBackglass);
   static float PanTo3D(float input);

   PinSound *LoadWaveFile(TCHAR* strFileName);
   HRESULT CreateStaticBuffer(TCHAR* strFileName, PinSound *pps);
   HRESULT FillBuffer(PinSound *pps);
   HRESULT CreateDirectFromNative(PinSound *pps);
   HRESULT RestoreBuffers();

   LPDIRECTSOUND       m_pDS;
   LPDIRECTSOUND3DLISTENER m_pDSListener;
   //LPDIRECTSOUNDBUFFER m_pDSBuffer;
   CWaveSoundRead*     m_pWaveSoundRead;
   DWORD               m_dwBufferBytes;
   int m_i3DSoundMode;
};

#endif // !defined(AFX_PINSOUND_H__61491D0B_9950_480C_B453_911B3A2CDB8E__INCLUDED_)
