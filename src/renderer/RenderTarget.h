// license:GPLv3+

#pragma once

#include "typedefs3D.h"
class RenderDevice;
class RenderPass;

class RenderTarget final
{
public:
   RenderTarget(RenderDevice* const rd, const SurfaceType type, const int width, const int height, const colorFormat format); // Default output render target
   #if defined(ENABLE_BGFX)
   RenderTarget(RenderDevice* const rd, const SurfaceType type, bgfx::FrameBufferHandle fbh, bgfx::TextureHandle colorTex, bgfx::TextureHandle depthTex, const string& name, const int width, const int height, const colorFormat format);
   #endif
   RenderTarget(RenderDevice* const rd, const SurfaceType type, const string& name, const int width, const int height, const colorFormat format, bool with_depth, int nMSAASamples, const char* failureMessage, RenderTarget* sharedDepth = nullptr);
   ~RenderTarget();

   void Activate(const int layer = -1);
   static RenderTarget* GetCurrentRenderTarget();
   static int GetCurrentRenderLayer();

   Sampler* GetColorSampler() const { return m_color_sampler; }
   void UpdateDepthSampler(bool insideBeginEnd);
   Sampler* GetDepthSampler() const { return m_depth_sampler; }

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

#if defined(ENABLE_BGFX)
   bgfx::FrameBufferHandle GetCoreFrameBuffer() const { return m_framebuffer; }
   static void OnFrameFlushed() { current_render_target = nullptr; current_render_layer = 0; }
#elif defined(ENABLE_OPENGL)
   GLuint GetCoreFrameBuffer() const { return m_framebuffer; }
#elif defined(ENABLE_DX9)
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

#if defined(ENABLE_BGFX)
   bgfx::FrameBufferHandle m_framebuffer = BGFX_INVALID_HANDLE;
   bgfx::TextureHandle m_color_tex = BGFX_INVALID_HANDLE;
   bgfx::TextureHandle m_depth_tex = BGFX_INVALID_HANDLE;
   bgfx::FrameBufferHandle m_framebuffer_layers[6];
#elif defined(ENABLE_OPENGL)
   GLuint m_framebuffer = 0;
   GLenum m_texTarget = 0;
   GLuint m_color_tex = 0;
   GLuint m_depth_tex = 0;
   GLuint m_framebuffer_layers[6];
#elif defined(ENABLE_DX9)
   bool m_use_alternate_depth = false;
   IDirect3DSurface9* m_color_surface = nullptr;
   IDirect3DTexture9* m_color_tex = nullptr;
   IDirect3DSurface9* m_depth_surface = nullptr;
   IDirect3DTexture9* m_depth_tex = nullptr;
#endif
};
