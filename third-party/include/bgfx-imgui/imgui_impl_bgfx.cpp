// Derived from this Gist by Richard Gale:
//     https://gist.github.com/RichardGale/6e2b74bc42b3005e08397236e4be0fd0

// WARNING WARNING heavily modified for VPX

// ImGui BFFX binding
// In this binding, ImTextureID is used to store an OpenGL 'GLuint' texture
// identifier. Read the FAQ about ImTextureID in imgui.cpp.

// You can copy and use unmodified imgui_impl_* files in your project. See
// main.cpp for an example of using this. If you use this binding you'll need to
// call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(),
// ImGui::Render() and ImGui_ImplXXXX_Shutdown(). If you are new to ImGui, see
// examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

#include "imgui_impl_bgfx.h"
#include "imgui/imgui.h"

// BGFX/BX
#include "bgfx/bgfx.h"
#include "bgfx/embedded_shader.h"
#include "bx/math.h"
//#include "bx/timer.h"

#include "renderer/Sampler.h"

// Data
static Sampler* g_FontTexture = nullptr;
static bgfx::ProgramHandle g_ShaderHandle = BGFX_INVALID_HANDLE;
static bgfx::UniformHandle g_AttribLocationTex = BGFX_INVALID_HANDLE;
static bgfx::UniformHandle g_AttribLocationCol = BGFX_INVALID_HANDLE;
static bgfx::UniformHandle g_AttribLocationOfs = BGFX_INVALID_HANDLE;
static bgfx::VertexLayout g_VertexLayout;

static float g_SDRColor[4] = { 1.f, 1.f, 1.f, 1.f };
static float g_stereoOfs[4] = { 0.f, 0.f, 0.f, 0.f };
static int g_rotate = 0;

void ImGui_Implbgfx_SetSDRColor(float* col) { memcpy(g_SDRColor, col, 4 * sizeof(float)); }
void ImGui_Implbgfx_SetStereoOfs(float ofs) { g_stereoOfs[0] = ofs; }
void ImGui_Implbgfx_SetRotation(int rotate) { g_rotate = rotate; }

// This is the main rendering function that you have to implement and call after
// ImGui::Render(). Pass ImGui::GetDrawData() to this function.
// Note: If text or lines are blurry when integrating ImGui into your engine,
// in your Render function, try translating your projection matrix by
// (0.5f,0.5f) or (0.375f,0.375f)
void ImGui_Implbgfx_RenderDrawLists(int view, int instanceCount, ImDrawData* draw_data)
{
   // Avoid rendering when minimized, scale coordinates for retina displays
   // (screen coordinates != framebuffer coordinates)
   ImGuiIO& io = ImGui::GetIO();
   int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
   int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
   if (fb_width == 0 || fb_height == 0) {
      return;
   }

   draw_data->ScaleClipRects(io.DisplayFramebufferScale);

   // Setup render state: alpha-blending enabled, no face culling,
   // no depth testing, scissor enabled
   /* uint64_t state =
        BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_MSAA |
        BGFX_STATE_BLEND_FUNC(
            BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);*/

   const bgfx::Caps* caps = bgfx::getCaps();

   // Setup viewport, orthographic projection matrix
   float ortho[16];
   bx::mtxOrtho(ortho, 
      0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, 
      0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);
   bgfx::setViewTransform(view, NULL, ortho);
   if (g_rotate == 0 || g_rotate == 2)
      bgfx::setViewRect(view, 0, 0, (uint16_t)fb_width, (uint16_t)fb_height);
   else
      bgfx::setViewRect(view, 0, 0, (uint16_t)fb_height, (uint16_t)fb_width);

   // Render command lists
   for (int n = 0; n < draw_data->CmdListsCount; n++) {
      const ImDrawList* cmd_list = draw_data->CmdLists[n];

      bgfx::TransientVertexBuffer tvb;
      bgfx::TransientIndexBuffer tib;

      uint32_t numVertices = (uint32_t)cmd_list->VtxBuffer.size();
      uint32_t numIndices = (uint32_t)cmd_list->IdxBuffer.size();

      if ((numVertices != 0) && (numIndices != 0))
      {
         if ((numVertices != bgfx::getAvailTransientVertexBuffer(numVertices, g_VertexLayout)) || (numIndices != bgfx::getAvailTransientIndexBuffer(numIndices)))
         {
            // not enough space in transient buffer, quit drawing the rest...
            break;
         }

         bgfx::allocTransientVertexBuffer(&tvb, numVertices, g_VertexLayout);
         bgfx::allocTransientIndexBuffer(&tib, numIndices);

         ImDrawVert* verts = (ImDrawVert*)tvb.data;
         memcpy(verts, cmd_list->VtxBuffer.begin(), numVertices * sizeof(ImDrawVert));

         ImDrawIdx* indices = (ImDrawIdx*)tib.data;
         memcpy(indices, cmd_list->IdxBuffer.begin(), numIndices * sizeof(ImDrawIdx));
      }

      for (const ImDrawCmd *cmd = cmd_list->CmdBuffer.begin(), *cmdEnd = cmd_list->CmdBuffer.end(); cmd != cmdEnd; ++cmd)
      {
         if (cmd->UserCallback)
         {
            cmd->UserCallback(cmd_list, cmd);
         }
         else if (0 != cmd->ElemCount)
         {
            uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_MSAA | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);

            // TODO implement scissor for stereo rendering (not really used) and for rotated view (not really used either)
            if (g_stereoOfs[0] == 0.f && g_rotate == 0)
            {
               const uint16_t xx = (uint16_t)bx::max(cmd->ClipRect.x, 0.0f);
               const uint16_t yy = (uint16_t)bx::max(cmd->ClipRect.y, 0.0f);
               bgfx::setScissor(xx, yy,
                  (uint16_t)bx::min(cmd->ClipRect.z, 65535.0f) - xx,
                  (uint16_t)bx::min(cmd->ClipRect.w, 65535.0f) - yy);
            }

            Sampler* sampler = (Sampler*)cmd->TextureId;
            if (sampler)
               bgfx::setTexture(0, g_AttribLocationTex, sampler->GetCoreTexture(true));
            bgfx::setState(state);
            bgfx::setUniform(g_AttribLocationCol, g_SDRColor);
            bgfx::setUniform(g_AttribLocationOfs, g_stereoOfs);
            bgfx::setVertexBuffer(0, &tvb, 0, numVertices);
            bgfx::setIndexBuffer(&tib, cmd->IdxOffset, cmd->ElemCount);
            bgfx::setInstanceCount(instanceCount);
            bgfx::submit(view, g_ShaderHandle);
         }
      }
   }
}

#include "shaders/bgfx_imgui.h"

static const bgfx::EmbeddedShader s_embeddedShaders[] = {
   BGFX_EMBEDDED_SHADER(vs_imgui), BGFX_EMBEDDED_SHADER(vs_imgui_st),
   BGFX_EMBEDDED_SHADER(fs_imgui), BGFX_EMBEDDED_SHADER(fs_imgui_st),
   BGFX_EMBEDDED_SHADER_END()};

bool ImGui_Implbgfx_CreateDeviceObjects()
{
   // Uniforms must be created before program to link them properly on OpenGL backend
   g_AttribLocationTex = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);
   g_AttribLocationCol = bgfx::createUniform("u_sdrScale", bgfx::UniformType::Vec4);
   g_AttribLocationOfs = bgfx::createUniform("u_stereoOfs", bgfx::UniformType::Vec4);

   bgfx::RendererType::Enum type = bgfx::getRendererType();
   // OpenGL ES does not define ARB_shader_viewport_layer_array which is needed for layered rendering.
   // TODO this is hacky and should not be needed since BGFX_CAPS_VIEWPORT_LAYER_ARRAY should be false on OpenGL ES but tests shows it isn't
   if ((bgfx::getCaps()->supported & (BGFX_CAPS_INSTANCING | BGFX_CAPS_TEXTURE_2D_ARRAY | BGFX_CAPS_VIEWPORT_LAYER_ARRAY)) && (bgfx::getRendererType() != bgfx::RendererType::OpenGLES) && (bgfx::getRendererType() != bgfx::RendererType::Metal))
      g_ShaderHandle = bgfx::createProgram(bgfx::createEmbeddedShader(s_embeddedShaders, type, "vs_imgui_st"), bgfx::createEmbeddedShader(s_embeddedShaders, type, "fs_imgui_st"), true);
   else
      g_ShaderHandle = bgfx::createProgram(bgfx::createEmbeddedShader(s_embeddedShaders, type, "vs_imgui"), bgfx::createEmbeddedShader(s_embeddedShaders, type, "fs_imgui"), true);

   g_VertexLayout.begin()
      .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
      .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
      .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
      .end();

   // Build Font texture atlas
   ImGuiIO& io = ImGui::GetIO();
   unsigned char* pixels;
   int width, height;
   io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
   bgfx::TextureHandle fontTex = bgfx::createTexture2D((uint16_t)width, (uint16_t)height, false, 1, bgfx::TextureFormat::BGRA8, BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP, bgfx::copy(pixels, (size_t)width * height * 4));
   g_FontTexture = new Sampler(nullptr, SurfaceType::RT_DEFAULT, fontTex, width, height, true, false);
   io.Fonts->TexID = (ImTextureID)g_FontTexture;

   return true;
}

void ImGui_Implbgfx_InvalidateDeviceObjects()
{
   if (isValid(g_AttribLocationTex))
      bgfx::destroy(g_AttribLocationTex);

   if (isValid(g_AttribLocationCol))
      bgfx::destroy(g_AttribLocationCol);

   if (isValid(g_AttribLocationOfs))
      bgfx::destroy(g_AttribLocationOfs);

   if (isValid(g_ShaderHandle))
      bgfx::destroy(g_ShaderHandle);

   delete g_FontTexture;
   g_FontTexture = nullptr;
   ImGui::GetIO().Fonts->TexID = 0;
}

void ImGui_Implbgfx_Init()
{
}

void ImGui_Implbgfx_Shutdown()
{
   ImGui_Implbgfx_InvalidateDeviceObjects();
}

void ImGui_Implbgfx_NewFrame()
{
   if (g_FontTexture == nullptr)
      ImGui_Implbgfx_CreateDeviceObjects();
}
