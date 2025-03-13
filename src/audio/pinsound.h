// license:GPLv3+

#pragma once

#include <mutex>
#include "core/Settings.h"
#include <SDL3_mixer/SDL_mixer.h>

enum SoundOutTypes : char { SNDOUT_TABLE = 0, SNDOUT_BACKGLASS = 1 };
enum SoundConfigTypes : int { SNDCFG_SND3D2CH = 0, SNDCFG_SND3DALLREAR = 1, SNDCFG_SND3DFRONTISREAR = 2, 
                              SNDCFG_SND3DFRONTISFRONT = 3, SNDCFG_SND3D6CH = 4, SNDCFG_SND3DSSF = 5};

// Surround modes
// ==============
//
// 2CH:  Standard stereo output
//
// ALLREAR: All table effects shifted to rear channels.   This can replace the need to use two sound cards to move table audio
// inside the cab.  Default backglass audio and VPinMAME audio plays from front speakers.
//
// FRONTISFRONT: Recommended mapping for a dedicated sound card attached to the playfield.   Front channel maps to the front
// of the cab.   We "flip" the rear to the standard 2 channels, so older versions of VP still play sounds on the front most
// channels of the cab.    This mapping could also be used to place 6 channels on the playfield. 
//
// FRONTISREAR: Table effects are mapped such that the front of the cab is the rear surround channels.   If you were to play
// VPX in a home theater system with the TV in front of you, this would produce an appropriate result with the ball coming 
// from the rear channels as it gets closer to you.  
//
// 6CH: Rear of playfield shifted to the sides, and front of playfield shifted to the far rear.   Leaves front channels open
// for default backglass and VPinMAME. 
//
// SSF: 6CH still doesn't map sounds for SSF as distinctly as it could.. In this mode horizontal panning and vertical fading 
// are enhanced for a more realistic experience.

struct AudioDevice
{
   int id;
   char name[100];
   unsigned int channels; //number of speakers in this case
};

// this gets passed to all Mix_RegisterEffect callbacks
struct MixEffectsData
{
   // The output device format info.  This is the format of the audio stream that comes in to be resampled (Mix_RegisterEffect). 
   int outputFrequency;
   SDL_AudioFormat outputFormat; 
   int outputChannels;

   // These are the data points provided by VPinball to adjust the sample when resampling
   float pitch;
   float randompitch;
   float front_rear_fade;
   float pan;
   float volume;
   float nVolume; // remove once custom vol is in.
   float globalTableVolume; // Holds a 0-1 based off the Global Sound Volume Setting.  
};

class PinSound final
{
public:

   // SDL3_mixer
   Mix_Chunk * m_pMixChunkOrg = nullptr; // the original unmodified loaded sound
   Mix_Music * m_pMixMusic = nullptr; // used by PlayMusic
   Mix_Chunk * m_pMixChunk = nullptr; // we use this one when we resample for pitch changes

   //SDL Audio
   //SDL_AudioSpec m_audioSpec; // audio spec format 
   SDL_AudioStream *m_pstream = nullptr; // VPinMAME streamer
   float m_streamVolume = 0.f;

   // if the Reinitialize comes back good, we should free these in pintable.cpp or we're keeping two copies
   // one here and one from pintable.  Once everything is good we only need Mix_Chunk.   S_FIX S_REMOVE
   char *m_pdata = nullptr; // wav data set by caller directly
   int m_cdata = 0;         // wav data length set by caller directly

   // Sound filename and path
   string m_szName; // only filename, no ext
   string m_szPath; // full filename, incl. path

   // can be set in the Sound Resource Manager, only used so far by PlaySound() in pintable.cpp/VBS-script
   int m_volume = 0;
   int m_balance = 0;
   int m_fade = 0;

   // What type of sound? table or BG?  Used to route sound to the right device or channel. set by pintable
   SoundOutTypes m_outputTarget; //Is it table sound device or BG sound device. 

   // This is because when VP imports WAVs into the Windows versions it stores them in WAVEFORMATEX
   // format.  We need WAV.  So this keeps the original format for exporting/import, etc for windows. 
   // old wav code only, but also used to convert raw wavs for SDL
   WAVEFORMATEX m_wfx;
   int m_cdata_org;
   char *m_pdata_org = nullptr; // for saving WAVs in the original raw format, points into m_pdata, so do not delete it!

   PinSound() {};
   PinSound(const Settings& settings);
   ~PinSound();

   void UnInitialize();
   HRESULT ReInitialize(); // also uninits the sound
   void UpdateVolume();

   // plays the sound
   void Play(const float volume, const float randompitch, const int pitch, 
             const float pan, const float front_rear_fade, const int loopcount, const bool usesame, const bool restart);
   void Stop(); // stop sound

   // Music Playing from AudioPlayer (used by WMPCore, PlayMusic)
   bool SetMusicFile(const string& szFileName);
   void MusicPlay();
   void MusicStop();
   void MusicPause();
   void MusicUnpause();
   void MusicClose();
   bool MusicActive();
   double GetMusicPosition() const;
   void SetMusicPosition(double seconds);
   void MusicVolume(const float volume);
   bool MusicInit(const string& szFileName, const float volume);  //player.cpp

   // Plays sounds from VPinMAME and PUP.  These are streams
   bool StreamInit(DWORD frequency, int channels, const float volume);
   void StreamUpdate(void* buffer, DWORD length);
   void StreamVolume(const float volume); 

   // used by windows UI.  called by pintable
   SoundOutTypes GetOutputTarget() const { return m_outputTarget; } 

   // This is called by pintable just before Reinitialize().
   void SetOutputTarget(SoundOutTypes target) { m_outputTarget = target; }

   // Windows Editor?
   PinSound *LoadFile(const string& strFileName);

   // static class methods
   //
   // Retrieves detected audio devices detected by SDL
   static void EnumerateAudioDevices(vector<AudioDevice>& devices);

private:

   static std::mutex m_SDLAudioInitMutex;
   static bool isSDLAudioInitialized; // tracks the state of one time setup of sounds devices and mixer

   static Settings m_settings; // get key/value from VPinball.ini
   static int m_sdl_STD_idx;  // the table sound device to play sounds out of
   static int m_sdl_BG_idx;  //the BG sounds/music device to play sounds out of

   MixEffectsData m_mixEffectsData;

   // The output devices audio spec
   static SDL_AudioSpec m_audioSpecOutput;

   // SDL_mixer
   int m_assignedChannel = -1; // the mixer channel this MixChunk is assigned to

   static std::mutex m_channelUpdateMutex;
   static vector<bool> m_channelInUse; // channel pool for assignment

   // What 3Dsound Mode are we in from VPinball.ini "Sound3D" key.
   static SoundConfigTypes m_SoundMode3D;

   // This is for BG sounds that are stored in the VPX file.  Treated differently than table sounds
   void PlayBGSound(float nVolume, const int loopcount, const bool usesame, const bool restart);

   // sound file meta data extraction
   std::string getFileExt() const; // get the sound file extension
   uint16_t getChannelCountWav() const; //gets the number of channels the original WAV was encoded with

    // Play methods for each SNDCFG
   void Play_SNDCFG_SND3D2CH(float nVolume, const float randompitch, const int pitch, 
      const float pan, const float front_rear_fade, const int loopcount, const bool usesame, const bool restart);
   void Play_SNDCFG_SND3DSSF(float nVolume, const float randompitch, const int pitch, 
      const float pan, const float front_rear_fade, const int loopcount, const bool usesame, const bool restart);
   void Play_SNDCFG_SND3DALLREAR(float nVolume, const float randompitch, const int pitch, 
      const float pan, const float front_rear_fade, const int loopcount, const bool usesame, const bool restart);

   // deep copy of MixChunk's
   Mix_Chunk* copyMixChunk(const Mix_Chunk* const original);

   // Static class methods
   //
   static void initSDLAudio();
   static int getChannel(); // get an open channel assigned for the sound sample

   // we resample the original sound to match the pitch settings sent from the table each time.
   void setPitch(int pitch, float randompitch);

   // Mixer effects (Mix_RegisterEffect) callbacks
   void static SSFEffect(int chan, void *stream, int len, void *udata);
   void static MoveFrontToRearEffect(int chan, void *stream, int len, void *udata);
   void static Pan2ChannelEffect(int chan, void *stream, int len, void *udata);

   // MixEffects support funcs
   void static calcPan(float& leftPanRatio, float& rightPanRatio, float adjustedVolRatio, float pan);
   void static calcFade(float leftPanRatio, float rightPanRatio, float fadeRatio, float& frontLeft, float& frontRight, float& rearLeft, float& rearRight);
   float static PanSSF(float pan);
   float static PanTo3D(float input);
   float static FadeSSF(float front_rear_fade);
};
