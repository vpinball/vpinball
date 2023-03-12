#include "stdafx.h"
#include "RenderTarget.h"
#include "RenderDevice.h"

#ifdef ENABLE_SDL
#include "Shader.h"
#endif

#ifndef DISABLE_FORCE_NVIDIA_OPTIMUS
#include "inc/nvapi.h"
#endif

#ifdef ENABLE_SDL
int RenderTarget::m_current_stereo_mode = -1;
#endif

RenderTarget* RenderTarget::current_render_target = nullptr;
RenderTarget* RenderTarget::GetCurrentRenderTarget() { return current_render_target; }

RenderTarget::RenderTarget(RenderDevice* rd, int width, int height, colorFormat format)
   : m_name("BackBuffer"s)
{
   m_rd = rd;
   m_stereo = STEREO_OFF;
   m_is_back_buffer = true;
   m_format = format;
   m_shared_depth = false;
   m_width = width;
   m_height = height;
   m_has_depth = false;
   m_nMSAASamples = 1;
   m_color_sampler = nullptr;
   m_depth_sampler = nullptr;
#ifdef ENABLE_SDL
   m_framebuffer = 0;
   glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&m_framebuffer); // Not sure about this (taken from VPVR original implementation). Doesn't the back buffer alays bind to 0 on OpenGL ?
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

RenderTarget::RenderTarget(RenderDevice* rd, const string& name, const int width, const int height, const colorFormat format, bool with_depth, int nMSAASamples, StereoMode stereo, const char* failureMessage, RenderTarget* sharedDepth)
   : m_name(name)
{
   m_is_back_buffer = false;
   m_rd = rd;
   m_stereo = stereo;
   m_width = width;
   m_height = height;
   m_format = format;
   m_has_depth = with_depth;
   m_nMSAASamples = nMSAASamples;
   m_shared_depth = m_has_depth && (sharedDepth != nullptr);
   m_color_sampler = nullptr;
   m_depth_sampler = nullptr;
#ifdef ENABLE_SDL
   const GLuint col_type = ((format == RGBA32F) || (format == RGB32F)) ? GL_FLOAT : ((format == RGBA16F) || (format == RGB16F)) ? GL_HALF_FLOAT : GL_UNSIGNED_BYTE;
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

   glGenFramebuffers(1, &m_framebuffer);
   glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

   // Update bind cache
   auto tex_unit = m_rd->m_samplerBindings.back();
   if (tex_unit->sampler != nullptr)
      tex_unit->sampler->m_bindings.erase(tex_unit);
   tex_unit->sampler = nullptr;
   glActiveTexture(GL_TEXTURE0 + tex_unit->unit);

   if (nMSAASamples > 1)
   {
      glGenTextures(1, &m_color_tex);
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_color_tex);
      glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, nMSAASamples, format, width, height, GL_TRUE);
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_color_tex, 0);
      if (with_depth)
      {
         if (!m_shared_depth)
         {
            glGenRenderbuffers(1, &m_depth_tex);
            glBindRenderbuffer(GL_RENDERBUFFER, m_depth_tex);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, nMSAASamples, GL_DEPTH_COMPONENT, width, height);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
         }
         glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_tex);
      }
   }
   else
   {
      glGenTextures(1, &m_color_tex);
      glBindTexture(GL_TEXTURE_2D, m_color_tex);
      glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, col_format, col_type, nullptr);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_tex, 0);
      if (with_depth)
      {
         if (!m_shared_depth)
         {
            glGenTextures(1, &m_depth_tex);
            glBindTexture(GL_TEXTURE_2D, m_depth_tex);
#ifndef __OPENGLES__
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, col_type, 0);
#else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, 0); //!!
#endif
         }
         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_tex, 0);
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
      sprintf_s(msg, sizeof(msg), "glCheckFramebufferStatus returned 0x%0002X %s", glCheckFramebufferStatus(m_framebuffer), errorCode);
      ShowError(msg);
      exit(-1);
   }

   if (nMSAASamples == 1)
   {
      m_color_sampler = new Sampler(m_rd, m_color_tex, false, true);
      if (with_depth)
         m_depth_sampler = new Sampler(m_rd, m_depth_tex, false, true);
   }

   glClearDepthf(1.0f);
   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#else
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
   if (nMSAASamples > 1)
   {
      // MSAA is made through a rendering surface that must be resolved a texture to be sampled
      // In theory, we should check adapter support using CheckDeviceMultiSampleType before creating but nowadays most GPU do support what VPX request
      const D3DMULTISAMPLE_TYPE ms_type = (D3DMULTISAMPLE_TYPE) (D3DMULTISAMPLE_NONE + nMSAASamples);
      constexpr DWORD ms_quality = 0;
      CHECKD3D(m_rd->GetCoreDevice()->CreateRenderTarget(width, height, (D3DFORMAT)format, ms_type, ms_quality, FALSE, &m_color_surface, nullptr));
      if (with_depth && !m_shared_depth)
         CHECKD3D(m_rd->GetCoreDevice()->CreateDepthStencilSurface(width, height, D3DFMT_D16, ms_type, ms_quality, FALSE, &m_depth_surface, nullptr));
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
            CHECKD3D(m_rd->GetCoreDevice()->CreateDepthStencilSurface(width, height, D3DFMT_D16 /*D3DFMT_D24X8*/, D3DMULTISAMPLE_NONE, 0, FALSE, &m_depth_surface, nullptr));
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
#ifdef ENABLE_SDL
   if (m_nMSAASamples > 1)
   {
      glDeleteRenderbuffers(1, &m_color_tex);
      if (m_depth_tex && !m_shared_depth)
         glDeleteRenderbuffers(1, &m_depth_tex);
   }
   else
   {
      glDeleteTextures(1, &m_color_tex);
      if (m_depth_tex && !m_shared_depth)
         glDeleteTextures(1, &m_depth_tex);
   }
   glDeleteFramebuffers(1, &m_framebuffer);
#else
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

RenderTarget* RenderTarget::Duplicate(const string& name, const bool shareDepthSurface)
{
   assert(!m_is_back_buffer);
   return new RenderTarget(m_rd, name, m_width, m_height, m_format, m_has_depth, m_nMSAASamples, m_stereo, "Failed to duplicate render target", shareDepthSurface ? this : nullptr);
}

void RenderTarget::CopyTo(RenderTarget* dest, const bool copyColor, const bool copyDepth)
{
   int w1 = GetWidth(), h1 = GetHeight(), w2 = dest->GetWidth(), h2 = dest->GetHeight();
#ifdef ENABLE_SDL
   if (w1 == w2 && h1 == h2)
   {
      int bitmask = (copyColor ? GL_COLOR_BUFFER_BIT : 0) | (m_has_depth && dest->m_has_depth && copyDepth ? GL_DEPTH_BUFFER_BIT : 0);
#ifndef __OPENGLES__
      if (GLAD_GL_VERSION_4_5)
         glBlitNamedFramebuffer(GetCoreFrameBuffer(), dest->GetCoreFrameBuffer(), 0, 0, w1, h1, 0, 0, w2, h2, bitmask, GL_NEAREST);
      else
#endif
      {
         glBindFramebuffer(GL_READ_FRAMEBUFFER, GetCoreFrameBuffer());
         glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest->GetCoreFrameBuffer());
         glBlitFramebuffer(0, 0, w1, h1, 0, 0, w2, h2, bitmask, GL_NEAREST);
      }
   }
   else
   {
      if (copyColor)
      {
#ifndef __OPENGLES__
         if (GLAD_GL_VERSION_4_5)
            glBlitNamedFramebuffer(GetCoreFrameBuffer(), dest->GetCoreFrameBuffer(), 0, 0, w1, h1, 0, 0, w2, h2, GL_COLOR_BUFFER_BIT, GL_LINEAR);
         else
#endif
         {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, GetCoreFrameBuffer());
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest->GetCoreFrameBuffer());
            glBlitFramebuffer(0, 0, w1, h1, 0, 0, w2, h2, GL_COLOR_BUFFER_BIT, GL_LINEAR);
         }
      }
      if (m_has_depth && dest->m_has_depth && copyDepth)
      {
#ifndef __OPENGLES__
         if (GLAD_GL_VERSION_4_5)
            glBlitNamedFramebuffer(GetCoreFrameBuffer(), dest->GetCoreFrameBuffer(), 0, 0, w1, h1, 0, 0, w2, h2, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
         else
#endif
         {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, GetCoreFrameBuffer());
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest->GetCoreFrameBuffer());
            glBlitFramebuffer(0, 0, w1, h1, 0, 0, w2, h2, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
         }
      }
   }
#else
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

void RenderTarget::Activate(const bool ignoreStereo)
{
#ifdef ENABLE_SDL
   if (current_render_target == this && m_current_stereo_mode == (ignoreStereo ? STEREO_OFF : m_stereo))
      return;
   m_current_stereo_mode = ignoreStereo ? STEREO_OFF : m_stereo;
   static GLfloat viewPorts[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
   if (m_color_sampler)
      m_color_sampler->Unbind();
   if (m_depth_sampler)
      m_depth_sampler->Unbind();
   glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
   switch (ignoreStereo ? STEREO_OFF : m_stereo)
   {
   case STEREO_OFF:
      glViewport(0, 0, m_width, m_height);
      m_rd->lightShader->SetBool(SHADER_ignoreStereo, true); // For non-stereo lightbulb texture, can't use pre-processor for this
      break;
   case STEREO_TB: // Render left eye in the upper part, and right eye in the lower part
   case STEREO_INT:
   case STEREO_FLIPPED_INT:
      glViewport(0, 0, m_width, m_height / 2); // Set default viewport width/height values of all viewports before we define the array or we get undefined behaviour in shader (flickering viewports).
      viewPorts[2] = viewPorts[6] = (float)m_width;
      viewPorts[3] = viewPorts[7] = (float)(m_height * 0.5);
      viewPorts[4] = 0.0f;
      viewPorts[5] = (float)(m_height * 0.5);
      glViewportArrayv(0, 2, viewPorts);
      m_rd->lightShader->SetBool(SHADER_ignoreStereo, false);
      break;
   default: //For all other stereo mode, render left eye in the left part, and right eye in the right part
      glViewport(0, 0, m_width / 2, m_height); // Set default viewport width/height values of all viewports before we define the array or we get undefined behaviour in shader (flickering viewports).
      viewPorts[2] = viewPorts[6] = (float)(m_width * 0.5);
      viewPorts[3] = viewPorts[7] = (float)m_height;
      viewPorts[4] = (float)(m_width * 0.5);
      viewPorts[5] = 0.0f;
      glViewportArrayv(0, 2, viewPorts);
      m_rd->lightShader->SetBool(SHADER_ignoreStereo, false);
      break;
   }
#else
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
   current_render_target = this;
}

#ifdef _DEBUG
#ifdef ENABLE_SDL
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "inc/stb_image_write.h"
#endif

void RenderTarget::SaveToPng(string filename)
{
#ifdef ENABLE_SDL
   bool rt_is_stereo = m_current_stereo_mode != -1 && m_current_stereo_mode != STEREO_OFF;
   RenderTarget* rt = GetCurrentRenderTarget();
   Activate(true);
   glFlush();
   GLsizei nrChannels = 3;
   GLsizei stride = nrChannels * m_width;
   stride += (stride % 4) ? (4 - stride % 4) : 0;
   GLsizei bufferSize = stride * m_height;
   std::vector<char> buffer(bufferSize);
   glPixelStorei(GL_PACK_ALIGNMENT, 4);
   glReadPixels(0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
   stbi_flip_vertically_on_write(true);
   stbi_write_png(filename.c_str(), m_width, m_height, nrChannels, buffer.data(), stride);
   rt->Activate(!rt_is_stereo);
#else
   D3DXSaveSurfaceToFile(filename.c_str(), D3DXIFF_PNG, m_color_surface, nullptr, nullptr);
#endif
}
#endif
