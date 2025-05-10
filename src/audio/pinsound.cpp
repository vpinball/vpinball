// license:GPLv3+

#include "core/stdafx.h"

#include <iomanip>

#include <SDL3_mixer/SDL_mixer.h>

// SDL Sound Device Id for each output 
int PinSound::m_sdl_STD_idx = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;  // the table sounds
int PinSound::m_sdl_BG_idx  = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;  // the BG sounds/music

// state of sound device and mixer setup
std::mutex PinSound::m_SDLAudioInitMutex;
bool PinSound::isSDLAudioInitialized = false;

// The output audio spec that the device is actually set for.
SDL_AudioSpec PinSound::m_audioSpecOutput;

std::mutex PinSound::m_channelUpdateMutex;
vector<bool> PinSound::m_channelInUse; // channel pool for assignment

// holds the setting from VPinball.ini that says what SoundMode we're in.
SoundConfigTypes PinSound::m_SoundMode3D;

/**
 * @brief Constructor for the PinSound class. Initializes SDL audio, configures output devices,
 *        and prepares the audio specifications based on the provided settings.
 *
 * This constructor checks whether SDL audio has been initialized. If not, it initializes SDL audio
 * and configures the output audio specifications using the Mix_QuerySpec function. The output audio
 * specifications are stored in internal member variables for further processing.
 *
 * @param settings The configuration settings for the audio system, passed by reference.
 * 
 * @note SDL audio initialization is done only once. If it has already been initialized, the constructor
 *       will skip the initialization process and directly set up the output parameters.
 *
 * @note This function logs the output audio specifications (frequency, format, channels) after the SDL
 *       audio system has been initialized.
 */
PinSound::PinSound(const Settings& settings)
{
   {
      const std::lock_guard<std::mutex> lg(m_SDLAudioInitMutex);

      if (!isSDLAudioInitialized) {
         PinSound::initSDLAudio(settings);
         isSDLAudioInitialized = true;
         const char* pdriverName = SDL_GetCurrentAudioDriver();
        
         // Set the output devices AudioSpec
         Mix_QuerySpec(&m_mixEffectsData.outputFrequency, &m_mixEffectsData.outputFormat, &m_mixEffectsData.outputChannels); // the struct that gets passed to the MixEffect callbacks.
         Mix_QuerySpec(&m_audioSpecOutput.freq, &m_audioSpecOutput.format, &m_audioSpecOutput.channels);

         PLOGI << "Output Device Settings: " << "Freq: " << m_audioSpecOutput.freq << " Format (SDL_AudioFormat): " << m_audioSpecOutput.format
            << " channels: " << m_audioSpecOutput.channels << ", driver: " << (pdriverName ? pdriverName : "NULL") ;
      }
   }

   // set the MixEffects output params that are used for resampling the incoming stream to callback.
   Mix_QuerySpec(&m_mixEffectsData.outputFrequency, &m_mixEffectsData.outputFormat, &m_mixEffectsData.outputChannels); 
}

PinSound::~PinSound()
{
   UnInitialize();

   delete [] m_pdata;

   // do not de-init as many PinSound's potentially 'share' it
   //SDL_QuitSubSystem(SDL_INIT_AUDIO);
   //isSDLAudioInitialized = false;
}

/**
 * (Static)
 * @brief Initializes SDL audio and sets up the sound devices for table and background sounds.
 *
 * This function attempts to load sound device settings from a configuration file (VPinball.ini) and 
 * assigns the appropriate SDL audio devices for playing table and background sounds. If no matching 
 * sound devices are found, it defaults to the standard SDL audio devices. Additionally, the function 
 * initializes SDL audio and SDL_Mixer, configures the sound mode for 3D audio, and allocates the 
 * necessary audio channels.
 * 
 * @note If no sound device names are found in the configuration file, default devices are used. 
 *       A warning is logged in this case. SDL audio initialization is attempted, and failure is 
 *       logged with an error message.
 *
 * @see SDL_InitSubSystem(SDL_INIT_AUDIO), Mix_OpenAudio(), SDL_GetAudioDeviceFormat(), Mix_AllocateChannels()
 */
void PinSound::initSDLAudio(const Settings& settings)
{
   string soundDeviceName;
   string soundDeviceBGName;
   const bool good = settings.LoadValue(Settings::Player, "SoundDevice"s, soundDeviceName);
   const bool good2 = settings.LoadValue(Settings::Player, "SoundDeviceBG"s, soundDeviceBGName);

    if (!good && !good2) { // use the default SDL audio device
      PLOGI << "Sound Device not set in VPinball.ini.  Using default";
      m_sdl_STD_idx = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
      m_sdl_BG_idx =  SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
    }
    else { // this is all because the device id's are random: https://github.com/libsdl-org/SDL/issues/12278
      vector<AudioDevice> allAudioDevices;
      PinSound::EnumerateAudioDevices(allAudioDevices);
      for (size_t i = 0; i < allAudioDevices.size(); ++i) {
         const AudioDevice& audioDevice = allAudioDevices[i];
         if (audioDevice.name == soundDeviceName)
            m_sdl_STD_idx = audioDevice.id;
         if (audioDevice.name == soundDeviceBGName)
            m_sdl_BG_idx = audioDevice.id;
      }

      if (m_sdl_STD_idx == SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK) { // we didn't find a matching name
         PLOGE << "No sound device by that name found in VPinball.ini. " << "SoundDevice:\"" << soundDeviceName << "\" SoundDeviceBG:\"" << 
            soundDeviceBGName << "\" Using default.";
         m_sdl_STD_idx = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
         m_sdl_BG_idx  = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
      }
   }

   PinSound::m_SoundMode3D = static_cast<SoundConfigTypes>(settings.LoadValueUInt(Settings::Player, "Sound3D"s));

   if (!SDL_WasInit(SDL_INIT_AUDIO)) {
      if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
         PLOGE << "Failed to initialize SDL Audio: " << SDL_GetError();
         return;
      }
   }

   // change the AudioSpec param when we know what sound format output we want.  or get from device
   if (!Mix_OpenAudio(m_sdl_STD_idx, nullptr)) {
      PLOGE << "Failed to initialize SDL Mixer: " << SDL_GetError();
      return;
   }

   SDL_AudioSpec spec;
   int sample_frames;
   SDL_GetAudioDeviceFormat(m_sdl_STD_idx, &spec, &sample_frames);

   const int maxSDLMixerChannels = Mix_AllocateChannels(100); // set the max channel pool
   m_channelInUse.resize(maxSDLMixerChannels, false);
   PLOGI << "SDL Mixer allocated " << maxSDLMixerChannels << " channels.";
}

void PinSound::UnInitialize()
{
   if (m_assignedChannel != -1) {
      const std::lock_guard<std::mutex> lg(m_channelUpdateMutex);

      Mix_HaltChannel(m_assignedChannel);
      m_channelInUse[m_assignedChannel] = false;
      m_assignedChannel = -1;
   }

   if (m_pMixChunkOrg != nullptr) {
      Mix_FreeChunk(m_pMixChunkOrg);
      m_pMixChunkOrg = nullptr;
   }
   if (m_pMixChunk != nullptr) { // free the last converted sample
      Mix_FreeChunk(m_pMixChunk);
      m_pMixChunk = nullptr;
   }
   if (m_pMixMusic != nullptr) {
      Mix_FreeMusic(m_pMixMusic);
      m_pMixMusic = nullptr;
   }
   if (m_pstream) {
      SDL_DestroyAudioStream(m_pstream);
      m_pstream = nullptr;
   }
}

/**
 * @brief Reinitializes the sound by uninitializing and reloading the sound file.
 * 
 * This function performs the following tasks:
 * - Calls `UnInitialize()` to reset any previous state.
 * - Attempts to load a sound from memory using `SDL_IOFromMem` and stores it in `m_psdlIOStream`.
 * - If the sound loading fails, an error message is logged using `PLOGE`, and the function returns `E_FAIL`.
 * - Uses `Mix_LoadWAV_IO` to load the sound file as a `Mix_Chunk` for playback.
 * - If the sound loading fails, an error message is logged using `PLOGE`, and the function returns `E_FAIL`.
 * - Attempts to assign an available mixer channel using the `getChannel()` method. If no channels are available, an error message is logged using `PLOGE`, and the function returns `E_FAIL`.
 * 
 * @return HRESULT Returns `S_OK` on successful reinitialization, or `E_FAIL` if any errors occur during the process.
 * 
 * @note This is the main function for loading sounds from the tables via pintable.cpp
 */
HRESULT PinSound::ReInitialize()
{
   UnInitialize();

   SDL_IOStream* const psdlIOStream = SDL_IOFromMem(m_pdata, static_cast<int>(m_cdata)); 

   if (!psdlIOStream) {
      PLOGE << "SDL_IOFromMem error: " << SDL_GetError();
      return E_FAIL;
   }

   if (!(m_pMixChunkOrg = Mix_LoadWAV_IO(psdlIOStream, true))) {
      PLOGE << "Failed to load sound via Mix_LoadWAV_IO: " << SDL_GetError();
      return E_FAIL;
   }

   // assign a channel to sound
   if ((m_assignedChannel = getChannel()) == -1) { // no more channels
      PLOGE << "There are no more SDL mixer channels available to be allocated.";
      return E_FAIL;
   }

   /* PLOGI << "Loaded Sound File: " << m_szName << " Sound Type: " << extension_from_path(m_szPath) << 
      " # of Audio Channels: " << ( (extension_from_path(m_szPath) == "wav") ? std::to_string(getChannelCountWav() ) : "Unknown" ) <<
      " Assigned Channel: " << m_assignedChannel << " SoundOut (0=table, 1=bg): " << (int)m_outputTarget; */

   return S_OK;
}

void PinSound::UpdateVolume()
{
   if (m_outputTarget == SNDOUT_BACKGLASS) {
      if (m_pMixMusic != nullptr) {
         // Backglass music (requested volume is stored in the mix effect as for other sounds)
         MusicVolume(m_mixEffectsData.volume);
      }
      else if (m_pstream != nullptr) {
         // Backglass stream use stream volume (requested volume is stored in the mix effect as for other sounds)
         SDL_SetAudioStreamGain(m_pstream, m_streamVolume * (g_pplayer ? dequantizeSignedPercent(g_pplayer->m_MusicVolume) : 1.f));
      }
      else if (m_assignedChannel != -1) {
         // Backglass sound use mixer volume (requested volume is stored in the mix effect as for other sounds)
         const int nVolume = static_cast<int>(m_mixEffectsData.volume * (g_pplayer ? dequantizeSignedPercent(g_pplayer->m_MusicVolume) : 1.f) * static_cast<float>(MIX_MAX_VOLUME));
         Mix_Volume(m_assignedChannel, nVolume);
      }
   }
   else {
      // Playfield sounds use volume stored in m_mixEffectsData when rendering
      const float newSoundVolume = g_pplayer ? dequantizeSignedPercent(g_pplayer->m_SoundVolume) : 1.f;
      if (m_mixEffectsData.globalTableVolume == 0.f)
         m_mixEffectsData.nVolume = m_mixEffectsData.volume * newSoundVolume;
      else
         m_mixEffectsData.nVolume = m_mixEffectsData.nVolume * newSoundVolume / m_mixEffectsData.globalTableVolume;
      m_mixEffectsData.globalTableVolume = newSoundVolume;
   }
}

/**
 * @brief Plays a sound with specified properties such as volume, pitch, pan, and loop count.
 * 
 * This function adjusts the volume and applies various sound effects and mixing parameters based on the sound mode and output target.
 * It supports different sound configurations, including stereo, 3D surround, and background sounds, and adjusts them accordingly.
 * 
 * @param volume The desired volume level for the sound (from 0.0 to 1.0). The volume is clamped to a minimum threshold.
 * @param randompitch The random variation in pitch to apply to the sound. The value should be between 0.0 and 1.0.
 * @param pitch The base pitch of the sound (in Hertz).
 * @param pan The pan of the sound, ranging from -1.0 (left) to 1.0 (right), where 0.0 is the center.
 * @param front_rear_fade A fade value determining the balance between front and rear sound channels, from -1.0 (front) to 1.0 (rear).
 * @param loopcount The number of times the sound should loop. A value of 0 means no looping.
 * @param usesame Boolean flag indicating whether to use the same instance of the sound for playback.
 * @param restart Boolean flag indicating whether to restart the sound from the beginning.
 * 
 * This function first applies a volume adjustment based on the global table sound setting. It then chooses a sound playback path depending on the configured sound mode. 
 * The supported modes include 2-channel stereo, all-rear 3D surround sound, and various 3D configurations with 6 or 8 channels.
 * 
 * In case of background sounds (backglass sounds), the function adjusts the volume and plays the sound in a specific manner based on the table's configuration.
 * 
 * Possible sound modes are:
 * - SNDCFG_SND3D2CH: 2-channel stereo sound.
 * - SNDCFG_SND3DALLREAR: 3D surround sound with all rear channels.
 * - SNDCFG_SND3DFRONTISREAR: Not yet implemented.
 * - SNDCFG_SND3DFRONTISFRONT: Not yet implemented.
 * - SNDCFG_SND3D6CH: 6-channel sound configuration.
 * - SNDCFG_SND3DSSF: 8-channel sound configuration with specific pan and fade algorithms.
 * 
 * If the sound device does not support the required number of channels for the selected mode, an error message is logged.
 * 
 * @note If the output target is set to `SNDOUT_BACKGLASS`, the function processes the background sound differently.
 * 
 * @note Called to play the table sounds via pintable.cpp.  This is the main call from the tables for playing sound.
 */
void PinSound::Play(float volume, const float randompitch, const int pitch, 
                    float pan, float front_rear_fade, const int loopcount, const bool usesame, const bool restart)
{
   // cumulatively add the settings from the table/sound-manager to the incoming values
   volume += dequantizeSignedPercent(m_volume);
   pan += dequantizeSignedPercent(m_pan);
   front_rear_fade += dequantizeSignedPercent(m_frontRearFade);

   if (g_pplayer && g_pplayer->m_ptable->m_tblMirrorEnabled)
      pan = -pan;

   // Clamp volume
   constexpr float minVol = .08f;  // some table sounds like rolling are extremely low.  Set a minimum or you cant hear it.

   // setup the struct for the effects processing
   //m_mixEffectsData.pitch = static_cast<float>(pitch);
   //m_mixEffectsData.randompitch = randompitch;
   m_mixEffectsData.front_rear_fade = front_rear_fade;
   m_mixEffectsData.pan = pan;
   m_mixEffectsData.volume = clamp(volume, 0.0f, 1.0f);

   // BG Sound is handled differently then table sounds.  These are BG sounds stored in the table (vpx file).
   if (m_outputTarget == SNDOUT_BACKGLASS) {
      // adjust volume against the tables global sound setting
      PlayBGSound(clamp(volume * (g_pplayer ? dequantizeSignedPercent(g_pplayer->m_MusicVolume) : 1.0f) + minVol, 0.0f, 1.0f), loopcount, usesame, restart);
      return;
   }

   // adjust volume against the tables global sound setting
   m_mixEffectsData.globalTableVolume = g_pplayer ? dequantizeSignedPercent(g_pplayer->m_SoundVolume) : 1.0f;
   m_mixEffectsData.nVolume = clamp(volume * m_mixEffectsData.globalTableVolume + minVol, 0.0f, 1.0f);

   switch(PinSound::m_SoundMode3D)
   {
      case SNDCFG_SND3D2CH:
         Play_SNDCFG_SND3D2CH(m_mixEffectsData.nVolume, randompitch, pitch, pan, front_rear_fade, loopcount, usesame, restart);
         break;
      case SNDCFG_SND3DALLREAR:
         if (m_mixEffectsData.outputChannels < 4) { // channel count must be at least 4.  Front and Rear
            PLOGE << "Your sound device does not have the required number of channels (4+) to support this mode. <SND3DALLREAR>";
            break;
         }
         Play_SNDCFG_SND3DALLREAR(m_mixEffectsData.nVolume, randompitch, pitch, pan, front_rear_fade, loopcount, usesame, restart);
         break;
      case SNDCFG_SND3DFRONTISREAR:
         PLOGI << "Sound Mode SNDCFG_SND3DFRONTISREAR not implemented yet.";
         break;
      case SNDCFG_SND3DFRONTISFRONT:
         PLOGI << "Sound Mode SNDCFG_SND3DFRONTISFRONT not implemented yet.";
         break;
      case SNDCFG_SND3D6CH:
         // we just fall through to the SSF.  This mode is same but it used two different pan and fade algos.  No need to have two different ones now.
      case SNDCFG_SND3DSSF:
         if (m_mixEffectsData.outputChannels != 8) {
            PLOGE << "Your sound device does not have the required number of channels (8) to support this mode. <SNDCFG_SND3DSSF>";
            break;
         }
         Play_SNDCFG_SND3DSSF(m_mixEffectsData.nVolume, randompitch, pitch, pan, front_rear_fade, loopcount, usesame, restart);
         break;
      default:
         PLOGE << "Invalid setting for 'Sound3D' in VPinball.ini...";
         break;
   }
}

/**
 * @brief Plays background sound on a specified channel with volume control and looping options.
 * 
 * This function manages the playback of a background sound on the assigned audio channel, ensuring that it handles situations 
 * where the sound is already playing. It provides options for adjusting the volume, looping behavior, and whether or not 
 * to restart the sound if it is already playing.
 * 
 * @param volume The volume level at which the sound must be played. The value must be between 0 and 1, without global volume applied.
 * @param loopcount The number of times the sound should loop. A value of 0 means the sound will play only once.
 * @param usesame A boolean flag that determines if the current sound should be reused when already playing. If false, 
 *                the sound is reloaded and restarted.
 * @param restart A boolean flag that determines if the sound should be restarted if it is already playing. If true, 
 *                the sound will stop and start again; if false, it will continue playing.
 * 
 * @note This function uses the SDL_Mixer library to handle audio playback and assumes that the sound is loaded into 
 *       `m_pMixChunkOrg` before calling this function. It also uses the channel specified in `m_assignedChannel` to play the sound.
 * 
 * If the sound is already playing on the assigned channel, the function checks the `restart` and `usesame` flags:
 *   - If `restart` is true or `usesame` is false, the function stops the current sound and restarts it.
 *   - If neither condition is met, the sound continues playing without modification.
 * If the sound is not playing, the function starts the sound immediately with the specified volume.
 */
void PinSound::PlayBGSound(float volume, const int loopcount, const bool usesame, const bool restart)
{
   //PLOGI << "PlayBGSound File: " << m_szName << " BGSOUND nVolume: " << nVolume << " Table Music Volume: " << g_pplayer->m_MusicVolume;

   if (m_assignedChannel == -1)
      return;

   const int nVolume = static_cast<int>(volume * static_cast<float>(MIX_MAX_VOLUME));
   if (Mix_Playing(m_assignedChannel)) {
      if (restart || !usesame) // stop and reload
         Mix_HaltChannel(m_assignedChannel);
      Mix_Volume(m_assignedChannel, nVolume);
      if (restart || !usesame) // stop and reload
         Mix_PlayChannel(m_assignedChannel, m_pMixChunkOrg, loopcount > 0 ? loopcount -1 : loopcount);
   }
   else { // not playing
      Mix_Volume(m_assignedChannel, nVolume);
      Mix_PlayChannel(m_assignedChannel, m_pMixChunkOrg, loopcount > 0 ? loopcount -1 : loopcount);
   }
}

/**
 * @brief Sets the pitch of the sound, optionally with random variation.
 *
 * This function adjusts the pitch of the sound associated with the `PinSound` object. If a non-zero pitch or random 
 * pitch variation is provided, the function resamples the sound to the new pitch. If the pitch remains unchanged
 * (both pitch and randompitch are zero), the original sound is used.
 * 
 * The function first frees any previously allocated `Mix_Chunk` to ensure the memory is managed correctly. 
 * If the pitch change is needed, the function calculates the new frequency (taking into account any random variation)
 * and performs the resampling process to convert the original audio sample to the new pitch. It then converts the
 * audio back to the original format, ensuring the sound is ready for playback at the correct pitch.
 * 
 * @note Since mixer doesn't support adjusting the frequency or the stream buffer length via a MixEffect
 *       we must resample the audio each time and replace it.
 *
 * @param pitch The target pitch to apply to the sound. A positive or negative integer alters the frequency.
 * @param randompitch The random variation to apply to the pitch, influencing the final frequency. A value greater than 0 introduces a random factor, while 0 means no random variation.
 */
void PinSound::setPitch(int pitch, float randompitch)
{
   if (m_pMixChunk != nullptr) { // free the last converted sample
      Mix_FreeChunk(m_pMixChunk);
      m_pMixChunk = nullptr;
   }

   // check for pitch and resample or pass the original mixchunk if pitch didn't change
   if (pitch == 0 && randompitch == 0) // If the pitch isn't changed pass the original
      m_pMixChunk = copyMixChunk(m_pMixChunkOrg);
   else {
      Mix_Chunk* const mixChunkConvert = (Mix_Chunk*)SDL_malloc(sizeof(Mix_Chunk)); // need to use SDL_malloc as SDL_free will be used when freeing the chunk
      mixChunkConvert->allocated = 1; // you need this set or it won't get freed with Mix_FreeChunk
      mixChunkConvert->volume = 128;

      int newFreq = 0;

      if (randompitch > 0) {
         const float rndh = rand_mt_01();
         const float rndl = rand_mt_01();
         int freq = (int)((float)m_mixEffectsData.outputFrequency + ((float)m_mixEffectsData.outputFrequency * randompitch * rndh * rndh) - ((float)m_mixEffectsData.outputFrequency * 
           randompitch * rndl * rndl * 0.5f));
         newFreq = freq + pitch; // add the normal pitch in if its set
         //PLOGI << " random: new freq = " << newFreq;
      }
      else // just pitch is set
         newFreq = m_mixEffectsData.outputFrequency + pitch;

      //PLOGI << "Channel: " << m_assignedChannel << " Current freq: " << m_mixEffectsData.outputFrequency << " Pitch: " << pitch << " Random pitch: " << randompitch << " Ending new freq = " << newFreq;

      SDL_AudioSpec audioSpecConvert;
      Mix_QuerySpec(&audioSpecConvert.freq, &audioSpecConvert.format, &audioSpecConvert.channels);
      audioSpecConvert.freq = newFreq;

      // convert original sample to the new freq
      SDL_ConvertAudioSamples(&m_audioSpecOutput, m_pMixChunkOrg->abuf, (int) m_pMixChunkOrg->alen, &audioSpecConvert, &mixChunkConvert->abuf, (int *) &mixChunkConvert->alen);

      // now convert it back to the original output AudioSpec
      m_pMixChunk = (Mix_Chunk*)SDL_malloc(sizeof(Mix_Chunk)); // need to use SDL_malloc as SDL_free will be used when freeing the chunk
      m_pMixChunk->volume = 128;
      m_pMixChunk->allocated = 1; // you need this set, or it won't get freed with Mix_FreeChunk
      SDL_ConvertAudioSamples(&audioSpecConvert, mixChunkConvert->abuf, (int) mixChunkConvert->alen, &m_audioSpecOutput, &m_pMixChunk->abuf, (int *) &m_pMixChunk->alen);

      Mix_FreeChunk(mixChunkConvert);
   }
}

/**
 * @brief Plays a 3D sound effect with customizable volume, pitch, pan, and loop settings.
 * 
 * This function is responsible for playing a sound effect on the specified channel with 3D audio configuration. It allows
 * customization of the volume, pitch, pan, front-to-rear fade, loop count, and whether the sound should restart or 
 * use the same configuration as the previous playback. If the sound is already playing, it can either be 
 * restarted or kept using the same settings. The function also registers an effect for moving the sound from the
 * front to the rear, which is applied during playback.
 * 
 * @param nVolume The volume of the sound (range from 0.0 to 1.0).
 * @param randompitch A random variation to apply to the pitch.
 * @param pitch The pitch of the sound.
 * @param pan The pan position of the sound (left to right).
 * @param front_rear_fade A value controlling the fade effect from front to rear.
 * @param loopcount The number of times to loop the sound. A value of 0 means no looping.
 * @param usesame A boolean indicating whether to reuse the same sound configuration (true) or restart it (false).
 * @param restart A boolean that indicates if the sound should be restarted when already playing.
 */
void PinSound::Play_SNDCFG_SND3DALLREAR(float nVolume, const float randompitch, const int pitch, 
               const float pan, const float front_rear_fade, const int loopcount, const bool usesame, const bool restart)
{
   if (m_assignedChannel == -1)
      return;

   /* PLOGI << std::fixed << std::setprecision(7) << "Playing Sound: " << m_szName << " SoundOut (0=table, 1=bg): " << 
        (int) m_outputTarget << " nVol: " << nVolume << " pan: " << pan <<
        " Pitch: "<< pitch << " Random pitch: " << randompitch  << " front_rear_fade: " << front_rear_fade <<   " loopcount: " << loopcount << " usesame: " << 
        usesame <<  " Restart? " << restart; */

   if (Mix_Playing(m_assignedChannel)) {
      if (restart || !usesame) { // stop and reload  
         Mix_HaltChannel(m_assignedChannel);
         setPitch(pitch, randompitch);
         // register the effects.  must do this each time before PlayChannel and once the sound is done its unregistered automatically
         Mix_RegisterEffect(m_assignedChannel, PinSound::MoveFrontToRearEffect, nullptr, &m_mixEffectsData);
         Mix_PlayChannel(m_assignedChannel, m_pMixChunk, loopcount > 0 ? loopcount -1 : loopcount);
      }
   }
   else { // not playing
      setPitch(pitch, randompitch);
      // register the effects.  must do this each time before PlayChannel and once the sound is done its unregistered automatically
      Mix_RegisterEffect(m_assignedChannel, PinSound::MoveFrontToRearEffect, nullptr, &m_mixEffectsData);
      Mix_PlayChannel(m_assignedChannel, m_pMixChunk, loopcount > 0 ? loopcount -1 : loopcount);
   }
}

/**
 * @brief Plays a 3D sound on a given audio channel with specified attributes.
 * 
 * This function manages the playback of a 3D sound by adjusting parameters such as volume, pitch, pan, 
 * and looping behavior. It also handles restarting or stopping the sound if necessary, as well as applying
 * effects before playing the sound on the assigned channel.
 * 
 * @param nVolume The desired volume level for the sound (0.0 to 1.0).
 * @param randompitch A random pitch adjustment factor to add variation to the sound's pitch.
 * @param pitch The base pitch of the sound.
 * @param pan A value representing the panning of the sound (left-right balance).
 * @param front_rear_fade A factor for adjusting the front-to-rear fade of the sound.
 * @param loopcount The number of times the sound should loop. A value of 0 indicates no looping.
 * @param usesame If true, prevents restarting the sound if it's already playing. If false, the sound will restart.
 * @param restart If true, forces the sound to restart even if it's already playing.
 * 
 * @note This function uses SDL's Mix_Playing to check if the sound is already playing on the assigned channel.
 *       If the sound is playing, it can either be restarted or stopped and reloaded based on the `restart` and `usesame` parameters.
 *       It registers an effect for the panning (via `PinSound::Pan2ChannelEffect`) and plays the sound.
 */
void PinSound::Play_SNDCFG_SND3D2CH(float nVolume, const float randompitch, const int pitch, 
               const float pan, const float front_rear_fade, const int loopcount, const bool usesame, const bool restart)
{
   if (m_assignedChannel == -1)
      return;

   /* PLOGI << std::fixed << std::setprecision(7) << "Playing Sound: " << m_szName << " SoundOut (0=table, 1=bg): " << 
      (int) m_outputTarget << " nVol: " << nVolume << " pan: " << pan <<
      " Pitch: "<< pitch << " Random pitch: " << randompitch  << " front_rear_fade: " << front_rear_fade <<   " loopcount: " << loopcount << " usesame: " << 
      usesame <<  " Restart? " << restart; */

   if (Mix_Playing(m_assignedChannel)) {
      if (restart || !usesame) { // stop and reload
         Mix_HaltChannel(m_assignedChannel);
         setPitch(pitch, randompitch);
         // register the effects.  must do this each time before PlayChannel and once the sound is done its unregistered automatically
         Mix_RegisterEffect(m_assignedChannel, PinSound::Pan2ChannelEffect, nullptr, &m_mixEffectsData);
         Mix_PlayChannel(m_assignedChannel, m_pMixChunk, loopcount > 0 ? loopcount -1 : loopcount);
      }
   }
   else { // not playing
      setPitch(pitch, randompitch);
      // register the effects.  must do this each time before PlayChannel and once the sound is done its unregistered automatically
      Mix_RegisterEffect(m_assignedChannel, PinSound::Pan2ChannelEffect, nullptr, &m_mixEffectsData);
      Mix_PlayChannel(m_assignedChannel, m_pMixChunk, loopcount > 0 ? loopcount -1 : loopcount);
   }
}

/**
 * @brief Plays a 3D sound effect with various configurable parameters.
 *
 * This function handles playing a sound with specified attributes, such as volume, pitch, pan, 
 * front/rear fade, and loop count. It also supports restarting and modifying the sound playback 
 * based on whether the sound is already playing on the assigned channel.
 *
 * @param nVolume The volume of the sound, where 0.0 is silent and 1.0 is full volume.
 * @param randompitch A random pitch modifier applied to the sound (can be a value between 0.0 and 1.0).
 * @param pitch The base pitch of the sound. Affects the pitch shift applied to the sound playback.
 * @param pan The pan of the sound, controlling whether it’s played more towards the left or right.
 * @param front_rear_fade A fade effect that adjusts the balance between the front and rear sound channels.
 * @param loopcount The number of times the sound should loop. 0 means no looping.
 * @param usesame If true, the function will not restart the sound if it's already playing. 
 *                If false, it will stop and restart the sound playback.
 * @param restart If true, forces the sound to stop and restart even if it’s already playing.
 *
 * This function first checks if the sound is already playing. If it is, and `restart` or `usesame` is false,
 * it stops the current sound and replays it with the new settings. If the sound is not playing, it simply plays the sound.
 *
 * The MixEffects must be registered each time before the sound is played.  The effect is automatically unregistered once the sound finishes playing.
 */
void PinSound::Play_SNDCFG_SND3DSSF(float nVolume, const float randompitch, const int pitch, 
               const float pan, const float front_rear_fade, const int loopcount, const bool usesame, const bool restart)
{
   /* PLOGI << std::fixed << std::setprecision(7) << "SSF Playing Sound: " << m_szName << " SoundOut (0=table, 1=bg): " << 
         (int) m_outputTarget << " nVol: " << nVolume << " pan: " << pan <<
         " Pitch: "<< pitch << " Random pitch: " << randompitch << " front_rear_fade: " << front_rear_fade << " loopcount: " << loopcount << " usesame: " << 
         usesame <<  " Restart? " << restart; */

   if (m_assignedChannel == -1)
      return;

   if (Mix_Playing(m_assignedChannel)) { 
      if (restart || !usesame) { // stop and reload  
         Mix_HaltChannel(m_assignedChannel);
         setPitch(pitch, randompitch);
         // register the effects.  must do this each time before PlayChannel.  When the sound is done playing its automatically unregistered.
         Mix_RegisterEffect(m_assignedChannel, PinSound::SSFEffect, nullptr, &m_mixEffectsData);
         Mix_PlayChannel(m_assignedChannel, m_pMixChunk, loopcount > 0 ? loopcount -1 : loopcount);
      }
   }
   else { // not playing
      setPitch(pitch, randompitch);
      // register the pitch effect.  must do this each time before PlayChannel.  When the sound is done playing its automatically unregistered.
      Mix_RegisterEffect(m_assignedChannel, PinSound::SSFEffect, nullptr, &m_mixEffectsData);
      Mix_PlayChannel(m_assignedChannel, m_pMixChunk, loopcount > 0 ? loopcount -1 : loopcount);
   }
}

/**
 * @brief Called to stop/quickly-fade-out a table sound.
 */
void PinSound::Stop() 
{
   if (m_assignedChannel != -1)
      Mix_FadeOutChannel(m_assignedChannel, 300); // fade out in 300ms.  Also halts channel when done
}

/**
 * @brief Loads a music file into the `m_pMixMusic` object.
 * 
 * This function attempts to load a music file from the provided file path (`szFileName`).
 * If a music file is already loaded, it will be freed before attempting to load the new file.
 * On successful loading, the `m_pMixMusic` pointer will point to the new music object.
 * If loading fails, an error message will be logged and the function will return false.
 * 
 * @param szFileName The file path of the music file to load.
 * 
 * @return `true` if the music file was successfully loaded, `false` otherwise.
 * 
 * @note Used by WMP.
 */
bool PinSound::SetMusicFile(const string& szFileName)
{
   if (m_pMixMusic != nullptr)
      Mix_FreeMusic(m_pMixMusic);

   string path = find_case_insensitive_file_path(szFileName);
   if (path.empty()) {
      PLOGE << "Failed to find music file: " << szFileName;
      return false;
   }

   m_pMixMusic = Mix_LoadMUS(path.c_str());
   if (!m_pMixMusic) {
      PLOGE << "Failed to load sound: " << SDL_GetError();
      return false;
   }

   return true;
}

/**
 * @brief Initializes and plays music from a specified file.
 *
 * This function attempts to load a music file from different potential locations,
 * starting with the file path provided. It uses SDL_mixer's Mix_LoadMUS function
 * to load the music and adjusts the volume based on the given `volume` parameter
 * and the player's current music volume. If successful, it starts the music playback.
 * 
 * @param szFileName The name of the music file to load. The function will search for the file
 * in various directories.
 * @param volume A float representing the desired volume of the music (0.0 to 1.0). The final
 * volume is scaled based on the player's current music volume setting.
 * 
 * @return True if the music was successfully loaded and started, false otherwise.
 * 
 * @note Loads Music file from the table script when it uses 'PlayMusic'. These are typically
 *       in the music folder.
 */
bool PinSound::MusicInit(const string& szFileName, const float volume)
{
   m_outputTarget = SNDOUT_BACKGLASS;

   const string& filename = szFileName;

   if (m_pMixMusic != nullptr)
      Mix_FreeMusic(m_pMixMusic);

   // need to find the path of the music dir. This does hunt to find the file.
   for (int i = 0; i < 5; ++i) {
      string path;
      switch (i)
      {
      case 0: break;
      case 1: path = g_pvp->m_szMyPath + "music" + PATH_SEPARATOR_CHAR; break;
      case 2: path = g_pvp->m_currentTablePath; break;
      case 3: path = g_pvp->m_currentTablePath + "music" + PATH_SEPARATOR_CHAR; break;
      case 4: path = PATH_MUSIC; break;
      }
      path += filename;

      #ifdef __STANDALONE__
      path = find_case_insensitive_file_path(path);
      #endif

      if ((m_pMixMusic = Mix_LoadMUS(path.c_str()))) {
         const float nVolume = volume;
         MusicVolume(nVolume);
         MusicPlay();
         // PLOGI << "Loaded Music File: " << szFileName << " nVolume: " << nVolume << " to OutputTarget(0=table, 1=BG): " << static_cast<int>(m_outputTarget);
         return true;
      }
   }
   PLOGE << "Failed to load sound: " << szFileName << " SDL Error: " << SDL_GetError();
   return false;
}

void PinSound::MusicPlay()
{
   Mix_PlayMusic(m_pMixMusic, 0);
}

void PinSound::MusicPause()
{
   Mix_PauseMusic();
}

void PinSound::MusicUnpause()
{
   Mix_ResumeMusic();
}

void PinSound::MusicClose()
{
   MusicStop(); 
}

bool PinSound::MusicActive()
{
   return Mix_PlayingMusic();
}

void PinSound::MusicStop()
{
   Mix_HaltMusic();
}

double PinSound::GetMusicPosition() const
{
   return Mix_GetMusicPosition(m_pMixMusic);
}

void PinSound::SetMusicPosition(double seconds)
{
   Mix_SetMusicPosition(seconds);
}

// Volume, range in 0-1, without global music volume applied (as the function apply it)
void PinSound::MusicVolume(const float volume)
{
   const int nVolume = static_cast<int>(volume * (g_pplayer ? dequantizeSignedPercent(g_pplayer->m_MusicVolume) : 1.f) * static_cast<float>(MIX_MAX_VOLUME));
   Mix_VolumeMusic(nVolume);
}

/**
 * @brief Initializes an SDL audio stream for background music playback.
 *
 * This function sets up an audio stream with the specified frequency, channel count,
 * and volume, then attempts to open and configure the audio device for playback. 
 * If the stream is successfully initialized, the function resumes audio playback.
 * If initialization fails, an error message is logged.
 *
 * @param frequency The audio sample rate (in Hz) to be used by the audio stream.
 * @param channels The number of audio channels (e.g., 1 for mono, 2 for stereo).
 * @param volume A floating-point value representing the desired volume. It is scaled 
 *        by the current music volume setting of the player.
 * @return bool Returns true if the audio stream was successfully initialized 
 *         and started; false otherwise.
 * 
 * @note Used by VPinMAMEController and PUP. volume range 0-1 from both VPinMAME and PUP
 */
bool PinSound::StreamInit(DWORD frequency, int channels, const float volume) 
{
   SDL_AudioSpec audioSpec;
   audioSpec.freq = frequency;
   audioSpec.format =  SDL_AUDIO_S16LE;
   audioSpec.channels = channels;

   m_outputTarget = SNDOUT_BACKGLASS;
   m_pstream = SDL_OpenAudioDeviceStream(m_sdl_BG_idx, &audioSpec, nullptr, nullptr);

   if (m_pstream) {
      m_streamVolume = volume;
      SDL_SetAudioStreamGain(m_pstream, volume * (g_pplayer ? dequantizeSignedPercent(g_pplayer->m_MusicVolume) : 1.f));
      SDL_ResumeAudioStreamDevice(m_pstream); // it always stops paused
      return true;
   }
   else {
      PLOGE << "Failed to load stream: " << SDL_GetError();
   }
   return false;
}

/**
 * @brief Updates the audio stream with new data to be played.
 * 
 * This function is used to feed audio data into an active audio stream in the 
 * SDL audio system. It passes the provided audio buffer and its length to the
 * SDL audio stream, allowing for continuous playback of audio data.
 *
 * @param buffer A pointer to the audio data buffer, typically containing PCM audio samples.
 * @param length The length of the buffer in bytes, indicating how much audio data is available to process.
 * 
 * @note The function assumes that the stream (m_pstream) has been properly initialized
 * and that the audio data format is compatible with the stream's configuration.
 * 
 * @note called by VPinMAMEController and PUP
 */
void PinSound::StreamUpdate(void* buffer, DWORD length)
{
   SDL_PutAudioStreamData(m_pstream, buffer, length);
}

/**
 * @brief Adjusts the volume of the audio stream.
 *
 * This function sets the gain of the audio stream by adjusting it based on the
 * provided volume and the current music volume from the global player settings.
 * It ensures the stream's volume is updated only when the volume has changed.
 *
 * @param volume The desired volume level (range is expected to be from 0.0 to 1.0).
 *               This value is multiplied by the global music volume setting (in percentage)
 *               before being applied to the audio stream.
 *
 * @note The function only updates the stream's volume if the provided volume differs
 *       from the current stream volume, avoiding unnecessary updates.
 * 
 * @note Called by PUP. up sends a value between 0 and 1.. matches sdl stream volume scale
 */
void PinSound::StreamVolume(const float volume)
{
   //PLOGI << "STREAM VOL";
   if (m_streamVolume != volume) {
      SDL_SetAudioStreamGain(m_pstream, volume * (g_pplayer ? dequantizeSignedPercent(g_pplayer->m_MusicVolume) : 1.f));
      m_streamVolume = volume;
   }
}

/**
 * @brief Loads a sound file and initializes a PinSound object with its data.
 * 
 * This function attempts to load a sound file specified by the given file path (`strFileName`), 
 * reads its content into memory, and creates a `PinSound` object with the loaded data. The sound 
 * file's name is extracted from the file path and stored, and the file's data is read into dynamically 
 * allocated memory. The function also reinitializes the sound object, and if the initialization succeeds, 
 * it returns a pointer to the new `PinSound` object. If any part of the loading or initialization process fails, 
 * it returns `nullptr`.
 * 
 * @param strFileName The file path to the sound file to load.
 * 
 * @return PinSound* A pointer to the newly loaded and initialized `PinSound` object, or `nullptr` if 
 *         loading or initialization fails.
 * 
 * @note Windows UI?  Load sound into Sound Resource Manager?
 * 
 * @note The function uses raw file handling (`fopen_s`, `fseek`, `fread_s`, and `fclose`), and expects
 *       the sound file to be in a valid format that can be processed by `ReInitialize()`.
 */
PinSound *PinSound::LoadFile(const string& strFileName)
{
   PinSound * const pps = new PinSound();

   pps->m_szPath = strFileName;
   pps->m_szName = TitleFromFilename(strFileName);

   FILE *f;
   if (fopen_s(&f, strFileName.c_str(), "rb") != 0 || !f) {
      ShowError("Could not open sound file.");
      return nullptr;
   }
   fseek(f, 0, SEEK_END);
   pps->m_cdata = (int)ftell(f);
   fseek(f, 0, SEEK_SET);
   pps->m_pdata = new char[pps->m_cdata];
   if (fread_s(pps->m_pdata, pps->m_cdata, 1, pps->m_cdata, f) < 1) {
      fclose(f);
      ShowError("Could not read from sound file.");
      return nullptr;
   }
   fclose(f);

   HRESULT res = pps->ReInitialize();

   if (res == S_OK)
      return pps;
   else
      return nullptr;
}

Mix_Chunk* PinSound::copyMixChunk(const Mix_Chunk* const original)
{
   if (!original || original->abuf == nullptr)
      return nullptr;

   // Allocate a new Mix_Chunk
   Mix_Chunk* const copy = (Mix_Chunk*)SDL_malloc(sizeof(Mix_Chunk)); // need to use SDL_malloc as SDL_free will be used when freeing the chunk
   copy->alen = original->alen;
   copy->volume = original->volume;

   // Allocate memory for audio buffer
   copy->allocated = 1;
   copy->abuf = (Uint8*)SDL_malloc(original->alen*sizeof(Uint8)); // need to use SDL_malloc as SDL_free will be used when freeing the chunk
   std::memcpy(copy->abuf, original->abuf, original->alen);
   return copy;
}

/**
 * (Static)
 * @brief Calculates the left and right channel volume ratios based on the pan value.
 * 
 * This function takes an input pan value (ranging from -3 to 3) and normalizes it to a 
 * standard stereo pan range of [-1, 1]. It then computes the left and right channel 
 * volume ratios using a linear panning formula.
 * 
 * @param[out] leftPanRatio  Reference to a float where the computed left channel ratio will be stored.
 * @param[out] rightPanRatio Reference to a float where the computed right channel ratio will be stored.
 * @param[in] adjustedVolRatio The overall volume adjustment ratio (typically in range [0, 1]).
 * @param[in] pan The panning position, where -3 is full left, 0 is center, and 3 is full right.
 * 
 * @note The function ensures that the output volume ratios are clamped between 0 and 1 to prevent overflow.
 */
void PinSound::calcPan(float& leftPanRatio, float& rightPanRatio, float adjustedVolRatio, float pan)
{
    // Normalize pan from range [-3, 3] to [-1, 1]
    pan = pan / 3.0f;

    // Ensure pan is within -1 to 1 (in case of floating-point errors)
    pan = clamp(pan, -1.0f, 1.0f);

    // Use a more standard panning formula that keeps values within range
    float leftFactor  = 0.5f * (1.0f - pan); // Left decreases as pan increases
    float rightFactor = 0.5f * (1.0f + pan); // Right increases as pan increases

    leftPanRatio = adjustedVolRatio * leftFactor;
    rightPanRatio = adjustedVolRatio * rightFactor;

    // Ensure the values are properly clamped
    leftPanRatio  = clamp(leftPanRatio,  0.0f, 1.0f);
    rightPanRatio = clamp(rightPanRatio, 0.0f, 1.0f);

    //PLOGI << "Pan: " << pan << " AdjustedVolRatio: " << adjustedVolRatio << " Left: " << leftPanRatio << " Right: " << rightPanRatio;
}

/**
 * (Static)
 * @brief Calculates the fade distribution between front and rear channels based on the given fade ratio and panning ratios.
 *
 * This function computes the gain values for front-left, front-right, rear-left, and rear-right channels
 * based on the provided left/right panning ratios and a fade ratio. The fade ratio determines the balance
 * between front and rear channels, where:
 *  - A fadeRatio of 0.0 fully favors the rear speakers.
 *  - A fadeRatio of 2.5 fully favors the front speakers.
 *  - A fadeRatio between 0.0 and 2.5 interpolates linearly between rear and front.
 *
 * @param leftPanRatio  The left channel's panning ratio (typically in the range [0.0, 1.0]).
 * @param rightPanRatio The right channel's panning ratio (typically in the range [0.0, 1.0]).
 * @param fadeRatio     The fade control value determining the balance between front and rear channels.
 *                      It is clamped to the range [0.0, 2.5].
 * @param frontLeft     Output parameter for the front-left channel gain.
 * @param frontRight    Output parameter for the front-right channel gain.
 * @param rearLeft      Output parameter for the rear-left channel gain.
 * @param rearRight     Output parameter for the rear-right channel gain.
 */
void PinSound::calcFade(float leftPanRatio, float rightPanRatio, float fadeRatio, float& frontLeft, float& frontRight, float& rearLeft, float& rearRight)
{
   // Clamp fadeRatio to the range [0.0, 2.5]
   fadeRatio = clamp(fadeRatio, 0.0f, 2.5f);

   // Calculate front and rear weights (linear fade)
   //float rearWeight = max(0.0f, 2.5f - fadeRatio) / 2.5f; // 1 at fadeRatio=0, 0 at fadeRatio=2.5
   //float frontWeight = min(2.5f, fadeRatio) / 2.5f;       // 0 at fadeRatio=0, 1 at fadeRatio=2.5
   float rearWeight = (2.5f - fadeRatio) / 2.5f;
   float frontWeight = fadeRatio / 2.5f;

   // Apply panning ratios
   frontLeft  = frontWeight * leftPanRatio;
   frontRight = frontWeight * rightPanRatio;
   rearLeft   = rearWeight * leftPanRatio;
   rearRight  = rearWeight * rightPanRatio;

   //PLOGI << "FadeRatio: " << fadeRatio << " FrontLeft: " << frontLeft << " FrontRight: " << frontRight << " RearLeft: " << rearLeft << " RearRight: " << rearRight;
}

// The existing pan value in PlaySound function takes a -1 to 1 value, however it's extremely non-linear.
// -0.1 is very obviously to the left.  Table scripts like the ball rolling script seem to use x^10 to map
// linear positions, so we'll use that and reverse it.   Also multiplying by 3 since that seems to be the
// total distance necessary to fully pan away from one side at the center of the room.
// static
float PinSound::PanTo3D(float input)
{
   // DirectSound's position command does weird things at exactly 0. 
   if (fabsf(input) < 0.0001f)
      input = (input < 0.0f) ? -0.0001f : 0.0001f;
   if (input < 0.0f)
      return -powf(-max(input, -1.0f), (float)(1.0 / 10.0)) * 3.0f;
   else
      return powf(min(input, 1.0f), (float)(1.0 / 10.0)) * 3.0f;
}

// This is a replacement function for PanTo3D() for sound effect panning (audio x-axis).
// It performs the same calculations but maps the resulting values to an area of the 3D 
// sound stage that has the expected panning effect for this application. It is written 
// in a long form to facilitate tweaking the formulas.  *njk*
// static
float PinSound::PanSSF(float pan)
{
   // This math could probably be simplified but it is kept in long form
   // to aid in fine tuning and clarity of function.

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
// static
float PinSound::FadeSSF(float front_rear_fade)
{
   float z;

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

   return fabsf(z); // I changed this to get a positive range from 0 to 2.5. not sure why before they returned negative
}

/**
 * (Static, MixEffect Callback)
 * @brief Applies panning effects to the front left (FL) and front right (FR) channels of an audio stream.
 * 
 * This function modifies an audio stream by adjusting the volume of FL and FR channels based on pan settings.
 * 
 * @param chan Audio channel index (unused in this function).
 * @param stream Pointer to the audio data buffer.
 * @param len Length of the audio buffer in bytes.
 * @param udata User data containing the MixEffectsData structure.
 * @note The built in Mix_SetPanning does not work on 2+ channels: https://github.com/libsdl-org/SDL_mixer/issues/665 
 */
void PinSound::Pan2ChannelEffect(int chan, void *stream, int len, void *udata)
{
   MixEffectsData * const med = static_cast<MixEffectsData *> (udata);
   // pan vols ratios for left and right
   float leftPanRatio;
   float rightPanRatio;
   //calcPan(leftPanRatio, rightPanRatio, med->nVolume, PinSound::PanTo3D(med->pan));
   calcPan(leftPanRatio, rightPanRatio, med->nVolume, med->pan*3.0f);
   const int channels = med->outputChannels;

   switch (med->outputFormat)
   {
      case SDL_AUDIO_S16LE:
      {
         int16_t* const samples = static_cast<int16_t*>(stream);
         const int total_samples = len / (int)sizeof(int16_t);

         // 8 channels (7.1): FL, FR, FC, LFE, BL, BR, SL, SR
         for (int index = 0; index < total_samples; index += channels) {
            // Apply volume gains to Front Left and Right channels
            samples[index]   = (int16_t)((float)samples[index  ] * leftPanRatio);  // FL
            samples[index+1] = (int16_t)((float)samples[index+1] * rightPanRatio); // FR
         }
         break;
      }
      case SDL_AUDIO_F32LE:
      {
         float* const samples = static_cast<float*>(stream);
         const int total_samples = len / (int)sizeof(float);

         // 8 channels (7.1): FL, FR, FC, LFE, BL, BR, SL, SR
         for (int index = 0; index < total_samples; index += channels) {
            // Apply volume gains to Front Left and Right channels
            samples[index] *= leftPanRatio;  // FL
            samples[index+1] *= rightPanRatio; // FR
         }
         break;
      }
      case SDL_AUDIO_S32LE:
      {
         int32_t* const samples = static_cast<int32_t*>(stream);
         const int total_samples = len / (int)sizeof(int32_t);

         // 8 channels (7.1): FL, FR, FC, LFE, BL, BR, SL, SR
         for (int index = 0; index < total_samples; index += channels) {
            // Apply volume gains to Front Left and Right channels
            samples[index]   = (int32_t)((float)samples[index  ] * leftPanRatio);  // FL
            samples[index+1] = (int32_t)((float)samples[index+1] * rightPanRatio); // FR
         }
         break;
      }
      default:
      {
         PLOGE << "unknown audio format..";
         return;
      }
   }
}

/**
 * (Static, MixEffect Callback)
 * @brief Moves front left (FL) and front right (FR) audio channels to rear channels in multi-channel audio.  
 *        This is used for SNDCFG_SND3DALLREAR mode. 
 * 
 * This function redistributes the FL and FR channels to corresponding rear channels (BL, BR) and zeroes out the original front channels.
 * 
 * @param chan Audio channel index (unused in this function).
 * @param stream Pointer to the audio data buffer.
 * @param len Length of the audio buffer in bytes.
 * @param udata User data containing the MixEffectsData structure.
 */
void PinSound::MoveFrontToRearEffect(int chan, void *stream, int len, void *udata)
{
   MixEffectsData * const med = static_cast<MixEffectsData *> (udata);
   // pan vols ratios for left and right
   float leftPanRatio;
   float rightPanRatio;
   const int channels = med->outputChannels;
   const int offs = channels-2;

   // 4 channels (quad) layout: FL, FR, BL, BR
   // 5 channels (4.1)  layout: FL, FR, LFE, BL, BR
   // 6 channels (5.1)  layout: FL, FR, FC, LFE, BL, BR (last two can also be SL, SR)
   // 7 channels (6.1)  layout: FL, FR, FC, LFE, BC, SL, SR
   // 8 channels (7.1)  layout: FL, FR, FC, LFE, BL, BR, SL, SR
   switch (med->outputFormat)
   {
      case SDL_AUDIO_S16LE:
      {
         int16_t* const samples = static_cast<int16_t*>(stream);
         const int total_samples = len / (int)sizeof(int16_t);

         calcPan(leftPanRatio, rightPanRatio, med->nVolume, PinSound::PanTo3D(med->pan));

         // 8 channels (7.1): FL, FR, FC, LFE, BL, BR, SL, SR
         for (int index = 0; index < total_samples; index += channels) {
            if (channels >= 4 && channels <= 8) {
               // Apply volume gains and copy them to rear
               samples[index+offs] = (int16_t)((float)samples[index]   * leftPanRatio);  // copy FL to BL
               samples[index+1+offs] = (int16_t)((float)samples[index+1] * rightPanRatio); // copy FR to BR
            }

            // wipe front channels
            samples[index]   = 0;
            samples[index+1] = 0;
         }
         break;
      }
      case SDL_AUDIO_F32LE:
      {
         float* const samples = static_cast<float*>(stream);
         const int total_samples = len / (int)sizeof(float);

         calcPan(leftPanRatio, rightPanRatio, med->nVolume, PinSound::PanTo3D(med->pan));

         for (int index = 0; index < total_samples; index += channels) {
            if (channels >= 4 && channels <= 8) {
               // Apply volume gains and copy them to rear
               samples[index+offs] = samples[index]   * leftPanRatio;  // copy FL to BL
               samples[index+1+offs] = samples[index+1] * rightPanRatio; // copy FR to BR
            }

            // wipe front channels
            samples[index]   = 0.f;
            samples[index+1] = 0.f;
         }
         break;
      }
      case SDL_AUDIO_S32LE:
      {
         int32_t* const samples = static_cast<int32_t*>(stream);
         const int total_samples = len / (int)sizeof(int32_t);

         calcPan(leftPanRatio, rightPanRatio, med->nVolume, PinSound::PanTo3D(med->pan));

         // 8 channels (7.1): FL, FR, FC, LFE, BL, BR, SL, SR
         for (int index = 0; index < total_samples; index += channels) {
            if (channels >= 4 && channels <= 8) {
               // Apply volume gains and copy them to rear
               samples[index+offs] = (int32_t)((float)samples[index]   * leftPanRatio);  // copy FL to BL
               samples[index+1+offs] = (int32_t)((float)samples[index+1] * rightPanRatio); // copy FR to BR
            }

            // wipe front channels
            samples[index]   = 0;
            samples[index+1] = 0;
         }
         break;
      }
      default:
      {
         PLOGE << "unknown audio format.. Channel: " << chan;
         return;
      }
   }
}

/**
 * (Static, MixEffect Callback)
 * Applies spatial sound effects to table sound for a 7.1 surround sound system.
 *
 * This function processes an audio stream by applying panning and fading effects,
 * moving the front left (FL) and front right (FR) channels to the back (BL, BR) 
 * and side (SL, SR) channels while applying calculated volume adjustments. All audio
 * comes in on the FL and FR channels, but those channels are reserved for the backglass
 * sounds. Table sounds only get played on the back (BL, BR) and side (SL, SR) channels
 * in this mode.
 *
 * @param chan   The channel index being processed (provided by SDL_mixer).
 * @param stream Pointer to the audio stream buffer, containing raw PCM data.
 * @param len    Length of the audio buffer in bytes.
 * @param udata  Pointer to user-defined data, expected to be a MixEffectsData structure.
 *
 * The function supports the following audio formats:
 * - SDL_AUDIO_S16LE (16-bit signed PCM)
 * - SDL_AUDIO_F32LE (32-bit float PCM)
 *
 * The function performs the following operations:
 * 1. Retrieves the audio format and channel information from the provided MixEffectsData structure.
 * 2. Computes panning and fading values based on the user-defined parameters.
 * 3. Copies FL/FR samples to BL/BR and SL/SR channels.
 * 4. Applies volume scaling to back and side channels.
 * 5. Clears the original front channel samples because table sounds on go on side and back.
 *
 * If an unsupported audio format is encountered, an error is logged.
 */
void PinSound::SSFEffect(int chan, void *stream, int len, void *udata)
{
   // 8 channels (7.1): FL, FR, FC, LFE, BL, BR, SL, SR
   MixEffectsData * const med = static_cast<MixEffectsData *> (udata);

   // pan vols ratios for left and right
   float leftPanRatio;
   float rightPanRatio;

   // calc the fade
   float sideLeft;   // rear of table -1
   float sideRight;
   float rearLeft;   // front of table + 1
   float rearRight;
   calcPan(leftPanRatio, rightPanRatio, med->nVolume, PinSound::PanSSF(med->pan));
   calcFade(leftPanRatio, rightPanRatio, PinSound::FadeSSF(med->front_rear_fade), rearLeft, rearRight, sideLeft, sideRight);
   const int channels = med->outputChannels;

   switch (med->outputFormat)
   {
      case SDL_AUDIO_S16LE:
         {
            int16_t* const samples = static_cast<int16_t*>(stream);
            const int total_samples = len / (int)sizeof(int16_t);

            // cap all vol not to be over 1.  Over 1, and you get distortion.
            sideLeft = clamp(sideLeft, 0.f, 1.f);
            sideRight = clamp(sideRight, 0.f, 1.f);
            rearLeft = clamp(rearLeft, 0.f, 1.f);
            rearRight = clamp(rearRight, 0.f, 1.f);

            // 8 channels (7.1): FL, FR, FC, LFE, BL, BR, SL, SR
            for (int index = 0; index < total_samples; index += channels) {
               // copy the sound sample from Front to Back and Side channels.
               samples[index + 4] = (samples[index]);   // COPY FL to BL
               samples[index + 5] = (samples[index+1]); // Copy FR to BR
               samples[index + 6] = (samples[index]);   // Copy FL to SL
               samples[index + 7] = (samples[index+1]); // Copy FR to SR

               // Apply volume gains to back and side channels
               samples[index + 4] = (int16_t)((float)samples[index+4] * rearLeft);  // BL
               samples[index + 5] = (int16_t)((float)samples[index+5] * rearRight); // BR
               samples[index + 6] = (int16_t)((float)samples[index+6] * sideLeft);  // SL
               samples[index + 7] = (int16_t)((float)samples[index+7] * sideRight); // SR

               // wipe front channels
               samples[index]   = 0;
               samples[index+1] = 0;
            }
            break;
         }
      case SDL_AUDIO_F32LE:
         {
            float* const samples = static_cast<float*>(stream);
            const int total_samples = len / (int)sizeof(float);

            // 8 channels (7.1): FL, FR, FC, LFE, BL, BR, SL, SR
            for (int index = 0; index < total_samples; index += channels) {
               // copy the sound sample from Front to Back and Side channels.
               samples[index + 4] = (samples[index]);   // COPY FL to BL
               samples[index + 5] = (samples[index+1]); // Copy FR to BR
               samples[index + 6] = (samples[index]);   // Copy FL to SL
               samples[index + 7] = (samples[index+1]); // Copy FR to SR

               // Apply volume gains to back and side channels
               samples[index + 4] = (samples[index+4] * rearLeft);  // BL
               samples[index + 5] = (samples[index+5] * rearRight); // BR
               samples[index + 6] = (samples[index+6] * sideLeft);  // SL
               samples[index + 7] = (samples[index+7] * sideRight); // SR

               // wipe front channels
               samples[index]   = 0;
               samples[index+1] = 0;
            }
            break;
         }
         case SDL_AUDIO_S32LE:
         {
            int32_t* const samples = static_cast<int32_t*>(stream);
            const int total_samples = len / (int)sizeof(int32_t);

            // cap all vol not to be over 1.  Over 1, and you get distortion.
            sideLeft = clamp(sideLeft, 0.f, 1.f);
            sideRight = clamp(sideRight, 0.f, 1.f);
            rearLeft = clamp(rearLeft, 0.f, 1.f);
            rearRight = clamp(rearRight, 0.f, 1.f);

            // 8 channels (7.1): FL, FR, FC, LFE, BL, BR, SL, SR
            for (int index = 0; index < total_samples; index += channels) {
               // copy the sound sample from Front to Back and Side channels.
               samples[index + 4] = (samples[index]);   // COPY FL to BL
               samples[index + 5] = (samples[index+1]); // Copy FR to BR
               samples[index + 6] = (samples[index]);   // Copy FL to SL
               samples[index + 7] = (samples[index+1]); // Copy FR to SR

               // Apply volume gains to back and side channels
               samples[index + 4] = (int32_t)((float)samples[index+4] * rearLeft);  // BL
               samples[index + 5] = (int32_t)((float)samples[index+5] * rearRight); // BR
               samples[index + 6] = (int32_t)((float)samples[index+6] * sideLeft);  // SL
               samples[index + 7] = (int32_t)((float)samples[index+7] * sideRight); // SR

               // wipe front channels
               samples[index]   = 0;
               samples[index+1] = 0;
            }
            break;
         }
      default:
         {
            PLOGE << "unknown audio format..";
            return;
         }
   }

   //PLOGI << " rearLeft: " << rearLeft << " rearRight: " << rearRight << " sideLeft: " << sideLeft << " sideRight: " << sideRight;
}

/**
 * @brief Retrieves the number of audio channels from a WAV file.
 *
 * This function reads the WAV file header and extracts the number of audio channels
 * present in the file. The function assumes that `m_pdata` points to valid WAV file 
 * data in memory and that `m_cdata` specifies the size of this data.
 *
 * @return uint16_t The number of audio channels in the WAV file.
 *
 * @throws std::runtime_error If the data size is too small to contain a valid WAV header.
 */
uint16_t PinSound::getChannelCountWav() const
{
   struct WavHeader {
    char riff[4];              // "RIFF"
    uint32_t fileSize;         // File size - 8 bytes
    char wave[4];              // "WAVE"
    char fmtChunkMarker[4];    // "fmt "
    uint32_t fmtChunkSize;     // Size of fmt chunk
    uint16_t audioFormat;      // Audio format (1 = PCM)
    uint16_t numChannels;      // Number of channels
    uint32_t sampleRate;       // Sample rate
    uint32_t byteRate;         // Byte rate
    uint16_t blockAlign;       // Block align
    uint16_t bitsPerSample;    // Bits per sample
   };
   // Check that the data is at least the size of the WavHeader
   if (m_cdata < (int)sizeof(WavHeader))
      throw std::runtime_error("Invalid WAV data: too small to contain a valid header.");

   const WavHeader* header = reinterpret_cast<const WavHeader*>(m_pdata);

   // Return the number of channels
   return header->numChannels;
}

/**
 * (Static)
 * @brief Retrieves the next available audio mixer channel.
 *
 * This function returns the next available SDL mixer channel for audio playback.
 * If all channels are in use, 
 * it dynamically increases the number of allocated mixer channels by 100.
 *
 * @return The next available channel index.
 */
int PinSound::getChannel()
{
   const std::lock_guard<std::mutex> lg(m_channelUpdateMutex);

   for (size_t i = 0; i < m_channelInUse.size(); ++i) {
      if (!m_channelInUse[i]) {
         m_channelInUse[i] = true;
         return (int)i;
      }
   }

   // we're out of channels. increase by 100
   const int oldMaxSDLMixerChannels = (int)m_channelInUse.size();
   const int maxSDLMixerChannels = Mix_AllocateChannels(oldMaxSDLMixerChannels + 100);
   if (maxSDLMixerChannels <= oldMaxSDLMixerChannels)
      return -1;
   m_channelInUse.resize(maxSDLMixerChannels, false);
   PLOGI << "Allocated another 100 mixer channels.  Total Avail: " << maxSDLMixerChannels;

   return oldMaxSDLMixerChannels;
}

/**
 * (Static)
 * @brief Enumerates available audio playback devices and stores their details.
 *
 * This function initializes the SDL audio subsystem and retrieves a list of available
 * audio playback devices. It populates the provided vector with information about each device,
 * including its ID, name, and channel count.
 *
 * @param[out] audioDevices A vector to store the discovered audio playback devices.
 *
 * @note This function clears the provided vector before populating it with device information.
 * @note SDL must be properly initialized before calling this function.
 */ 
void PinSound::EnumerateAudioDevices(vector<AudioDevice>& audioDevices)
{
   if (!SDL_WasInit(SDL_INIT_AUDIO))
      if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
         PLOGE << "SDL Init Audio failed: " << SDL_GetError();
         return;
      }

   //output name of audio driver
   const char *pdriverName;
   if ((pdriverName = SDL_GetCurrentAudioDriver()) != nullptr) {
      PLOGI << "Current Audio Driver: " << pdriverName;
   }
   else {
      PLOGE << "SDL Get Audio Driver failed: " << SDL_GetError();
   }

   // log default audio device
   /* FIXME waiting for a fix of SDL 3.2.12
   SDL_AudioDeviceID devid = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
   const char *pDefaultDeviceName = SDL_GetAudioDeviceName(devid);
   if (pDefaultDeviceName) {
      PLOGI << "Default Audio Device: " << pDefaultDeviceName;
   }
   else {
      const char* pError = SDL_GetError();
      // workaround for https://github.com/libsdl-org/SDL/issues/12977
      if (pError && pError[0] != '\0') {
         PLOGE << "Failed to get name for Default Audio Device: " << pError;
      }
   }
   SDL_CloseAudioDevice(devid);*/

   audioDevices.clear();
   int count;
   SDL_AudioDeviceID* pAudioList = SDL_GetAudioPlaybackDevices(&count);
   for (int i = 0; i < count; ++i) {
      AudioDevice audioDevice = {};
      audioDevice.id = pAudioList[i];
      #ifdef __STANDALONE__
      strcpy(audioDevice.name, SDL_GetAudioDeviceName(pAudioList[i]));
      #else
      strcpy_s(audioDevice.name, SDL_GetAudioDeviceName(pAudioList[i]));
      #endif
      SDL_AudioSpec spec;
      SDL_GetAudioDeviceFormat(pAudioList[i], &spec, nullptr);
      audioDevice.channels = spec.channels;
      SDL_CloseAudioDevice(pAudioList[i]);
      audioDevices.push_back(audioDevice);
   }
}
