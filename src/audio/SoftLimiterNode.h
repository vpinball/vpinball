// license:GPLv3+

#pragma once

// A reusable miniaudio soft-limiter node. It rolls peaks off smoothly just below full
// scale so audio that would otherwise exceed 0 dBFS does not hard-clip when the engine's
// float mix is converted to the device format. Below the knee the signal is passed through
// unchanged, so quiet/normal audio is bit-identical and only material that would clip is shaped.
//
// It is a plain pass-through processing node (1 in bus, 1 out bus, same channel count), so
// it can sit anywhere in a node graph: today it limits the shared plugin-audio bus, but it
// can be moved onto an engine endpoint to limit all audio by changing only what attaches to
// it and where it attaches, not this node.

struct ma_engine;

namespace VPX
{

class SoftLimiterNode
{
public:
   // Creates a limiter node on the engine's node graph with the engine's channel count, and
   // attaches its output to the engine endpoint. Returns nullptr on failure.
   static std::unique_ptr<SoftLimiterNode> Create(ma_engine* engine);
   ~SoftLimiterNode();

   // The node to attach upstream sources' output buses to (so they are limited). Returned
   // as void* (which is what ma_node is) to avoid including miniaudio.h in this header.
   void* GetNode() const;

private:
   SoftLimiterNode() = default;
   struct Impl;
   std::unique_ptr<Impl> m_impl;
};

}
