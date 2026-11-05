// license:GPLv3+

#include "core/stdafx.h"
#include "SpatialAudio.h"
#include "core/VPApp.h"

#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_CUSTOM
#include "miniaudio/miniaudio.h"

#include "spatialaudio/AmbisonicBinauralizer.h"
#include "spatialaudio/AmbisonicEncoder.h"
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
// Fade time (ms) used by libspatialaudio to smooth source position changes
// (avoids clicks when scripts move sources or when the head moves in VR)
static constexpr float kPositionFadeMs = 10.f;
// Makeup gain for the binaural (VR headset) path: libspatialaudio's
// binauralizer normalizes its HRTF filters so that the worst case peak tap is
// 0.35 (~-9dB of headroom), rendering everything noticeably quieter than the
// direct stereo paths. +6dB restores most of the loss while keeping headroom
// for the up-to-3x close distance attenuation boost
static constexpr float kBinauralGain = 2.f;


struct vpx_spatial_mix_node
{
   ma_node_base base;
   SpatialAudioMixer::Impl* impl;
};

struct SpatialAudioMixer::Impl
{
   vpx_spatial_mix_node node;
   unsigned int outChannels = 0;
   spaudio::AmbisonicBinauralizer binauralizer;
   spaudio::BFormat bFormat;
   std::vector<std::vector<float>> outBuffers;
   std::vector<float*> outPtrs;
   std::vector<float> channelScratch[kAmbiChannels];
   bool binauralConfigured = false;

   void Process(const float* in, unsigned int count, float* out)
   {
      // Deinterleave the B-format input bus into per channel buffers, dropping
      // non finite samples so a bad source cannot permanently poison the
      // binauralizer's filter and convolution overlap state downstream
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
      // Binauralize on the 2 first output channels (VR headset). Stays silent
      // when the binauralizer failed to configure (the buffers are zeroed).
      if (binauralConfigured)
         binauralizer.Process(&bFormat, outPtrs.data(), count);
      for (unsigned int i = 0; i < count; i++)
         for (unsigned int ch = 0; ch < outChannels; ch++)
            out[i * outChannels + ch] = ch < 2 ? outBuffers[ch][i] * kBinauralGain : 0.f;
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

SpatialAudioMixer::SpatialAudioMixer(ma_engine* engine)
   : m_impl(std::make_unique<Impl>())
{
   m_impl->outChannels = ma_engine_get_channels(engine);
   const unsigned int sampleRate = ma_engine_get_sample_rate(engine);
   m_impl->bFormat.Configure(kAmbiOrder, true, kMaxAmbiBlock);

   // Configure the binauralizer (VR headset rendering on the 2 first output channels)
   unsigned int tailLength = 0;
#if SPATIALAUDIO_SUPPORTS_SOFA
   // Use the bundled full sphere FABIAN HRTF: the built-in MIT HRTF cannot cover
   // the virtual speakers below -40deg elevation used by 2nd order encoding
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
      PLOGE << "Failed to load the binaural HRTF '" << hrtfPath << "', spatial audio will be silent";
   }
#else
   m_impl->binauralConfigured = m_impl->binauralizer.Configure(kAmbiOrder, true, sampleRate, kMaxAmbiBlock, tailLength);
   if (!m_impl->binauralConfigured)
   {
      PLOGE << "Failed to configure binaural audio, spatial audio will be silent";
   }
#endif

   m_impl->outBuffers.resize(2, std::vector<float>(kMaxAmbiBlock, 0.f));
   for (auto& buf : m_impl->outBuffers)
      m_impl->outPtrs.push_back(buf.data());
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


struct vpx_spatial_src_node
{
   ma_node_base base;
   SpatialAudioSource::Impl* impl;
};

struct SpatialAudioSource::Impl
{
   vpx_spatial_src_node node;
   SpatialAudioListener* listener = nullptr;
   SpatialAudioListener* referenceListener = nullptr;
   unsigned int inChannels = 0;
   spaudio::AmbisonicEncoder encoderL;
   spaudio::AmbisonicEncoder encoderR;
   spaudio::BFormat bFormat;
   std::vector<float> scratchL;
   std::vector<float> scratchR;
   std::vector<float> channelScratch[kAmbiChannels];
   // Emitter positions in table units requested by the control thread, read on
   // the audio thread. All emitters are mono: mono sources emit from the
   // (posX, posY, posZ) point, stereo sources emit each channel at its
   // explicit position when channelPositions is set (e.g. the 2 backglass
   // speakers), both at the (posX, posY, posZ) point otherwise.
   std::atomic<float> posX { 0.f };
   std::atomic<float> posY { 0.f };
   std::atomic<float> posZ { 0.f };
   std::atomic<float> posLX { 0.f };
   std::atomic<float> posLY { 0.f };
   std::atomic<float> posLZ { 0.f };
   std::atomic<float> posRX { 0.f };
   std::atomic<float> posRY { 0.f };
   std::atomic<float> posRZ { 0.f };
   std::atomic<bool> channelPositions { false };
   std::atomic<float> volume { 1.f };
   std::atomic<float> channelBalance { 0.f };
   float gainL = 1.f; // computed by UpdatePosition on the audio thread
   float gainR = 1.f;

   void UpdatePosition()
   {
      float lx, ly, lz, yaw, pitch, roll;
      listener->GetPose(lx, ly, lz, yaw, pitch, roll);
      float rlx, rly, rlz, ryaw, rpitch, rroll;
      referenceListener->GetPose(rlx, rly, rlz, ryaw, rpitch, rroll);
      // Rotation applied to the emitter directions to express them in the
      // listener head frame (VR head tracking): our yaw is positive when
      // looking left, our pitch is negated as the ambisonic convention takes
      // positive pitch for tilting down, and our roll is positive when tilting
      // right, applied in yaw-pitch-roll order. This is done per source rather
      // than on the mixed soundfield: libspatialaudio's AmbisonicRotator fades
      // its SH matrix coefficients but drops the ones crossing zero (cos terms
      // at +-90deg yaw, sin2 terms at the same spot, ...), producing clicks on
      // every orientation retarget, while the encoders' coefficient
      // interpolation handles the crossings smoothly.
      const float cy = cosf(yaw), sy = sinf(yaw);
      const float cp = cosf(-pitch), sp = sinf(-pitch);
      const float cr = cosf(roll), sr = sinf(roll);
      // Emitter position (table units) to ambisonic direction relative to the
      // listener pose: azimuth/elevation in radians (positive azimuth = left),
      // distance in meters.
      auto toPolar = [&](float x, float y, float z, float& azimuth, float& elevation, float& distance)
      {
         const float dx = (x - lx) * kVPUToMeters; // right
         const float dy = (y - ly) * kVPUToMeters; // toward the lockbar/player
         const float dz = (z - lz) * kVPUToMeters; // up
         distance = sqrtf(dx * dx + dy * dy + dz * dz);
         // Direction as (front, left, up), front being toward the backglass (-y)
         const float inv = 1.f / std::max(0.001f, distance);
         float fx = -dy * inv, fy = -dx * inv, fz = dz * inv, t;
         t = cy * fx + sy * fy;
         fy = -sy * fx + cy * fy;
         fx = t; // yaw
         t = cp * fz + sp * fx;
         fx = -sp * fz + cp * fx;
         fz = t; // pitch
         t = cr * fy + sr * fz;
         fz = -sr * fy + cr * fz;
         fy = t; // roll
         azimuth = atan2f(fy, fx);
         elevation = atan2f(fz, sqrtf(fx * fx + fy * fy));
      };
      // Distance attenuation relative to the emitter's distance to the
      // canonical listener pose (the fake pose the tables' loudness is
      // authored for): the tracked pose only modulates loudness relative to
      // the authored balance instead of applying a global attenuation
      // (closer = louder, farther = quieter, bounded to 3x)
      auto distanceGain = [rlx, rly, rlz](float x, float y, float z, float distance)
      {
         const float rdx = (x - rlx) * kVPUToMeters;
         const float rdy = (y - rly) * kVPUToMeters;
         const float rdz = (z - rlz) * kVPUToMeters;
         const float refDistance = std::max(0.01f, sqrtf(rdx * rdx + rdy * rdy + rdz * rdz));
         return clamp(refDistance / std::max(0.01f, distance), 0.f, 3.f);
      };
      // angles in radians, distances in meters
      float azimuthL, elevationL, distanceL;
      float azimuthR = 0.f, elevationR = 0.f, distanceR = 0.f;
      float distGainL, distGainR = 0.f;
      if (inChannels == 2 && channelPositions.load(std::memory_order_relaxed))
      {
         // Stereo source with an explicit emitter position per channel
         const float xL = posLX.load(std::memory_order_relaxed);
         const float yL = posLY.load(std::memory_order_relaxed);
         const float zL = posLZ.load(std::memory_order_relaxed);
         const float xR = posRX.load(std::memory_order_relaxed);
         const float yR = posRY.load(std::memory_order_relaxed);
         const float zR = posRZ.load(std::memory_order_relaxed);
         toPolar(xL, yL, zL, azimuthL, elevationL, distanceL);
         toPolar(xR, yR, zR, azimuthR, elevationR, distanceR);
         distGainL = distanceGain(xL, yL, zL, distanceL);
         distGainR = distanceGain(xR, yR, zR, distanceR);
      }
      else
      {
         const float x = posX.load(std::memory_order_relaxed);
         const float y = posY.load(std::memory_order_relaxed);
         const float z = posZ.load(std::memory_order_relaxed);
         toPolar(x, y, z, azimuthL, elevationL, distanceL);
         azimuthR = azimuthL;
         elevationR = elevationL;
         distanceR = distanceL;
         distGainL = distGainR = distanceGain(x, y, z, distanceL);
      }
      const float vol = volume.load(std::memory_order_relaxed);
      const float balance = clamp(channelBalance.load(std::memory_order_relaxed), -1.f, 1.f);
      const float newGainL = vol * distGainL * (balance < 0.f ? 1.f : 1.f - balance);
      const float newGainR = vol * distGainR * (balance > 0.f ? 1.f : 1.f + balance);
      // Keep the previous position and gains on non finite values (NaN/Inf
      // would poison the encoder coefficients and the downstream DSP state)
      if (!std::isfinite(azimuthL + elevationL + distanceL + azimuthR + elevationR + distanceR + newGainL + newGainR))
         return;
      gainL = newGainL;
      gainR = newGainR;
      encoderL.SetPosition({ azimuthL, elevationL, distanceL });
      if (inChannels == 2)
         encoderR.SetPosition({ azimuthR, elevationR, distanceR });
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
   m_impl->referenceListener = mixer->GetReferenceListener();
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
   m_impl->channelPositions = false;
}

void SpatialAudioSource::SetChannelPositions(float xLeft, float yLeft, float zLeft, float xRight, float yRight, float zRight)
{
   if (m_impl == nullptr)
      return;
   m_impl->posLX = xLeft;
   m_impl->posLY = yLeft;
   m_impl->posLZ = zLeft;
   m_impl->posRX = xRight;
   m_impl->posRY = yRight;
   m_impl->posRZ = zRight;
   m_impl->channelPositions = true;
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

}
