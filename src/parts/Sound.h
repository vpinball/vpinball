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

class Sound final
{
public:
   static Sound* CreateFromFile(const string& filename);
   static Sound* CreateFromStream(IStream* pstm, const int LoadFileVersion);

   Sound(string name, string path, vector<uint8_t> data)
      : m_name(std::move(name))
      , m_path(std::move(path))
      , m_data(std::move(data))
   {
   }

   const string& GetName() const { return m_name; }
   void SetName(const string& name) { m_name = name; }
   const string& GetImportPath() const { return m_path; }
   size_t GetFileSize() const { return m_data.size(); }
   const uint8_t* GetFileRaw() const { return m_data.data(); }
   void SetFromFileData(string filename, vector<uint8_t> filedata);

   bool SaveToFile(const string& filename) const;
   void SaveToStream(IStream* pstm) const;

   // What type of sound? table or backglass?  Used to route sound to the right device or channel. set by pintable
   SoundOutTypes GetOutputTarget() const { return m_outputTarget; }
   void SetOutputTarget(SoundOutTypes target)
   {
      assert(target == SoundOutTypes::SNDOUT_TABLE || target == SoundOutTypes::SNDOUT_BACKGLASS);
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

private:
   static bool isWav(const string& path) { return path_has_extension(path, "wav"s); }

   string m_name;
   string m_path; // Original file import path
   vector<uint8_t> m_data; // Undecoded data from original file
   int m_volume = 0;
   int m_pan = 0;
   int m_frontRearFade = 0;
   SoundOutTypes m_outputTarget = SoundOutTypes::SNDOUT_BACKGLASS; //Is it table sound device or BG sound device.
};

}
