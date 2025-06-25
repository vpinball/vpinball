// license:GPLv3+

#pragma once

namespace VPX
{

enum SoundOutTypes : char
{
   // Playfield sounds, rendered according to the sound config type
   SNDOUT_TABLE = 0,

   // Backglass audio, rendered to the front channels of the backglass audio device
   SNDOUT_BACKGLASS = 1
};

// Playfield sound playback mode
// =============================
// This option select how the playfield table sounds are rendered. While default backglass audio and streamed audio (VPinMAME, 
// AltSound, PuP, FlexDMD, ...) always plays from the front channels of the selected audio device (which may be a
// different device or not)
enum SoundConfigTypes : int
{
   // 2CH: Render playfield sounds as 2 channels to the front channel of the selected audio device (which should be a different 
   // one than the audio device playing backglass audio)
   SNDCFG_SND3D2CH = 0,

   // ALLREAR: Render playfield sounds as 2 channels to the rear channel of the audio card
   // This can replace the need to use two sound cards to move table audio inside the cab.
   SNDCFG_SND3DALLREAR = 1,

   // FRONTISREAR: Render playfield sounds as up to 6 channels to the front/side/rear channels of the audio card (depending 
   // on the audio card type), with the rear channels near the front of the cab (where the player stands).
   // Table effects are mapped such that the front of the cab is the rear surround channels. If you were to play
   // VPX in a home theater system with the TV in front of you, this would produce an appropriate result with the ball coming
   // from the rear channels as it gets closer to you.
   SNDCFG_SND3DFRONTISREAR = 2,

   // FRONTISFRONT: Render playfield sounds as up to 6 channels to the front/side/rear channels of the audio card (depending 
   // on the audio card type), with the front channels near the front of the cab (where the player stands).
   // Recommended mapping for a dedicated sound card attached to the playfield. Front channel maps to the front
   // of the cab. We "flip" the rear to the standard 2 channels, so older versions of VP still play sounds on the front most
   // channels of the cab. This mapping could also be used to place 6 channels on the playfield.
   SNDCFG_SND3DFRONTISFRONT = 3,

   // 6CH: Render playfield sounds to 4 channels: side (back of the cab) & rear (front of the cab) channels of the audio 
   // card, leaving the front channels for backglass audio (similar to SNDCFG_SND3DFRONTISFRONT but leaving front channels empty)
   // Rear of playfield shifted to the sides, and front of playfield shifted to the far rear. Leaves front channels open
   // for default backglass and VPinMAME.
   SNDCFG_SND3D6CH = 4,

   // SSF: Same as 6CH but with a different sound horizontal panning and vertical fading are enhanced for a more realistic experience.
   SNDCFG_SND3DSSF = 5
};

class Sound final
{
public:
   ~Sound();

   static Sound* CreateFromFile(const string& filename);
   static Sound* CreateFromStream(IStream* pstm, const int LoadFileVersion);

   size_t GetFileSize() const { return m_cdata; }
   const uint8_t* GetFileRaw() const { return m_pdata; }
   void SetFromFileData(const string& filename, uint8_t* data, size_t size);

   bool SaveToFile(const string& filename) const;
   void SaveToStream(IStream* pstm) const;

   // What type of sound? table or backglass?  Used to route sound to the right device or channel. set by pintable
   SoundOutTypes GetOutputTarget() const { return m_outputTarget; }
   void SetOutputTarget(SoundOutTypes target)
   {
      assert(SoundOutTypes::SNDOUT_TABLE <= target && target <= SoundOutTypes::SNDOUT_BACKGLASS);
      m_outputTarget = target;
   }

   // -100..100
   int GetVolume() const { return m_volume; }
   void SetVolume(const int volume) { m_volume = volume; }

   // -100..100
   int GetPan() const { return m_pan; }
   void SetPan(const int pan) { m_pan = pan; }

   // -100..100
   int GetFrontRearFade() const { return m_frontRearFade; }
   void SetFrontRearFade(const int front_rear_fade) { m_frontRearFade = front_rear_fade; }

   // Sound filename and path
   string m_name; // only filename, no ext
   string m_path; // full filename, incl. path

private:
   Sound() { }

   // Undecoded data from original file
   uint8_t* m_pdata = nullptr;
   size_t m_cdata = 0;

   // at the moment, can only be set in the Sound Resource Manager, stored with the table
   int m_volume = 0;
   int m_pan = 0;
   int m_frontRearFade = 0;

   SoundOutTypes m_outputTarget = SoundOutTypes::SNDOUT_BACKGLASS; //Is it table sound device or BG sound device.

   static bool isWav(const string& path) { return path_has_extension(path, "wav"s); }
};

}
