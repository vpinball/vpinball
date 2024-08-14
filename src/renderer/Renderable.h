// license:GPLv3+

#pragma once

class RenderDevice;

// Interface used by the renderer to render each part.
//
// All functions may only be called after RenderSetup and before RenderRelease.
// RenderRelease is guaranteed to be called before the object is disposed.
class Renderable
{
public:
   virtual void RenderSetup(RenderDevice *device) = 0;
   virtual void UpdateAnimation(const float diff_time_msec) = 0;
   virtual void Render(const unsigned int renderMask) = 0;
   virtual float GetDepth(const Vertex3Ds& viewDir) const { return 0.0f; }
   virtual void RenderRelease() = 0;
};
