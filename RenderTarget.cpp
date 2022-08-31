#include "stdafx.h"
#include "RenderTarget.h"
#include "RenderDevice.h"

#ifdef ENABLE_SDL
#include "Shader.h"
#endif

#ifndef DISABLE_FORCE_NVIDIA_OPTIMUS
#include "nvapi.h"
#endif

RenderTarget::RenderTarget(RenderDevice* rd, int width, int height)
{
   m_rd = rd;
   m_stereo = STEREO_OFF;
   m_is_back_buffer = true;
   // FIXME we should gather backbuffer informations
   // m_format = ;
   m_width = width;
   m_height = height;
   m_has_depth = false;
   m_use_mSAA = false;
   m_color_sampler = nullptr;
   m_depth_sampler = nullptr;
#ifdef ENABLE_SDL
   m_framebuffer = 0;
   glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)(&m_framebuffer)); // Not sure about this (taken from VPVR original implementation). Doesn't the back buffer alays bind to 0 on OpenGL ?
#else
   HRESULT hr = m_rd->GetCoreDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_color_surface);
   if (FAILED(hr))
      ReportError("Fatal Error: unable to create back buffer!", hr, __FILE__, __LINE__);
   m_use_alternate_depth = m_rd->m_useNvidiaApi || !m_rd->m_INTZ_support;
   m_color_tex = nullptr;
   m_color_sampler = nullptr;
   m_depth_tex = nullptr;
   m_depth_surface = nullptr;
   m_depth_sampler = nullptr;
#endif
}

RenderTarget::RenderTarget(RenderDevice* rd, const int width, const int height, const colorFormat format, bool with_depth, bool use_MSAA, int stereo, char* failureMessage)
{
   m_is_back_buffer = false;
   m_rd = rd;
   m_stereo = stereo;
   m_width = width;
   m_height = height;
   m_format = format;
   m_has_depth = with_depth;
   m_use_mSAA = use_MSAA;
#ifdef ENABLE_SDL
   const GLuint col_type = ((format == RGBA32F) || (format == RGB32F)) ? GL_FLOAT : ((format == RGBA16F) || (format == RGB16F)) ? GL_HALF_FLOAT : GL_UNSIGNED_BYTE;
   const GLuint col_format = ((format == GREY8) || (format == RED16F))                                                                                                      ? GL_RED
      : ((format == GREY_ALPHA) || (format == RG16F))                                                                                                                       ? GL_RG
      : ((format == RGB) || (format == RGB8) || (format == SRGB) || (format == SRGB8) || (format == RGB5) || (format == RGB10) || (format == RGB16F) || (format == RGB32F)) ? GL_RGB
                                                                                                                                                                              : GL_RGBA;
   const bool col_is_linear = (format == GREY8) || (format == RED16F) || (format == GREY_ALPHA) || (format == RG16F) || (format == RGB5) || (format == RGB) || (format == RGB8)
      || (format == RGB10) || (format == RGB16F) || (format == RGB32F) || (format == RGBA16F) || (format == RGBA32F) || (format == RGBA) || (format == RGBA8) || (format == RGBA10)
      || (format == DXT5) || (format == BC6U) || (format == BC6S) || (format == BC7);

   glGenFramebuffers(1, &m_framebuffer);
   glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
   glGenTextures(1, &m_color_tex);

   if (g_pplayer->m_MSAASamples > 1 && use_MSAA)
   {
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_color_tex);
      glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, g_pplayer->m_MSAASamples, format, width, height, GL_TRUE);
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_color_tex, 0);

      if (with_depth)
      {
         glGenRenderbuffers(1, &m_depth_tex);
         glBindRenderbuffer(GL_RENDERBUFFER, m_depth_tex);
         glRenderbufferStorageMultisample(GL_RENDERBUFFER, g_pplayer->m_MSAASamples, GL_DEPTH_COMPONENT, width, height);
         glBindRenderbuffer(GL_RENDERBUFFER, 0);
         glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_tex);
      }
      else
      {
         m_depth_tex = 0;
      }
   }
   else // RENDERTARGET & RENDERTARGET_DEPTH
   {
      glBindTexture(GL_TEXTURE_2D, m_color_tex);
      glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, col_type, nullptr);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_tex, 0);

      if (with_depth)
      {
         glGenRenderbuffers(1, &m_depth_tex);
         glBindRenderbuffer(GL_RENDERBUFFER, m_depth_tex);
         glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
         glBindRenderbuffer(GL_RENDERBUFFER, 0);
         glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_tex);
      }
      else
      {
         m_depth_tex = 0;
      }
   }

   constexpr GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
   glDrawBuffers(1, DrawBuffers);

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
      case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: errorCode = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"; break;
      case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: errorCode = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"; break;
      case GL_FRAMEBUFFER_UNSUPPORTED: errorCode = "GL_FRAMEBUFFER_UNSUPPORTED"; break;
      case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: errorCode = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"; break;
      case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: errorCode = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"; break;
      default: errorCode = "unknown"; break;
      }
      sprintf_s(msg, sizeof(msg), "glCheckFramebufferStatus returned 0x%0002X %s", glCheckFramebufferStatus(m_framebuffer), errorCode);
      ShowError(msg);
      exit(-1);
   }

   m_color_sampler = new Sampler(m_rd, m_color_tex, false, use_MSAA, true);
   m_depth_sampler = with_depth ? new Sampler(m_rd, m_depth_tex, false, use_MSAA, true) : nullptr;

#else

   HRESULT hr = m_rd->GetCoreDevice()->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, (D3DFORMAT)format, (D3DPOOL)memoryPool::DEFAULT, &m_color_tex, nullptr);
   if (FAILED(hr))
      ReportError(failureMessage, hr, __FILE__, __LINE__);
   m_color_tex->GetSurfaceLevel(0, &m_color_surface);
   m_color_sampler = new Sampler(m_rd, m_color_tex, false, true);
   m_use_alternate_depth = m_rd->m_useNvidiaApi || !m_rd->m_INTZ_support;
   if (with_depth)
   {
      CHECKD3D(m_rd->GetCoreDevice()->CreateTexture(width, height, 1, D3DUSAGE_DEPTHSTENCIL, (D3DFORMAT)MAKEFOURCC('I', 'N', 'T', 'Z'), (D3DPOOL)memoryPool::DEFAULT, &m_depth_tex, nullptr)); // D3DUSAGE_AUTOGENMIPMAP?
      if (m_use_alternate_depth)
      {
         // Alternate depth path. Depth surface and depth texture are separated, synced with a copy.
         D3DSURFACE_DESC desc;
         m_color_surface->GetDesc(&desc);
         const HRESULT hr = m_rd->GetCoreDevice()->CreateDepthStencilSurface(width, height, D3DFMT_D16 /*D3DFMT_D24X8*/, //!!
            desc.MultiSampleType, desc.MultiSampleQuality, FALSE, &m_depth_surface, nullptr);
         if (FAILED(hr))
            ReportError("Fatal Error: unable to create depth buffer!", hr, __FILE__, __LINE__);
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
   else
   {
      m_depth_tex = nullptr;
      m_depth_surface = nullptr;
      m_depth_sampler = nullptr;
   }
#endif
}

RenderTarget::~RenderTarget()
{
#ifdef ENABLE_SDL

#else
   // Texture share its refcount with surface, it must be decremented, but it won't be 0 until surface is also released
   SAFE_RELEASE_NO_RCC(m_color_tex);
   SAFE_RELEASE(m_color_surface);
   if (m_has_depth)
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
   delete m_color_sampler;
   delete m_depth_sampler;
}
   
void RenderTarget::UpdateDepthSampler()
{
#if !defined(DISABLE_FORCE_NVIDIA_OPTIMUS) && !defined(ENABLE_SDL)
   if (m_has_depth && m_use_alternate_depth && m_rd->NVAPIinit)
   {
      // From IDirect3DSurface9 to IDirect3DTexture9
      // CHECKNVAPI(NvAPI_D3D9_AliasSurfaceAsTexture(m_rd->GetCoreDevice(), m_depth_surface, &m_depth_tex, 0));
      CHECKNVAPI(NvAPI_D3D9_StretchRectEx(m_rd->GetCoreDevice(), m_depth_surface, nullptr, m_depth_tex, nullptr, D3DTEXF_NONE));
   }
#endif
}

RenderTarget* RenderTarget::Duplicate()
{
   assert(!m_is_back_buffer);
   return new RenderTarget(m_rd, m_width, m_height, m_format, m_has_depth, m_use_mSAA, m_stereo, "Failed to duplicate render target");
}

void RenderTarget::CopyTo(RenderTarget* dest)
{
#ifdef ENABLE_SDL
   // FIXME unimplementd (only used to copy render target for static pass)
#else
   CHECKD3D(m_rd->GetCoreDevice()->StretchRect(m_color_surface, nullptr, dest->m_color_surface, nullptr, D3DTEXF_NONE));
   if (m_has_depth && dest->m_has_depth)
      CHECKD3D(m_rd->GetCoreDevice()->StretchRect(m_depth_surface, nullptr, dest->m_depth_surface, nullptr, D3DTEXF_NONE));
#endif
}

void RenderTarget::Activate(bool ignoreStereo)
{
#ifdef ENABLE_SDL
   static GLfloat viewPorts[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
   static RenderTarget* currentFrameBuffer = nullptr;
   static int currentStereoMode = -1;
   if (currentFrameBuffer == this && currentStereoMode == (ignoreStereo || m_is_back_buffer ? STEREO_OFF : m_stereo))
      return;
   currentFrameBuffer = this;
   currentStereoMode = ignoreStereo || m_is_back_buffer ? STEREO_OFF : m_stereo;
   glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
   if (m_is_back_buffer)
   {
      glViewport(0, 0, m_width, m_height);
   }
   else
   {
      Shader::setTextureDirty(m_color_tex);
      if (m_has_depth)
         Shader::setTextureDirty(m_depth_tex);
      if (ignoreStereo)
      {
         glViewport(0, 0, m_width, m_height);
      }
      else
      {
         switch (m_stereo)
         {
         case STEREO_OFF:
            glViewport(0, 0, m_width, m_height);
            m_rd->lightShader->SetBool(SHADER_ignoreStereo, true); // For non-stereo lightbulb texture, can't use pre-processor for this
            break;
         case STEREO_TB:
         case STEREO_INT:
            glViewport(0, 0, m_width,
               m_height / 2); // Set default viewport width/height values of all viewports before we define the array or we get undefined behaviour in shader (flickering viewports).
            viewPorts[2] = viewPorts[6] = (float)m_width;
            viewPorts[3] = viewPorts[7] = (float)(m_height * 0.5);
            viewPorts[4] = 0.0f;
            viewPorts[5] = (float)(m_height * 0.5);
            glViewportArrayv(0, 2, viewPorts);
            m_rd->lightShader->SetBool(SHADER_ignoreStereo, false);
            break;
         case STEREO_SBS:
         case STEREO_VR:
            glViewport(0, 0, m_width / 2,
               m_height); // Set default viewport width/height values of all viewports before we define the array or we get undefined behaviour in shader (flickering viewports).
            viewPorts[2] = viewPorts[6] = (float)(m_width * 0.5);
            viewPorts[3] = viewPorts[7] = (float)m_height;
            viewPorts[4] = (float)(m_width * 0.5);
            viewPorts[5] = 0.0f;
            glViewportArrayv(0, 2, viewPorts);
            m_rd->lightShader->SetBool(SHADER_ignoreStereo, false);
            break;
         }
      }
   }
#else
   CHECKD3D(m_rd->GetCoreDevice()->SetRenderTarget(0, m_color_surface));
   if (m_depth_surface)
      CHECKD3D(m_rd->GetCoreDevice()->SetDepthStencilSurface(m_depth_surface));
#endif
}
