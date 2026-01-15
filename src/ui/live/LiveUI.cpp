// license:GPLv3+

#include "core/stdafx.h"

#include "LiveUI.h"

#include "renderer/VRDevice.h"

#include "fonts/DroidSans.h"
#include "fonts/DroidSansBold.h"
#include "fonts/IconsForkAwesome.h"
#include "fonts/ForkAwesome.h"

#include "plugins/VPXPlugin.h"
#include "core/VPXPluginAPIImpl.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_stdlib.h"
#include "imgui_markdown/imgui_markdown.h"

#ifndef __STANDALONE__
#include "BAM/BAMView.h"
#endif


ImGui::MarkdownConfig LiveUI::markdown_config;

// Implementation of supporting function for logging of all math objects
namespace plog
{
Record &operator<<(Record &record, const ImVec2 &pt) { return record << '(' << pt.x << ", " << pt.y << ')'; }
}

LiveUI::LiveUI(RenderDevice *const rd)
   : m_inGameUI(*this) 
   , m_editorUI(*this)
   , m_ballControl(*this)
   , m_rd(rd)
   , m_perfUI(g_pplayer)
{
   m_app = g_pvp;
   m_player = g_pplayer;
   m_pininput = &(m_player->m_pininput);
   m_renderer = m_player->m_renderer;
   
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO &io = ImGui::GetIO();
   io.IniFilename = nullptr; //don't use an ini file for configuration

   // using the specialized initializer is not needed
   // ImGui_ImplSDL3_InitForOpenGL(m_player->m_playfieldSdlWnd, rd->m_sdl_context);
   ImGui_ImplSDL3_InitForOther(m_player->m_playfieldWnd->GetCore());
   //int displayIndex = SDL_GetDisplayForWindow(m_player->m_playfieldWnd->GetCore());

   SetupImGuiStyle(false);

   UpdateScale();

   // UI fonts
   m_baseFont = io.Fonts->AddFontFromMemoryCompressedTTF(droidsans_compressed_data, droidsans_compressed_size, 13.0f * m_uiScale);
   ImFontConfig icons_config;
   icons_config.MergeMode = true;
   icons_config.PixelSnapH = true;
   icons_config.GlyphMinAdvanceX = 13.0f * m_uiScale;
   static constexpr ImWchar icons_ranges[] = { ICON_MIN_FK, ICON_MAX_16_FK, 0 };
   io.Fonts->AddFontFromMemoryCompressedTTF(fork_awesome_compressed_data, fork_awesome_compressed_size, 13.0f * m_uiScale, &icons_config, icons_ranges);

   const float overlaySize = 13.0f * m_uiScale;
   m_overlayFont = io.Fonts->AddFontFromMemoryCompressedTTF(droidsans_compressed_data, droidsans_compressed_size, overlaySize);
   m_overlayBoldFont = io.Fonts->AddFontFromMemoryCompressedTTF(droidsansbold_compressed_data, droidsansbold_compressed_size, overlaySize);
   ImFont *H1 = io.Fonts->AddFontFromMemoryCompressedTTF(droidsansbold_compressed_data, droidsansbold_compressed_size, overlaySize * 20.0f / 13.f);
   ImFont *H2 = io.Fonts->AddFontFromMemoryCompressedTTF(droidsansbold_compressed_data, droidsansbold_compressed_size, overlaySize * 18.0f / 13.f);
   ImFont *H3 = io.Fonts->AddFontFromMemoryCompressedTTF(droidsansbold_compressed_data, droidsansbold_compressed_size, overlaySize * 15.0f / 13.f);
   markdown_config.linkCallback = MarkdownLinkCallback;
   markdown_config.tooltipCallback = nullptr;
   markdown_config.imageCallback = MarkdownImageCallback;
   //markdown_config.linkIcon = ICON_FA_LINK;
   markdown_config.headingFormats[0] = { H1, true };
   markdown_config.headingFormats[1] = { H2, true };
   markdown_config.headingFormats[2] = { H3, false };
   markdown_config.userData = this;
   markdown_config.formatCallback = MarkdownFormatCallback;

   io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;

   NewFrame();

   m_showTouchOverlay = g_pvp->m_settings.GetPlayer_TouchOverlay();
}

LiveUI::~LiveUI()
{
   HideUI();
   if (ImGui::GetCurrentContext())
   {
      ImGui::EndFrame();

      ImGuiIO &io = ImGui::GetIO();
      io.BackendFlags &= ~ImGuiBackendFlags_RendererHasTextures;
      for (auto tex : ImGui::GetPlatformIO().Textures)
      {
         tex->SetTexID(ImTextureID_Invalid);
         tex->SetStatus(ImTextureStatus_Destroyed);
      }

      ImGui_ImplSDL3_Shutdown();

      ImGui::DestroyContext();
   }
}

void LiveUI::MarkdownFormatCallback(const ImGui::MarkdownFormatInfo &markdownFormatInfo, bool start)
{
   const LiveUI *const ui = static_cast<LiveUI*>(markdownFormatInfo.config->userData);
   switch (markdownFormatInfo.type)
   {
   case ImGui::MarkdownFormatType::EMPHASIS:
      ImGui::defaultMarkdownFormatCallback(markdownFormatInfo, start);
      if (markdownFormatInfo.level == 1)
      { // normal emphasis
         if (start)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
         else
            ImGui::PopStyleColor();
      }
      else
      { // strong emphasis
         if (start)
            ImGui::PushFont(ui->m_overlayBoldFont, ui->m_overlayBoldFont->LegacySize);
         else
            ImGui::PopFont();
      }
      break;
   case ImGui::MarkdownFormatType::HEADING:
   {
      ImGui::MarkdownHeadingFormat fmt;
      if (markdownFormatInfo.level > ImGui::MarkdownConfig::NUMHEADINGS)
         fmt = markdownFormatInfo.config->headingFormats[ImGui::MarkdownConfig::NUMHEADINGS - 1];
      else
         fmt = markdownFormatInfo.config->headingFormats[markdownFormatInfo.level - 1];
      if (start)
      {
         if (fmt.font)
            ImGui::PushFont(fmt.font, fmt.font->LegacySize);
         if (ImGui::GetItemID() != ui->markdown_start_id)
            ImGui::NewLine();
      }
      else
      {
         if (fmt.separator)
         {
            ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(1.f, 1.f, 1.f, 1.f));
            ImGui::Separator();
            ImGui::PopStyleColor();
            ImGui::NewLine();
         }
         else
         {
            ImGui::NewLine();
         }
         if (fmt.font)
            ImGui::PopFont();
      }
      break;
   }
   default: ImGui::defaultMarkdownFormatCallback(markdownFormatInfo, start); break;
   }
}

void LiveUI::MarkdownLinkCallback(ImGui::MarkdownLinkCallbackData data)
{
   if (!data.isImage)
   {
      std::string url(data.link, data.linkLength);
      SDL_OpenURL(url.c_str());
   }
}

ImGui::MarkdownImageData LiveUI::MarkdownImageCallback(ImGui::MarkdownLinkCallbackData data)
{
   LiveUI *const ui = (LiveUI *)data.userData;
   Texture *const ppi = ui->m_player->m_ptable->GetImage(std::string(data.link, data.linkLength));
   if (ppi == nullptr)
      return ImGui::MarkdownImageData {};
   std::shared_ptr<Sampler> sampler = ui->m_renderer->m_renderDevice->m_texMan.LoadTexture(ppi, false);
   if (sampler == nullptr)
      return ImGui::MarkdownImageData {};

   // Fixed image scaling corresponding to InGameUI overlay size for an image of 1024px width
   const ImGuiIO &io = ImGui::GetIO();
   const float imgScale = (io.DisplaySize.x > io.DisplaySize.y ? 0.35f : 0.75f) * io.DisplaySize.x / 1024.f;
   ImGui::MarkdownImageData imageData { true, false, sampler, ImVec2(static_cast<float>(sampler->GetWidth()), static_cast<float>(sampler->GetHeight())) * imgScale };
   ImVec2 const contentSize = ImGui::GetContentRegionAvail();
   if (imageData.size.x > contentSize.x)
   {
      float const ratio = imageData.size.y / imageData.size.x;
      imageData.size.x = contentSize.x;
      imageData.size.y = contentSize.x * ratio;
   }
   return imageData;
}

void LiveUI::UpdateScale()
{
   const float prevDPI = m_uiScale;
   float overlayScale;
   if (m_player->m_vrDevice)
   {
      // VR headset cover full view range, so use a relative part of the full range for the DPI
      m_uiScale = static_cast<float>(min(m_player->m_vrDevice->GetEyeWidth(), m_player->m_vrDevice->GetEyeHeight())) / 2000.f;
      overlayScale = m_uiScale;
   }
   else
   {
      // Use display DPI setting
      // On macOS/iOS, keep m_uiScale at 1.0f. ImGui_ImplSDL3_NewFrame applies a 2.0f DisplayFramebufferScale
      // for SDL_WINDOW_HIGH_PIXEL_DENSITY windows. A m_uiScale of 2.0 would cause the UI to scale at 400%.
      // See: https://wiki.libsdl.org/SDL3/README/highdpi
      m_uiScale = SDL_GetWindowDisplayScale(m_player->m_playfieldWnd->GetCore()) / SDL_GetWindowPixelDensity(m_player->m_playfieldWnd->GetCore());
      overlayScale = m_uiScale;

      // For cabinet mode, the user is not standing in front of screen, so scale out the UI based on display size to be more readable (more "game like")
      if (m_player->m_ptable->GetViewMode() == ViewSetupID::BG_FULLSCREEN)
      {
         m_uiScale = max(m_uiScale, static_cast<float>(m_player->m_playfieldWnd->GetWidth()) / 750.f);
      }
   }
   m_uiScale = min(m_uiScale, 10.f); // To avoid texture size overflows
#ifdef __LIBVPINBALL__
   m_uiScale *= 1.35f;
#endif
   if (m_uiScale != prevDPI)
   {
      m_perfUI.SetUIScale(overlayScale);
      m_plumbOverlay.SetUIScale(overlayScale);
      if (prevDPI == 0.f)
         ImGui::GetStyle().ScaleAllSizes(m_uiScale);
      else
         ImGui::GetStyle().ScaleAllSizes(m_uiScale / prevDPI);
   }
}

void LiveUI::AddMousePosEvent(bool isTouch, float x, float y) const
{
   ImGuiIO &io = ImGui::GetIO();
   io.AddMouseSourceEvent(isTouch ? ImGuiMouseSource_TouchScreen : ImGuiMouseSource_Mouse);
   switch (m_rotate)
   {
   case 0: io.AddMousePosEvent(x, y); break;
   case 1: io.AddMousePosEvent(y, io.DisplaySize.y - x); break;
   case 2: io.AddMousePosEvent(x, io.DisplaySize.y - y); break;
   case 3: io.AddMousePosEvent(io.DisplaySize.x - y, x); break;
   default: assert(false); return;
   }
}

void LiveUI::HandleSDLEvent(SDL_Event &e) const
{
   if (e.type == SDL_EVENT_MOUSE_MOTION)
   {
      // Custom implementation of ImGui_ImplSDL3_ProcessEvent supporting screen rotation and event filtering
      AddMousePosEvent(e.motion.which == SDL_TOUCH_MOUSEID, e.motion.x, e.motion.y);
   }
   else
   {
      ImGui_ImplSDL3_ProcessEvent(&e);
   }
}

void LiveUI::NewFrame()
{
   UpdateScale();
   ImGui_ImplSDL3_NewFrame();

   ImGuiIO &io = ImGui::GetIO();
   switch (m_player->m_renderer->m_stereo3Denabled ? m_player->m_renderer->m_stereo3D : STEREO_OFF)
   {
   // Render is a vertically squashed view which is stretched back by the display
   case STEREO_TB:
   case STEREO_INT:
   case STEREO_FLIPPED_INT:
      io.DisplayFramebufferScale.y *= 0.5f;
      break;

   // Render is a horizontally squashed view which is stretched back by the display
   case STEREO_SBS:
      io.DisplayFramebufferScale.x *= 0.5f;
      break;

   default:
      break;
   }
   const int width = m_rd->GetCurrentPass() ? m_rd->GetCurrentPass()->m_rt->GetWidth() : 1920;
   const int height = m_rd->GetCurrentPass() ? m_rd->GetCurrentPass()->m_rt->GetHeight() : 1080;
   io.DisplaySize.x = static_cast<float>(width) / io.DisplayFramebufferScale.x;
   io.DisplaySize.y = static_cast<float>(height) / io.DisplayFramebufferScale.y;
   m_rotate = m_renderer->m_stereo3D == STEREO_VR
      ? 0 : ((int)(m_player->m_ptable->GetViewSetup().GetRotation((int)io.DisplaySize.x, (int)io.DisplaySize.y) / 90.0f));
   if (m_rotate == 1 || m_rotate == 3)
   {
      const float size = io.DisplaySize.x;
      io.DisplaySize.x = io.DisplaySize.y;
      io.DisplaySize.y = size;
      const float scale = io.DisplayFramebufferScale.x;
      io.DisplayFramebufferScale.x = io.DisplayFramebufferScale.y;
      io.DisplayFramebufferScale.y = scale;
   }

   // Enable mouse capture when dragging (needed when dragging main windows)
   {
      bool want_capture = false;
      for (int button_n = 0; button_n < ImGuiMouseButton_COUNT && !want_capture; button_n++)
         if (ImGui::IsMouseDragging(button_n, 1.0f))
            want_capture = true;
      SDL_CaptureMouse(want_capture);
   }

   // Late mouse position update to latest (async) global state (needed when dragging main windows)
   {
      SDL_Point windowPos;
      SDL_FPoint globalMouse;
      SDL_GetGlobalMouseState(&globalMouse.x, &globalMouse.y);
      SDL_GetWindowPosition(m_player->m_playfieldWnd->GetCore(), &windowPos.x, &windowPos.y);
      AddMousePosEvent(false, globalMouse.x - static_cast<float>(windowPos.x), globalMouse.y - static_cast<float>(windowPos.y));
   }

   // We implement our own keyboard navigation using flipper keys
   io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;

   ImGui::NewFrame();
}

void LiveUI::Render3D()
{
   if (m_editorUI.IsOpened())
      m_editorUI.Render3D();
}

void LiveUI::RenderUI()
{
   // For the time being, the UI is only available inside a running player
   if (m_player == nullptr || m_player->GetCloseState() != Player::CS_PLAYING || m_rd->GetCurrentPass() == nullptr)
      return;

   const int width = m_rd->GetCurrentPass()->m_rt->GetWidth();
   const int height = m_rd->GetCurrentPass()->m_rt->GetHeight();

   UpdateTouchUI();

   ImGui::PushFont(m_baseFont, m_baseFont->LegacySize);

   if (!m_deviceLayoutName.empty())
      UpdateDeviceLayoutPopup();

   // Tweak UI (aligned to playfield view, using custom flipper controls)
   m_inGameUI.Update();

   if (!m_player->IsPlaying() && !m_editorUI.IsOpened())
   {
      ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 24 * m_uiScale, 4 * m_uiScale));
      ImGui::Begin("PauseOverlay", nullptr, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus // Prevent focus issues
            | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
      ImGui::Text(ICON_FK_PAUSE);
      ImGui::End();
   }

   if (ImGui::IsPopupOpen(ID_BAM_SETTINGS))
   { // BAM headtracking UI (aligned to desktop, using traditional mouse interaction) => hacky, remove and use plugin + plugin settings instead
      #ifndef __STANDALONE__
         BAMView::drawMenu();
      #endif
   }
   else if (m_editorUI.IsOpened())
   { // Editor UI (aligned to desktop, using traditional mouse interaction)
      SetupImGuiStyle(true);
      m_editorUI.RenderUI();
      SetupImGuiStyle(false);
   }
   else if (!m_inGameUI.IsOpened())
   { // No UI displayed: process ball control & throw balls
      m_ballControl.Update(width, height);
   }

   // Display plumb state overlay
   m_plumbOverlay.Update();

   // Display notification overlays except when script has an unaligned rotation
   m_notificationOverlay.Update(true, m_overlayFont);

   // Display performance overlays
   m_perfUI.Update();

   ImGui::PopFont();

   ImGui::EndFrame();

   ImGui::Render();

   ImDrawData *const draw_data = ImGui::GetDrawData();

   // Update textures
   if (draw_data->Textures != nullptr)
      for (ImTextureData *tex : *draw_data->Textures)
      {
         if (tex->Status == ImTextureStatus_WantCreate || tex->Status == ImTextureStatus_WantUpdates)
         {
            // Somewhat overkill as we treat update as destroy/create but fine enough (just slightly impact performance)
            assert(tex->GetPitch() == tex->Width * 4);
            assert(tex->Format == ImTextureFormat_RGBA32);
            std::shared_ptr<BaseTexture> texture;
            BaseTexture::Update(texture, tex->Width, tex->Height, BaseTexture::RGBA, static_cast<const uint8_t *>(tex->GetPixels()));
            tex->SetTexID(m_renderer->m_renderDevice->m_texMan.LoadTexture(texture.get(), false));
            tex->SetStatus(ImTextureStatus_OK);
         }
         if (tex->Status == ImTextureStatus_WantDestroy && tex->UnusedFrames > 0)
         {
            tex->SetTexID(ImTextureID_Invalid);
            tex->SetStatus(ImTextureStatus_Destroyed);
         }
      }

   // Update meshes and renders
   const ImGuiIO &io = ImGui::GetIO();
   const Matrix3D matRotate = Matrix3D::MatrixRotateZ(static_cast<float>(m_rotate * (M_PI / 2.0)));
   Matrix3D matTranslate;
   switch (m_rotate)
   {
   case 0: matTranslate = Matrix3D::MatrixIdentity(); break;
   case 1: matTranslate = Matrix3D::MatrixTranslate(io.DisplaySize.y, 0, 0); break;
   case 2: matTranslate = Matrix3D::MatrixTranslate(io.DisplaySize.x, io.DisplaySize.y, 0); break;
   case 3: matTranslate = Matrix3D::MatrixTranslate(0, io.DisplaySize.x, 0); break;
   default: assert(false); return;
   }
   const float right = (m_rotate == 1 || m_rotate == 3) ? io.DisplaySize.y : io.DisplaySize.x;
   const float bottom = (m_rotate == 1 || m_rotate == 3) ? io.DisplaySize.x : io.DisplaySize.y;
   const Matrix3D matProj = matRotate * matTranslate * Matrix3D::MatrixOrthoOffCenterRH(0.f, right, bottom, 0.f, 0.f, 1.f);
   m_rd->m_uiShader->SetMatrix(SHADER_matWorldView, &matProj);
   m_rd->m_uiShader->SetVector(SHADER_staticColor_Alpha,
      m_player->m_vrDevice ? ((float)m_player->m_vrDevice->GetEyeWidth() * 0.15f) : 0.f, // Stereo offset for VR (fake depth)
      0.f, // Unused
      0.f, // Unused
      // A value of 1.0 should be sdrWhite * 80, while in the WCG colorspace 80 nits is 0.5
      m_player->m_playfieldWnd->IsWCGBackBuffer() ? (2.0f / m_player->m_playfieldWnd->GetSDRWhitePoint()) : 1.f); // SDR color scaling
   m_rd->ResetRenderState();
   m_rd->SetRenderState(RenderState::COLORWRITEENABLE, RenderState::RGBMASK_RGBA);
   m_rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_TRUE);
   m_rd->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_ADD);
   m_rd->SetRenderState(RenderState::SRCBLEND, RenderState::SRC_ALPHA);
   m_rd->SetRenderState(RenderState::DESTBLEND, RenderState::INVSRC_ALPHA);
   m_rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   m_rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
   m_rd->m_uiShader->SetTechnique(SHADER_TECHNIQUE_LiveUI);
   if (static_cast<int>(m_meshBuffers.size()) < draw_data->CmdListsCount)
      m_meshBuffers.resize(draw_data->CmdListsCount);
   for (int n = 0; n < draw_data->CmdListsCount; n++)
   {
      const ImDrawList * const cmd_list = draw_data->CmdLists[n];
      const unsigned int numVertices = cmd_list->VtxBuffer.size();
      const unsigned int numIndices = cmd_list->IdxBuffer.size();

      if ((numVertices != 0) && (numIndices != 0))
      {
         if ((m_meshBuffers[n] == nullptr) || (m_meshBuffers[n]->m_ib->m_count < numIndices) || (m_meshBuffers[n]->m_vb->m_count < numVertices))
         {
            auto ib = std::make_shared<IndexBuffer>(m_rd, max(m_meshBuffers[n] ? m_meshBuffers[n]->m_ib->m_count : 0, numIndices), true, IndexBuffer::Format::FMT_INDEX32);
            auto vb = std::make_shared<VertexBuffer>(m_rd, max(m_meshBuffers[n] ? m_meshBuffers[n]->m_vb->m_count : 0, numVertices), nullptr, true);
            m_meshBuffers[n] = std::make_shared<MeshBuffer>(vb, ib, true);
         }

         Vertex3D_NoTex2 *vb;
         m_meshBuffers[n]->m_vb->Lock(vb);
         for (unsigned int i = 0; i < numVertices; i++)
         {
            const uint32_t rgba = cmd_list->VtxBuffer[i].col;
            vb[i].x = cmd_list->VtxBuffer[i].pos.x;
            vb[i].y = cmd_list->VtxBuffer[i].pos.y;
            vb[i].z = (float)((rgba >> 24) & 0xFFu) * (float)(1.0 / 255.0); // alpha
            vb[i].nx = (float)(rgba & 0x000000FFu) * (float)(1.0 / 255.0); // red
            vb[i].ny = (float)(rgba & 0x0000FF00u) * (float)(1.0 / 65280.0); // green
            vb[i].nz = (float)(rgba & 0x00FF0000u) * (float)(1.0 / 16711680.0); // blue
            vb[i].tu = cmd_list->VtxBuffer[i].uv.x;
            vb[i].tv = cmd_list->VtxBuffer[i].uv.y;
         }
         m_meshBuffers[n]->m_vb->Unlock();

         uint32_t *ib;
         m_meshBuffers[n]->m_ib->Lock(ib);
         memcpy(ib, cmd_list->IdxBuffer.begin(), numIndices * sizeof(ImDrawIdx));
         m_meshBuffers[n]->m_ib->Unlock();
      }

      for (const ImDrawCmd *cmd = cmd_list->CmdBuffer.begin(), *cmdEnd = cmd_list->CmdBuffer.end(); cmd != cmdEnd; cmd++)
      {
         if (cmd->ElemCount != 0)
         {
            #ifdef ENABLE_BGFX
            // FIXME Hacky forced mesh buffer upload before actually drawing, not sure why this is needed: uploads are supposed to happen in the 'preCmd' list (before any render command)
            // so this should not have any effect, still it does. This definitely needs more investigation...
            m_rd->m_uiShader->SetVector(SHADER_clip_plane, 0.f, 0.f, 0.f, 0.f);
            m_rd->DrawMesh(m_rd->m_uiShader, true, Vertex3Ds(), -10000.f, m_meshBuffers[n], RenderDevice::TRIANGLELIST, 0, 1);
            #endif

            m_rd->m_uiShader->SetVector(SHADER_clip_plane, cmd->ClipRect.x, cmd->ClipRect.y, cmd->ClipRect.z, cmd->ClipRect.w);
            m_rd->m_uiShader->SetTexture(SHADER_tex_base_color, cmd->GetTexID());
            m_rd->DrawMesh(m_rd->m_uiShader, true, Vertex3Ds(), -10000.f, m_meshBuffers[n], RenderDevice::TRIANGLELIST, cmd->IdxOffset, cmd->ElemCount);
         }
      }
   }

   NewFrame();
}

void LiveUI::UpdateTouchUI()
{
   if (!m_player->m_pininput.HasTouchInput())
      return;

   if (!m_showTouchOverlay)
      return;

   const ImGuiIO &io = ImGui::GetIO();

   const float screenWidth = io.DisplaySize.x;
   const float screenHeight = io.DisplaySize.y;

   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize
      | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

   ImGui::SetNextWindowPos(ImVec2(0, 0));
   ImGui::SetNextWindowSize(ImVec2(screenWidth, screenHeight));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

   ImGui::Begin("Touch Controls", nullptr, window_flags);
   ImDrawList *drawList = ImGui::GetWindowDrawList();
   for (const auto& region : m_pininput->GetTouchState())
   {
      const RECT& rect = region.region;

      const ImVec2 topLeft((float)rect.left * screenWidth / 100.0f, (float)rect.top * screenHeight / 100.0f);
      const ImVec2 bottomRight((float)rect.right * screenWidth / 100.0f, (float)rect.bottom * screenHeight / 100.0f);

      constexpr ImColor fillColor(255, 255, 255, 5);
      drawList->AddRectFilled(topLeft, bottomRight, fillColor);

      constexpr ImColor borderColor(255, 255, 255, 20);
      drawList->AddRect(topLeft, bottomRight, borderColor, 0.0f, ImDrawFlags_RoundCornersAll, 2.0f);
   }
   ImGui::End();
   ImGui::PopStyleVar(2);
}

void LiveUI::OpenInGameUI(const string& page)
{
   HideUI();
   m_inGameUI.Open(page);
}

void LiveUI::HideUI()
{ 
   m_renderer->InitLayout();
   if (m_inGameUI.IsOpened())
      m_inGameUI.Close();
   m_editorUI.Close();
   m_player->m_ptable->m_settings.Save();
   g_pvp->m_settings.Save();
   m_player->SetPlayState(true);
}

bool LiveUI::ProposeInputLayout(const string &deviceName, const std::function<void(bool, bool)> &handler)
{
   if (!m_deviceLayoutName.empty())
      return false;
   m_deviceLayoutName = deviceName;
   m_deviceLayoutHandler = handler;
   m_deviceLayoutDontAskAgain = false;
   return true;
}

void LiveUI::UpdateDeviceLayoutPopup()
{
   if (!m_deviceLayoutName.empty())
      ImGui::OpenPopup("Apply Device Layout ?");
   ImGui::SetNextWindowSize(ImVec2(350.f * m_uiScale, 0.f));
   if (ImGui::BeginPopupModal("Apply Device Layout ?"))
   {
      ImGui::TextWrapped("Device '%s' was detected. Would you like the default input layout to be applied ?", m_deviceLayoutName.c_str());
      ImGui::Separator();
      ImGui::Checkbox("Don't ask again", &m_deviceLayoutDontAskAgain);
      ImGui::Separator();
      if (ImGui::Button("Discard"))
      {
         m_deviceLayoutName = ""s;
         m_deviceLayoutHandler(false, m_deviceLayoutDontAskAgain);
         ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Apply").x - ImGui::GetStyle().FramePadding.x * 2.f);
      if (ImGui::Button("Apply"))
      {
         m_deviceLayoutName = ""s;
         m_deviceLayoutHandler(true, m_deviceLayoutDontAskAgain);
         ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
   }
}

extern ImGuiKey ImGui_ImplSDL3_KeyEventToImGuiKey(SDL_Keycode keycode, SDL_Scancode scancode);
ImGuiKey LiveUI::GetImGuiKeyFromSDLScancode(const SDL_Scancode sdlk)
{
   return ImGui_ImplSDL3_KeyEventToImGuiKey(SDL_GetKeyFromScancode(sdlk, SDL_KMOD_NONE, false), sdlk);
}

void LiveUI::CenteredText(const string &text)
{
   const ImVec2 win_size = ImGui::GetWindowSize();
   const ImVec2 text_size = ImGui::CalcTextSize(text.c_str());

   // calculate the indentation that centers the text on one line, relative
   // to window left, regardless of the `ImGuiStyleVar_WindowPadding` value
   float text_indentation = (win_size.x - text_size.x) * 0.5f;

   // if text is too long to be drawn on one line, `text_indentation` can
   // become too small or even negative, so we check a minimum indentation
   constexpr float min_indentation = 20.0f;
   if (text_indentation <= min_indentation)
      text_indentation = min_indentation;

   ImGui::SameLine(text_indentation);
   ImGui::PushTextWrapPos(win_size.x - text_indentation);
   ImGui::TextWrapped("%s", text.c_str());
   ImGui::PopTextWrapPos();
}

void LiveUI::SetupImGuiStyle(const bool isEditor) const
{
   // Theme looking somewhat like Blender's style, based on 'Rounded Visual Studio' style by RedNicStone from ImThemes
   ImGuiStyle &style = ImGui::GetStyle();

   style.Alpha = 1.0f;
   style.DisabledAlpha = 0.6f;
   style.WindowPadding = ImVec2(8.0f, 8.0f);
   style.WindowRounding = 4.0f;
   style.WindowBorderSize = 0.0f;
   style.WindowMinSize = ImVec2(32.0f, 32.0f);
   style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
   style.WindowMenuButtonPosition = ImGuiDir_Left;
   style.ChildRounding = 0.0f;
   style.ChildBorderSize = 1.0f;
   style.PopupRounding = 4.0f;
   style.PopupBorderSize = 1.0f;
   style.FramePadding = ImVec2(4.0f, 3.0f);
   style.FrameRounding = 2.5f;
   style.FrameBorderSize = 0.0f;
   style.ItemSpacing = ImVec2(8.0f, 4.0f);
   style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
   style.CellPadding = ImVec2(4.0f, 2.0f);
   style.IndentSpacing = 21.0f;
   style.ColumnsMinSpacing = 6.0f;
   style.ScrollbarSize = 11.0f;
   style.ScrollbarRounding = 2.5f;
   style.GrabMinSize = 10.0f;
   style.GrabRounding = 2.0f;
   style.TabRounding = 3.5f;
   style.TabBorderSize = 0.0f;
   style.TabCloseButtonMinWidthUnselected = 0.0f;
   style.ColorButtonPosition = ImGuiDir_Right;
   style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
   style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

   style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
   style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.592f, 0.592f, 0.592f, 1.0f);
   style.Colors[ImGuiCol_WindowBg] = isEditor ? ImColor(0xFF363636) : ImColor(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_ChildBg] = ImVec4(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_PopupBg] = ImVec4(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_Border] = ImVec4(0.306f, 0.306f, 0.306f, 1.0f);
   style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.306f, 0.306f, 0.306f, 1.0f);
   style.Colors[ImGuiCol_FrameBg] = isEditor ? ImColor(0xFF545454) : ImColor(0.2f, 0.2f, 0.216f, 1.0f);
   style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.114f, 0.592f, 0.925f, 1.0f);
   style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.0f, 0.467f, 0.784f, 1.0f);
   style.Colors[ImGuiCol_TitleBg] = ImVec4(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.2f, 0.2f, 0.216f, 1.0f);
   style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.2f, 0.2f, 0.216f, 1.0f);
   style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.322f, 0.322f, 0.333f, 1.0f);
   style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.353f, 0.353f, 0.373f, 1.0f);
   style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.353f, 0.353f, 0.373f, 1.0f);
   style.Colors[ImGuiCol_CheckMark] = isEditor ? ImColor(0xFFdddddd) : ImColor(0.0f, 0.467f, 0.784f, 1.0f);
   style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.114f, 0.592f, 0.925f, 1.0f);
   style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.467f, 0.784f, 1.0f);
   style.Colors[ImGuiCol_Button] = ImVec4(0.2f, 0.2f, 0.216f, 1.0f);
   style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.114f, 0.592f, 0.925f, 1.0f);
   style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.114f, 0.592f, 0.925f, 1.0f);
   style.Colors[ImGuiCol_Header] = isEditor ? ImColor(0xFF3d3d3d) : ImColor(0.2f, 0.2f, 0.216f, 1.0f);
   style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.114f, 0.592f, 0.925f, 1.0f);
   style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.467f, 0.784f, 1.0f);
   style.Colors[ImGuiCol_Separator] = ImVec4(0.306f, 0.306f, 0.306f, 1.0f);
   style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.306f, 0.306f, 0.306f, 1.0f);
   style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.306f, 0.306f, 0.306f, 1.0f);
   style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.2f, 0.2f, 0.216f, 1.0f);
   style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.322f, 0.322f, 0.333f, 1.0f);
   style.Colors[ImGuiCol_Tab] = ImVec4(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_TabHovered] = ImVec4(0.114f, 0.592f, 0.925f, 1.0f);
   style.Colors[ImGuiCol_TabActive] = ImVec4(0.0f, 0.467f, 0.784f, 1.0f);
   style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.0f, 0.467f, 0.784f, 1.0f);
   style.Colors[ImGuiCol_PlotLines] = ImVec4(0.0f, 0.467f, 0.784f, 1.0f);
   style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.114f, 0.592f, 0.925f, 1.0f);
   style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.0f, 0.467f, 0.784f, 1.0f);
   style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.114f, 0.592f, 0.925f, 1.0f);
   style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.188f, 0.188f, 0.2f, 1.0f);
   style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.310f, 0.310f, 0.349f, 1.0f);
   style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.227f, 0.227f, 0.247f, 1.0f);
   style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f * 1.0f);
   style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.060f * 1.0f);
   style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.467f, 0.784f, 1.0f);
   style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.700f);
   style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800f, 0.800f, 0.800f, 0.20f);
   style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.145f, 0.145f, 0.149f, 0.35f);
}
