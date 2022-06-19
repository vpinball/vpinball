#pragma once

#include "typedefs3D.h"
class RenderDevice;

class RenderTarget
{
public:
   RenderTarget(RenderDevice* rd); // Default output render target
   RenderTarget(RenderDevice* rd, const int width, const int height, const colorFormat format, bool with_depth, bool use_MSAA, int stereo, char* failureMessage);
   ~RenderTarget();

   void Activate(bool ignoreStereo);

   Sampler* GetColorSampler() { return m_color_sampler; }
   void UpdateDepthSampler();
   Sampler* GetDepthSampler() { return m_depth_sampler; }

   RenderTarget* Duplicate();
   void CopyTo(RenderTarget* dest);

#ifndef ENABLE_SDL
   IDirect3DSurface9* GetCoreColorSurface() { return m_color_surface; }
#endif

private:
   bool m_is_back_buffer;
   int m_width;
   int m_height;
   colorFormat m_format;
   bool m_has_depth;
   bool m_use_mSAA;
   int m_stereo;
   RenderDevice* m_rd;
   Sampler* m_color_sampler;
   Sampler* m_depth_sampler;

#ifdef ENABLE_SDL
   GLuint m_framebuffer;
   GLuint m_color_tex;
   GLuint m_depth_tex;
#else
   bool m_use_alternate_depth;
   IDirect3DSurface9* m_color_surface;
   IDirect3DTexture9* m_color_tex;
   IDirect3DSurface9* m_depth_surface;
   IDirect3DTexture9* m_depth_tex;
#endif
};
