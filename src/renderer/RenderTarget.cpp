// license:GPLv3+

#include "core/stdafx.h"
#include "RenderTarget.h"
#include "RenderDevice.h"
#include "VRDevice.h"

#if defined(ENABLE_OPENGL)
#include "Shader.h"
#endif

#ifndef DISABLE_FORCE_NVIDIA_OPTIMUS
#include "nvapi/nvapi.h"
#endif

RenderTarget* RenderTarget::current_render_target = nullptr;
RenderTarget* RenderTarget::GetCurrentRenderTarget() { return current_render_target; }

int RenderTarget::current_render_layer = 0; // For layered render targets (stereo, cubemaps,...)
int RenderTarget::GetCurrentRenderLayer() { return current_render_layer; }

RenderTarget::RenderTarget(RenderDevice* const rd, const SurfaceType type, const int width, const int height, const colorFormat format)
   : m_name("BackBuffer"s)
   , m_type(type)
   , m_is_back_buffer(true)
   , m_nLayers(type == RT_DEFAULT ? 1 : type == RT_CUBEMAP ? 6 : 2)
   , m_rd(rd)
   , m_format(format)
   , m_width(width)
   , m_height(height)
   , m_nMSAASamples(1)
   , m_has_depth(false)
   , m_shared_depth(false)
{
   assert((type == RT_DEFAULT) || (type == RT_STEREO));
   m_color_sampler = nullptr;
   m_depth_sampler = nullptr;

   #if defined(ENABLE_BGFX)
   m_framebuffer = BGFX_INVALID_HANDLE; // Invalid handle is the reserved Id for BGFX's back buffer

   #elif defined(ENABLE_OPENGL)
   #ifdef __APPLE__
   glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&m_framebuffer); // Hack to run on Apple device where the #0 id does not seem to bind to the windowing backbuffer
   #else
   m_framebuffer = 0; // 0 is the reserved Id for the windowing back buffer (see https://registry.khronos.org/OpenGL-Refpages/es2.0/xhtml/glBindFramebuffer.xml)
   #endif
   m_color_tex = 0;
   m_depth_tex = 0;
   m_texTarget = GL_TEXTURE_2D;
   int rSize, gSize, bSize;
   SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &rSize);
   SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &gSize);
   SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &bSize);
   PLOGI << "Backbuffer channel size: " << rSize << ", " << gSize << ", " << bSize;

   #elif defined(ENABLE_DX9)
   HRESULT hr = m_rd->GetCoreDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_color_surface);
   if (FAILED(hr))
      ReportError("Fatal Error: unable to create back buffer!", hr, __FILE__, __LINE__);
   m_use_alternate_depth = m_rd->m_useNvidiaApi || !m_rd->m_INTZ_support;
   m_color_tex = nullptr;
   m_depth_tex = nullptr;
   m_depth_surface = nullptr;
   #endif
}

#if defined(ENABLE_BGFX)
RenderTarget::RenderTarget(RenderDevice* const rd, const SurfaceType type, bgfx::FrameBufferHandle fbh, bgfx::TextureHandle colorTex, bgfx::TextureHandle depthTex, 
   const string& name, const int width, const int height, const colorFormat format)
   : m_name(name)
   , m_type(type)
   , m_is_back_buffer(true)
   , m_nLayers(type == RT_DEFAULT ? 1 : type == RT_CUBEMAP ? 6 : 2)
   , m_rd(rd)
   , m_format(format)
   , m_width(width)
   , m_height(height)
   , m_nMSAASamples(1)
   , m_has_depth(bgfx::isValid(depthTex))
   , m_shared_depth(false)
   , m_color_sampler(nullptr)
   , m_depth_sampler(nullptr)
   , m_framebuffer(fbh)
   , m_color_tex(colorTex)
   , m_depth_tex(depthTex)
{
   if (bgfx::isValid(colorTex))
      m_color_sampler = new Sampler(rd, type, colorTex, width, height, false, true, SA_UNDEFINED, SA_UNDEFINED, SF_UNDEFINED);
   if (bgfx::isValid(depthTex))
      m_depth_sampler = new Sampler(rd, type, depthTex, width, height, false, true, SA_UNDEFINED, SA_UNDEFINED, SF_UNDEFINED);
}
#endif

RenderTarget::RenderTarget(RenderDevice* const rd, const SurfaceType type, const string& name, const int width, const int height, const colorFormat format, bool with_depth, int nMSAASamples, const char* failureMessage, RenderTarget* sharedDepth)
   : m_name(name)
   , m_type(type)
   , m_is_back_buffer(false)
   , m_nLayers(type == RT_DEFAULT ? 1 : type == RT_CUBEMAP ? 6 : 2)
   , m_rd(rd)
   , m_format(format)
   , m_width(width)
   , m_height(height)
   , m_nMSAASamples(nMSAASamples)
   , m_has_depth(with_depth)
   , m_shared_depth(with_depth && (sharedDepth != nullptr))
{
   assert(nMSAASamples >= 1);
   assert(type != RT_CUBEMAP || nMSAASamples == 1); // Cubemap render target do not support multisampling

   m_color_sampler = nullptr;
   m_depth_sampler = nullptr;

#if defined(ENABLE_BGFX)
   bgfx::TextureFormat::Enum fmt;
   uint64_t msaaFlags;
   if (nMSAASamples > 8)
      msaaFlags = BGFX_TEXTURE_RT_MSAA_X16;
   else if (nMSAASamples > 4)
      msaaFlags = BGFX_TEXTURE_RT_MSAA_X8;
   else if (nMSAASamples > 2)
      msaaFlags = BGFX_TEXTURE_RT_MSAA_X4;
   else if (nMSAASamples > 1)
      msaaFlags = BGFX_TEXTURE_RT_MSAA_X2;
   else
      msaaFlags = BGFX_TEXTURE_RT;
   // FIXME BGFX add support for MSAA (not that obvious: resolving by blitting is not supported by bgfx, depth attachment must be write only... see https://github.com/bkaradzic/bgfx/issues/2862)
   msaaFlags = BGFX_TEXTURE_RT;
   // FIXME most render target are not blit destination and are only used as write target (then GPU sampling, no readback) => BGFX_TEXTURE_READ_BACK
   const uint64_t colorFlags = BGFX_TEXTURE_BLIT_DST | msaaFlags;
   const uint64_t depthFlags = BGFX_TEXTURE_BLIT_DST | msaaFlags /* MSAA depth buffer must be write only | BGFX_TEXTURE_RT_WRITE_ONLY */;
   switch (format)
   {
   case colorFormat::RED16F: fmt = bgfx::TextureFormat::R16F; break;
   case colorFormat::RG16F: fmt = bgfx::TextureFormat::RG16F; break;
   case colorFormat::RGB16F: fmt = bgfx::TextureFormat::RGBA16F; break;
   case colorFormat::RGBA16F: fmt = bgfx::TextureFormat::RGBA16F; break;
   case colorFormat::RGB5: fmt = bgfx::TextureFormat::RGB5A1; break;
   case colorFormat::RGB8: fmt = bgfx::TextureFormat::RGB8; break;
   case colorFormat::RGB10: fmt = bgfx::TextureFormat::RGB10A2; break;
   case colorFormat::RGBA8: fmt = bgfx::TextureFormat::RGBA8; break;
   case colorFormat::RGBA10: fmt = bgfx::TextureFormat::RGB10A2; break;
   case colorFormat::GREY8: fmt = bgfx::TextureFormat::R8; break;
   default: assert(false); // Unsupported texture format 
   }
   m_color_tex = bgfx::createTexture2D(m_width, m_height, false, m_nLayers, fmt, colorFlags);
   m_color_sampler = new Sampler(m_rd, m_type, m_color_tex, m_width, m_height, false, true);
   m_color_sampler->SetName(name + ".Color");

   if (m_shared_depth)
   {
      m_depth_tex = sharedDepth->m_depth_tex;
      m_depth_sampler = sharedDepth->m_depth_sampler;
   }
   else if (with_depth)
   {
      bgfx::TextureFormat::Enum depthFormat = bgfx::TextureFormat::D24;
      #ifdef ENABLE_XR
      // For OpenXR, we need to be able to copy from the render depth buffer to the swapchain's depth buffer.
      // TODO we should use directly the vr's swapchain depth buffer instead of creating a compatible one to avoid the blit
      if (g_pplayer->m_vrDevice)
         depthFormat = g_pplayer->m_vrDevice->GetDepthFormat();
      #endif
      m_depth_tex = bgfx::createTexture2D(m_width, m_height, false, m_nLayers, depthFormat, depthFlags);
      m_depth_sampler = new Sampler(m_rd, m_type, m_depth_tex, m_width, m_height, false, true);
      m_depth_sampler->SetName(name + ".Depth");
   }

   if (with_depth)
   {
      bgfx::Attachment colorAttachment, depthAttachment;
      colorAttachment.init(m_color_tex, bgfx::Access::Write, 0, m_nLayers, 0, BGFX_RESOLVE_AUTO_GEN_MIPS);
      depthAttachment.init(m_depth_tex, bgfx::Access::Write, 0, m_nLayers, 0, BGFX_RESOLVE_AUTO_GEN_MIPS);
      const bgfx::Attachment attachments[] = { colorAttachment, depthAttachment };
      m_framebuffer = bgfx::createFrameBuffer(2, attachments);
   }
   else
   {
      bgfx::Attachment colorAttachment;
      colorAttachment.init(m_color_tex, bgfx::Access::Write, 0, m_nLayers, 0, BGFX_RESOLVE_AUTO_GEN_MIPS);
      m_framebuffer = bgfx::createFrameBuffer(1, &colorAttachment);
   }
   if (!bgfx::isValid(m_framebuffer))
   {
      PLOGE << failureMessage;
      PLOGE << "Failed to create render target";
      exit(-1);
   }
   bgfx::setName(m_framebuffer, name.c_str());

#elif defined(ENABLE_OPENGL)
   const GLuint col_type = ((format == RGBA32F) || (format == RGB32F)) ? GL_FLOAT : ((format == RGB16F) || (format == RGBA16F)) ? GL_HALF_FLOAT : GL_UNSIGNED_BYTE;
   const GLuint col_format = ((format == GREY8) || (format == RED16F))                                                                                                      ? GL_RED
      : ((format == GREY_ALPHA) || (format == RG16F))                                                                                                                       ? GL_RG
      : ((format == RGB) || (format == RGB8) || (format == SRGB) || (format == SRGB8) || (format == RGB5) || (format == RGB10) || (format == RGB16F) || (format == RGB32F)) ? GL_RGB
                                                                                                                                                                              : GL_RGBA;
   const bool col_is_linear = (format == GREY8) || (format == RED16F) || (format == GREY_ALPHA) || (format == RG16F) || (format == RGB5) || (format == RGB) || (format == RGB8)
      || (format == RGB10) || (format == RGB16F) || (format == RGB32F) || (format == RGBA16F) || (format == RGBA32F) || (format == RGBA) || (format == RGBA8) || (format == RGBA10)
      || (format == DXT5) || (format == BC6U) || (format == BC6S) || (format == BC7);

   m_color_tex = 0;
   m_depth_tex = 0;
   if (m_shared_depth)
   {
      m_depth_tex = sharedDepth->m_depth_tex;
      m_depth_sampler = sharedDepth->m_depth_sampler;
   }

#ifndef __OPENGLES__
   if (GLAD_GL_VERSION_4_3)
   {
      std::stringstream info;
      info << "Create FrameBuffer '" << m_name << "'";
      glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, info.str().c_str());
   }
#endif

   glGenFramebuffers(1, &m_framebuffer);
   glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

   // Update bind cache
   auto tex_unit = m_rd->m_samplerBindings.back();
   if (tex_unit->sampler != nullptr)
      tex_unit->sampler->m_bindings.erase(tex_unit);
   tex_unit->sampler = nullptr;
   glActiveTexture(GL_TEXTURE0 + tex_unit->unit);

#ifndef __OPENGLES__
   m_texTarget = nMSAASamples > 1 ? ((type == RT_DEFAULT) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D_MULTISAMPLE_ARRAY)
                                  : ((type == RT_DEFAULT) ? GL_TEXTURE_2D : type == RT_STEREO ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_CUBE_MAP);
#else
   m_texTarget = (type == RT_DEFAULT ? GL_TEXTURE_2D : type == RT_STEREO ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_CUBE_MAP);
#endif

   if (nMSAASamples > 1)
   {
#ifndef __OPENGLES__
      glGenTextures(1, &m_color_tex);
      glBindTexture(m_texTarget, m_color_tex);
      glTexParameteri(m_texTarget, GL_TEXTURE_BASE_LEVEL, 0);
      glTexParameteri(m_texTarget, GL_TEXTURE_MAX_LEVEL, 0);
      switch (m_type)
      {
      case RT_DEFAULT: glTexImage2DMultisample(m_texTarget, nMSAASamples, format, width, height, GL_FALSE); break;
      case RT_STEREO: glTexImage3DMultisample(m_texTarget, nMSAASamples, format, width, height, 2, GL_FALSE); break;
      case RT_CUBEMAP: assert(false); break;
      }
      glBindTexture(m_texTarget, 0);
#ifndef __OPENGLES__
      glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_color_tex, 0);
#else
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_tex, 0);
#endif
      if (with_depth)
      {
         if (!m_shared_depth)
         {
            glGenTextures(1, &m_depth_tex);
            glBindTexture(m_texTarget, m_depth_tex);
            glTexParameteri(m_texTarget, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(m_texTarget, GL_TEXTURE_MAX_LEVEL, 0);
            switch (m_type)
            {
            case RT_DEFAULT: glTexImage2DMultisample(m_texTarget, nMSAASamples, GL_DEPTH_COMPONENT, width, height, GL_FALSE); break;
            case RT_STEREO: glTexImage3DMultisample(m_texTarget, nMSAASamples, GL_DEPTH_COMPONENT, width, height, 2, GL_FALSE); break;
            case RT_CUBEMAP: assert(false); break;
            }
            glBindTexture(m_texTarget, 0);
         }
         glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depth_tex, 0);
      }
#endif
   }
   else
   {
      glGenTextures(1, &m_color_tex);
      glBindTexture(m_texTarget, m_color_tex);
      glTexParameteri(m_texTarget, GL_TEXTURE_BASE_LEVEL, 0);
      glTexParameteri(m_texTarget, GL_TEXTURE_MAX_LEVEL, 0);
      switch (m_type)
      {
      case RT_DEFAULT: glTexImage2D(m_texTarget, 0, format, width, height, 0, col_format, col_type, nullptr); break;
      case RT_STEREO: glTexImage3D(m_texTarget, 0, format, width, height, 2, 0, col_format, col_type, nullptr); break;
      case RT_CUBEMAP:
         for (int i = 0; i < 6; i++)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, col_format, col_type, nullptr);
         break;
      }
      glTexParameteri(m_texTarget, GL_TEXTURE_MAX_LEVEL, 0);
      glBindTexture(m_texTarget, 0);
#ifndef __OPENGLES__
      glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_color_tex, 0);
#else
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_tex, 0);
#endif
      if (with_depth)
      {
         if (!m_shared_depth)
         {
            #ifdef __STANDALONE__
            #ifndef __OPENGLES__
            GLint internalFormat = GL_FLOAT; // Needed for BloodMachine table on standalone (waiting for the table to be updated and remove its hacky turbo insert)
            #else
            GLint internalFormat = GL_UNSIGNED_SHORT;
            #endif
            #else
            GLint internalFormat = GL_UNSIGNED_SHORT;
            #endif
            glGenTextures(1, &m_depth_tex);
            glBindTexture(m_texTarget, m_depth_tex);
            glTexParameteri(m_texTarget, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(m_texTarget, GL_TEXTURE_MAX_LEVEL, 0);
            switch (m_type)
            {
#ifndef __OPENGLES__
            case RT_DEFAULT: glTexImage2D(m_texTarget, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, internalFormat, nullptr); break;
#else
            case RT_DEFAULT: glTexImage2D(m_texTarget, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, internalFormat, nullptr); break;
#endif
            case RT_STEREO: glTexImage3D(m_texTarget, 0, GL_DEPTH_COMPONENT, width, height, 2, 0, GL_DEPTH_COMPONENT, internalFormat, nullptr); break;
            case RT_CUBEMAP:
               for (int i = 0; i < 6; i++)
                  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, internalFormat, nullptr);
               break;
            }
            glBindTexture(m_texTarget, 0);
         }
#ifndef __OPENGLES__
         glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depth_tex, 0);
#else
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_tex, 0);
#endif
      }
   }
#ifndef __OPENGLES__
   if (GLAD_GL_VERSION_4_3)
      glObjectLabel(GL_FRAMEBUFFER, m_framebuffer, (GLsizei) name.length(), name.c_str());
#endif

   constexpr GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
   glDrawBuffers(1, DrawBuffers);

   // Create anciliary FBOs to be able to blit (especially resolve MSAA) from/to the other layers
   if (m_nLayers > 1)
   {
      glGenFramebuffers(m_nLayers, m_framebuffer_layers);
      for (int i = 0; i < m_nLayers; i++)
      {
         glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_layers[i]);
         glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_color_tex, 0, i);
         if (with_depth)
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depth_tex, 0, i);
         glDrawBuffers(1, DrawBuffers);
#ifndef __OPENGLES__
         if (GLAD_GL_VERSION_4_3)
         {
            string layer_name = name + ".Layer" + std::to_string(i);
            glObjectLabel(GL_FRAMEBUFFER, m_framebuffer_layers[i], (GLsizei)layer_name.length(), layer_name.c_str());
         }
#endif
      }
      glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
   }

   const int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
   if (status != GL_FRAMEBUFFER_COMPLETE)
   {
      char msg[256];
      const char* errorCode;
      switch (status)
      {
      case GL_FRAMEBUFFER_UNDEFINED: errorCode = "GL_FRAMEBUFFER_UNDEFINED"; break;
      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: errorCode = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"; break;
      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: errorCode = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
#ifndef __OPENGLES__
      case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: errorCode = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"; break;
      case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: errorCode = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"; break;
#endif
      case GL_FRAMEBUFFER_UNSUPPORTED: errorCode = "GL_FRAMEBUFFER_UNSUPPORTED"; break;
      case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: errorCode = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"; break;
#ifndef __OPENGLES__
      case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: errorCode = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"; break;
#endif
      default: errorCode = "unknown"; break;
      }
      sprintf_s(msg, sizeof(msg), "glCheckFramebufferStatus returned 0x%08X %s", glCheckFramebufferStatus(m_framebuffer), errorCode);
      ShowError(msg);

#ifndef __OPENGLES__
      PLOGE.printf("failed - message=%s (rd=%p, width=%d, height=%d, colorFormat=%d, with_depth=%d, nMSAASamples=%d)",
              failureMessage, rd, width, height, format, with_depth, nMSAASamples);
#endif

      exit(-1);
   }

   if (nMSAASamples == 1)
   {
      m_color_sampler = new Sampler(m_rd, m_type, m_color_tex, false, true);
      m_color_sampler->SetName(name + ".Color");
      if (with_depth)
      {
         if (m_shared_depth)
            m_depth_sampler = sharedDepth->GetDepthSampler();
         else
         {
            m_depth_sampler = new Sampler(m_rd, m_type, m_depth_tex, false, true);
            m_depth_sampler->SetName(name + ".Depth");
         }
      }
   }
#ifndef __OPENGLES__
   else if (GLAD_GL_VERSION_4_3)
   {
      glObjectLabel(GL_TEXTURE, m_color_tex, (GLsizei)(name + ".Color").length(), (name + ".Color").c_str());
      if (with_depth && !m_shared_depth)
         glObjectLabel(GL_TEXTURE, m_depth_tex, (GLsizei)(name + ".Depth").length(), (name + ".Depth").c_str());
   }
#endif

   glClearDepthf(1.0f);
   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifndef __OPENGLES__
   if (GLAD_GL_VERSION_4_3)
      glPopDebugGroup();
#endif

#elif defined(ENABLE_DX9)
   assert(m_type == RT_DEFAULT); // Layered rendering is not yet supported by the DX9 backend
   m_color_tex = nullptr;
   m_color_surface = nullptr;
   m_depth_tex = nullptr;
   m_depth_surface = nullptr;
   if (m_shared_depth)
   {
      m_depth_tex = sharedDepth->m_depth_tex;
      m_depth_surface = sharedDepth->m_depth_surface;
      m_depth_sampler = sharedDepth->m_depth_sampler;
   }
   m_use_alternate_depth = m_rd->m_useNvidiaApi || !m_rd->m_INTZ_support;
   constexpr D3DFORMAT depthFormat = D3DFMT_D16; // Some VR room needs more depth precision but since DX9 build is not meant for VR we keep it like this for the time being
   if (nMSAASamples > 1)
   {
      // MSAA is made through a rendering surface that must be resolved a texture to be sampled
      // In theory, we should check adapter support using CheckDeviceMultiSampleType before creating but nowadays most GPU do support what VPX request
      const D3DMULTISAMPLE_TYPE ms_type = (D3DMULTISAMPLE_TYPE) (D3DMULTISAMPLE_NONE + nMSAASamples);
      constexpr DWORD ms_quality = 0;
      CHECKD3D(m_rd->GetCoreDevice()->CreateRenderTarget(width, height, (D3DFORMAT)format, ms_type, ms_quality, FALSE, &m_color_surface, nullptr));
      if (with_depth && !m_shared_depth)
         CHECKD3D(m_rd->GetCoreDevice()->CreateDepthStencilSurface(width, height, depthFormat, ms_type, ms_quality, FALSE, &m_depth_surface, nullptr));
   }
   else
   {
      HRESULT hr = m_rd->GetCoreDevice()->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, (D3DFORMAT)format, (D3DPOOL)memoryPool::DEFAULT, &m_color_tex, nullptr);
      if (FAILED(hr))
         ReportError(failureMessage, hr, __FILE__, __LINE__);
      m_color_tex->GetSurfaceLevel(0, &m_color_surface);
      m_color_sampler = new Sampler(m_rd, m_color_tex, false, true);
      if (with_depth && !m_shared_depth)
      {
         CHECKD3D(m_rd->GetCoreDevice()->CreateTexture(width, height, 1, D3DUSAGE_DEPTHSTENCIL, (D3DFORMAT)MAKEFOURCC('I', 'N', 'T', 'Z'), (D3DPOOL)memoryPool::DEFAULT, &m_depth_tex, nullptr)); // D3DUSAGE_AUTOGENMIPMAP?
         if (m_use_alternate_depth)
         {
            // Alternate depth path. Depth surface and depth texture are separated, synced with a copy.
            CHECKD3D(m_rd->GetCoreDevice()->CreateDepthStencilSurface(width, height, depthFormat /*D3DFMT_D24X8*/, D3DMULTISAMPLE_NONE, 0, FALSE, &m_depth_surface, nullptr));
#ifndef DISABLE_FORCE_NVIDIA_OPTIMUS
            if (m_rd->NVAPIinit)
            {
               CHECKNVAPI(NvAPI_D3D9_RegisterResource(m_depth_surface));
               CHECKNVAPI(NvAPI_D3D9_RegisterResource(m_depth_tex));
            }
#endif
         }
         else
         {
            CHECKD3D(m_depth_tex->GetSurfaceLevel(0, &m_depth_surface));
         }
         m_depth_sampler = new Sampler(m_rd, m_depth_tex, false, true);
      }
   }
#endif
}

RenderTarget::~RenderTarget()
{
   delete m_color_sampler;
   if (!m_shared_depth)
      delete m_depth_sampler;

#if defined(ENABLE_BGFX)
   if (bgfx::isValid(m_framebuffer))
      bgfx::destroy(m_framebuffer);
   if (bgfx::isValid(m_color_tex))
      bgfx::destroy(m_color_tex);
   if (bgfx::isValid(m_depth_tex))
      bgfx::destroy(m_depth_tex);

#elif defined(ENABLE_OPENGL)
   glDeleteTextures(1, &m_color_tex);
   if (m_depth_tex && !m_shared_depth)
      glDeleteTextures(1, &m_depth_tex);
   glDeleteFramebuffers(1, &m_framebuffer);
   if (m_nLayers > 1)
      glDeleteFramebuffers(m_nLayers, m_framebuffer_layers);

#elif defined(ENABLE_DX9)
   // Texture share its refcount with surface, it must be decremented, but it won't be 0 until surface is also released
   SAFE_RELEASE_NO_RCC(m_color_tex);
   SAFE_RELEASE(m_color_surface);
   if (m_has_depth && !m_shared_depth)
   {
      if (m_use_alternate_depth)
      {
   #ifndef DISABLE_FORCE_NVIDIA_OPTIMUS
         if (m_rd->NVAPIinit)
         {
            CHECKNVAPI(NvAPI_D3D9_UnregisterResource(m_depth_surface));
            CHECKNVAPI(NvAPI_D3D9_UnregisterResource(m_depth_tex));
         }
   #endif
         SAFE_RELEASE(m_depth_tex);
         SAFE_RELEASE(m_depth_surface);
      }
      else
      {
         // Texture share its refcount with surface, it must be decremented, but it won't be 0 until surface is also released
         SAFE_RELEASE_NO_RCC(m_depth_tex);
         SAFE_RELEASE(m_depth_surface);
      }
   }
#endif
}

void RenderTarget::UpdateDepthSampler(bool insideBeginEnd)
{
#if !defined(DISABLE_FORCE_NVIDIA_OPTIMUS) && defined(ENABLE_DX9)
   if (m_has_depth && m_use_alternate_depth && m_rd->NVAPIinit)
   {
      // do not put inside BeginScene/EndScene Block
      if (insideBeginEnd)
         m_rd->GetCoreDevice()->EndScene();
      // From IDirect3DSurface9 to IDirect3DTexture9
      // CHECKNVAPI(NvAPI_D3D9_AliasSurfaceAsTexture(m_rd->GetCoreDevice(), m_depth_surface, &m_depth_tex, 0));
      CHECKNVAPI(NvAPI_D3D9_StretchRectEx(m_rd->GetCoreDevice(), m_depth_surface, nullptr, m_depth_tex, nullptr, D3DTEXF_NONE));
      if (insideBeginEnd)
         m_rd->GetCoreDevice()->BeginScene();
   }
#endif
}

RenderTarget* RenderTarget::Duplicate(const string& name, const bool shareDepthSurface)
{
   assert(!m_is_back_buffer);
   return new RenderTarget(m_rd, m_type, name, m_width, m_height, m_format, m_has_depth, m_nMSAASamples, "Failed to duplicate render target", shareDepthSurface ? this : nullptr);
}

void RenderTarget::CopyTo(RenderTarget* dest, const bool copyColor, const bool copyDepth,
   const int x1, const int y1, const int w1, const int h1,
   const int x2, const int y2, const int w2, const int h2, 
   const int srcLayer, const int dstLayer)
{
   int px1 = x1 == -1 ? 0 : x1, py1 = y1 == -1 ? 0 : y1, pz1 = srcLayer == -1 ? 0 : srcLayer;
   int pw1 = w1 == -1 ? GetWidth() : w1, ph1 = h1 == -1 ? GetHeight() : h1;
   int px2 = x2 == -1 ? 0 : x2, py2 = y2 == -1 ? 0 : y2, pz2 = dstLayer == -1 ? 0 : dstLayer;
   int pw2 = w2 == -1 ? dest->GetWidth() : w2, ph2 = h2 == -1 ? dest->GetHeight() : h2;
   int nLayers = srcLayer == -1 ? m_nLayers : 1;
   assert(srcLayer != -1 || dstLayer != -1 || m_nLayers == dest->m_nLayers); // Either we copy a single layer or the full set in which case they must match

#if defined(ENABLE_BGFX)
   if (w1 == w2 && h1 == h2)
   {
      // BGFX does not support blitting multiple layers at once on all target platform (supported on Vulkan, not supported on DX11, untested for the other backends)
      for (int z = 0; z < nLayers; z++)
      {
         if (copyColor)
            bgfx::blit(m_rd->m_activeViewId, dest->m_color_tex, 0, px2, py2, pz2 + z, m_color_tex, 0, px1, py1, pz1 + z, w1, h1, 1);
         if (m_has_depth && dest->m_has_depth && copyDepth)
            bgfx::blit(m_rd->m_activeViewId, dest->m_depth_tex, 0, px2, py2, pz2 + z, m_depth_tex, 0, px1, py1, pz1 + z, w1, h1, 1);
      }
   }
   else
   {
      assert((dstLayer == 0) || ((dstLayer == -1) && (dest->m_nLayers == 1)));
      assert(srcLayer >= 0);
      const float px1 = 2.f * (static_cast<float>(x2)      / static_cast<float>(dest->GetWidth())) - 1.f;
      const float py1 = 2.f * (static_cast<float>(y2)      / static_cast<float>(dest->GetHeight())) - 1.f;
      const float px2 = 2.f * (static_cast<float>(x2 + w2) / static_cast<float>(dest->GetWidth())) - 1.f;
      const float py2 = 2.f * (static_cast<float>(y2 + h2) / static_cast<float>(dest->GetHeight())) - 1.f;
      const float qx1 =        static_cast<float>(x1)      / static_cast<float>(m_width);
      const float qy1 = 1.f -  static_cast<float>(y1)      / static_cast<float>(m_height);
      const float qx2 =        static_cast<float>(x1 + w1) / static_cast<float>(m_width);
      const float qy2 = 1.f -  static_cast<float>(y1 + h1) / static_cast<float>(m_height);
      const Vertex3D_TexelOnly verts[4] = {
         { px1, py1, 0.0f, qx1, qy1 },
         { px2, py1, 0.0f, qx2, qy1 },
         { px1, py2, 0.0f, qx1, qy2 },
         { px2, py2, 0.0f, qx2, qy2 }
      };
      Shader* shader = m_rd->m_FBShader;
      shader->SetTechnique(SHADER_TECHNIQUE_fb_mirror);
      shader->SetVector(SHADER_w_h_height, 1.f, 1.f, 1.f, 1.f);
      shader->SetInt(SHADER_layer, srcLayer);
      shader->SetTexture(SHADER_tex_fb_unfiltered, GetColorSampler());
      shader->Begin();
      bgfx::TransientVertexBuffer tvb; // TODO only allocate one per frame instead of one per CopyTo
      bgfx::allocTransientVertexBuffer(&tvb, 4, *m_rd->m_pVertexTexelDeclaration);
      memcpy(tvb.data, verts, 4 * sizeof(Vertex3D_TexelOnly));
      bgfx::setVertexBuffer(0, &tvb);
      bgfx::setInstanceCount(1);
      bgfx::setState(m_rd->m_bgfxState | BGFX_STATE_PT_TRISTRIP);
      bgfx::submit(m_rd->m_activeViewId, shader->GetCore());
      shader->End();
   }
   
#elif defined(ENABLE_OPENGL)
   #ifndef __OPENGLES__
   if (GLAD_GL_VERSION_4_3 && m_texTarget == dest->m_texTarget && pw1 == pw2 && ph1 == ph2)
   {
      // No MSAA resolution => glCopyImageSubData is more efficient and supports all configurations (copying one layer to another, all of them,...) but not scaling
      if (copyColor)
         glCopyImageSubData(m_color_tex, m_texTarget, 0, px1, py1, pz1, dest->m_color_tex, dest->m_texTarget, 0, px2, py2, pz2, pw1, ph1, nLayers);
      if (copyDepth)
         glCopyImageSubData(m_depth_tex, m_texTarget, 0, px1, py1, pz1, dest->m_depth_tex, dest->m_texTarget, 0, px2, py2, pz2, pw1, ph1, nLayers);
   }
   else
   #endif
   {
      // OpenGl ES or MSAA resolve => We need glBlitFramebuffer. It will perform MSAA resolution but it will only copy the first layer
      // Therefore we need to use anciliary FBOs with only the wanted layer bound to them to perform all of the wanted blit
      int bitmask = (copyColor ? GL_COLOR_BUFFER_BIT : 0) | (m_has_depth && dest->m_has_depth && copyDepth ? GL_DEPTH_BUFFER_BIT : 0);
      for (int i = 0; i < nLayers; i++)
      {
         glBindFramebuffer(GL_READ_FRAMEBUFFER, m_nLayers == 1 ? m_framebuffer : m_framebuffer_layers[pz1 + i]);
         glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest->m_nLayers == 1 ? dest->m_framebuffer : dest->m_framebuffer_layers[pz2 + i]);
         glBlitFramebuffer(px1, py1, px1 + pw1, py1 + ph1, px2, py2, px2 + pw2, py2 + ph2, bitmask, pw1 == pw2 && ph1 == ph2 ? GL_NEAREST : GL_LINEAR);
      }
      glBindFramebuffer(GL_FRAMEBUFFER, current_render_target->m_framebuffer);
   }
   
#elif defined(ENABLE_DX9)
   assert(pw1 == pw2 && ph1 == ph2); // we do not support scaling (only used for VR preview)
   assert(srcLayer == -1); // Layered rendering is not supported for DirectX 9
   if (copyColor)
   {
      CHECKD3D(m_rd->GetCoreDevice()->StretchRect(m_color_surface, nullptr, dest->m_color_surface, nullptr, w1 == w2 && h1 == h2 ? D3DTEXF_NONE : D3DTEXF_LINEAR));
   }
   if (m_has_depth && dest->m_has_depth && copyDepth)
   {
      assert(m_nMSAASamples == 1 && dest->m_nMSAASamples == 1); // DX9 does not support resolving an MSAA depth surface
      CHECKD3D(m_rd->GetCoreDevice()->StretchRect(m_depth_surface, nullptr, dest->m_depth_surface, nullptr, D3DTEXF_NONE));
   }
#endif
}

void RenderTarget::Activate(const int layer)
{
   if (current_render_target == this && current_render_layer == layer)
      return;
   current_render_target = this;
   current_render_layer = layer;

   #if defined(ENABLE_BGFX)
   assert(layer == -1 || m_nLayers == 1);
   m_rd->NextView();
   #ifdef _DEBUG
   bgfx::setViewName(m_rd->m_activeViewId, m_name.c_str());
   #endif
   // Either bind all layers for instanced rendering or the only requested one for normal rendering (one pass per layer)
   bgfx::setViewFrameBuffer(m_rd->m_activeViewId, (layer == -1 || m_nLayers == 1) ? m_framebuffer : m_framebuffer_layers[layer]);
   bgfx::setViewRect(m_rd->m_activeViewId, 0, 0, m_width, m_height);

   #elif defined(ENABLE_OPENGL)
   if (m_color_sampler)
      m_color_sampler->Unbind();
   if (m_depth_sampler)
      m_depth_sampler->Unbind();
   // Either bind all layers for instanced rendering or the only requested one for normal rendering (one pass per layer)
   glBindFramebuffer(GL_FRAMEBUFFER, (layer == -1 || m_nLayers == 1)  ? m_framebuffer : m_framebuffer_layers[layer]);
   glViewport(0, 0, m_width, m_height);

   #elif defined(ENABLE_DX9)
   assert(layer == -1 || m_nLayers == 1);
   static IDirect3DSurface9* currentColorSurface = nullptr;
   if (currentColorSurface != m_color_surface)
   {
      currentColorSurface = m_color_surface;
      CHECKD3D(m_rd->GetCoreDevice()->SetRenderTarget(0, m_color_surface));
   }
   static IDirect3DSurface9* currentDepthSurface = nullptr;
   if (currentDepthSurface != m_depth_surface)
   {
      currentDepthSurface = m_depth_surface;
      CHECKD3D(m_rd->GetCoreDevice()->SetDepthStencilSurface(m_depth_surface));
   }
   #endif
}
