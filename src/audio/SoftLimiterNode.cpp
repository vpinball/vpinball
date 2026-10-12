// license:GPLv3+

#include "core/stdafx.h"
#include "SoftLimiterNode.h"

#include "miniaudio/miniaudio.h"

namespace VPX
{

// Soft limiter curve. Below the knee the sample is returned unchanged; above it the
// overshoot is compressed by a cheap rational so the output approaches but never reaches
// 1.0. Continuous in value at the knee. The 0.90 knee sits just above the loudest level
// normal content was measured to reach, so typical audio passes through untouched.
static inline float vpx_soft_limit(const float x)
{
   constexpr float knee = 0.90f;
   const float a = fabsf(x);
   if (a <= knee)
      return x;
   const float over = a - knee;
   const float limited = knee + (1.f - knee) * (over / (over + (1.f - knee)));
   return (x < 0.f) ? -limited : limited;
}

static void vpx_limiter_process(ma_node* pNode, const float** ppFramesIn, ma_uint32* pFrameCountIn, float** ppFramesOut, ma_uint32* pFrameCountOut)
{
   const float* in = ppFramesIn[0];
   float* out = ppFramesOut[0];
   const ma_uint32 channels = ma_node_get_output_channels(pNode, 0);
   const ma_uint32 n = *pFrameCountOut * channels;
   for (ma_uint32 i = 0; i < n; i++)
      out[i] = vpx_soft_limit(in[i]);
   *pFrameCountIn = *pFrameCountOut;
}

static ma_node_vtable g_vpx_limiter_vtable = { vpx_limiter_process, nullptr, 1, 1, 0 };

struct SoftLimiterNode::Impl
{
   ma_node_base node {};
   bool nodeInit = false;
};

std::unique_ptr<SoftLimiterNode> SoftLimiterNode::Create(ma_engine* engine)
{
   if (engine == nullptr)
      return nullptr;

   auto limiter = std::unique_ptr<SoftLimiterNode>(new SoftLimiterNode());
   limiter->m_impl = std::make_unique<Impl>();

   const ma_uint32 channels = ma_engine_get_channels(engine);
   ma_node_config config = ma_node_config_init();
   config.vtable = &g_vpx_limiter_vtable;
   config.pInputChannels = &channels;
   config.pOutputChannels = &channels;
   if (ma_node_init(ma_engine_get_node_graph(engine), &config, nullptr, &limiter->m_impl->node) != MA_SUCCESS)
   {
      PLOGE << "SoftLimiterNode: failed to init node";
      return nullptr;
   }
   limiter->m_impl->nodeInit = true;
   ma_node_attach_output_bus(&limiter->m_impl->node, 0, ma_engine_get_endpoint(engine), 0);
   return limiter;
}

SoftLimiterNode::~SoftLimiterNode()
{
   if (m_impl && m_impl->nodeInit)
      ma_node_uninit(&m_impl->node, nullptr);
}

void* SoftLimiterNode::GetNode() const
{
   return (m_impl && m_impl->nodeInit) ? static_cast<void*>(&m_impl->node) : nullptr;
}

}
