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

class PinDirectSoundWavCopy
{
public:
   PinDirectSoundWavCopy(class PinSound * const pOriginal);

protected:
   void TestPlayInternal() { m_pDSBuffer->Play(0, 0, 0); }
   void StopInternal() { m_pDSBuffer->Stop(); }

public:
   void PlayInternal(const float volume, const float randompitch, const int pitch, const float pan, const float front_rear_fade, const int flags, const bool restart);
   HRESULT Get3DBuffer();

   class PinSound *m_ppsOriginal;
   LPDIRECTSOUNDBUFFER m_pDSBuffer;
   LPDIRECTSOUND3DBUFFER m_pDS3DBuffer;
};

class PinSound : public PinDirectSoundWavCopy
{
public:
   PinSound();
   ~PinSound();

   class PinDirectSound *GetPinDirectSound();
   void UnInitialize();
   HRESULT ReInitialize();
   void SetDevice(); //!! BASS only
#ifdef ONLY_USE_BASS
   bool IsWav() const { return false; }
   bool IsWav2() const { return (_stricmp(strrchr(m_szPath, '.'), ".wav") == 0); }
#else
   bool IsWav() const { return (_stricmp(strrchr(m_szPath, '.'), ".wav") == 0); }
   bool IsWav2() const { return IsWav(); }
#endif
   void Play(const float volume, const float randompitch, const int pitch, const float pan, const float front_rear_fade, const int flags, const bool restart);
   void TestPlay();
   void Stop();

   union
   {
      class PinDirectSound *m_pPinDirectSound;
      //struct {
         HSTREAM m_BASSstream;
      // float m_pan;
      //};
   };

   char m_szName[MAXTOKEN]; // only filename, no ext
   char m_szInternalName[MAXTOKEN]; // only lower case filename, no ext
   char m_szPath[MAX_PATH]; // full filename, incl. path

   SoundOutTypes m_outputTarget;
   int m_balance;
   int m_fade;
   int m_volume;

   char *m_pdata; // wav: copy of the buffer/sample data so we can save it out, else: the contents of the original file
   int m_cdata;

   // old wav code only, but also used to convert raw wavs back to BASS
   WAVEFORMATEX m_wfx;

#ifdef ONLY_USE_BASS
   char *m_pdata_org; // save wavs in original raw format
   int m_cdata_org;
#endif
};



class PinDirectSound
{
public:
   PinDirectSound() : m_pDS(NULL), m_pDSListener(NULL) {}
   ~PinDirectSound();

   void InitDirectSound(const HWND hwnd, const bool IsBackglass);
   static float PanTo3D(float input);

   LPDIRECTSOUND       m_pDS;

private:
   LPDIRECTSOUND3DLISTENER m_pDSListener;
};


class AudioMusicPlayer
{
public:
	AudioMusicPlayer() : m_pbackglassds(NULL) {}
	~AudioMusicPlayer() { if (m_pbackglassds != &m_pds) delete m_pbackglassds; }

	void InitPinDirectSound(const HWND hwnd)
	{
		const int DSidx1 = LoadValueIntWithDefault("Player", "SoundDevice", 0);
		const int DSidx2 = LoadValueIntWithDefault("Player", "SoundDeviceBG", 0);
		const SoundConfigTypes SoundMode3D = (SoundConfigTypes)LoadValueIntWithDefault("Player", "Sound3D", (int)SNDCFG_SND3D2CH);

		m_pds.InitDirectSound(hwnd, false);
		// If these are the same device, and we are not in 3d mode, just point the backglass device to the main one.
		// For 3D we want two separate instances, one in basic stereo for music, and the other surround mode.
		if (SoundMode3D == SNDCFG_SND3D2CH && DSidx1 == DSidx2)
		{
			m_pbackglassds = &m_pds;
		}
		else
		{
			m_pbackglassds = new PinDirectSound();
			m_pbackglassds->InitDirectSound(hwnd, true);
		}
	}

	void ReInitPinDirectSound(const HWND hwnd)
	{
		if (m_pbackglassds != &m_pds)
			delete m_pbackglassds;

		InitPinDirectSound(hwnd);
	}

	PinDirectSound* GetPinDirectSound(const SoundOutTypes outputTarget)
	{
		return (outputTarget == SNDOUT_BACKGLASS) ? m_pbackglassds : &m_pds;
	}

	void StopCopiedWav(const char* const szName)
	{
		for (size_t i = 0; i < m_copiedwav.size(); i++)
		{
			PinDirectSoundWavCopy * const ppsc = m_copiedwav[i];
			if (!lstrcmp(ppsc->m_ppsOriginal->m_szInternalName, szName))
			{
				ppsc->m_pDSBuffer->Stop();
				break;
			}
		}
	}

	void StopCopiedWavs()
	{
		for (size_t i = 0; i < m_copiedwav.size(); i++)
			m_copiedwav[i]->m_pDSBuffer->Stop();
	}

	void StopAndClearCopiedWavs()
	{
		for (size_t i = 0; i < m_copiedwav.size(); i++)
		{
			m_copiedwav[i]->m_pDSBuffer->Stop();
			m_copiedwav[i]->m_pDSBuffer->Release();
			delete m_copiedwav[i];
		}
		m_copiedwav.clear();
	}

	void ClearStoppedCopiedWavs()
	{
	   size_t i = 0;
	   while (i < m_copiedwav.size())
	   {
		  PinDirectSoundWavCopy * const ppsc = m_copiedwav[i];
		  DWORD status;
		  ppsc->m_pDSBuffer->GetStatus(&status);
		  if (!(status & DSBSTATUS_PLAYING)) // sound is done, we can throw it away now
		  {
			 ppsc->m_pDSBuffer->Release();
			 m_copiedwav.erase(m_copiedwav.begin() + i);
			 delete ppsc;
		  }
		  else
			 i++;
	   }
	}

	void Play(PinSound * const pps, const float volume, const float randompitch, const int pitch, const float pan, const float front_rear_fade, const int loopcount, const bool usesame, const bool restart)
	{
		const int flags = (loopcount == -1) ? DSBPLAY_LOOPING : 0;

		if (!pps->IsWav())
		{
			pps->Play(volume, randompitch, pitch, pan, front_rear_fade, flags, restart);
			return;
		}

		ClearStoppedCopiedWavs();

		PinDirectSoundWavCopy * ppsc = NULL;
		bool foundsame = false;
		if (usesame)
		{
			const LPDIRECTSOUNDBUFFER pdsb = pps->m_pDSBuffer;
			for (size_t i2 = 0; i2 < m_copiedwav.size(); i2++)
			{
				if (m_copiedwav[i2]->m_ppsOriginal->m_pDSBuffer == pdsb)
				{
					ppsc = m_copiedwav[i2];
					foundsame = true;
					break;
				}
			}
		}

		if (ppsc == NULL)
			ppsc = new PinDirectSoundWavCopy(pps);

		if (ppsc->m_pDSBuffer)
		{
			ppsc->PlayInternal(volume, randompitch, pitch, pan, front_rear_fade, flags, restart);
			if (!foundsame)
				m_copiedwav.push_back(ppsc);
		}
		else // Couldn't or didn't want to create a copy - just play the original
		{
			delete ppsc;

			pps->Play(volume, randompitch, pitch, pan, front_rear_fade, flags, restart);
		}
	}

	PinSound *LoadFile(const TCHAR* const strFileName);

private:
	PinDirectSound m_pds;
	PinDirectSound *m_pbackglassds;

	vector< PinDirectSoundWavCopy* > m_copiedwav; // copied sounds currently playing
};

#endif // !defined(AFX_PINSOUND_H__61491D0B_9950_480C_B453_911B3A2CDB8E__INCLUDED_)
