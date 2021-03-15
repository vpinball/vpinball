#include "StdAfx.h"

PinSound::PinSound() : PinSoundCopy(this)
{
   m_pDSBuffer = NULL;
   m_pDS3DBuffer = NULL;
   m_pdata = NULL;
   m_pPinDirectSound = NULL;
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
	SAFE_RELEASE(m_pDS3DBuffer);
	SAFE_RELEASE(m_pDSBuffer);
}

class PinDirectSound *PinSound::GetPinDirectSound()
{
   if (m_pPinDirectSound)
      return m_pPinDirectSound;

   if (m_outputTarget == SNDOUT_BACKGLASS)
      return g_pvp->m_pbackglassds;

   return &(g_pvp->m_pds);
}

void PinSound::ReInitialize()
{
   SAFE_RELEASE(m_pDS3DBuffer);
   SAFE_RELEASE(m_pDSBuffer);
   m_pPinDirectSound = NULL;
   GetPinDirectSound()->CreateDirectFromNative(this);
}

PinDirectSound::PinDirectSound()
{
   m_3DSoundMode = SNDCFG_SND3D2CH;
   m_pDS = NULL;
   m_pWaveSoundRead = NULL;
   m_pDSListener = NULL;
}

PinDirectSound::~PinDirectSound()
{
   SAFE_DELETE(m_pWaveSoundRead);
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
   SAFE_DELETE(m_pWaveSoundRead);
   SAFE_RELEASE(m_pDSListener);
   SAFE_RELEASE(m_pDS);

   HRESULT hr;
   LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

   // Initialize COM
   //if (hr = CoInitialize(NULL))
   //return hr;

   DSAudioDevices DSads;
   int DSidx = 0;
   if (!FAILED(DirectSoundEnumerate(DSEnumCallBack, &DSads)))
   {
      hr = LoadValueInt("Player", IsBackglass ? "SoundDeviceBG" : "SoundDevice", &DSidx);
      if ((hr != S_OK) || ((size_t)DSidx >= DSads.size()))
         DSidx = 0; // The default primary sound device
   }

   // Create IDirectSound using the selected sound device
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
   if (!IsBackglass)
   {
	   if (m_3DSoundMode != SNDCFG_SND3D2CH)
		   dsbd.dwFlags |= DSBCAPS_CTRL3D;
   }
   dsbd.dwBufferBytes = 0;
   dsbd.lpwfxFormat = NULL;

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
	   HRESULT result = pDSBPrimary->QueryInterface(IID_IDirectSound3DListener, (LPVOID*)&m_pDSListener);
	   if (FAILED(result))
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

PinSound *PinDirectSound::LoadWaveFile(const TCHAR* const strFileName)
{
   PinSound * const pps = new PinSound();

   // Create the sound buffer object from the wave file data
   if (FAILED(CreateStaticBuffer(strFileName, pps)))
   {
      ShowError("Could not create static sound buffer.");
      delete pps;
      return NULL;
      //SetFileUI( hDlg, TEXT("Couldn't create sound buffer.") ); 
   }
   else // The sound buffer was successfully created
   {
      // Fill the buffer with wav data
      FillBuffer(pps);

      // Update the UI controls to show the sound as the file is loaded
      //SetFileUI( hDlg, strFileName );
      //OnEnablePlayUI( hDlg, TRUE );
   }

   strncpy_s(pps->m_szPath, strFileName, MAX_PATH);

   TitleFromFilename(strFileName, pps->m_szName);

   strncpy_s(pps->m_szInternalName, pps->m_szName, MAXTOKEN);

   CharLowerBuff(pps->m_szInternalName, lstrlen(pps->m_szInternalName));

   return pps;
}


//-----------------------------------------------------------------------------
// Name: CreateStaticBuffer()
// Desc: Creates a wave file, sound buffer and notification events 
//-----------------------------------------------------------------------------
HRESULT PinDirectSound::CreateStaticBuffer(const TCHAR* const strFileName, PinSound * const pps)
{
   // Free any previous globals 
   SAFE_DELETE(m_pWaveSoundRead);
   //SAFE_RELEASE( m_pDSBuffer );

   // Create a new wave file class
   m_pWaveSoundRead = new CWaveSoundRead();

   // Load the wave file
   if (FAILED(m_pWaveSoundRead->Open(strFileName)))
   {
      ShowError("Could not open file.");
      return E_FAIL;
   }

   // Set up the direct sound buffer, and only request the flags needed
   // since each requires some overhead and limits if the buffer can 
   // be hardware accelerated
   DSBUFFERDESC dsbd;
   ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
   dsbd.dwSize = sizeof(DSBUFFERDESC);
   dsbd.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN;
   if (m_3DSoundMode != SNDCFG_SND3D2CH)
      dsbd.dwFlags |= DSBCAPS_CTRL3D;
   dsbd.dwBufferBytes = m_pWaveSoundRead->m_ckIn.cksize;
   dsbd.lpwfxFormat = m_pWaveSoundRead->m_pwfx;
   memcpy(&pps->m_wfx, m_pWaveSoundRead->m_pwfx, sizeof(pps->m_wfx));

   // Create the static DirectSound buffer 
   HRESULT hr;
   if (FAILED(hr = m_pDS->CreateSoundBuffer(&dsbd, &pps->m_pDSBuffer, NULL)))
   {
      ShowError("Could not create sound buffer.");
      return hr;
   }
   if (m_3DSoundMode != SNDCFG_SND3D2CH)
      pps->Get3DBuffer();

   // Remember how big the buffer is
   m_dwBufferBytes = dsbd.dwBufferBytes;
   pps->m_pPinDirectSound = this;

   return S_OK;
}


//-----------------------------------------------------------------------------
// Name: FillBuffer()
// Desc: Fill the DirectSound buffer with data from the wav file
//-----------------------------------------------------------------------------
HRESULT PinDirectSound::FillBuffer(PinSound * const pps)
{
   BYTE*   pbWavData; // Pointer to actual wav data 
   UINT    cbWavSize; // Size of data
   VOID*   pbData = NULL;
   VOID*   pbData2 = NULL;
   DWORD   dwLength;
   DWORD   dwLength2;

   // The size of wave data is in pWaveFileSound->m_ckIn
   INT nWaveFileSize = m_pWaveSoundRead->m_ckIn.cksize;

   // Allocate that buffer.
   pbWavData = new BYTE[nWaveFileSize];
   if (NULL == pbWavData)
      return E_OUTOFMEMORY;

   HRESULT hr;
   if (FAILED(hr = m_pWaveSoundRead->Read(nWaveFileSize,
      pbWavData,
      &cbWavSize)))
   {
      delete[] pbWavData;
      ShowError("Could not read wav file.");
      return hr;
   }

   // Reset the file to the beginning 
   m_pWaveSoundRead->Reset();

   // Lock the buffer down
   if (FAILED(hr = pps->m_pDSBuffer->Lock(0, m_dwBufferBytes, &pbData, &dwLength,
      &pbData2, &dwLength2, 0L)))
   {
      delete[] pbWavData;
      ShowError("Could not lock sound buffer.");
      return hr;
   }

   // Copy the memory to it.
   memcpy(pbData, pbWavData, m_dwBufferBytes);

   // Unlock the buffer, we don't need it anymore.
   pps->m_pDSBuffer->Unlock(pbData, m_dwBufferBytes, NULL, 0);
   pbData = NULL;

   pps->m_pdata = new char[m_dwBufferBytes];

   memcpy(pps->m_pdata, pbWavData, m_dwBufferBytes);

   pps->m_cdata = m_dwBufferBytes;

   // We dont need the wav file data buffer anymore, so delete it 
   SAFE_VECTOR_DELETE(pbWavData);

   return S_OK;
}

HRESULT PinDirectSound::CreateDirectFromNative(PinSound * const pps)
{
   DSBUFFERDESC dsbd;
   ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
   dsbd.dwSize = sizeof(DSBUFFERDESC);
   dsbd.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;
   if (m_3DSoundMode != SNDCFG_SND3D2CH)
	   dsbd.dwFlags |= DSBCAPS_CTRL3D;
   else
	   dsbd.dwFlags |= DSBCAPS_CTRLPAN;

   dsbd.dwBufferBytes = pps->m_cdata;
   dsbd.lpwfxFormat = &pps->m_wfx;

   if (m_pDS == NULL)
   {
      pps->m_pDSBuffer = NULL;
      return E_FAIL;
   }

   // Create the static DirectSound buffer 
   HRESULT hr;
   if (FAILED(hr = m_pDS->CreateSoundBuffer(&dsbd, &pps->m_pDSBuffer, NULL)))
   {
      ShowError("Could not create sound buffer for load.");
      return hr;
   }

   pps->m_pPinDirectSound = this;
   VOID*   pbData = NULL;
   VOID*   pbData2 = NULL;
   DWORD   dwLength;
   DWORD   dwLength2;

   // Lock the buffer down
   if (FAILED(hr = pps->m_pDSBuffer->Lock(0, pps->m_cdata, &pbData, &dwLength,
      &pbData2, &dwLength2, 0L)))
   {
      ShowError("Could not lock sound buffer for load.");
      return hr;
   }

   // Copy the memory to it.
   memcpy(pbData, pps->m_pdata, pps->m_cdata);

   // Unlock the buffer, we don't need it anymore.
   pps->m_pDSBuffer->Unlock(pbData, pps->m_cdata, NULL, 0);
   pbData = NULL;

   if (m_3DSoundMode != SNDCFG_SND3D2CH)
      pps->Get3DBuffer();

   return S_OK;
}

// The existing pan value in PlaySound function takes a -1 to 1 value, however it's extremely non-linear.
// -0.1 is very obviously to the left.  Table scripts like the ball rolling script seem to use x^10 to map
// linear positions, so we'll use that and reverse it.   Also multiplying by 3 since that seems to be the
// the total distance necessary to fully pan away from one side at the center of the room.

float PinDirectSound::PanTo3D(float input)
{
	// DirectSound's position command does weird things at exactly 0. 
	if (fabsf(input) < 0.0001f)
		input = 0.0001f;
	if (input < 0.0f)
	{
		return -powf(-max(input, -1.0f), (float)(1.0 / 10.0)) * 3.0f;
	}
	else
	{
		return powf(min(input, 1.0f), (float)(1.0 / 10.0)) * 3.0f;
	}
}

// This is a replacement function for PanTo3D() for sound effect panning (audio x-axis).
// It performs the same calculations but maps the resulting values to an area of the 3D
// sound stage that has the expected panning effect for this application. It is written
// in a long form to facilitate tweaking the formulas.  *njk*

float PinDirectSound::PanSSF(float pan)
{
	float x = 0.0f;

	// This math could probably be simplified but it is kept in long form
	// to aide in fine tuning and clarity of function.

	// Clip the pan input range to -1.0 to 1.0

	if (pan < -1.0f)
		x = -1.0f;
	else if (pan > 1.0f)
		x = 1.0f;
	else
		x = pan;

	// Rescale pan range from an exponential [-1,0] and [0,1] to a linear [-1.0, 1.0]
	// Do not avoid values close to zero like PanTo3D() does as that
	// prevents the middle range of the exponential curves converting back to
	// a linear scale (which would leave a gap in the center of the range).
	// This basically undoes the Pan() fading function in the table scripts.

	x = (x < 0.0f) ? -powf(-x, 0.10f) : powf(x, 0.10f);

	// Increase the pan range from [-1.0, 1.0] to [-3.0, 3.0] to improve the surround sound fade effect

	x = x * 3.0f;

	// Unfortuneately the above scaling is insufficient to provide a good panning effect.
	// Through experimentation it was determined that sounds on the left side of the
	// playfield pan nicely from -2.25 to -2.50 and sounds on the right side of the
	// playfield pan nicely from 2.25 to 2.50. So we scale each side of the playfield
	// accordingly.

	// Rescale [-3.0,0.0) to [-2.50,-2.25] and [0,3.0] to [2.25,2.50]

	// Reminder: Linear Conversion Formula [o1,o2] to [n1,n2]
	// x' = ( (x - o1) / (o2 - o1) ) * (n2 - n1) + n1
	//
	// We retain the full formulas below to make it easier to tweak the values.
	// The compiler will optimize away the excess math.

	if (x >= 1.0f)
		x = ((x - 0.00f) / (3.00f - 0.00f)) * (2.50f - 2.25f) + 2.25f;	// map [0,3.0] to [2.25,2.50]
	else if (x <= -1.0f)
		x = ((x - -3.00f) / (0.00f - -3.00f)) * (-2.25f - -2.50f) + -2.50f;	// map to [-3.0,0] to [-2.50,-2.25]
	else if (x >= 0.0f)
		x = 2.0f;
	else if (x <= 0.0f)
		x = -2.0f;

	// If the final value is sufficiently close to zero it causes sound to come from
	// all speakers and lose it's positional effect. We scale well away from zero
	// above but will keep this check to document the effect or catch the condition
	// if the formula above is later changed to one that can result in x = 0.0.

	if (fabsf(x) < 0.0001f)
		x = 0.0001f;

	return x;
}

// This is a replacement function for PanTo3D() for sound effect fading (audio z-axis).
// It performs the same calculations but maps the resulting values to
// an area of the 3D sound stage that has the expected fading
// effect for this application. It is written in a long form to facilitate tweaking the
// values (which turned out to be more straightforward than originally coded). *njk*

float PinDirectSound::FadeSSF(float front_rear_fade)
{
	float z = 0.0f;

	// Clip the fade input range to -1.0 to 1.0

	if (front_rear_fade < -1.0f)
		z = -1.0f;
	else if (front_rear_fade > 1.0f)
		z = 1.0f;
	else
		z = front_rear_fade;

	// Rescale fade range from an exponential [0,-1] and [0,1] to a linear [-1.0, 1.0]
	// Do not avoid values close to zero like PanTo3D() does at this point as that
	// prevents the middle range of the exponential curves converting back to
	// a linear scale (which would leave a gap in the center of the range).
	// This basically undoes the AudioFade() fading function in the table scripts.

	z = (z < 0.0f) ? -powf(-z, 0.10f) : powf(z, 0.10f);

	// Increase the fade range from [-1.0, 1.0] to [-3.0, 3.0] to improve the surround sound fade effect

	z = z * 3.0f;

	// Rescale fade range from [-3.0,3.0] to [0.0,-3.0] in an attempt to remove all sound from
	// the surround sound front (backbox) speakers and place them close to the surround sound
	// side (cabinet rear) speakers.
	//
	// Reminder: Linear Conversion Formula [o1,o2] to [n1,n2]
	// z' = ( (z - o1) / (o2 - o1) ) * (n2 - n1) + n1
	//
	// We retain the full formulas below to make it easier to tweak the values.
	// The compiler will optimize away the excess math.

	z = ((z - -3.00f) / (3.00f - -3.00f)) * (-3.00f - 0.00f) + 0.00f;

	// Unfortuneately the above scaling is insufficient to keep the playfield sounds in the
	// fade range (-0.9,0.0) from playing into the surround sound front (backbox) speakers.
	// Rescaling to (-0.9.3.0] isn't ideal as -0.9 is not full effect volume on the
	// side channel. Instead we force values in the top third of the playfield (-1.0,0.0)
	// to exactly 0.0 which just plays full effect volume on the side channels.

	if (z > -1.0f)
		z = 0.0f;

	// To improve (exaggerate) the fade effect we apply the same logic to the bottom third
	// [-2,-3] of the playfield so it plays entirely on the surround sound rear channels. Mostly
	// this prevented the slingshots from being heard at all out of the side channels
	// although the accuracy of this will vary by table.

	if (z < -2.00f)
		z = -3.00f;

	// The remaining range [-1,-2] fades per DirectSound3D calculations

	z = z;

	// Clip the fade output range to 0.0 to -3.0
	//
	// This probably can never happen but is here in case the formulas above
	// change or there is a rounding issue. A result even slightly greater
	// than zero can bleed to the backbox speakers.

	if (z > 0.0f)
		z = 0.0f;
	else if (z < -3.0f)
		z = -3.0f;


	// If the final value is sufficiently close to zero it causes sound to come from
	// all speakers on some systems and lose it's positional effect. We do use 0.0 
	// above and could set the safe value there. Instead will keep this check to document 
	// the effect or catch the condition if the formula/conditions above are later changed

	if (fabsf(z) < 0.0001f)
		z = -0.0001f;

	return z;
}

PinSoundCopy::PinSoundCopy(class PinSound *pOriginal)
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

void PinSoundCopy::Play(const float volume, const float randompitch, const int pitch, const float pan, const float front_rear_fade, const int flags, const bool restart)
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
	case SNDCFG_SND3DSSF:
		m_pDS3DBuffer->SetPosition(PinDirectSound::PanSSF(pan), 0.0f, PinDirectSound::FadeSSF(front_rear_fade), DS3D_IMMEDIATE);
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

void PinSoundCopy::Stop()
{
	m_pDSBuffer->Stop();
}

HRESULT PinSoundCopy::Get3DBuffer()
{
	const HRESULT hr = m_pDSBuffer->QueryInterface(IID_IDirectSound3DBuffer, (void**)&m_pDS3DBuffer);
	if (FAILED(hr))
		ShowError("Could not get interface to 3D sound buffer.");
	else
		m_pDS3DBuffer->SetMinDistance(5.0f, DS3D_IMMEDIATE);
	return hr;
}
