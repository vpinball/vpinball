// PinSound.h: interface for the PinSound class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_PINSOUND_H__61491D0B_9950_480C_B453_911B3A2CDB8E__INCLUDED_)
#define AFX_PINSOUND_H__61491D0B_9950_480C_B453_911B3A2CDB8E__INCLUDED_

#include "core/Settings.h"

void BASS_ErrorMapCode(const int code, string& text);

struct DSAudioDevice
{
   LPGUID guid;
   string description;
   string module;

   DSAudioDevice() {
      guid = nullptr;
   }
   ~DSAudioDevice() {
      delete guid;
   }
};

typedef vector<DSAudioDevice*> DSAudioDevices;

BOOL CALLBACK DSEnumCallBack(LPGUID guid, LPCSTR desc, LPCSTR mod, LPVOID list);

enum SoundOutTypes : char { SNDOUT_TABLE = 0, SNDOUT_BACKGLASS = 1 };
enum SoundConfigTypes : int { SNDCFG_SND3D2CH = 0, SNDCFG_SND3DALLREAR = 1, SNDCFG_SND3DFRONTISREAR = 2, 
                              SNDCFG_SND3DFRONTISFRONT = 3, SNDCFG_SND3D6CH = 4, SNDCFG_SND3DSSF = 5};

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
//
// SSF: 6CH still doesn't map sounds for SSF as distinctly as it could.. In this mode horizontal panning and vertical fading 
// are enhanced for a more realistic experience.

class PinDirectSoundWavCopy
{
public:
   PinDirectSoundWavCopy(class PinSound * const pOriginal);

protected:
   void StopInternal()
   {
#ifndef __STANDALONE__
      m_pDSBuffer->Stop();
#endif
   }

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

	void SetOutputTarget(SoundOutTypes target) {if (m_outputTarget != target) { m_outputTarget = target; ReInitialize(); } }
	SoundOutTypes GetOutputTarget() const { return m_outputTarget; }

   void UnInitialize();
   HRESULT ReInitialize();
   void SetBassDevice(); //!! BASS only // FIXME move loading code to PinSound and make private
#ifdef ONLY_USE_BASS
   bool IsWav() const { return false; }
   bool IsWav2() const
   {
      const size_t pos = m_szPath.find_last_of('.');
      if(pos == string::npos)
         return true;
      return (lstrcmpi(m_szPath.substr(pos+1).c_str(), "wav") == 0);
   }
#else
   bool IsWav() const
   {
      const size_t pos = m_szPath.find_last_of('.');
      if(pos == string::npos)
         return true;
      return (lstrcmpi(m_szPath.substr(pos+1).c_str(), "wav") == 0);
   }
   bool IsWav2() const { return IsWav(); }
#endif
   void Play(const float volume, const float randompitch, const int pitch, const float pan, const float front_rear_fade, const int flags, const bool restart);
   void Stop();

   union
   {
      class PinDirectSound *m_pPinDirectSound;
      //struct {
         HSTREAM m_BASSstream;
      // float m_pan;
      //};
   };

   string m_szName; // only filename, no ext
   string m_szPath; // full filename, incl. path

   int m_balance;
   int m_fade;
   int m_volume;
   float m_freq;

   char *m_pdata; // wav: copy of the buffer/sample data so we can save it out, else: the contents of the original file
   int m_cdata;

   // old wav code only, but also used to convert raw wavs back to BASS
   WAVEFORMATEX m_wfx;

#ifdef ONLY_USE_BASS
   char *m_pdata_org; // save wavs in original raw format
   int m_cdata_org;
#endif

private:
   SoundOutTypes m_outputTarget;
};



class PinDirectSound
{
public:
   PinDirectSound() : m_pDS(nullptr), m_pDSListener(nullptr) {}
   ~PinDirectSound();

   void InitDirectSound(const HWND hwnd, const bool IsBackglass);
   static float PanTo3D(float input);
   static float PanSSF(float input);
   static float FadeSSF(float input);

   LPDIRECTSOUND m_pDS;

private:
   LPDIRECTSOUND3DLISTENER m_pDSListener;
};


class AudioMusicPlayer
{
public:
	AudioMusicPlayer() : m_pbackglassds(nullptr) {}
	~AudioMusicPlayer()
	{
		if (m_pbackglassds != &m_pds) delete m_pbackglassds;
      BASS_Stop();
      BASS_Free();
	}

	void InitPinDirectSound(const Settings& settings, const HWND hwnd);

	void ReInitPinDirectSound(const Settings& settings, const HWND hwnd)
	{
		if (m_pbackglassds != &m_pds) delete m_pbackglassds;
      BASS_Stop();
      BASS_Free();

		InitPinDirectSound(settings, hwnd);
	}

	PinDirectSound* GetPinDirectSound(const SoundOutTypes outputTarget)
	{
		return (outputTarget == SNDOUT_BACKGLASS) ? m_pbackglassds : &m_pds;
	}

	void StopCopiedWav(const char* const szName)
	{
#ifndef __STANDALONE__
		for (size_t i = 0; i < m_copiedwav.size(); i++)
		{
			const PinDirectSoundWavCopy * const ppsc = m_copiedwav[i];
			if (!lstrcmpi(ppsc->m_ppsOriginal->m_szName.c_str(), szName))
			{
				ppsc->m_pDSBuffer->Stop();
				break;
			}
		}
#endif
	}

	void StopCopiedWavs()
	{
#ifndef __STANDALONE__
		for (size_t i = 0; i < m_copiedwav.size(); i++)
			m_copiedwav[i]->m_pDSBuffer->Stop();
#endif
	}

	void StopAndClearCopiedWavs()
	{
#ifndef __STANDALONE__
		for (size_t i = 0; i < m_copiedwav.size(); i++)
		{
			m_copiedwav[i]->m_pDSBuffer->Stop();
			m_copiedwav[i]->m_pDSBuffer->Release();
			delete m_copiedwav[i];
		}
		m_copiedwav.clear();
#endif
	}

	void ClearStoppedCopiedWavs()
	{
		size_t i = 0;
#ifndef __STANDALONE__
		while (i < m_copiedwav.size())
		{
			const PinDirectSoundWavCopy * const ppsc = m_copiedwav[i];
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
#endif
	}

	void Play(PinSound * const pps, const float volume, const float randompitch, const int pitch, const float pan, const float front_rear_fade, const int loopcount, const bool usesame, const bool restart)
	{
		const int flags = (loopcount == -1) ? DSBPLAY_LOOPING : 0;

		if (!pps->IsWav())
		{
#ifdef ONLY_USE_BASS
			if (pps->IsWav2())
				pps->Play(volume, randompitch, pitch, pan, front_rear_fade, flags, (!usesame) ? true : restart);
			else
#endif
				pps->Play(volume, randompitch, pitch, pan, front_rear_fade, flags, restart);

			return;
		}

#ifndef __STANDALONE__
		ClearStoppedCopiedWavs();

		PinDirectSoundWavCopy * ppsc = nullptr;
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

		if (ppsc == nullptr)
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
#endif
	}

	PinSound *LoadFile(const string& strFileName);

   int bass_STD_idx = -2, bass_BG_idx = -2;

private:
	PinDirectSound m_pds;
	PinDirectSound *m_pbackglassds;

	vector< PinDirectSoundWavCopy* > m_copiedwav; // copied sounds currently playing
};

#ifdef __STANDALONE__
struct AudioDevice
{
	int id;
	const char name[MAX_DEVICE_IDENTIFIER_STRING];
	bool enabled;
};

void EnumerateAudioDevices(vector<AudioDevice>& devices);
#endif

#endif // !defined(AFX_PINSOUND_H__61491D0B_9950_480C_B453_911B3A2CDB8E__INCLUDED_)
