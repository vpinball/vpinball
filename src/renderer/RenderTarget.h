#pragma once

#include "typedefs3D.h"
class RenderDevice;
class RenderPass;

class RenderTarget final
{
public:
   RenderTarget(RenderDevice* const rd, const int width, const int height, const colorFormat format); // Default output render target
   RenderTarget(RenderDevice* const rd, const SurfaceType type, const string& name, const int width, const int height, const colorFormat format, bool with_depth, int nMSAASamples, const char* failureMessage, RenderTarget* sharedDepth = nullptr);
   ~RenderTarget();

   void Activate(const int layer = -1);
   static RenderTarget* GetCurrentRenderTarget();
   static int GetCurrentRenderLayer();

   Sampler* GetColorSampler() { return m_color_sampler; }
   void UpdateDepthSampler(bool insideBeginEnd);
   Sampler* GetDepthSampler() { return m_depth_sampler; }

   RenderTarget* Duplicate(const string& name, const bool shareDepthSurface = false);
   void CopyTo(RenderTarget* dest, const bool copyColor = true, const bool copyDepth = true, 
      const int x1 = -1, const int y1 = -1, const int w1 = -1, const int h1 = -1,
      const int x2 = -1, const int y2 = -1, const int w2 = -1, const int h2 = -1,
      const int srcLayer = -1, const int dstLayer = -1);

   void SetSize(const int w, const int h) { assert(m_is_back_buffer); m_width = w; m_height = h; }
   int GetWidth() const { return m_width; }
   int GetHeight() const { return m_height; }
   bool IsMSAA() const { return m_nMSAASamples > 1; }
   bool HasDepth() const { return m_has_depth; }
   colorFormat GetColorFormat() const { return m_format; }
   RenderDevice* GetRenderDevice() const { return m_rd; }

#ifdef ENABLE_SDL
   GLuint GetCoreFrameBuffer() const { return m_framebuffer; }
#else
   IDirect3DSurface9* GetCoreColorSurface() { return m_color_surface; }
#endif

   const string m_name;
   const SurfaceType m_type;
   const int m_nLayers;

   RenderPass* m_lastRenderPass = nullptr;

private:
   RenderDevice* const m_rd;
   const bool m_is_back_buffer;
   const colorFormat m_format;
   int m_width;
   int m_height;
   const int m_nMSAASamples;
   const bool m_has_depth;
   const bool m_shared_depth;
   Sampler* m_color_sampler;
   Sampler* m_depth_sampler;

   static RenderTarget* current_render_target;
   static int current_render_layer;

#ifdef ENABLE_SDL
   GLuint m_framebuffer;
   GLenum m_texTarget = 0;
   GLuint m_color_tex;
   GLuint m_depth_tex;
   GLuint m_framebuffer_layers[6];
#else
   bool m_use_alternate_depth;
   IDirect3DSurface9* m_color_surface;
   IDirect3DTexture9* m_color_tex;
   IDirect3DSurface9* m_depth_surface;
   IDirect3DTexture9* m_depth_tex;
#endif
};
