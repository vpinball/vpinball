// license:GPLv3+

#pragma once

namespace VPX
{

enum SoundOutTypes : char
{
   SNDOUT_TABLE = 0,
   SNDOUT_BACKGLASS = 1
};

// Surround modes
// ==============
//
// 2CH:  Standard stereo output
//
// ALLREAR: All table effects shifted to rear channels. This can replace the need to use two sound cards to move table audio
// inside the cab.  Default backglass audio and VPinMAME audio plays from front speakers.
//
// FRONTISFRONT: Recommended mapping for a dedicated sound card attached to the playfield. Front channel maps to the front
// of the cab.   We "flip" the rear to the standard 2 channels, so older versions of VP still play sounds on the front most
// channels of the cab. This mapping could also be used to place 6 channels on the playfield.
//
// FRONTISREAR: Table effects are mapped such that the front of the cab is the rear surround channels. If you were to play
// VPX in a home theater system with the TV in front of you, this would produce an appropriate result with the ball coming
// from the rear channels as it gets closer to you.
//
// 6CH: Rear of playfield shifted to the sides, and front of playfield shifted to the far rear. Leaves front channels open
// for default backglass and VPinMAME.
//
// SSF: 6CH still doesn't map sounds for SSF as distinctly as it could.. In this mode horizontal panning and vertical fading
// are enhanced for a more realistic experience.
enum SoundConfigTypes : int
{
   SNDCFG_SND3D2CH = 0,
   SNDCFG_SND3DALLREAR = 1,
   SNDCFG_SND3DFRONTISREAR = 2,
   SNDCFG_SND3DFRONTISFRONT = 3,
   SNDCFG_SND3D6CH = 4,
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

   // This is because when VP imports WAVs into the Windows versions it stores them in WAVEFORMATEX
   // format.  We need WAV.  So this keeps the original format for exporting/import, etc for windows.
   // old wav code only, but also used to convert raw wavs for SDL
   WAVEFORMATEX m_wfx { 0 };
   size_t m_cdata_org = 0;
   uint8_t* m_pdata_org = nullptr; // for saving WAVs in the original raw format, points into m_pdata, so do not delete it!

   // at the moment, can only be set in the Sound Resource Manager, stored with the table
   int m_volume = 0;
   int m_pan = 0;
   int m_frontRearFade = 0;

   SoundOutTypes m_outputTarget = SoundOutTypes::SNDOUT_BACKGLASS; //Is it table sound device or BG sound device.

   static bool isWav(const string& path) { return path_has_extension(path, "wav"s); }
};

}
