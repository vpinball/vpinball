#include "StdAfx.h"

extern bool bass_init;

PinSound::PinSound() : PinDirectSoundWavCopy(this)
{
   if(!bass_init)
   {
      AudioPlayer ap; //!! just to get BASS init'ed, remove again as soon as all is unified! // also need (optionally) 2 devices, one BG, one normal
   }

   m_pDSBuffer = NULL;
   m_pDS3DBuffer = NULL;
   m_pdata = NULL;
   m_pPinDirectSound = NULL; // m_BASSstream = 0;
   m_outputTarget = SNDOUT_TABLE;
   m_balance = 0;
   m_fade = 0;
   m_volume = 0;
}

PinSound::~PinSound()
{
   UnInitialize();

   if (m_pdata)
      delete [] m_pdata;
}

void PinSound::UnInitialize()
{
   if (IsWav())
   {
      SAFE_RELEASE(m_pDS3DBuffer);
      SAFE_RELEASE(m_pDSBuffer);
   }
   else
   {
      if (m_BASSstream)
         BASS_StreamFree(m_BASSstream);
   }
}

class PinDirectSound *PinSound::GetPinDirectSound()
{
   if (m_pPinDirectSound)
      return m_pPinDirectSound;
   else
      return g_pvp->m_ps.GetPinDirectSound(m_outputTarget);
}

HRESULT PinSound::ReInitialize()
{
   SAFE_RELEASE(m_pDS3DBuffer);
   SAFE_RELEASE(m_pDSBuffer);

   PinDirectSound * const pds = GetPinDirectSound();
   if (pds->m_pDS == NULL)
   {
      m_pPinDirectSound = NULL;
      m_pDSBuffer = NULL;
      return E_FAIL;
   }

   DSBUFFERDESC dsbd;
   ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
   dsbd.dwSize = sizeof(DSBUFFERDESC);
   dsbd.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | ((pds->m_3DSoundMode != SNDCFG_SND3D2CH) ? DSBCAPS_CTRL3D : DSBCAPS_CTRLPAN);
   dsbd.dwBufferBytes = m_cdata;
   dsbd.lpwfxFormat = &m_wfx;

   // Create the static DirectSound buffer 
   HRESULT hr;
   if (FAILED(hr = pds->m_pDS->CreateSoundBuffer(&dsbd, &m_pDSBuffer, NULL)))
   {
      ShowError("Could not create sound buffer for load.");
      m_pPinDirectSound = NULL;
      m_pDSBuffer = NULL;
      return hr;
   }

   m_pPinDirectSound = pds;

   // Lock the buffer down
   VOID*   pbData = NULL;
   VOID*   pbData2 = NULL;
   DWORD   dwLength,dwLength2;
   if (FAILED(hr = m_pDSBuffer->Lock(0, m_cdata, &pbData, &dwLength,
      &pbData2, &dwLength2, 0L)))
   {
      ShowError("Could not lock sound buffer for load.");
      return hr;
   }
   // Copy the memory to it.
   memcpy(pbData, m_pdata, m_cdata);
   // Unlock the buffer, we don't need it anymore.
   m_pDSBuffer->Unlock(pbData, m_cdata, NULL, 0);

   if (pds->m_3DSoundMode != SNDCFG_SND3D2CH)
      Get3DBuffer();

   return S_OK;
}

PinDirectSound::~PinDirectSound()
{
   SAFE_RELEASE(m_pDSListener);
   SAFE_RELEASE(m_pDS);
}



BOOL CALLBACK DSEnumCallBack(LPGUID guid, LPCSTR desc, LPCSTR mod, LPVOID list)
{
   DSAudioDevice *ad = new DSAudioDevice;
   if (guid == NULL)
      ad->guid = NULL;
   else {
      ad->guid = new GUID;
      memcpy(ad->guid, guid, sizeof(GUID));
   }
   ad->description = desc;
   ad->module = mod;
   (static_cast<DSAudioDevices*>(list))->push_back(ad);
   return fTrue;
}

void PinDirectSound::InitDirectSound(const HWND hwnd, const bool IsBackglass)
{
#ifdef DEBUG_NO_SOUND
   return;
#endif
   SAFE_RELEASE(m_pDSListener);
   SAFE_RELEASE(m_pDS);

   DSAudioDevices DSads;
   int DSidx = 0;
   if (!FAILED(DirectSoundEnumerate(DSEnumCallBack, &DSads)))
   {
      const HRESULT hr = LoadValueInt("Player", IsBackglass ? "SoundDeviceBG" : "SoundDevice", &DSidx);
      if ((hr != S_OK) || ((size_t)DSidx >= DSads.size()))
         DSidx = 0; // The default primary sound device
   }

   // Create IDirectSound using the selected sound device
   HRESULT hr;
   if (FAILED(hr = DirectSoundCreate((DSidx != 0) ? DSads[DSidx]->guid : NULL, &m_pDS, NULL)))
   {
      ShowError("Could not create Direct Sound.");
      return;// hr;
   }

   // free audio devices list
   for (size_t i = 0; i < DSads.size(); i++)
      delete DSads[i];

   // Set coop level to DSSCL_PRIORITY
   if (FAILED(hr = m_pDS->SetCooperativeLevel(hwnd, DSSCL_PRIORITY)))
   {
      ShowError("Could not set Direct Sound Priority.");
      return;// hr;
   }

   // Get the primary buffer 
   DSBUFFERDESC dsbd;
   ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
   dsbd.dwSize = sizeof(DSBUFFERDESC);
   dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
   if (!IsBackglass && (m_3DSoundMode != SNDCFG_SND3D2CH))
      dsbd.dwFlags |= DSBCAPS_CTRL3D;
   dsbd.dwBufferBytes = 0;
   dsbd.lpwfxFormat = NULL;

   LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;
   if (FAILED(hr = m_pDS->CreateSoundBuffer(&dsbd, &pDSBPrimary, NULL)))
   {
      ShowError("Could not create primary sound buffer.");
      return;// hr;
   }

   // Set primary buffer format to 44kHz and 16-bit output.
   WAVEFORMATEX wfx;
   ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
   wfx.wFormatTag = WAVE_FORMAT_PCM;
   wfx.nChannels = (m_3DSoundMode != SNDCFG_SND3D2CH) ?  1 : 2;
   wfx.nSamplesPerSec = 44100;
   wfx.wBitsPerSample = 16;
   wfx.nBlockAlign = wfx.wBitsPerSample / (WORD)8 * wfx.nChannels;
   wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

   if (FAILED(hr = pDSBPrimary->SetFormat(&wfx)))
   {
      ShowError("Could not set sound format.");
      return;// hr;
   }
   if (!IsBackglass && (m_3DSoundMode != SNDCFG_SND3D2CH))
   {
	   // Obtain a listener interface.
	   hr = pDSBPrimary->QueryInterface(IID_IDirectSound3DListener, (LPVOID*)&m_pDSListener);
	   if (FAILED(hr))
	   {
		   ShowError("Could not acquire 3D listener interface.");
		   return;// hr;
	   }

	   // Set the initial position of the listener to be sitting in between the front two speakers. 
	   m_pDSListener->SetPosition(0.0f, 0.0f, 0.0f, DS3D_IMMEDIATE);
   }
   SAFE_RELEASE(pDSBPrimary);

   //return S_OK;
}

PinSound *AudioMusicPlayer::LoadFile(const TCHAR* const strFileName)
{
   PinSound * const pps = new PinSound();

   strncpy_s(pps->m_szPath, strFileName, MAX_PATH);
   TitleFromFilename(strFileName, pps->m_szName);
   strncpy_s(pps->m_szInternalName, pps->m_szName, MAXTOKEN);
   CharLowerBuff(pps->m_szInternalName, lstrlen(pps->m_szInternalName));

   if (pps->IsWav()) // only use old direct sound code and wav reader if playing wav's
   {
	   // Create a new wave file class
	   CWaveSoundRead* const pWaveSoundRead = new CWaveSoundRead();

	   // Load the wave file
	   if (FAILED(pWaveSoundRead->Open(strFileName)))
	   {
		   ShowError("Could not open wav file.");
		   delete pWaveSoundRead;
		   delete pps;
		   return NULL;
	   }

	   // Set up the direct sound buffer, and only request the flags needed
	   // since each requires some overhead and limits if the buffer can
	   // be hardware accelerated
	   DSBUFFERDESC dsbd;
	   ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
	   dsbd.dwSize = sizeof(DSBUFFERDESC);
	   dsbd.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN;
	   if (m_pds.m_3DSoundMode != SNDCFG_SND3D2CH)
		   dsbd.dwFlags |= DSBCAPS_CTRL3D;
	   dsbd.dwBufferBytes = pWaveSoundRead->m_ckIn.cksize;
	   dsbd.lpwfxFormat = pWaveSoundRead->m_pwfx;
	   memcpy(&pps->m_wfx, pWaveSoundRead->m_pwfx, sizeof(pps->m_wfx));

	   // Create the static DirectSound buffer
	   HRESULT hr;
	   if (FAILED(hr = m_pds.m_pDS->CreateSoundBuffer(&dsbd, &pps->m_pDSBuffer, NULL)))
	   {
		   ShowError("Could not create static sound buffer.");
		   delete pWaveSoundRead;
		   delete pps;
		   return NULL;
	   }
	   if (m_pds.m_3DSoundMode != SNDCFG_SND3D2CH)
		   pps->Get3DBuffer();

	   // Remember how big the buffer is
	   pps->m_cdata = dsbd.dwBufferBytes;
	   pps->m_pPinDirectSound = &m_pds;

	   // Fill the buffer with wav data

		  // Allocate that buffer.
		  pps->m_pdata = new char[pps->m_cdata];

		  UINT cbWavSize; // Size of data
		  if (FAILED(hr = pWaveSoundRead->Read(pps->m_cdata,
			  (BYTE*)pps->m_pdata,
			  &cbWavSize)))
		  {
			  ShowError("Could not read wav file.");
			  delete pWaveSoundRead;
			  delete pps;
			  return NULL;
		  }

		  delete pWaveSoundRead;

		  // Lock the buffer down
		  VOID* pbData = NULL;
		  VOID* pbData2 = NULL;
		  DWORD dwLength, dwLength2;
		  if (FAILED(hr = pps->m_pDSBuffer->Lock(0, pps->m_cdata, &pbData, &dwLength,
			  &pbData2, &dwLength2, 0L)))
		  {
			  ShowError("Could not lock sound buffer.");
			  delete pps;
			  return NULL;
		  }
		  // Copy the memory to it.
		  memcpy(pbData, pps->m_pdata, pps->m_cdata);
		  // Unlock the buffer, we don't need it anymore.
		  pps->m_pDSBuffer->Unlock(pbData, pps->m_cdata, NULL, 0);

         // Update the UI controls to show the sound as the file is loaded
         //SetFileUI( hDlg, strFileName );
         //OnEnablePlayUI( hDlg, TRUE );
   }
   else
   {
	   FILE *f;
	   fopen_s(&f, strFileName, "rb");
	   fseek(f, 0, SEEK_END);
	   pps->m_cdata = (int)ftell(f);
	   fseek(f, 0, SEEK_SET);
	   pps->m_pdata = new char[pps->m_cdata];
	   fread_s(pps->m_pdata, pps->m_cdata, 1, pps->m_cdata, f);
	   fclose(f);

	   pps->m_BASSstream = BASS_StreamCreateFile(
		   TRUE,
		   pps->m_pdata,
		   0,
		   pps->m_cdata,
		   0 /*BASS_SAMPLE_3D | BASS_SAMPLE_LOOP*/ //!!
	   );

	   if (pps->m_BASSstream == NULL)
	   {
		   delete pps;
		   char bla[MAX_PATH];
		   sprintf_s(bla, "BASS music/sound library cannot load %s", strFileName);
		   MessageBox(g_pvp->m_hwnd, bla, "Error", MB_ICONERROR);
		   return NULL;
	   }
   }

   return pps;
}

// The existing pan value in PlaySound function takes a -1 to 1 value, however it's extremely non-linear.
// -0.1 is very obviously to the left.  Table scripts like the ball rolling script seem to use x^10 to map
// linear positions, so we'll use that and reverse it.   Also multiplying by 3 since that seems to be the
// the total distance necessary to fully pan away from one side at the center of the room.

float PinDirectSound::PanTo3D(float input)
{
	// DirectSound's position command does weird things at exactly 0. 
	if (fabsf(input) < 0.0001f)
		input = (input < 0.0f) ? -0.0001f : 0.0001f;
	if (input < 0.0f)
	{
		return -powf(-max(input, -1.0f), (float)(1.0 / 10.0)) * 3.0f;
	}
	else
	{
		return powf(min(input, 1.0f), (float)(1.0 / 10.0)) * 3.0f;
	}
}

PinDirectSoundWavCopy::PinDirectSoundWavCopy(class PinSound * const pOriginal)
{
	m_ppsOriginal = pOriginal;

	if (this != pOriginal)
	{
		m_pDSBuffer = NULL;
		m_pDS3DBuffer = NULL;
		pOriginal->GetPinDirectSound()->m_pDS->DuplicateSoundBuffer(pOriginal->m_pDSBuffer, &m_pDSBuffer);
		if (m_pDSBuffer && pOriginal->m_pDS3DBuffer != NULL)
			Get3DBuffer();
	}
}

void PinDirectSoundWavCopy::PlayInternal(const float volume, const float randompitch, const int pitch, const float pan, const float front_rear_fade, const int flags, const bool restart)
{
	const float totalvolume = max(min(volume, 100.0f), 0.0f);
	const int decibelvolume = (totalvolume == 0.0f) ? DSBVOLUME_MIN : (int)(logf(totalvolume)*(float)(1000.0 / log(10.0)) - 2000.0f);
	m_pDSBuffer->SetVolume(decibelvolume);
	// Frequency tweaks are relative to original sound.  If the copy failed for some reason, don't alter original
	if (m_ppsOriginal != this)
	{
		if (randompitch > 0.f)
		{
			DWORD freq;
			m_ppsOriginal->m_pDSBuffer->GetFrequency(&freq);
			freq += pitch;
			const float rndh = rand_mt_01();
			const float rndl = rand_mt_01();
			m_pDSBuffer->SetFrequency(freq + (DWORD)((float)freq * randompitch * rndh * rndh) - (DWORD)((float)freq * randompitch * rndl * rndl * 0.5f));
		}
		else if (pitch != 0)
		{
			DWORD freq;
			m_ppsOriginal->m_pDSBuffer->GetFrequency(&freq);
			m_pDSBuffer->SetFrequency(freq + pitch);
		}
	}
	switch (m_ppsOriginal->GetPinDirectSound()->m_3DSoundMode)
	{
	case SNDCFG_SND3DALLREAR:
		m_pDS3DBuffer->SetPosition(PinDirectSound::PanTo3D(pan), 0.0f, -PinDirectSound::PanTo3D(1.0f), DS3D_IMMEDIATE);
		break;
	case SNDCFG_SND3DFRONTISFRONT:
		m_pDS3DBuffer->SetPosition(PinDirectSound::PanTo3D(pan), 0.0f, PinDirectSound::PanTo3D(front_rear_fade), DS3D_IMMEDIATE);
		break;
	case SNDCFG_SND3DFRONTISREAR:
		m_pDS3DBuffer->SetPosition(PinDirectSound::PanTo3D(pan), 0.0f, -PinDirectSound::PanTo3D(front_rear_fade), DS3D_IMMEDIATE);
		break;
	case SNDCFG_SND3D6CH:
		m_pDS3DBuffer->SetPosition(PinDirectSound::PanTo3D(pan), 0.0f, -((PinDirectSound::PanTo3D(front_rear_fade) + 3.0f) / 2.0f), DS3D_IMMEDIATE);
		break;
	case SNDCFG_SND3D2CH:
	default:
		if (pan != 0.f)
			m_pDSBuffer->SetPan((LONG)(pan*DSBPAN_RIGHT));
		break;
	}

	DWORD status;
	m_pDSBuffer->GetStatus(&status);
	if (!(status & DSBSTATUS_PLAYING))
		m_pDSBuffer->Play(0, 0, flags);
	else if (restart)
		m_pDSBuffer->SetCurrentPosition(0);
}

HRESULT PinDirectSoundWavCopy::Get3DBuffer()
{
	const HRESULT hr = m_pDSBuffer->QueryInterface(IID_IDirectSound3DBuffer, (void**)&m_pDS3DBuffer);
	if (FAILED(hr))
		ShowError("Could not get interface to 3D sound buffer.");
	else
		m_pDS3DBuffer->SetMinDistance(5.0f, DS3D_IMMEDIATE);
	return hr;
}
