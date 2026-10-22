// license:GPLv3+

#pragma once

#include "common.h"

#include <unordered_dense.h>

namespace PUP {

class PUPManager;

// PinUP sound bank: every .wav directly in <pack>\PUPSOUNDS, played through the controller
// audio stream API. Positions only pan (PinUP's 3D rolloff is negligible). Files named
// ballroll0.wav .. ballroll9.wav fill ten looping ball-roll slots driven by the 'sp' speed.
class PUPSoundBank final
{
public:
   explicit PUPSoundBank(PUPManager* manager);
   ~PUPSoundBank();

   void Load(const std::filesystem::path& packPath);
   void Unload();
   void Play(const string& name, int x, int y, int z, int vol, const string& special);

private:
   struct Sample
   {
      vector<float> samples; // interleaved stereo float
      int sampleRate = 0;
      int frames = 0;
   };

   struct BallRoll
   {
      std::shared_ptr<Sample> sample;
      uint32_t streamId = 0;
      bool playing = false;
      int volume = 100;
      int x = 0;
      uint64_t generation = 0;
   };

   bool Queue(uint32_t streamId, const Sample& sample, int volume, int x);
   void LoopBallRoll(int slot, uint64_t generation);

   PUPManager* const m_pManager;
   ankerl::unordered_dense::map<string, std::shared_ptr<Sample>> m_samples;
   BallRoll m_ballRoll[10];
   uint32_t m_nextStreamId = 0x40000000;
};

}
