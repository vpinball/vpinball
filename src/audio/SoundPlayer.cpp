// license:GPLv3+

#include "core/stdafx.h"
#include "SoundPlayer.h"

#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS 1
#define MA_ENABLE_CUSTOM 1
#include "miniaudio/miniaudio.h"


namespace VPX
{

SoundPlayer* SoundPlayer::Create(AudioPlayer* audioPlayer, Sound* sound)
{
   // Decode and resample the sound on the anciliary thread as this is fairly heavy
   return new SoundPlayer(audioPlayer, sound);
}

SoundPlayer* SoundPlayer::Create(AudioPlayer* audioPlayer, string filename)
{
   // Decode and resample the sound on the anciliary thread as this is fairly heavy
   return new SoundPlayer(audioPlayer, filename);
}

SoundPlayer::SoundPlayer(AudioPlayer* audioPlayer, string filename)
   : m_audioPlayer(audioPlayer)
   , m_outputTarget(SoundOutTypes::SNDOUT_BACKGLASS)
   , m_commandQueue(1)
{
   m_commandQueue.enqueue([this, filename]()
   {
      m_sound = std::make_unique<ma_sound>();
      if (ma_sound_init_from_file(m_audioPlayer->GetEngine(), filename.c_str(), MA_SOUND_FLAG_STREAM, nullptr, nullptr, m_sound.get()))
      {
         m_decoder = nullptr;
         m_sound = nullptr;
         return;
      }
      ma_sound_set_spatialization_enabled(m_sound.get(), (m_outputTarget != SNDOUT_BACKGLASS && m_audioPlayer->GetSoundMode3D() != SNDCFG_SND3D2CH) ? MA_TRUE : MA_FALSE);
      ma_sound_set_end_callback(m_sound.get(), OnSoundEnd, this);
   });
}

SoundPlayer::SoundPlayer(AudioPlayer* audioPlayer, Sound* sound)
   : m_audioPlayer(audioPlayer)
   , m_outputTarget(sound->GetOutputTarget())
   , m_commandQueue(1)
{
   m_commandQueue.enqueue([this, sound]()
   {
      m_decoder = std::make_unique<ma_decoder>();
      if (ma_decoder_init_memory(sound->GetFileRaw(), sound->GetFileSize(), nullptr, m_decoder.get()) != MA_SUCCESS)
      {
         m_decoder = nullptr;
         return;
      }
      m_sound = std::make_unique<ma_sound>();
      if (ma_sound_init_from_data_source(m_audioPlayer->GetEngine(), m_decoder.get(), 0, nullptr, m_sound.get()))
      {
         m_decoder = nullptr;
         m_sound = nullptr;
         return;
      }
      ma_sound_set_spatialization_enabled(m_sound.get(), (m_outputTarget != SNDOUT_BACKGLASS && m_audioPlayer->GetSoundMode3D() != SNDCFG_SND3D2CH) ? MA_TRUE : MA_FALSE);
      ma_sound_set_end_callback(m_sound.get(), OnSoundEnd, this);
   });
}

SoundPlayer::~SoundPlayer()
{
   m_commandQueue.wait_until_empty();
   m_commandQueue.wait_until_nothing_in_flight();
   if (m_sound)
      ma_sound_uninit(m_sound.get());
   if (m_decoder)
      ma_decoder_uninit(m_decoder.get());
}

void SoundPlayer::SetMainVolume(float backglassVolume, float playfieldVolume)
{
   m_mainVolume = m_outputTarget == SNDOUT_BACKGLASS ? backglassVolume : playfieldVolume;
   m_commandQueue.enqueue([this]() { ApplyVolume(); });
}

void SoundPlayer::SetVolume(float volume) {
   m_soundVolume = volume;
   m_commandQueue.enqueue([this]() { ApplyVolume(); });
}


void SoundPlayer::ApplyVolume()
{
   if (m_sound)
   {
      const float totalvolume = clamp(m_soundVolume * m_mainVolume, 0.0f, 1.0f);
      // VP legacy conversion:
      // const float decibelvolume = (totalvolume == 0.0f) ? -100.f : max(logf(totalvolume) * (float)(10.0 / log(10.0)) - 20.0f, -100.f);
      // const float decibelvolume = logf(totalvolume) * (float)(10.0 / log(10.0)) - 20.0f; // as we don't need to handle silence separately with linear volume
      // const float decibelvolume = 10.f * log10f(totalvolume) - 20.f;
      // const float linearvolume = powf(10.f, 10.f * log10f(totalvolume) / 20.f - 1.f); // since linear = powf(10.f, decibel gain / 20.f)
      // const float linearvolume = powf(10.f, log10f(sqrt(totalvolume)) - 1.f);
      // const float linearvolume = sqrt(totalvolume) / 10.f; // we don't keep the 1/10 factor as this is (and should) be part of the main volume setup
      ma_sound_set_volume(m_sound.get(), sqrtf(totalvolume));
   }
}

void SoundPlayer::Play(float volume, const float randompitch, const int pitch, float pan, float frontRearFade, const int loopcount)
{
   m_commandQueue.enqueue([this, volume, randompitch, pitch, pan, frontRearFade, loopcount]()
   {
      if (m_sound == nullptr)
         return;

      switch (m_outputTarget == SNDOUT_BACKGLASS ? SNDCFG_SND3D2CH : m_audioPlayer->GetSoundMode3D())
      {
      case SNDCFG_SND3D2CH: ma_sound_set_pan(m_sound.get(), pan); break;
      case SNDCFG_SND3DALLREAR: ma_sound_set_position(m_sound.get(), PanTo3D(pan), 0.0f, -PanTo3D(1.0f)); break;
      case SNDCFG_SND3DFRONTISFRONT: ma_sound_set_position(m_sound.get(), PanTo3D(pan), 0.0f, PanTo3D(frontRearFade)); break;
      case SNDCFG_SND3DFRONTISREAR: ma_sound_set_position(m_sound.get(), PanTo3D(pan), 0.0f, -PanTo3D(frontRearFade)); break;
      case SNDCFG_SND3D6CH: ma_sound_set_position(m_sound.get(), PanTo3D(pan), 0.0f, -(PanTo3D(frontRearFade) + 3.f) / 2.f); break;
      case SNDCFG_SND3DSSF: ma_sound_set_position(m_sound.get(), PanSSF(pan), 0.0f, -FadeSSF(frontRearFade)); break;
      default: assert(false); return;
      }

      m_loopCount = loopcount;

      ma_format format;
      ma_uint32 channels;
      ma_uint32 sampleRate;
      ma_sound_get_data_format(m_sound.get(), &format, &channels, &sampleRate, nullptr, 0);
      const float sampleFreq = static_cast<float>(sampleRate);
      float newFreq = sampleFreq + pitch;
      if (randompitch > 0.f)
      {
         const float rndh = rand_mt_01();
         const float rndl = rand_mt_01();
         newFreq *= 1.f + (randompitch * rndh * rndh) - (randompitch * rndl * rndl * 0.5f);
      }
      ma_sound_set_pitch(m_sound.get(), newFreq / sampleFreq);

      m_soundVolume = volume;
      ApplyVolume();

      if (!IsPlaying())
         ma_sound_start(m_sound.get());
   });
}

void SoundPlayer::Pause()
{
   m_commandQueue.enqueue([this]()
   {
      if (m_sound)
         ma_sound_stop(m_sound.get());
   });
}

void SoundPlayer::Unpause()
{
   m_commandQueue.enqueue([this]()
   {
      if (m_sound)
         ma_sound_start(m_sound.get());
   });
}

void SoundPlayer::Stop()
{
   m_commandQueue.enqueue([this]()
   {
      if (m_sound)
         ma_sound_stop(m_sound.get());
   });
}

float SoundPlayer::GetPosition()
{
   float pos = 0.f;
   if (ma_sound_get_cursor_in_seconds(m_sound.get(), &pos) == MA_SUCCESS)
      return pos;
   return 0.f;
}

void SoundPlayer::SetPosition(float pos)
{
   m_commandQueue.enqueue([this, pos]()
   {
      if (m_sound)
         ma_sound_seek_to_second(m_sound.get(), pos);
   });
}

bool SoundPlayer::IsPlaying() const
{
   return m_sound && ma_sound_is_playing(m_sound.get());
}

void SoundPlayer::OnSoundEnd(void* pUserData, ma_sound* pSound)
{
   SoundPlayer* me = static_cast<SoundPlayer*>(pUserData);
   if (me->m_loopCount == 0)
      return;
   if (me->m_loopCount > 0)
      me->m_loopCount--;
   // Dispatch through the command queue since we can not restart the sound from the callback as the sound is still playing and command would be discarded
   me->m_commandQueue.enqueue([pSound]() { ma_sound_start(pSound); });
}

// The existing pan value in PlaySound function takes a -1 to 1 value, however it's extremely non-linear.
// -0.1 is very obviously to the left.  Table scripts like the ball rolling script seem to use x^10 to map
// linear positions, so we'll use that and reverse it.   Also multiplying by 3 since that seems to be the
// the total distance necessary to fully pan away from one side at the center of the room.

float SoundPlayer::PanTo3D(float input)
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

// This is a replacement function for PanTo3D() for sound effect panning (audio x-axis).
// It performs the same calculations but maps the resulting values to an area of the 3D 
// sound stage that has the expected panning effect for this application. It is written 
// in a long form to facilitate tweaking the formulas.  *njk*

float SoundPlayer::PanSSF(float pan)
{
	// This math could probably be simplified but it is kept in long form
	// to aide in fine tuning and clarity of function.

	// Clip the pan input range to -1.0 to 1.0
	float x = clamp(pan, -1.f, 1.f);

	// Rescale pan range from an exponential [-1,0] and [0,1] to a linear [-1.0, 1.0]
	// Do not avoid values close to zero like PanTo3D() does as that
	// prevents the middle range of the exponential curves converting back to 
	// a linear scale (which would leave a gap in the center of the range).
	// This basically undoes the Pan() fading function in the table scripts.

	x = (x < 0.0f) ? -powf(-x, 0.1f) : powf(x, 0.1f);

	// Increase the pan range from [-1.0, 1.0] to [-3.0, 3.0] to improve the surround sound fade effect

	x *= 3.0f;

	// BASS pan effect is much better than VPX 10.6/DirectSound3d but it
	// could still stand a little enhancement to exaggerate the effect.
	// The effect goal is to place slingshot effects almost entirely left/right
	// and flipper effects in the cross fade region (louder on their corresponding
	// sides but still audible on the opposite side..)

	// Rescale [-3.0,0.0) to [-3.00,-2.00] and [0,3.0] to [2.00,3.00]

	// Reminder: Linear Conversion Formula [o1,o2] to [n1,n2]
	// x' = ( (x - o1) / (o2 - o1) ) * (n2 - n1) + n1
	//
	// We retain the full formulas below to make it easier to tweak the values.
	// The compiler will optimize away the excess math.

	if (x >= 0.0f)
		x = ((x -  0.0f) / (3.0f -  0.0f)) * ( 3.0f -  2.0f) +  2.0f;
	else
		x = ((x - -3.0f) / (0.0f - -3.0f)) * (-2.0f - -3.0f) + -2.0f;

	// Clip the pan output range to 3.0 to -3.0
	//
	// This probably can never happen but is here in case the formulas above
	// change or there is a rounding issue.

	if (x > 3.0f)
		x = 3.0f;
	else if (x < -3.0f)
		x = -3.0f;

	// If the final value is sufficiently close to zero it causes sound to come from
	// all speakers and lose it's positional effect. We scale well away from zero
	// above but will keep this check to document the effect or catch the condition
	// if the formula above is later changed to one that can result in x = 0.0.

	// NOTE: This no longer seems to be the case with VPX 10.7/BASS

	// HOWEVER: Weird things still happen NEAR 0.0 or if both x and z are at 0.0.
	//          So we keep the fix here with wider margins to prevent that case.
	//          The current formula won't produce values in this weird range.

	if (fabsf(x) < 0.1f)
		x = (x < 0.0f) ? -0.1f : 0.1f;

	return x;
}

// This is a replacement function for PanTo3D() for sound effect fading (audio z-axis).
// It performs the same calculations but maps the resulting values to 
// an area of the 3D sound stage that has the expected fading
// effect for this application. It is written in a long form to facilitate tweaking the 
// values (which turned out to be more straightforward than originally coded). *njk*

float SoundPlayer::FadeSSF(float front_rear_fade)
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

	z = (z < 0.0f) ? -powf(-z, 0.1f) : powf(z, 0.1f);

	// Increase the fade range from [-1.0, 1.0] to [-3.0, 3.0] to improve the surround sound fade effect

	z *= 3.0f;

	// Rescale fade range from [-3.0,3.0] to [0.0,-2.5] in an attempt to remove all sound from
	// the surround sound front (backbox) speakers and place them close to the surround sound
	// side (cabinet rear) speakers.
	//
	// Reminder: Linear Conversion Formula [o1,o2] to [n1,n2]
	// z' = ( (z - o1) / (o2 - o1) ) * (n2 - n1) + n1
	//
	// We retain the full formulas below to make it easier to tweak the values.
	// The compiler will optimize away the excess math.

	// Rescale to -2.5 instead of -3.0 to further push sound away from rear channels
	z = ((z - -3.0f) / (3.0f - -3.0f)) * (-2.5f - 0.0f) + 0.0f;

	// With BASS the above scaling is sufficient to keep the playfield sounds out of 
	// the backbox. However playfield sounds are heavily weighted to the rear channels. 
	// For BASS we do a simple scale of the top third [0,-1.0] BY 0.10 to favor
	// the side channels. This is better than we could do in VPX 10.6 where z just
	// had to be set to 0.0 as there was no fade range that didn't leak to the backbox
	// as well.
	
	if (z > -1.0f)
		z = z / 10.0f;

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

	// NOTE: This no longer seems to be the case with VPX 10.7/BASS

	// HOWEVER: Weird things still happen near 0.0 or if both x and z are at 0.0.
	//          So we keep the fix here to prevent that case. This does push a tiny bit 
	//          of audio to the rear channels but that is perfectly ok.

	if (fabsf(z) < 0.0001f)
		z = -0.0001f;
	
	return z;
}

}
