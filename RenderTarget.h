#pragma once

#include "typedefs3D.h"
class RenderDevice;

class RenderTarget final
{
public:
   RenderTarget(RenderDevice* rd, int width = -1, int height = -1); // Default output render target
   RenderTarget(RenderDevice* rd, const int width, const int height, const colorFormat format, bool with_depth, bool use_MSAA, int stereo, char* failureMessage);
   ~RenderTarget();

   void Activate(bool ignoreStereo);

   Sampler* GetColorSampler() { return m_color_sampler; }
   void UpdateDepthSampler();
   Sampler* GetDepthSampler() { return m_depth_sampler; }

   RenderTarget* Duplicate();
   void CopyTo(RenderTarget* dest);

   int GetWidth() { return m_width; }
   int GetHeight() { return m_height; }

#ifdef ENABLE_SDL
   GLuint GetCoreFrameBuffer() { return m_framebuffer; }
#else
   IDirect3DSurface9* GetCoreColorSurface() { return m_color_surface; }
#endif

private:
   int m_width;
   int m_height;
   colorFormat m_format;
   int m_stereo;
   RenderDevice* m_rd;
   Sampler* m_color_sampler;
   Sampler* m_depth_sampler;
   bool m_is_back_buffer;
   bool m_has_depth;
   bool m_use_mSAA;

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
