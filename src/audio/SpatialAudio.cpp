// license:GPLv3+

#include "core/stdafx.h"
#include "SpatialAudio.h"
#include "core/VPApp.h"

#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_CUSTOM
#include "miniaudio/miniaudio.h"

#include "spatialaudio/AmbisonicBinauralizer.h"
#include "spatialaudio/AmbisonicDecoder.h"
#include "spatialaudio/AmbisonicEncoder.h"
#include "spatialaudio/AmbisonicRotator.h"
#include "spatialaudio/SpatialaudioConfig.h"


namespace VPX
{

void SpatialAudioListener::SetPose(float x, float y, float z, float yaw, float pitch, float roll)
{
   // Reject invalid poses (tracking loss can produce NaN/Inf through the view
   // matrix inversion): a single non finite value would permanently poison the
   // DSP state (rotation matrix, filter and convolution overlap buffers)
   if (!std::isfinite(x + y + z + yaw + pitch + roll))
      return;
   m_x = x;
   m_y = y;
   m_z = z;
   m_yaw = yaw;
   m_pitch = pitch;
   m_roll = roll;
}

void SpatialAudioListener::GetPose(float& x, float& y, float& z, float& yaw, float& pitch, float& roll) const
{
   x = m_x;
   y = m_y;
   z = m_z;
   yaw = m_yaw;
   pitch = m_pitch;
   roll = m_roll;
}


// VPU (table unit) to meters, matching the scale used by the VR renderer
static constexpr float kVPUToMeters = static_cast<float>(0.0254 * 1.0625 / 50.);
// Second order ambisonics needs a SOFA (full sphere) HRTF for binaural rendering: its
// 20 virtual speakers reach -69deg elevation while the built-in MIT HRTF rejects
// anything below -40deg (Configure fails, leading to an assert in Process)
#if SPATIALAUDIO_SUPPORTS_SOFA
static constexpr unsigned int kAmbiOrder = 2;
#else
static constexpr unsigned int kAmbiOrder = 1;
#endif
static constexpr unsigned int kAmbiChannels = (kAmbiOrder + 1) * (kAmbiOrder + 1); // 3D B-format, ACN channel order
// Maximum frames processed per ambisonic DSP call. This size directly sets the
// binauralizer's FFT convolution length (next pow2 of block + HRTF taps - 1),
// and the cost of each Process call is constant in that FFT size whatever the
// number of frames it is asked to output. A value much larger than the audio
// device period (typically ~480 frames) therefore multiplies the per-period CPU
// cost by the wasted FFT size (e.g. ~4.5ms per 10ms period at 4096 -> audio
// underruns and eventual device failure). 512 frames gives a 1024 points FFT
// (~10x cheaper) while staying efficient vs the ~256 tap HRTFs.
static constexpr unsigned int kMaxAmbiBlock = 512;
// Fade times (ms) used by libspatialaudio to smooth source position and head
// orientation changes (avoids clicks when scripts move sources or in VR)
static constexpr float kPositionFadeMs = 10.f;
static constexpr float kRotationFadeMs = 20.f;
// Head orientation change (radians, ~0.2 deg) below which the soundfield
// rotator is not retargeted: filters tracker jitter that would otherwise
// restart the rotation fade on every audio block in VR
static constexpr float kOrientationEpsilon = 0.004f;

// Playfield speaker positions, in spherical coordinates relative to the default
// listener (standing at the lockbar, facing the backglass): azimuth in degrees
// (positive = left), elevation in degrees (positive = up), distance in meters.
static constexpr SpatialSpeakerPosition kSpkCabFrontL { 45.f, -40.f, 0.40f, true }; // Under the lockbar, left
static constexpr SpatialSpeakerPosition kSpkCabFrontR { -45.f, -40.f, 0.40f, true }; // Under the lockbar, right
static constexpr SpatialSpeakerPosition kSpkMidL { 38.f, -22.f, 0.62f, true }; // Middle of the playfield, left
static constexpr SpatialSpeakerPosition kSpkMidR { -38.f, -22.f, 0.62f, true }; // Middle of the playfield, right
static constexpr SpatialSpeakerPosition kSpkCabBackL { 22.f, -12.f, 1.05f, true }; // Back of the cabinet, left
static constexpr SpatialSpeakerPosition kSpkCabBackR { -22.f, -12.f, 1.05f, true }; // Back of the cabinet, right
static constexpr SpatialSpeakerPosition kSpkInactive { 0.f, 0.f, 1.f, false };

// Backglass speaker positions (in the backbox, behind the far end of the playfield)
static constexpr SpatialSpeakerPosition kSpkBackglassL { 25.f, 15.f, 1.0f, true };
static constexpr SpatialSpeakerPosition kSpkBackglassR { -25.f, 15.f, 1.0f, true };

// SDL audio channel layout:
// 1 channel (mono) layout: FRONT
// 2 channels (stereo) layout: FL, FR
// 3 channels (2.1) layout: FL, FR, LFE
// 4 channels (quad) layout: FL, FR, BL, BR
// 5 channels (4.1) layout: FL, FR, LFE, BL, BR
// 6 channels (5.1) layout: FL, FR, FC, LFE, BL, BR (last two can also be SL, SR)
// 7 channels (6.1) layout: FL, FR, FC, LFE, BC, SL, SR
// 8 channels (7.1) layout: FL, FR, FC, LFE, BL, BR, SL, SR

// Index of the channel pairs in SDL layout, or -1 when not present
struct ChannelMap
{
   int fl, fr, fc, lfe, bc, bl, br, sl, sr;
};

static ChannelMap GetChannelMap(unsigned int nChannels)
{
   switch (nChannels)
   {
   case 1: return { -1, -1, -1, -1, -1, -1, -1, -1, -1 };
   case 2: return { 0, 1, -1, -1, -1, -1, -1, -1, -1 };
   case 3: return { 0, 1, -1, 2, -1, -1, -1, -1, -1 };
   case 4: return { 0, 1, -1, -1, -1, 2, 3, -1, -1 };
   case 5: return { 0, 1, -1, 2, -1, 3, 4, -1, -1 };
   case 6: return { 0, 1, 2, 3, -1, 4, 5, -1, -1 };
   case 7: return { 0, 1, 2, 3, 4, -1, -1, 5, 6 };
   default: return { 0, 1, 2, 3, -1, 4, 5, 6, 7 };
   }
}

std::vector<SpatialSpeakerPosition> SpatialAudioMixer::GetPlayfieldSpeakerLayout(int soundConfigMode, unsigned int nChannels)
{
   std::vector<SpatialSpeakerPosition> layout(nChannels < 8 ? nChannels : 8, kSpkInactive);
   if (nChannels == 0)
      return layout;
   const ChannelMap map = GetChannelMap(nChannels);
   auto place = [&layout](int ch, const SpatialSpeakerPosition& pos)
   {
      if (ch >= 0)
         layout[ch] = pos;
   };
   switch (soundConfigMode)
   {
   case 0: // SNDCFG_SND3D2CH: stereo pair under the lockbar
      if (nChannels == 1)
         place(0, kSpkCabFrontL);
      else
      {
         place(map.fl, kSpkCabFrontL);
         place(map.fr, kSpkCabFrontR);
      }
      break;
   case 1: // SNDCFG_SND3DALLREAR: stereo pair under the lockbar on the rear channels
      if (nChannels <= 3)
      {
         place(map.fl, kSpkCabFrontL);
         place(map.fr, kSpkCabFrontR);
      }
      else
      {
         place(map.bl, kSpkCabFrontL);
         place(map.br, kSpkCabFrontR);
      }
      break;
   case 3: // SNDCFG_SND3DFRONTISFRONT: front channels near the front of the cab (lockbar)
      place(map.fl, kSpkCabFrontL);
      place(map.fr, kSpkCabFrontR);
      place(map.sl, kSpkMidL);
      place(map.sr, kSpkMidR);
      place(map.bl, kSpkCabBackL);
      place(map.br, kSpkCabBackR);
      break;
   case 2: // SNDCFG_SND3DFRONTISREAR: rear channels near the front of the cab (lockbar)
      place(map.bl, kSpkCabFrontL);
      place(map.br, kSpkCabFrontR);
      place(map.sl, kSpkMidL);
      place(map.sr, kSpkMidR);
      place(map.fl, kSpkCabBackL);
      place(map.fr, kSpkCabBackR);
      break;
   default: // SNDCFG_SND3D6CH & SNDCFG_SND3DSSF: side & rear channels, front kept free for backglass audio
      place(map.bl, kSpkCabFrontL);
      place(map.br, kSpkCabFrontR);
      place(map.sl, kSpkCabBackL);
      place(map.sr, kSpkCabBackR);
      break;
   }
   return layout;
}

std::vector<SpatialSpeakerPosition> SpatialAudioMixer::GetBackglassSpeakerLayout(unsigned int nChannels)
{
   std::vector<SpatialSpeakerPosition> layout(nChannels < 8 ? nChannels : 8, kSpkInactive);
   if (nChannels == 0)
      return layout;
   const ChannelMap map = GetChannelMap(nChannels);
   if (nChannels == 1)
      layout[0] = kSpkBackglassL;
   else
   {
      if (map.fl >= 0)
         layout[map.fl] = kSpkBackglassL;
      if (map.fr >= 0)
         layout[map.fr] = kSpkBackglassR;
   }
   return layout;
}


struct vpx_spatial_mix_node
{
   ma_node_base base;
   SpatialAudioMixer::Impl* impl;
};

struct SpatialAudioMixer::Impl
{
   vpx_spatial_mix_node node;
   SpatialAudioListener* listener = nullptr;
   unsigned int outChannels = 0;
   spaudio::AmbisonicRotator rotator;
   spaudio::AmbisonicDecoder decoder;
   spaudio::AmbisonicBinauralizer binauralizer;
   spaudio::BFormat bFormat;
   std::vector<int> speakerToChannel; // output channel index for each decoded speaker
   std::vector<std::vector<float>> speakerBuffers;
   std::vector<float*> speakerPtrs;
   std::vector<float> channelScratch[kAmbiChannels];
   std::atomic<bool> binaural { false };
   bool binauralConfigured = false;
   // Last orientation sent to the rotator (audio thread only, no atomics needed)
   float appliedYaw = 0.f, appliedPitch = 0.f, appliedRoll = 0.f;
   bool orientationApplied = false;

   void Process(const float* in, unsigned int count, float* out)
   {
      // Deinterleave the B-format input bus into per channel buffers, dropping
      // non finite samples so a bad source cannot permanently poison the
      // rotator, shelf filters and convolution overlap state downstream
      for (unsigned int ch = 0; ch < kAmbiChannels; ch++)
      {
         float* scratch = channelScratch[ch].data();
         for (unsigned int i = 0; i < count; i++)
         {
            const float v = in[i * kAmbiChannels + ch];
            scratch[i] = std::isfinite(v) ? v : 0.f;
         }
         bFormat.InsertStream(scratch, ch, count);
      }
      // Rotate the whole soundfield to the listener head orientation (VR head
      // tracking). Sources are encoded in table space, so only the orientation
      // is applied here (translation is handled per source).
      {
         float lx, ly, lz, yaw, pitch, roll;
         listener->GetPose(lx, ly, lz, yaw, pitch, roll);
         // Conventions: our yaw is positive when looking left while a positive
         // rotator yaw moves a front source to the left (equivalent to turning
         // the head to the right), hence the sign change. Pitch (positive =
         // looking up) and roll (positive = tilting the head to the right)
         // already match the rotator conventions.
         const float rotYaw = -yaw;
         // Skip non finite poses: they would poison the rotation matrix, and the
         // epsilon gate would then never recover (fabsf(NaN) > eps is false)
         if (std::isfinite(rotYaw + pitch + roll)
            && (!orientationApplied || fabsf(rotYaw - appliedYaw) > kOrientationEpsilon || fabsf(pitch - appliedPitch) > kOrientationEpsilon
               || fabsf(roll - appliedRoll) > kOrientationEpsilon))
         {
            rotator.SetOrientation({ rotYaw, pitch, roll });
            appliedYaw = rotYaw;
            appliedPitch = pitch;
            appliedRoll = roll;
            orientationApplied = true;
         }
         rotator.Process(&bFormat, count);
      }
      const bool binauralize = binaural.load(std::memory_order_relaxed) && binauralConfigured && outChannels >= 2;
      if (binauralize)
         binauralizer.Process(&bFormat, speakerPtrs.data(), count);
      else
         decoder.Process(&bFormat, count, speakerPtrs.data());
      // Interleave the speaker feeds to the output channels
      const unsigned int nSpeakers = static_cast<unsigned int>(speakerToChannel.size());
      if (binauralize)
      {
         for (unsigned int i = 0; i < count; i++)
         {
            out[i * outChannels] = speakerPtrs[0][i];
            out[i * outChannels + 1] = speakerPtrs[1][i];
            for (unsigned int ch = 2; ch < outChannels; ch++)
               out[i * outChannels + ch] = 0.f;
         }
      }
      else
      {
         for (unsigned int i = 0; i < count; i++)
            for (unsigned int ch = 0; ch < outChannels; ch++)
               out[i * outChannels + ch] = 0.f;
         for (unsigned int spk = 0; spk < nSpeakers; spk++)
         {
            const int ch = speakerToChannel[spk];
            const float* feed = speakerPtrs[spk];
            for (unsigned int i = 0; i < count; i++)
               out[i * outChannels + ch] = feed[i];
         }
      }
   }
};

static void vpx_spatial_mix_process(ma_node* pNode, const float** ppFramesIn, ma_uint32* pFrameCountIn, float** ppFramesOut, ma_uint32* pFrameCountOut)
{
   auto* node = reinterpret_cast<vpx_spatial_mix_node*>(pNode);
   const float* in = ppFramesIn[0];
   float* out = ppFramesOut[0];
   ma_uint32 remaining = *pFrameCountOut;
   while (remaining > 0)
   {
      const unsigned int count = remaining < kMaxAmbiBlock ? remaining : kMaxAmbiBlock;
      node->impl->Process(in, count, out);
      in += count * kAmbiChannels;
      out += count * node->impl->outChannels;
      remaining -= count;
   }
}

static ma_node_vtable vpx_spatial_mix_vtable = { vpx_spatial_mix_process, nullptr, 1, 1, 0 };

SpatialAudioMixer::SpatialAudioMixer(ma_engine* engine, const std::vector<SpatialSpeakerPosition>& speakerLayout)
   : m_impl(std::make_unique<Impl>())
{
   m_impl->listener = &m_listener;
   m_impl->outChannels = ma_engine_get_channels(engine);
   const unsigned int sampleRate = ma_engine_get_sample_rate(engine);
   m_impl->bFormat.Configure(kAmbiOrder, true, kMaxAmbiBlock);
   m_impl->rotator.Configure(kAmbiOrder, true, kMaxAmbiBlock, sampleRate, kRotationFadeMs);

   // Configure the loudspeaker decoder with the active speakers of the layout
   unsigned int nSpeakers = 0;
   for (unsigned int ch = 0; ch < m_impl->outChannels && ch < speakerLayout.size(); ch++)
      if (speakerLayout[ch].active)
         nSpeakers++;
   m_impl->decoder.Configure(kAmbiOrder, true, kMaxAmbiBlock, sampleRate, spaudio::Amblib_SpeakerSetUps::kAmblib_CustomSpeakerSetUp, nSpeakers);
   m_impl->speakerToChannel.reserve(nSpeakers);
   for (unsigned int ch = 0, spk = 0; ch < m_impl->outChannels && ch < speakerLayout.size(); ch++)
   {
      if (!speakerLayout[ch].active)
         continue;
      m_impl->decoder.SetPosition(spk, { spaudio::DegreesToRadians(speakerLayout[ch].azimuth), spaudio::DegreesToRadians(speakerLayout[ch].elevation), speakerLayout[ch].distance });
      m_impl->speakerToChannel.push_back(static_cast<int>(ch));
      spk++;
   }
   m_impl->decoder.Refresh();

   // Configure the binauralizer (used for headphones and VR play)
   unsigned int tailLength = 0;
#if SPATIALAUDIO_SUPPORTS_SOFA
   // Use the bundled full sphere FABIAN HRTF: the built-in MIT HRTF cannot cover
   // the virtual speakers below -40deg elevation used by 2nd order decoding
   const std::string hrtfPath = g_app ? (g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets) / "fabian-hrir.sofa").string() : std::string();
   try
   {
      m_impl->binauralConfigured = m_impl->binauralizer.Configure(kAmbiOrder, true, sampleRate, kMaxAmbiBlock, tailLength, hrtfPath, false);
   }
   catch (const std::exception&)
   {
      m_impl->binauralConfigured = false;
   }
   if (!m_impl->binauralConfigured)
   {
      PLOGE << "Failed to load the binaural HRTF '" << hrtfPath << "', falling back to loudspeaker decoding";
   }
#else
   m_impl->binauralConfigured = m_impl->binauralizer.Configure(kAmbiOrder, true, sampleRate, kMaxAmbiBlock, tailLength);
   if (!m_impl->binauralConfigured)
   {
      PLOGE << "Failed to configure binaural audio, falling back to loudspeaker decoding";
   }
#endif

   const unsigned int nBuffers = std::max(nSpeakers, 2u);
   m_impl->speakerBuffers.resize(nBuffers, std::vector<float>(kMaxAmbiBlock, 0.f));
   for (auto& buf : m_impl->speakerBuffers)
      m_impl->speakerPtrs.push_back(buf.data());
   for (auto& scratch : m_impl->channelScratch)
      scratch.resize(kMaxAmbiBlock, 0.f);

   ma_node_config nodeConfig = ma_node_config_init();
   nodeConfig.vtable = &vpx_spatial_mix_vtable;
   const ma_uint32 inChannels = kAmbiChannels;
   nodeConfig.pInputChannels = &inChannels;
   nodeConfig.pOutputChannels = &m_impl->outChannels;
   m_impl->node.impl = m_impl.get();
   if (ma_node_init(ma_engine_get_node_graph(engine), &nodeConfig, nullptr, &m_impl->node.base) != MA_SUCCESS)
   {
      PLOGE << "Failed to initialize spatial audio mixer node";
      m_impl = nullptr;
      return;
   }
   ma_node_attach_output_bus(&m_impl->node.base, 0, ma_engine_get_endpoint(engine), 0);
}

SpatialAudioMixer::~SpatialAudioMixer()
{
   if (m_impl)
      ma_node_uninit(&m_impl->node.base, nullptr);
}

void* SpatialAudioMixer::GetMixBus() const { return m_impl ? &m_impl->node.base : nullptr; }

void SpatialAudioMixer::SetBinauralEnabled(bool enabled)
{
   if (m_impl)
      m_impl->binaural = enabled;
}


struct vpx_spatial_src_node
{
   ma_node_base base;
   SpatialAudioSource::Impl* impl;
};

struct SpatialAudioSource::Impl
{
   vpx_spatial_src_node node;
   SpatialAudioListener* listener = nullptr;
   unsigned int inChannels = 0;
   spaudio::AmbisonicEncoder encoderL;
   spaudio::AmbisonicEncoder encoderR;
   spaudio::BFormat bFormat;
   std::vector<float> scratchL;
   std::vector<float> scratchR;
   std::vector<float> channelScratch[kAmbiChannels];
   // Position requested by the control thread, read on the audio thread
   std::atomic<float> posX { 0.f }; // table units, or polar mode fields below
   std::atomic<float> posY { 0.f };
   std::atomic<float> posZ { 0.f };
   std::atomic<float> polAzimuth { 0.f }; // degrees
   std::atomic<float> polElevation { 0.f };
   std::atomic<float> polDistance { 1.f }; // meters
   std::atomic<bool> polarMode { false };
   std::atomic<float> volume { 1.f };
   std::atomic<float> channelBalance { 0.f };
   std::atomic<float> referenceDistance { 0.55f };
   float stereoSpread = 25.f; // degrees around the center azimuth for stereo (backglass) sources
   float gainL = 1.f; // computed by UpdatePosition on the audio thread
   float gainR = 1.f;

   void UpdatePosition()
   {
      float azimuth, elevation, distance; // angles in radians
      if (polarMode.load(std::memory_order_relaxed))
      {
         azimuth = spaudio::DegreesToRadians(polAzimuth.load(std::memory_order_relaxed));
         elevation = spaudio::DegreesToRadians(polElevation.load(std::memory_order_relaxed));
         distance = polDistance.load(std::memory_order_relaxed);
      }
      else
      {
         float lx, ly, lz, yaw, pitch, roll;
         listener->GetPose(lx, ly, lz, yaw, pitch, roll);
         // Source position relative to the listener, in meters. Orientation is
         // not applied here: sources are encoded in table space (listener
         // looking toward the backglass, i.e. -y) and the mixer rotates the
         // whole soundfield to the actual head orientation.
         const float dx = (posX.load(std::memory_order_relaxed) - lx) * kVPUToMeters; // right
         const float dy = (posY.load(std::memory_order_relaxed) - ly) * kVPUToMeters; // toward the lockbar/player
         const float dz = (posZ.load(std::memory_order_relaxed) - lz) * kVPUToMeters; // up
         const float forward = -dy;
         const float flat = sqrtf(dx * dx + forward * forward);
         azimuth = atan2f(-dx, forward); // positive azimuth = left
         elevation = atan2f(dz, flat);
         distance = sqrtf(flat * flat + dz * dz);
      }
      // Distance attenuation around the reference distance (closer = louder, farther = quieter)
      const float refDistance = std::max(0.01f, referenceDistance.load(std::memory_order_relaxed));
      const float distGain = clamp(refDistance / std::max(refDistance, distance), 0.f, 3.f);
      const float vol = volume.load(std::memory_order_relaxed);
      const float balance = clamp(channelBalance.load(std::memory_order_relaxed), -1.f, 1.f);
      const float newGainL = vol * distGain * (balance < 0.f ? 1.f : 1.f - balance);
      const float newGainR = vol * distGain * (balance > 0.f ? 1.f : 1.f + balance);
      // Keep the previous position and gains on non finite values (NaN/Inf
      // would poison the encoder coefficients and the downstream DSP state)
      if (!std::isfinite(azimuth + elevation + distance + newGainL + newGainR))
         return;
      gainL = newGainL;
      gainR = newGainR;
      const float spread = spaudio::DegreesToRadians(stereoSpread);
      if (inChannels == 2)
      {
         // Stereo sources are spread around the center azimuth
         encoderL.SetPosition({ azimuth + spread, elevation, distance });
         encoderR.SetPosition({ azimuth - spread, elevation, distance });
      }
      else
      {
         encoderL.SetPosition({ azimuth, elevation, distance });
      }
   }

   void Process(const float* in, unsigned int count, float* out)
   {
      UpdatePosition();
      // Reset the B-format accumulator: GainInterp does not write channels
      // whose target gain is ~0, so without a reset they would keep stale
      // content, and worse, ProcessAccumul (stereo right channel) would keep
      // adding into them, growing without bound until Inf/NaN
      bFormat.Reset();
      if (inChannels == 2)
      {
         // Deinterleave stereo input, applying volume, distance attenuation and channel balance
         for (unsigned int i = 0; i < count; i++)
         {
            scratchL[i] = in[i * 2] * gainL;
            scratchR[i] = in[i * 2 + 1] * gainR;
         }
         encoderL.Process(scratchL.data(), count, &bFormat);
         encoderR.ProcessAccumul(scratchR.data(), count, &bFormat);
      }
      else
      {
         for (unsigned int i = 0; i < count; i++)
            scratchL[i] = in[i] * gainL;
         encoderL.Process(scratchL.data(), count, &bFormat);
      }
      // Interleave the B-format channels (ACN order) to the output
      for (unsigned int ch = 0; ch < kAmbiChannels; ch++)
      {
         float* scratch = channelScratch[ch].data();
         bFormat.ExtractStream(scratch, ch, count);
         for (unsigned int i = 0; i < count; i++)
            out[i * kAmbiChannels + ch] = scratch[i];
      }
   }
};

static void vpx_spatial_src_process(ma_node* pNode, const float** ppFramesIn, ma_uint32* pFrameCountIn, float** ppFramesOut, ma_uint32* pFrameCountOut)
{
   auto* node = reinterpret_cast<vpx_spatial_src_node*>(pNode);
   const float* in = ppFramesIn[0];
   float* out = ppFramesOut[0];
   ma_uint32 remaining = *pFrameCountOut;
   while (remaining > 0)
   {
      const unsigned int count = remaining < kMaxAmbiBlock ? remaining : kMaxAmbiBlock;
      node->impl->Process(in, count, out);
      in += count * node->impl->inChannels;
      out += count * kAmbiChannels;
      remaining -= count;
   }
}

static ma_node_vtable vpx_spatial_src_vtable = { vpx_spatial_src_process, nullptr, 1, 1, 0 };

SpatialAudioSource::SpatialAudioSource(ma_engine* engine, SpatialAudioMixer* mixer, unsigned int inChannels)
   : m_impl(std::make_unique<Impl>())
{
   m_impl->inChannels = inChannels;
   m_impl->listener = mixer->GetListener();
   const unsigned int sampleRate = ma_engine_get_sample_rate(engine);
   m_impl->encoderL.Configure(kAmbiOrder, true, sampleRate, kPositionFadeMs);
   m_impl->encoderR.Configure(kAmbiOrder, true, sampleRate, kPositionFadeMs);
   m_impl->bFormat.Configure(kAmbiOrder, true, kMaxAmbiBlock);
   m_impl->scratchL.resize(kMaxAmbiBlock, 0.f);
   m_impl->scratchR.resize(kMaxAmbiBlock, 0.f);
   for (auto& scratch : m_impl->channelScratch)
      scratch.resize(kMaxAmbiBlock, 0.f);

   ma_node_config nodeConfig = ma_node_config_init();
   nodeConfig.vtable = &vpx_spatial_src_vtable;
   const ma_uint32 outChannels = kAmbiChannels;
   nodeConfig.pInputChannels = &m_impl->inChannels;
   nodeConfig.pOutputChannels = &outChannels;
   m_impl->node.impl = m_impl.get();
   if (ma_node_init(ma_engine_get_node_graph(engine), &nodeConfig, nullptr, &m_impl->node.base) != MA_SUCCESS)
   {
      PLOGE << "Failed to initialize spatial audio source node";
      m_impl = nullptr;
      return;
   }
   if (mixer->GetMixBus() != nullptr)
      ma_node_attach_output_bus(&m_impl->node.base, 0, static_cast<ma_node*>(mixer->GetMixBus()), 0);
   m_node = &m_impl->node.base;
}

SpatialAudioSource::~SpatialAudioSource()
{
   if (m_impl)
      ma_node_uninit(&m_impl->node.base, nullptr);
}

void SpatialAudioSource::SetPosition(float x, float y, float z)
{
   if (m_impl == nullptr)
      return;
   m_impl->posX = x;
   m_impl->posY = y;
   m_impl->posZ = z;
   m_impl->polarMode = false;
}

void SpatialAudioSource::SetPositionPolar(float azimuth, float elevation, float distance)
{
   if (m_impl == nullptr)
      return;
   m_impl->polAzimuth = azimuth;
   m_impl->polElevation = elevation;
   m_impl->polDistance = distance;
   m_impl->polarMode = true;
}

void SpatialAudioSource::SetVolume(float volume)
{
   if (m_impl)
      m_impl->volume = volume;
}

void SpatialAudioSource::SetChannelBalance(float balance)
{
   if (m_impl)
      m_impl->channelBalance = balance;
}

void SpatialAudioSource::SetReferenceDistance(float distance)
{
   if (m_impl)
      m_impl->referenceDistance = distance;
}

}
