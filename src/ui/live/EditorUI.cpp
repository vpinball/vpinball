// license:GPLv3+

#include "core/stdafx.h"

#include "EditorUI.h"

#include "renderer/VRDevice.h"
#include "renderer/Shader.h"
#include "renderer/Anaglyph.h"

#include "core/TableDB.h"

#include "fonts/DroidSans.h"
#include "fonts/DroidSansBold.h"
#include "fonts/IconsForkAwesome.h"
#include "fonts/ForkAwesome.h"

#include "plugins/VPXPlugin.h"
#include "core/VPXPluginAPIImpl.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h" // Needed for FindRenderedTextEnd in HelpSplash (should be adapted when this function will refactored in ImGui)

#if defined(ENABLE_DX9)
  #include <shellapi.h>
#endif

#include "imgui/imgui_stdlib.h"
#include "imguizmo/ImGuizmo.h"
#include "imgui_markdown/imgui_markdown.h"

// Titles (used as Ids) of modal dialogs
#define ID_VIDEO_SETTINGS "Video Options"
#define ID_RENDERER_INSPECTION "Renderer Inspection"
#define ID_ANAGLYPH_CALIBRATION "Anaglyph Calibration"
#define ID_PLUMB_SETTINGS "Nudge & Plumb Settings"

#define PROP_WIDTH (125.f * m_liveUI.GetDPI())
#define PROP_TIMER(is_live, startup_obj, live_obj) \
   if (ImGui::CollapsingHeader("Timer", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE) \
   { \
      PropCheckbox("Enabled", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_tdr.m_TimerEnabled) : nullptr, live_obj ? &(live_obj->m_d.m_tdr.m_TimerEnabled) : nullptr); \
      PropInt("Interval (ms)", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_tdr.m_TimerInterval) : nullptr, live_obj ? &(live_obj->m_d.m_tdr.m_TimerInterval) : nullptr); \
      ImGui::EndTable(); \
   }
#define BEGIN_PROP_TABLE ImGui::BeginTable("props", 2, ImGuiTableFlags_Borders)
#define PROP_TABLE_SETUP \
   if (ImGui::TableGetRowIndex() == -1) \
   { \
      ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthStretch); \
      ImGui::TableSetupColumn("Sync", ImGuiTableColumnFlags_WidthFixed); \
   }
#define PROP_ACCESS(startup_obj, live_obj, prop) startup_obj ? &(startup_obj->prop) : nullptr, live_obj ? &(live_obj->prop) : nullptr

#define ICON_SAVE ICON_FK_FLOPPY_O

// Helper to display a little (?) mark which shows a tooltip when hovered.
static void HelpMarker(const char *desc)
{
   ImGui::TextDisabled("(?)");
   if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
   {
      ImGui::BeginTooltip();
      ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
      ImGui::TextUnformatted(desc);
      ImGui::PopTextWrapPos();
      ImGui::EndTooltip();
   }
}

template <class T> static std::vector<T> SortedCaseInsensitive(std::vector<T>& list, const std::function<string(T)>& map)
{
   std::vector<T> sorted(list.begin(), list.end());
   std::ranges::sort(sorted, [map](const T &a, const T &b) -> bool 
      {
         const string str1 = map(a), str2 = map(b);
         for (string::const_iterator c1 = str1.begin(), c2 = str2.begin(); c1 != str1.end() && c2 != str2.end(); ++c1, ++c2)
         {
            const auto cl1 = cLower(*c1);
            const auto cl2 = cLower(*c2);
            if (cl1 > cl2)
               return false;
            if (cl1 < cl2)
               return true;
         }
         return str1.size() > str2.size();
      });
   return sorted;
}


static void HelpSplash(const string &text, int rotation)
{
   const ImVec2 win_size = ImGui::GetIO().DisplaySize;

   vector<string> lines;
   ImVec2 text_size(0, 0);

   constexpr float padding = 60.f;
   const float maxWidth = win_size.x - padding;
   ImFont *const font = ImGui::GetFont();
   ImFontBaked *const fontBaked = ImGui::GetFontBaked();

   string line;
   std::istringstream iss(text);
   while (std::getline(iss, line)) {
       if (line.empty()) {
          lines.push_back(line);
          continue;
       }
       const char *textEnd = line.c_str();
       while (*textEnd)
       {
          const char *nextLineTextEnd = ImGui::FindRenderedTextEnd(textEnd, nullptr);
          ImVec2 lineSize = font->CalcTextSizeA(fontBaked->Size, FLT_MAX, 0.0f, textEnd, nextLineTextEnd);
          if (lineSize.x > maxWidth)
          {
             const char *wrapPoint = font->CalcWordWrapPositionA(font->Scale, textEnd, nextLineTextEnd, maxWidth);
             if (wrapPoint == textEnd)
                wrapPoint++;
             nextLineTextEnd = wrapPoint;
             lineSize = font->CalcTextSizeA(fontBaked->Size, FLT_MAX, 0.0f, textEnd, wrapPoint);
          }

          string newLine(textEnd, nextLineTextEnd);
          lines.push_back(newLine);

          if (lineSize.x > text_size.x)
             text_size.x = lineSize.x;

          textEnd = nextLineTextEnd;

          while (*textEnd == ' ')
             textEnd++;
       }
   }

   text_size.x += padding / 2.f;
   text_size.y = (float)lines.size() * ImGui::GetTextLineHeightWithSpacing() + (padding / 2.f);

   constexpr ImGuiWindowFlags window_flags
      = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
   ImGui::SetNextWindowBgAlpha(0.35f);
   ImGui::SetNextWindowPos(ImVec2((win_size.x - text_size.x) / 2, (win_size.y - text_size.y) / 2));
   ImGui::SetNextWindowSize(ImVec2(text_size.x, text_size.y));
   ImGui::Begin("ToolTip", nullptr, window_flags);
   ImGui::SetCursorPosY(padding / 4.f);
   for (const string& curline : lines)
   {
      const ImVec2 lineSize = font->CalcTextSizeA(fontBaked->Size, FLT_MAX, 0.0f, curline.c_str());
      ImGui::SetCursorPosX((text_size.x - lineSize.x) / 2.f);
      ImGui::TextUnformatted(curline.c_str());
   }
   ImGui::End();
}

static void HelpEditableHeader(bool is_live, IEditable *editable, IEditable *live_editable)
{
   IEditable *notnull_editable = editable ? editable : live_editable;
   IEditable *select_editable = is_live ? live_editable : editable;
   if (notnull_editable == nullptr)
      return;
   string title;
   switch (notnull_editable->GetItemType())
   {
   // Missing: eItemLightCenter, eItemDragPoint, eItemCollection
   case eItemBumper: title = "Bumper"s; break;
   case eItemDecal: title = "Decal"s; break;
   case eItemDispReel: title = "Reel"s; break;
   case eItemGate: title = "Gate"s; break;
   case eItemFlasher: title = "Flasher"s; break;
   case eItemFlipper: title = "Flipper"s; break;
   case eItemHitTarget: title = "Target"s; break;
   case eItemKicker: title = "Kicker"s; break;
   case eItemLight: title = "Light"s; break;
   case eItemLightSeq: title = "Light Sequencer"s; break;
   case eItemPlunger: title = "Plunger"s; break;
   case eItemPrimitive: title = ((Primitive *)notnull_editable)->IsPlayfield() ? "Playfield"s : "Primitive"s; break;
   case eItemRamp: title = "Ramp"s; break;
   case eItemRubber: title = "Rubber"s; break;
   case eItemSpinner: title = "Spinner"s; break;
   case eItemSurface: title = "Surface"s; break;
   case eItemTable: title = "Table"s; break;
   case eItemTextbox: title = "TextBox"s; break;
   case eItemTimer: title = "Timer"s; break;
   case eItemTrigger: title = "Trigger"s; break;
   default: break;
   }
   LiveUI::CenteredText(title);
   ImGui::BeginDisabled(is_live); // Do not edit name of live objects, it would likely break the script
   string name = select_editable ? select_editable->GetName() : string();
   if (ImGui::InputText("Name", &name))
   {
      editable->SetName(name);
   }
   ImGui::EndDisabled();
   ImGui::Separator();
}



EditorUI::EditorUI(LiveUI &liveUI)
   : m_liveUI(liveUI)
{
   m_StartTime_msec = msec();
   m_app = g_pvp;
   m_player = g_pplayer;
   m_table = m_player->m_pEditorTable;
   m_live_table = m_player->m_ptable;
   m_pininput = &(m_player->m_pininput);
   m_renderer = m_player->m_renderer;
   
   m_selection.type = Selection::SelectionType::S_NONE;
   m_useEditorCam = false;

   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO &io = ImGui::GetIO();
   io.IniFilename = nullptr; //don't use an ini file for configuration

   // Editor camera position. We use a right handed system for easy ImGuizmo integration while VPX renderer is left handed, so reverse X axis
   m_orthoCam = true;
   m_camDistance = m_live_table->m_bottom * 0.7f;
   const vec3 eye(m_live_table->m_right * 0.5f, m_live_table->m_bottom * 0.5f, -m_camDistance);
   const vec3 at(m_live_table->m_right * 0.5f, m_live_table->m_bottom * 0.5f, 0.f);
   constexpr vec3 up{0.f, -1.f, 0.f};
   m_camView = Matrix3D::MatrixLookAtRH(eye, at, up);
   ImGuizmo::AllowAxisFlip(false);
}

EditorUI::~EditorUI()
{

}

void EditorUI::Open()
{
   if (m_isOpened)
      return;
   m_isOpened = true;
   ResetCameraFromPlayer();
   m_player->SetPlayState(false);
   m_renderer->DisableStaticPrePass(true);
}

void EditorUI::Close()
{
   if (!m_isOpened)
      return;
   m_isOpened = false;
   m_flyMode = false;
   m_renderer->DisableStaticPrePass(false);
}

void EditorUI::ResetCameraFromPlayer()
{
   // Try to setup editor camera to match the used one, but only mostly since the EditorUI does not have some view setup features like off-center, ...
   m_camView = Matrix3D::MatrixScale(1.f, 1.f, -1.f) * m_renderer->GetMVP().GetView() * Matrix3D::MatrixScale(1.f, -1.f, 1.f);
}

void EditorUI::Update()
{
   const ImGuiIO &io = ImGui::GetIO();
   ImGuizmo::SetOrthographic(m_orthoCam);
   ImGuizmo::BeginFrame();
   ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

   Selection previousSelection = m_selection;

#if !((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))
   m_menubar_height = 0.0f;
   m_toolbar_height = 0.0f;

   // Gives some transparency when positioning camera to better view camera view bounds
   // TODO for some reasons, this breaks the modal background behavior
   //SetupImGuiStyle(m_selection.type == EditorUI::Selection::SelectionType::S_CAMERA ? 0.3f : 1.0f);

   bool showFullUI = true;
   showFullUI &= !m_showRendererInspection;
   showFullUI &= !ImGui::IsPopupOpen(ID_VIDEO_SETTINGS);
   showFullUI &= !ImGui::IsPopupOpen(ID_ANAGLYPH_CALIBRATION);
   showFullUI &= !m_flyMode;

   if (showFullUI)
   {
      // Main menubar
      bool openVideoSettings = false;
      bool openPlumbSettings = false;
      if (ImGui::BeginMainMenuBar())
      {
         if (!m_table->IsLocked() && ImGui::BeginMenu("Debug"))
         {
            if (ImGui::MenuItem("Open debugger"))
               m_player->m_showDebugger = true;
            if (ImGui::MenuItem("Renderer Inspection"))
               m_showRendererInspection = true;
            if (ImGui::MenuItem(m_player->m_debugWindowActive ? "Play" : "Pause"))
               m_player->SetPlayState(!m_player->IsPlaying());
            ImGui::EndMenu();
         }
         // TODO move to in game ui
         if (ImGui::BeginMenu("Preferences"))
         {
            if (ImGui::MenuItem("Video Options"))
               openVideoSettings = true;
            if (ImGui::MenuItem("Nudge Options"))
               openPlumbSettings = true;
            ImGui::EndMenu();
         }
         float buttonWidth = ImGui::CalcTextSize(ICON_FK_REPLY, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2.0f
            + ImGui::CalcTextSize(ICON_FK_WINDOW_CLOSE, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2.0f;
         ImVec2 padding = ImGui::GetCursorScreenPos();
         padding.x += ImGui::GetContentRegionAvail().x - buttonWidth;
         ImGui::SetCursorScreenPos(padding);
         if (ImGui::Button(ICON_FK_REPLY)) // ICON_FK_STOP
            Close();
         if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Get back to player");
         if (ImGui::Button(ICON_FK_WINDOW_CLOSE))
            m_table->QuitPlayer(Player::CS_STOP_PLAY);
         if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Close editor");
         m_menubar_height = ImGui::GetWindowSize().y;
         ImGui::EndMainMenuBar();
      }
      if (openVideoSettings)
         ImGui::OpenPopup(ID_VIDEO_SETTINGS, ImGuiPopupFlags_NoReopen);
      if (openPlumbSettings)
         ImGui::OpenPopup(ID_PLUMB_SETTINGS, ImGuiPopupFlags_NoReopen);

      // Main toolbar
      m_toolbar_height = 20.f * m_liveUI.GetDPI();
      const ImGuiViewport *const viewport = ImGui::GetMainViewport();
      ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + m_menubar_height));
      ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, m_toolbar_height));
      constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
      ImGui::Begin("TOOLBAR", nullptr, window_flags);
      ImGui::PopStyleVar();
      if (ImGui::Button(m_player->IsPlaying() ? ICON_FK_PAUSE : ICON_FK_PLAY))
         m_player->SetPlayState(!m_player->IsPlaying());
      ImGui::SameLine();
      ImGui::BeginDisabled(m_player->IsPlaying());
      if (ImGui::Button(ICON_FK_STEP_FORWARD))
         m_player->m_step = true;
      ImGui::EndDisabled();
      float buttonWidth = ImGui::CalcTextSize(ICON_FK_STICKY_NOTE, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2.0f
         + ImGui::CalcTextSize(ICON_FK_FILTER, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2.0f;
      ImGui::SameLine(ImGui::GetContentRegionAvail().x - buttonWidth);
      if (ImGui::Button(ICON_FK_STICKY_NOTE)) // Overlay option menu
         ImGui::OpenPopup("Overlay Popup");
      if (ImGui::BeginPopup("Overlay Popup"))
      {
         ImGui::TextUnformatted("Overlays:");
         ImGui::Separator();
         ImGui::Checkbox("Overlay selection", &m_selectionOverlay);
         ImGui::Separator();
         ImGui::TextUnformatted("Physic Overlay:");
         if (ImGui::RadioButton("None", m_physOverlay == PO_NONE))
            m_physOverlay = PO_NONE;
         if (ImGui::RadioButton("Selected", m_physOverlay == PO_SELECTED))
            m_physOverlay = PO_SELECTED;
         if (ImGui::RadioButton("All", m_physOverlay == PO_ALL))
            m_physOverlay = PO_ALL;
         ImGui::EndPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button(ICON_FK_FILTER)) // Selection filter
         ImGui::OpenPopup("Selection filter Popup");
      if (ImGui::BeginPopup("Selection filter Popup"))
      {
         bool pf = m_selectionFilter & SelectionFilter::SF_Playfield;
         bool prims = m_selectionFilter & SelectionFilter::SF_Primitives;
         bool lights = m_selectionFilter & SelectionFilter::SF_Lights;
         bool flashers = m_selectionFilter & SelectionFilter::SF_Flashers;
         ImGui::TextUnformatted("Selection filters:");
         ImGui::Separator();
         if (ImGui::Checkbox("Playfield", &pf))
            m_selectionFilter = (m_selectionFilter & ~SelectionFilter::SF_Playfield) | (pf ? SelectionFilter::SF_Playfield : 0x0000);
         if (ImGui::Checkbox("Primitives", &prims))
            m_selectionFilter = (m_selectionFilter & ~SelectionFilter::SF_Primitives) | (prims ? SelectionFilter::SF_Primitives : 0x0000);
         if (ImGui::Checkbox("Lights", &lights))
            m_selectionFilter = (m_selectionFilter & ~SelectionFilter::SF_Lights) | (lights ? SelectionFilter::SF_Lights : 0x0000);
         if (ImGui::Checkbox("Flashers", &flashers))
            m_selectionFilter = (m_selectionFilter & ~SelectionFilter::SF_Flashers) | (flashers ? SelectionFilter::SF_Flashers : 0x0000);
         ImGui::EndPopup();
      }
      ImGui::End();

      // Overlay Info Text
      ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - 200.f * m_liveUI.GetDPI(), io.DisplaySize.y - m_toolbar_height - m_menubar_height - 5.f * m_liveUI.GetDPI())); // Fixed outliner width (to be adjusted when moving ImGui to the docking branch)
      ImGui::SetNextWindowPos(ImVec2(200.f * m_liveUI.GetDPI(), m_toolbar_height + m_menubar_height + 5.f * m_liveUI.GetDPI()));
      ImGui::Begin("text overlay", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav);
      switch (m_gizmoOperation)
      {
      case ImGuizmo::NONE: ImGui::TextUnformatted("Select"); break;
      case ImGuizmo::TRANSLATE: ImGui::TextUnformatted("Grab"); break;
      case ImGuizmo::ROTATE: ImGui::TextUnformatted("Rotate"); break;
      case ImGuizmo::SCALE: ImGui::TextUnformatted("Scale"); break;
      default: break;
      }
      ImGui::End();

      // Side panels
      UpdateOutlinerUI();
      UpdatePropertyUI();
   }

   // Popups & Modal dialogs
   UpdateVideoOptionsModal();
   UpdateAnaglyphCalibrationModal();
   UpdatePlumbWindow();

   if (m_showRendererInspection)
      UpdateRendererInspectionModal();

#endif

   // Invisible full frame window for overlays
   ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
   ImGui::SetNextWindowPos(ImVec2(0, 0));
   ImGui::PushStyleColor(ImGuiCol_WindowBg, 0);
   ImGui::PushStyleColor(ImGuiCol_Border, 0);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
   ImGui::Begin("overlays", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs 
      | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
   ImDrawList *const overlayDrawList = ImGui::GetWindowDrawList();
   ImGui::End();
   ImGui::PopStyleVar();
   ImGui::PopStyleColor(2);

   // Update editor camera
   if (m_useEditorCam)
   {
      // Convert from right handed (ImGuizmo view manipulate is right handed) to VPX's left handed coordinate system
      // Right Hand to Left Hand (note that RH2LH = inverse(RH2LH), so RH2LH.RH2LH is identity, which property is used below)
      const Matrix3D RH2LH = Matrix3D::MatrixScale(1.f,  1.f, -1.f);
      const Matrix3D YAxis = Matrix3D::MatrixScale(1.f, -1.f, -1.f);
      float zNear, zFar;
      m_live_table->ComputeNearFarPlane(RH2LH * m_camView * YAxis, 1.f, zNear, zFar);

      if (m_orthoCam)
      {
         float viewHeight = m_camDistance;
         float viewWidth = viewHeight * io.DisplaySize.x / io.DisplaySize.y;
         m_camProj = Matrix3D::MatrixOrthoOffCenterRH(-viewWidth, viewWidth, -viewHeight, viewHeight, zNear, -zFar);
      }
      else
      {
         m_camProj = Matrix3D::MatrixPerspectiveFovRH(39.6f, io.DisplaySize.x / io.DisplaySize.y, zNear, zFar);
      }

      /*Matrix3D gridMatrix = Matrix3D::MatrixRotateX(M_PI * 0.5);
      gridMatrix.Scale(10.0f, 1.0f, 10.0f);
      ImGuizmo::DrawGrid((const float *)(m_camView.m), (const float *)(m_camProj.m), (const float *)(gridMatrix.m), 100.f);*/
   }

   // Selection manipulator
   Matrix3D transform;
   bool isSelectionTransformValid = GetSelectionTransform(transform);
   if (isSelectionTransformValid)
   {
      float camViewLH[16];
      memcpy(camViewLH, &m_camView.m[0][0], sizeof(float) * 4 * 4);
      for (int i = 8; i < 12; i++)
         camViewLH[i] = -camViewLH[i];
      Matrix3D prevTransform(transform);
      ImGuizmo::Manipulate(camViewLH, (float *)(m_camProj.m), m_gizmoOperation, m_gizmoMode, (float *)(transform.m));
      if (memcmp(transform.m, prevTransform.m, 16 * sizeof(float)) != 0)
         SetSelectionTransform(transform);
   }

   // Selection and physic colliders overlay
   {
      const float rClipWidth = (float)m_player->m_playfieldWnd->GetWidth() * 0.5f;
      const float rClipHeight = (float)m_player->m_playfieldWnd->GetHeight() * 0.5f;
      Matrix3D mvp = m_renderer->GetMVP().GetModelViewProj(0);
      auto project = [mvp, rClipWidth, rClipHeight](Vertex3Ds v)
      {
         const float xp = mvp._11 * v.x + mvp._21 * v.y + mvp._31 * v.z + mvp._41;
         const float yp = mvp._12 * v.x + mvp._22 * v.y + mvp._32 * v.z + mvp._42;
         //const float zp = mvp._13 * v.x + mvp._23 * v.y + mvp._33 * v.z + mvp._43;
         const float wp = mvp._14 * v.x + mvp._24 * v.y + mvp._34 * v.z + mvp._44;
         if (wp <= 1e-10f) // behind camera (or degenerated)
            return Vertex2D{FLT_MAX, FLT_MAX};
         const float inv_wp = 1.0f / wp;
         return Vertex2D{(wp + xp) * rClipWidth * inv_wp, (wp - yp) * rClipHeight * inv_wp};
      };

      if (isSelectionTransformValid)
      {
         Vertex2D pos = project(transform.GetOrthoNormalPos());
         overlayDrawList->AddCircleFilled(ImVec2(pos.x, pos.y), 3.f * m_liveUI.GetDPI(), IM_COL32(255, 255, 255, 255), 16);
      }

      if (m_selection.type == Selection::S_EDITABLE && m_selectionOverlay)
      {
         ImGui::PushStyleColor(ImGuiCol_PlotLines, IM_COL32(255, 128, 0, 255));
         ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(255, 128, 0, 32));
         for (auto pho : m_player->m_physics->GetUIHitObjects(m_selection.editable))
            if (overlayDrawList->VtxBuffer.Size < 40000)
               pho->DrawUI(project, overlayDrawList, true);
         ImGui::PopStyleColor(2);
      }
      
      if (m_physOverlay == PO_ALL || (m_physOverlay == PO_SELECTED && m_selection.type == Selection::S_EDITABLE))
      {
         ImGui::PushStyleColor(ImGuiCol_PlotLines, IM_COL32(255, 0, 0, 255)); // We abuse ImGui colors to pass render colors
         ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(255, 0, 0, 64));
         for (auto pho : m_player->m_physics->GetHitObjects())
            if (pho != nullptr && (m_physOverlay == PO_ALL || (m_physOverlay == PO_SELECTED && pho->m_editable == m_selection.editable)) && overlayDrawList->VtxBuffer.Size < 40000)
               pho->DrawUI(project, overlayDrawList, true);
         ImGui::PopStyleColor(2);
      }
   }

   // Handle uncaught mouse & keyboard shortcuts
   if (!io.WantCaptureMouse)
   {
      // Zoom in/out with mouse wheel
      if (io.MouseWheel != 0)
      {
         m_useEditorCam = true;
         Matrix3D view(m_camView);
         view.Invert();
         const vec3 up = view.GetOrthoNormalUp(), dir = view.GetOrthoNormalDir(), pos = view.GetOrthoNormalPos();
         const vec3 camTarget = pos - dir * m_camDistance;
         m_camDistance *= powf(1.1f, -ImGui::GetIO().MouseWheel);
         const vec3 newEye = camTarget + dir * m_camDistance;
         m_camView = Matrix3D::MatrixLookAtRH(newEye, camTarget, up);
      }

      // Mouse pan
      if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
      {
         const ImVec2 drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
         ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);
         if (drag.x != 0.f || drag.y != 0.f)
         {
            m_useEditorCam = true;
            Matrix3D viewInverse(m_camView);
            viewInverse.Invert();
            vec3 up = viewInverse.GetOrthoNormalUp(), dir = viewInverse.GetOrthoNormalDir();
            vec3 pos = viewInverse.GetOrthoNormalPos(), right = viewInverse.GetOrthoNormalRight();
            vec3 camTarget = pos - dir * m_camDistance;
            if (ImGui::GetIO().KeyShift)
            {
               camTarget = camTarget - right * drag.x + up * drag.y;
               m_camView = Matrix3D::MatrixLookAtRH(pos - right * drag.x + up * drag.y, camTarget, up);
            }
            else
            {
               const Matrix3D rx = Matrix3D::MatrixRotate(drag.x * 0.01f, up);
               const Matrix3D ry = Matrix3D::MatrixRotate(drag.y * 0.01f, right);
               const Matrix3D roll = rx * ry;
               dir = roll.MultiplyVectorNoPerspective(dir);
               dir.Normalize();

               // clamp
               vec3 planDir = CrossProduct(right, up);
               planDir.y = 0.f;
               planDir.Normalize();
               float dt = planDir.Dot(dir);
               if (dt < 0.0f)
               {
                  dir += planDir * dt;
                  dir.Normalize();
               }

               m_camView = Matrix3D::MatrixLookAtRH(camTarget + dir * m_camDistance, camTarget, up);
               m_orthoCam = false; // switch to perspective when user orbit the view (ortho is only really useful when seen from predefined ortho views)
            }
         }
      }

      // Select
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
      {
         // Compute mouse position in clip space
         const float rClipWidth = (float)m_player->m_playfieldWnd->GetWidth() * 0.5f;
         const float rClipHeight = (float)m_player->m_playfieldWnd->GetHeight() * 0.5f;
         const float xcoord = (ImGui::GetMousePos().x - rClipWidth) / rClipWidth;
         const float ycoord = (rClipHeight - ImGui::GetMousePos().y) / rClipHeight;

         // Use the inverse of our 3D transform to determine where in 3D space the
         // screen pixel the user clicked on is at.  Get the point at the near
         // clipping plane (z=0) and the far clipping plane (z=1) to get the whole
         // range we need to hit test
         Matrix3D invMVP = m_renderer->GetMVP().GetModelViewProj(0);
         invMVP.Invert();
         const Vertex3Ds v3d  = invMVP * Vertex3Ds{xcoord, ycoord, 0.f};
         const Vertex3Ds v3d2 = invMVP * Vertex3Ds{xcoord, ycoord, 1.f};
         
         // FIXME This is not really great as:
         // - picking depends on what was visible/enabled when quadtree was built (lazily at first pick), and also uses the physics quadtree for some parts
         // - primitives can have hit bug (Apron Top and Gottlieb arm of default table for example): degenerated geometry ?
         // We would need a dedicated quadtree for UI with all parts, and filter after picking by visibility
         vector<HitTestResult> vhoUnfilteredHit;
         m_player->m_physics->RayCast(v3d, v3d2, true, vhoUnfilteredHit);

         vector<HitTestResult> vhoHit;
         bool noPF = !(m_selectionFilter & SelectionFilter::SF_Playfield);
         bool noPrims = !(m_selectionFilter & SelectionFilter::SF_Primitives);
         bool noLights = !(m_selectionFilter & SelectionFilter::SF_Lights);
         bool noFlashers = !(m_selectionFilter & SelectionFilter::SF_Flashers);
         for (const auto& hr : vhoUnfilteredHit)
         {
            if (noPF && hr.m_obj->m_editable && hr.m_obj->m_editable->GetItemType() == ItemTypeEnum::eItemPrimitive && ((Primitive*)hr.m_obj->m_editable)->IsPlayfield())
               continue;
            if (noPrims && hr.m_obj->m_editable && hr.m_obj->m_editable->GetItemType() == ItemTypeEnum::eItemPrimitive)
               continue;
            if (noLights && hr.m_obj->m_editable && hr.m_obj->m_editable->GetItemType() == ItemTypeEnum::eItemLight)
               continue;
            if (noFlashers && hr.m_obj->m_editable && hr.m_obj->m_editable->GetItemType() == ItemTypeEnum::eItemFlasher)
               continue;
            vhoHit.push_back(hr);
         }

         if (vhoHit.empty())
            m_selection.type = Selection::SelectionType::S_NONE;
         else
         {
            size_t selectionIndex = vhoHit.size();
            for (size_t i = 0; i <= vhoHit.size(); i++)
            {
               if (i < vhoHit.size() && m_selection.type == Selection::S_EDITABLE && vhoHit[i].m_obj->m_editable == m_selection.editable)
                  selectionIndex = i + 1;
               if (i == selectionIndex)
               {
                  size_t p = selectionIndex % vhoHit.size();
                  m_selection = Selection(true, vhoHit[p].m_obj->m_editable);
               }
            }
            // TODO add debug action to make ball active: m_player->m_pactiveballDebug = m_pHitBall;
         }
      }
   }
   if (!io.WantCaptureKeyboard)
   {
      if (ImGui::IsKeyReleased(ImGuiKey_Escape) && m_gizmoOperation != ImGuizmo::NONE)
      {
         // Cancel current operation
         m_gizmoOperation = ImGuizmo::NONE;
      }
      else if (ImGui::IsKeyReleased(ImGuiKey_Escape) && m_selection.type != Selection::S_NONE)
      {
         // Cancel current selection
         m_selection = Selection();
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_F))
      {
         m_flyMode = !m_flyMode;
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_A) && ImGui::GetIO().KeyAlt)
      {
         m_selection = Selection();
      }
      else if (m_useEditorCam && ImGui::IsKeyPressed(ImGuiKey_G))
      {
         // Grab (translate)
         if (io.KeyAlt)
         {
            Matrix3D tmp;
            if (GetSelectionTransform(tmp))
               SetSelectionTransform(tmp, true, false, false);
         }
         else
         {
            m_gizmoOperation = ImGuizmo::TRANSLATE;
            m_gizmoMode = m_gizmoOperation == ImGuizmo::TRANSLATE ? (m_gizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL) : ImGuizmo::WORLD;
         }
      }
      else if (m_useEditorCam && ImGui::IsKeyPressed(ImGuiKey_S))
      {
         // Scale
         if (io.KeyAlt)
         {
            Matrix3D tmp;
            if (GetSelectionTransform(tmp))
               SetSelectionTransform(tmp, false, true, false);
         }
         else
         {
            m_gizmoOperation = ImGuizmo::SCALE;
            m_gizmoMode = m_gizmoOperation == ImGuizmo::SCALE ? (m_gizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL) : ImGuizmo::WORLD;
         }
      }
      else if (m_useEditorCam && ImGui::IsKeyPressed(ImGuiKey_R))
      {
         // Rotate
         if (io.KeyAlt)
         {
            Matrix3D tmp;
            if (GetSelectionTransform(tmp))
               SetSelectionTransform(tmp, false, false, true);
         }
         else
         {
            m_gizmoOperation = ImGuizmo::ROTATE;
            m_gizmoMode = m_gizmoOperation == ImGuizmo::ROTATE ? (m_gizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL) : ImGuizmo::WORLD;
         }
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_Keypad0))
      {
         // Editor toggle play camera / editor camera
         m_useEditorCam = !m_useEditorCam;
         if (m_useEditorCam)
            ResetCameraFromPlayer();
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_Keypad5))
      {
         // Editor toggle orthographic / perspective
         m_useEditorCam = true;
         m_orthoCam = !m_orthoCam;
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_KeypadDecimal))
      {
         // Editor Camera center on selection
         m_useEditorCam = true;
         Matrix3D view(m_camView);
         view.Invert();
         const vec3 up = view.GetOrthoNormalUp(), dir = view.GetOrthoNormalDir(), pos = view.GetOrthoNormalPos();
         const vec3 camTarget = pos - dir * m_camDistance;
         vec3 newTarget = camTarget;
         Matrix3D tmp;
         if (GetSelectionTransform(tmp))
            newTarget = vec3(tmp._41, tmp._42, tmp._43);
         const vec3 newEye = newTarget + dir * m_camDistance;
         m_camView = Matrix3D::MatrixLookAtRH(newEye, newTarget, up);
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_Keypad7))
      {
         // Editor Camera to Top / Bottom
         m_useEditorCam = true;
         m_orthoCam = true;
         Matrix3D view(m_camView);
         view.Invert();
         const vec3 /*up = view.GetOrthoNormalUp(),*/ dir = view.GetOrthoNormalDir(), pos = view.GetOrthoNormalPos();
         const vec3 camTarget = pos - dir * m_camDistance;
         constexpr vec3 newUp{0.f, -1.f, 0.f};
         const vec3 newDir(0.f, 0.f, ImGui::GetIO().KeyCtrl ? 1.f : -1.f);
         const vec3 newEye = camTarget + newDir * m_camDistance;
         m_camView = Matrix3D::MatrixLookAtRH(newEye, camTarget, newUp);
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_Keypad1))
      {
         // Editor Camera to Front / Back
         m_useEditorCam = true;
         m_orthoCam = true;
         Matrix3D view(m_camView);
         view.Invert();
         const vec3 /*up = view.GetOrthoNormalUp(),*/ dir = view.GetOrthoNormalDir(), pos = view.GetOrthoNormalPos();
         const vec3 camTarget = pos - dir * m_camDistance;
         constexpr vec3 newUp{0.f, 0.f, -1.f};
         const vec3 newDir(0.f, ImGui::GetIO().KeyCtrl ? -1.f : 1.f, 0.f);
         const vec3 newEye = camTarget + newDir * m_camDistance;
         m_camView = Matrix3D::MatrixLookAtRH(newEye, camTarget, newUp);
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_Keypad3))
      {
         // Editor Camera to Right / Left
         m_useEditorCam = true;
         m_orthoCam = true;
         Matrix3D view(m_camView);
         view.Invert();
         const vec3 /*up = view.GetOrthoNormalUp(),*/ dir = view.GetOrthoNormalDir(), pos = view.GetOrthoNormalPos();
         const vec3 camTarget = pos - dir * m_camDistance;
         constexpr vec3 newUp{0.f, 0.f, -1.f};
         const vec3 newDir(ImGui::GetIO().KeyCtrl ? 1.f : -1.f, 0.f, 0.f);
         const vec3 newEye = camTarget + newDir * m_camDistance;
         m_camView = Matrix3D::MatrixLookAtRH(newEye, camTarget, newUp);
      }
   }

   const Matrix3D RH2LH = Matrix3D::MatrixScale(1.f,  1.f, -1.f);
   const Matrix3D YAxis = Matrix3D::MatrixScale(1.f, -1.f, 1.f);
   if (m_useEditorCam)
   {
      // Apply editor camera to renderer (move view/projection from right handed to left handed)
      // Convert from right handed (ImGuizmo view manipulate is right handed) to VPX's left handed coordinate system
      // Right Hand to Left Hand (note that RH2LH = inverse(RH2LH), so RH2LH.RH2LH is identity, which property is used below)
      const Matrix3D view = RH2LH * m_camView * YAxis;
      const Matrix3D proj = YAxis * m_camProj;
      m_renderer->GetMVP().SetView(view);
      m_renderer->GetMVP().SetProj(0, proj);
      m_renderer->GetMVP().SetProj(1, proj);
   }
   else
   {
      m_renderer->InitLayout();
      m_camView = RH2LH * m_renderer->GetMVP().GetView() * YAxis;
      m_camProj = YAxis * m_renderer->GetMVP().GetProj(0);
   }

   if (m_selection != previousSelection)
   {
      if ((previousSelection.type == Selection::S_EDITABLE) && (previousSelection.editable->GetIHitable() != nullptr) && (previousSelection.editable->GetItemType() != eItemBall))
         m_player->m_physics->SetStatic(previousSelection.editable);
      if ((m_selection.type == Selection::S_EDITABLE) && (m_selection.editable->GetIHitable() != nullptr) && (m_selection.editable->GetItemType() != eItemBall))
         m_player->m_physics->SetDynamic(m_selection.editable);
   }
   if (m_selection.type == Selection::S_EDITABLE)
   {
      switch (m_selection.editable->GetItemType())
      {
      case eItemBall: m_renderer->ReinitRenderable(static_cast<Ball *>(m_selection.editable)); break;
      case eItemBumper: m_renderer->ReinitRenderable(static_cast<Bumper *>(m_selection.editable)); break;
      case eItemDecal: m_renderer->ReinitRenderable(static_cast<Decal *>(m_selection.editable)); break;
      case eItemFlasher: m_renderer->ReinitRenderable(static_cast<Flasher *>(m_selection.editable)); break;
      case eItemFlipper: m_renderer->ReinitRenderable(static_cast<Flipper *>(m_selection.editable)); break;
      case eItemGate: m_renderer->ReinitRenderable(static_cast<Gate *>(m_selection.editable)); break;
      case eItemHitTarget: m_renderer->ReinitRenderable(static_cast<HitTarget *>(m_selection.editable)); break;
      case eItemKicker: m_renderer->ReinitRenderable(static_cast<Kicker *>(m_selection.editable)); break;
      case eItemLight: m_renderer->ReinitRenderable(static_cast<Light *>(m_selection.editable)); break;
      case eItemPlunger: m_renderer->ReinitRenderable(static_cast<Plunger *>(m_selection.editable)); break;
      case eItemPrimitive: m_renderer->ReinitRenderable(static_cast<Primitive *>(m_selection.editable)); break;
      case eItemRamp: m_renderer->ReinitRenderable(static_cast<Ramp *>(m_selection.editable)); break;
      case eItemRubber: m_renderer->ReinitRenderable(static_cast<Rubber *>(m_selection.editable)); break;
      case eItemSpinner: m_renderer->ReinitRenderable(static_cast<Spinner *>(m_selection.editable)); break;
      case eItemSurface: m_renderer->ReinitRenderable(static_cast<Surface *>(m_selection.editable)); break;
      case eItemTextbox: m_renderer->ReinitRenderable(static_cast<Textbox *>(m_selection.editable)); break;
      case eItemTrigger: m_renderer->ReinitRenderable(static_cast<Trigger *>(m_selection.editable)); break;
      default: break;
      }
      if ((m_selection.editable->GetIHitable() != nullptr) && (m_selection.editable->GetItemType() != eItemBall))
         m_player->m_physics->Update(m_selection.editable);
   }
}

bool EditorUI::GetSelectionTransform(Matrix3D& transform) const
{
   if (m_selection.type == EditorUI::Selection::SelectionType::S_EDITABLE)
   switch (m_selection.editable->GetItemType())
   {
   case eItemBall:
   {
      const Ball *const ball = static_cast<Ball *>(m_selection.editable);
      transform = Matrix3D::MatrixTranslate(ball->m_hitBall.m_d.m_pos);
      return true;
   }
   case eItemBumper:
   {
      const Bumper *const p = static_cast<Bumper *>(m_selection.editable);
      const float height = (m_selection.is_live ? m_live_table : m_table)->GetSurfaceHeight(p->m_d.m_szSurface, p->m_d.m_vCenter.x, p->m_d.m_vCenter.y);
      transform = Matrix3D::MatrixTranslate(p->m_d.m_vCenter.x, p->m_d.m_vCenter.y, height);
      return true;
   }
   case eItemFlasher:
   {
      const Flasher *const p = static_cast<Flasher *>(m_selection.editable);
      const Matrix3D trans = Matrix3D::MatrixTranslate(p->m_d.m_vCenter.x, p->m_d.m_vCenter.y, p->m_d.m_height);
      const Matrix3D rotx = Matrix3D::MatrixRotateX(ANGTORAD(p->m_d.m_rotX));
      const Matrix3D roty = Matrix3D::MatrixRotateY(ANGTORAD(p->m_d.m_rotY));
      const Matrix3D rotz = Matrix3D::MatrixRotateZ(ANGTORAD(p->m_d.m_rotZ));
      transform = rotz * roty * rotx * trans;
      return true;
   }
   case eItemFlipper:
   {
      const Flipper *const f = static_cast<Flipper *>(m_selection.editable);
      const float height = (m_selection.is_live ? m_live_table : m_table)->GetSurfaceHeight(f->m_d.m_szSurface, f->m_d.m_Center.x, f->m_d.m_Center.y);
      transform = Matrix3D::MatrixTranslate(f->m_d.m_Center.x, f->m_d.m_Center.y, height);
      return true;
   }
   case eItemLight:
   {
      const Light *const l = static_cast<Light *>(m_selection.editable);
      const float height = (m_selection.is_live ? m_live_table : m_table)->GetSurfaceHeight(l->m_d.m_szSurface, l->m_d.m_vCenter.x, l->m_d.m_vCenter.y);
      transform = Matrix3D::MatrixTranslate(l->m_d.m_vCenter.x, l->m_d.m_vCenter.y, height + l->m_d.m_height);
      return true;
   }
   case eItemPrimitive:
   {
      const Primitive *const p = static_cast<Primitive *>(m_selection.editable);
      const Matrix3D Smatrix = Matrix3D::MatrixScale(p->m_d.m_vSize.x, p->m_d.m_vSize.y, p->m_d.m_vSize.z);
      const Matrix3D Tmatrix = Matrix3D::MatrixTranslate(p->m_d.m_vPosition.x, p->m_d.m_vPosition.y, p->m_d.m_vPosition.z);
      const Matrix3D Rmatrix = (Matrix3D::MatrixRotateZ(ANGTORAD(p->m_d.m_aRotAndTra[2]))
                              * Matrix3D::MatrixRotateY(ANGTORAD(p->m_d.m_aRotAndTra[1])))
                              * Matrix3D::MatrixRotateX(ANGTORAD(p->m_d.m_aRotAndTra[0]));
      transform = (Smatrix * Rmatrix) * Tmatrix; // fullMatrix = Scale * Rotate * Translate
      return true;
   }
   case eItemSurface:
   {
      const Surface *const obj = static_cast<Surface *>(m_selection.editable);
      Vertex2D center = obj->GetPointCenter();
      transform = Matrix3D::MatrixTranslate(center.x, center.y, 0.5f * (obj->m_d.m_heightbottom + obj->m_d.m_heighttop));
      return true;
   }
   }

   return false;
}

void EditorUI::SetSelectionTransform(const Matrix3D &newTransform, bool clearPosition, bool clearScale, bool clearRotation)
{
   Matrix3D transform = newTransform;
   const Vertex3Ds right(transform._11, transform._12, transform._13);
   const Vertex3Ds up(transform._21, transform._22, transform._23);
   const Vertex3Ds dir(transform._31, transform._32, transform._33);
   float xscale = right.Length();
   float yscale = up.Length();
   float zscale = dir.Length();
   transform._11 /= xscale; // Normalize transform to evaluate rotation
   transform._12 /= xscale;
   transform._13 /= xscale;
   transform._21 /= yscale;
   transform._22 /= yscale;
   transform._23 /= yscale;
   transform._31 /= zscale;
   transform._32 /= zscale;
   transform._33 /= zscale;
   if (clearScale)
      xscale = yscale = zscale = 1.f;

   float posX = transform._41;
   float posY = transform._42;
   float posZ = transform._43;
   if (clearPosition)
      posX = posY = posZ = 0.f;

   // Derived from https://learnopencv.com/rotation-matrix-to-euler-angles/
   float rotX, rotY, rotZ;
   const float sy = sqrtf(transform._11 * transform._11 + transform._21 * transform._21);
   if (sy > 1e-6f)
   {
      rotX = -RADTOANG(atan2f(transform._32, transform._33));
      rotY = -RADTOANG(atan2f(-transform._31, sy));
      rotZ = -RADTOANG(atan2f(transform._21, transform._11));
   }
   else
   {
      rotX = -RADTOANG(atan2f(transform._23, transform._22));
      rotY = -RADTOANG(atan2f(-transform._22, sy));
      rotZ = 0.f;
   }
   if (clearRotation)
      rotX = rotY = rotZ = 0.f;

   if (m_selection.type == EditorUI::Selection::SelectionType::S_EDITABLE)
   switch (m_selection.editable->GetItemType())
   {
   case eItemBall:
   {
      Ball *const ball = static_cast<Ball *>(m_selection.editable); //!! dynamic_cast and below
      ball->m_hitBall.m_d.m_pos.x = posX;
      ball->m_hitBall.m_d.m_pos.y = posY;
      ball->m_hitBall.m_d.m_pos.z = posZ;
      break;
   }
   case eItemBumper:
   {
      Bumper *const f = static_cast<Bumper *>(m_selection.editable);
      const float px = f->m_d.m_vCenter.x, py = f->m_d.m_vCenter.y;
      f->Translate(Vertex2D{posX - px, posY - py});
      break;
   }
   case eItemFlasher:
   {
      Flasher *const p = static_cast<Flasher *>(m_selection.editable);
      const float px = p->m_d.m_vCenter.x, py = p->m_d.m_vCenter.y;
      p->TranslatePoints(Vertex2D{posX - px, posY - py});
      p->put_Height(posZ);
      p->put_RotX(rotX);
      p->put_RotY(rotY);
      p->put_RotZ(rotZ);
      break;
   }
   case eItemFlipper:
   {
      Flipper *const f = static_cast<Flipper *>(m_selection.editable);
      const float px = f->m_d.m_Center.x, py = f->m_d.m_Center.y;
      f->Translate(Vertex2D{posX - px, posY - py});
      break;
   }
   case eItemLight:
   {
      Light *const l = static_cast<Light *>(m_selection.editable);
      const float height = (m_selection.is_live ? m_live_table : m_table)->GetSurfaceHeight(l->m_d.m_szSurface, l->m_d.m_vCenter.x, l->m_d.m_vCenter.y);
      const float px = l->m_d.m_vCenter.x, py = l->m_d.m_vCenter.y, pz = height + l->m_d.m_height;
      l->Translate(Vertex2D{posX - px, posY - py});
      l->m_d.m_height += posZ - pz;
      l->m_d.m_bulbHaloHeight += posZ - pz;
      break;
   }
   case eItemPrimitive:
   {
      Primitive *const p = static_cast<Primitive *>(m_selection.editable);
      p->m_d.m_vPosition.x = posX;
      p->m_d.m_vPosition.y = posY;
      p->m_d.m_vPosition.z = posZ;
      p->m_d.m_aRotAndTra[0] = rotX;
      p->m_d.m_aRotAndTra[1] = rotY;
      p->m_d.m_aRotAndTra[2] = rotZ;
      p->m_d.m_vSize.x = xscale;
      p->m_d.m_vSize.y = yscale;
      p->m_d.m_vSize.z = zscale;
      break;
   }
   case eItemSurface:
   {
      Surface *const obj = static_cast<Surface *>(m_selection.editable);
      Vertex2D center = obj->GetPointCenter();
      const float px = center.x, py = center.y, pz = 0.5f * (obj->m_d.m_heightbottom + obj->m_d.m_heighttop);
      obj->TranslatePoints(Vertex2D{posX - px, posY - py});
      obj->m_d.m_heightbottom += posZ - pz;
      obj->m_d.m_heighttop += posZ - pz;
      break;
   }
   }
}

bool EditorUI::IsOutlinerFiltered(const string& name) const
{
   if (m_outlinerFilter.empty())
      return true;
   const string name_lcase = lowerCase(name);
   const string filter_lcase = lowerCase(m_outlinerFilter);
   return name_lcase.find(filter_lcase) != std::string::npos;
}

void EditorUI::UpdateOutlinerUI()
{
   if (m_table && m_table->IsLocked())
      return;
   const ImGuiViewport * const viewport = ImGui::GetMainViewport();
   const float pane_width = 200.f * m_liveUI.GetDPI();
   ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + m_menubar_height + m_toolbar_height));
   ImGui::SetNextWindowSize(ImVec2(pane_width, viewport->Size.y - m_menubar_height - m_toolbar_height));
   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
      | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f * m_liveUI.GetDPI(), 4.0f * m_liveUI.GetDPI()));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
   ImGui::Begin("OUTLINER", nullptr, window_flags);

   ImGui::InputTextWithHint("Filter", "Name part filter", &m_outlinerFilter);

   if (ImGui::BeginTabBar("Startup/Live", ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton))
   {
      for (int tab = 0; tab < 2; tab++)
      {
         const bool is_live = (tab == 1);
         PinTable * const table = is_live ? m_live_table : m_table;
         if (ImGui::BeginTabItem(is_live ? "Live" : "Startup", nullptr, (is_live && m_outlinerSelectLiveTab) ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
         {
            if (is_live)
               m_outlinerSelectLiveTab = false;
            if (ImGui::TreeNodeEx("View Setups"))
            {
               if (ImGui::Selectable("Live Editor Camera"))
               {
                  m_selection.type = Selection::SelectionType::S_NONE;
                  m_useEditorCam = true;
               }
               Selection cam0(Selection::SelectionType::S_CAMERA, is_live, 0);
               if (ImGui::Selectable("Desktop", m_selection == cam0))
               {
                  m_selection = cam0;
                  m_useEditorCam = false;
                  table->m_BG_override = BG_DESKTOP;
                  table->UpdateCurrentBGSet();
               }
               Selection cam1(Selection::SelectionType::S_CAMERA, is_live, 1);
               if (ImGui::Selectable("Cabinet", m_selection == cam1))
               {
                  m_selection = cam1;
                  m_useEditorCam = false;
                  table->m_BG_override = BG_FULLSCREEN;
                  table->UpdateCurrentBGSet();
               }
               Selection cam2(Selection::SelectionType::S_CAMERA, is_live, 2);
               if (ImGui::Selectable("Full Single Screen", m_selection == cam2))
               {
                  m_selection = cam2;
                  m_useEditorCam = false;
                  table->m_BG_override = BG_FSS;
                  table->UpdateCurrentBGSet();
               }
               ImGui::TreePop();
            }
            if (ImGui::TreeNode("Materials"))
            {
               const std::function<string(Material *)> map = [](Material *image) -> string { return image->m_name; };
               for (Material *&material : SortedCaseInsensitive(table->m_materials, map))
               {
                  Selection sel(is_live, material);
                  if (IsOutlinerFiltered(material->m_name) && ImGui::Selectable(material->m_name.c_str(), m_selection == sel))
                     m_selection = sel;
               }
               ImGui::TreePop();
            }
            if (ImGui::TreeNode("Images"))
            {
               const std::function<string(Texture *)> map = [](Texture *image) -> string { return image->m_name; };
               for (Texture *&image : SortedCaseInsensitive(table->m_vimage, map))
               {
                  Selection sel(is_live, image);
                  if (IsOutlinerFiltered(image->m_name) && ImGui::Selectable(image->m_name.c_str(), m_selection == sel))
                     m_selection = sel;
               }
               ImGui::TreePop();
            }
            if (ImGui::TreeNode("Render Probes"))
            {
               for (RenderProbe *probe : table->m_vrenderprobe)
               {
                  Selection sel(is_live, probe);
                  if (ImGui::Selectable(probe->GetName().c_str(), m_selection == sel))
                     m_selection = sel;
               }
               ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("Layers", ImGuiTreeNodeFlags_DefaultOpen))
            {
               // Sort by path (this is heavy but simple, ensuring part groups appear before their children as well as stable alphabetical ordering)
               vector<IEditable*> editables = table->m_vedit;
               std::ranges::sort(editables, [](IEditable *a, IEditable *b) { return a->GetPathString(false) < b->GetPathString(false); });
               // Live objects are the one created at runtime, not parented to any group
               if (is_live && ImGui::TreeNode("Live Objects"))
               {
                  for (IEditable* edit : editables)
                  {
                     if (edit->GetPartGroup() == nullptr && edit->GetItemType() != eItemPartGroup)
                     {
                        Selection sel(is_live, edit);
                        if (IsOutlinerFiltered(edit->GetName()) && ImGui::Selectable(edit->GetName().c_str(), m_selection == sel))
                           m_selection = sel;
                     }
                  }
                  ImGui::TreePop();
               }
               // Table definition parts
               struct Node
               {
                  PartGroup* group;
                  bool opened;
               };
               vector<Node> stack;
               for (IEditable* edit : editables)
               {
                  const PartGroup* parent = edit->GetPartGroup();
                  if ((parent == nullptr) && (edit->GetItemType() != eItemPartGroup))
                     continue;
                  while (!stack.empty() && ((parent == nullptr) || !edit->IsChild(stack.back().group)))
                  {
                     if (stack.back().opened)
                        ImGui::TreePop();
                     stack.pop_back();
                  }
                  if (edit->GetItemType() == eItemPartGroup)
                  {
                     stack.push_back({
                        static_cast<PartGroup*>(edit), 
                        (stack.empty() || stack.back().opened) ? ImGui::TreeNodeEx(edit->GetName().c_str(), ImGuiTreeNodeFlags_None) : false});
                  }
                  else if (stack.back().opened)
                  {
                     Selection sel(is_live, edit);
                     if (IsOutlinerFiltered(edit->GetName()) && ImGui::Selectable(edit->GetName().c_str(), m_selection == sel))
                        m_selection = sel;
                  }
               }
               while (!stack.empty())
               {
                  if (stack.back().opened)
                     ImGui::TreePop();
                  stack.pop_back();
               }
               ImGui::TreePop();
            }
            ImGui::EndTabItem();
         }
      }
      ImGui::EndTabBar();
   }
   m_outliner_width = ImGui::GetWindowWidth();
   ImGui::End();
   ImGui::PopStyleVar(3);
}

void EditorUI::UpdatePropertyUI()
{
   if (m_table && m_table->IsLocked())
      return;
   const ImGuiViewport *const viewport = ImGui::GetMainViewport();
   const float pane_width = 250.f * m_liveUI.GetDPI();
   ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + viewport->Size.x - pane_width, viewport->Pos.y + m_menubar_height + m_toolbar_height));
   ImGui::SetNextWindowSize(ImVec2(pane_width, viewport->Size.y - m_menubar_height - m_toolbar_height));
   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
      | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f * m_liveUI.GetDPI(), 4.0f * m_liveUI.GetDPI()));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
   ImGui::Begin("PROPERTIES", nullptr, window_flags);
   ImGui::PushItemWidth(PROP_WIDTH);

   if (m_selection.type == Selection::S_IMAGE)
      ImageProperties();
   else if (ImGui::BeginTabBar("Startup/Live", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton))
   {
      for (int tab = 0; tab < 2; tab++)
      {
         const bool is_live = (tab == 1);
         if (ImGui::BeginTabItem(is_live ? "Live" : "Startup", nullptr, (is_live && m_propertiesSelectLiveTab) ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
         {
            if (is_live)
               m_propertiesSelectLiveTab = false;
            ImGui::NewLine();
            switch (m_selection.type)
            {
            case Selection::SelectionType::S_NONE: TableProperties(is_live); break; // Use header tab for live since table is displayed when there is no selection
            case Selection::SelectionType::S_CAMERA: CameraProperties(is_live); break;
            case Selection::SelectionType::S_MATERIAL: MaterialProperties(is_live); break;
            case Selection::SelectionType::S_RENDERPROBE: RenderProbeProperties(is_live); break;
            case Selection::SelectionType::S_EDITABLE:
            {
               const bool is_live_selected = m_selection.is_live;
               IEditable *live_obj = (IEditable *)(is_live_selected ? m_selection.editable : m_live_table->m_startupToLive[m_selection.editable]);
               IEditable *startup_obj = (IEditable *)(is_live_selected ? m_live_table->m_liveToStartup[m_selection.editable] : m_selection.editable);
               assert(live_obj == nullptr || std::find(m_live_table->m_vedit.begin(), m_live_table->m_vedit.end(), live_obj) != m_live_table->m_vedit.end());
               assert(startup_obj == nullptr || std::find(m_table->m_vedit.begin(), m_table->m_vedit.end(), startup_obj) != m_table->m_vedit.end());
               if ((is_live && live_obj == nullptr) || (!is_live && startup_obj == nullptr))
               {
                  m_liveUI.CenteredText("No Object"s);
               }
               else
               {
                  HelpEditableHeader(is_live, startup_obj, live_obj);
                  switch (m_selection.editable->GetItemType())
                  {
                  // eItemFlipper, eItemTimer, eItemPlunger, eItemTextbox, eItemDecal, eItemGate, eItemSpinner, eItemTable,
                  // eItemLightCenter, eItemDragPoint, eItemCollection, eItemDispReel, eItemLightSeq, eItemHitTarget,
                  case eItemBall: BallProperties(is_live, (Ball *)startup_obj, (Ball *)live_obj); break;
                  case eItemBumper: BumperProperties(is_live, (Bumper *)startup_obj, (Bumper *)live_obj); break;
                  case eItemFlasher: FlasherProperties(is_live, (Flasher *)startup_obj, (Flasher *)live_obj); break;
                  case eItemKicker: KickerProperties(is_live, (Kicker *)startup_obj, (Kicker *)live_obj); break;
                  case eItemLight: LightProperties(is_live, (Light *)startup_obj, (Light *)live_obj); break;
                  case eItemPrimitive: PrimitiveProperties(is_live, (Primitive *)startup_obj, (Primitive *)live_obj); break;
                  case eItemSurface: SurfaceProperties(is_live, (Surface *)startup_obj, (Surface *)live_obj); break;
                  case eItemRamp: RampProperties(is_live, (Ramp *)startup_obj, (Ramp *)live_obj); break;
                  case eItemRubber: RubberProperties(is_live, (Rubber *)startup_obj, (Rubber *)live_obj); break;
                  case eItemTrigger: TriggerProperties(is_live, (Trigger *)startup_obj, (Trigger *)live_obj); break;
                  default: break;
                  }
               }
               break;
            }
            default: break;
            }
            ImGui::EndTabItem();
         }
      }
      ImGui::EndTabBar();
   }
   ImGui::PopItemWidth();
   m_properties_width = ImGui::GetWindowWidth();
   ImGui::End();
   ImGui::PopStyleVar(3);
}

void EditorUI::UpdateVideoOptionsModal()
{
   bool popup_anaglyph_calibration = false;
   bool p_open = true;
   if (ImGui::BeginPopupModal(ID_VIDEO_SETTINGS, &p_open, ImGuiWindowFlags_AlwaysAutoResize))
   {
      if (ImGui::CollapsingHeader("Anti-Aliasing", ImGuiTreeNodeFlags_DefaultOpen))
      {
         const char *postprocessed_aa_items[] = { "Disabled", "Fast FXAA", "Standard FXAA", "Quality FXAA", "Fast NFAA", "Standard DLAA", "Quality SMAA" };
         if (ImGui::Combo("Postprocessed AA", &m_renderer->m_FXAA, postprocessed_aa_items, IM_ARRAYSIZE(postprocessed_aa_items)))
            g_pvp->m_settings.SaveValue(Settings::Player, "FXAA"s, m_renderer->m_FXAA);
         const char *sharpen_items[] = { "Disabled", "CAS", "Bilateral CAS" };
         if (ImGui::Combo("Sharpen", &m_renderer->m_sharpen, sharpen_items, IM_ARRAYSIZE(sharpen_items)))
            g_pvp->m_settings.SaveValue(Settings::Player, "Sharpen"s, m_renderer->m_sharpen);
      }
      
      if (ImGui::CollapsingHeader("Performance & Troubleshooting", ImGuiTreeNodeFlags_DefaultOpen))
      {
         if (ImGui::Checkbox("Force Tonemapping off on HDR display", &m_renderer->m_HDRforceDisableToneMapper))
            g_pvp->m_settings.SaveValue(Settings::Player, "HDRDisableToneMapper"s, m_renderer->m_HDRforceDisableToneMapper);
         if (ImGui::Checkbox("Force Bloom filter off", &m_renderer->m_bloomOff))
            g_pvp->m_settings.SaveValue(Settings::Player, "ForceBloomOff"s, m_renderer->m_bloomOff);
         if (ImGui::Checkbox("Force Motion blur off", &m_renderer->m_motionBlurOff))
            g_pvp->m_settings.SaveValue(Settings::Player, "ForceMotionBlurOff"s, m_renderer->m_motionBlurOff);
      }
      
      if (ImGui::CollapsingHeader("Ball Rendering", ImGuiTreeNodeFlags_DefaultOpen))
      {
         bool antiStretch = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "BallAntiStretch"s, false);
         if (ImGui::Checkbox("Force round ball", &antiStretch))
         {
            g_pvp->m_settings.SaveValue(Settings::Player, "BallAntiStretch"s, antiStretch);
            for (auto ball : m_player->m_vball)
               m_renderer->ReinitRenderable(ball->m_pBall);
         }
      }

      if (m_renderer->m_stereo3D != STEREO_VR && ImGui::CollapsingHeader("3D Stereo Output", ImGuiTreeNodeFlags_DefaultOpen))
      {
         if (ImGui::Checkbox("Enable stereo rendering", &m_renderer->m_stereo3Denabled))
            g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3DEnabled"s, m_renderer->m_stereo3Denabled);
         if (m_renderer->m_stereo3Denabled)
         {
            m_renderer->UpdateStereoShaderState();
            bool modeChanged = false;
            const char *stereo_output_items[] = { "Disabled", "3D TV", "Anaglyph" };
            int stereo_mode = m_renderer->m_stereo3D == STEREO_OFF ? 0 : Is3DTVStereoMode(m_renderer->m_stereo3D) ? 1 : 2;
            int tv_mode = Is3DTVStereoMode(m_renderer->m_stereo3D) ? (int)m_renderer->m_stereo3D - STEREO_TB : 0;
            int glassesIndex = IsAnaglyphStereoMode(m_renderer->m_stereo3D) ? (m_renderer->m_stereo3D - STEREO_ANAGLYPH_1) : 0;
            if (stereo_mode == 0) // Stereo mode may not be activated if the player was started without it (wen can only change between the stereo modes)
               ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            if (ImGui::Combo("Stereo Output", &stereo_mode, stereo_output_items, IM_ARRAYSIZE(stereo_output_items)))
               modeChanged = true;
            if (stereo_mode == 0)
               ImGui::PopItemFlag();
            if (stereo_mode != 0) // Stereo settings
            {
               // The renderer does not support switching between fake/real stereo 
               // ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
               // ImGui::Checkbox("Use Fake Stereo", &m_renderer->m_stereo3DfakeStereo);
               // ImGui::PopItemFlag();
               ImGui::TextUnformatted(m_renderer->m_stereo3DfakeStereo ? "Renderer uses 'fake' stereo from single render" : "Renderer performs real stereo rendering");
               if (m_renderer->m_stereo3DfakeStereo)
               {
                  float stereo3DEyeSep = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DMaxSeparation"s, 0.03f);
                  if (ImGui::InputFloat("Max Separation", &stereo3DEyeSep, 0.001f, 0.01f, "%.3f"))
                     g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3DMaxSeparation"s, stereo3DEyeSep);
                  bool stereo3DY = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DYAxis"s, false);
                  if (ImGui::Checkbox("Use Y axis", &stereo3DY))
                     g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3DYAxis"s, stereo3DY);
               }
               else
               {
                  int stereo3DEyeSep = (int)g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DEyeSeparation"s, 63.0f);
                  if (ImGui::InputInt("Eye Separation (mm)", &stereo3DEyeSep, 1, 5))
                     g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3DEyeSeparation"s, (float)stereo3DEyeSep);
               }
            }
            if (stereo_mode == 1) // 3D TV
            {
               const char *tv_mode_items[] = { "Top / Bottom", "Interlaced", "Flipped Interlaced", "Side by Side" };
               if (ImGui::Combo("TV type", &tv_mode, tv_mode_items, IM_ARRAYSIZE(tv_mode_items)))
                  modeChanged = true;
            }
            else if (stereo_mode == 2) // Anaglyph
            {
               // Global anaglyph settings
               float anaglyphSaturation = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DSaturation"s, 1.f);
               if (ImGui::InputFloat("Saturation", &anaglyphSaturation, 0.01f, 0.1f))
                  g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3DSaturation"s, anaglyphSaturation);
               float anaglyphBrightness = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DBrightness"s, 1.f);
               if (ImGui::InputFloat("Brightness", &anaglyphBrightness, 0.01f, 0.1f))
                  g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3DBrightness"s, anaglyphBrightness);
               float anaglyphLeftEyeContrast = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DLeftContrast"s, 1.f);
               if (ImGui::InputFloat("Left Eye Contrast", &anaglyphLeftEyeContrast, 0.01f, 0.1f))
                  g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3DLeftContrast"s, anaglyphLeftEyeContrast);
               float anaglyphRightEyeContrast = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DRightContrast"s, 1.f);
               if (ImGui::InputFloat("Right Eye Contrast", &anaglyphRightEyeContrast, 0.01f, 0.1f))
                  g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3DRightContrast"s, anaglyphRightEyeContrast);
               float anaglyphDefocus = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DDefocus"s, 0.f);
               if (ImGui::InputFloat("Lesser Eye Defocus", &anaglyphDefocus, 0.01f, 0.1f))
                  g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3DDefocus"s, anaglyphDefocus);

               // Glasses settings
               static const string defaultNames[] = { "Red/Cyan"s, "Green/Magenta"s, "Blue/Amber"s, "Cyan/Red"s, "Magenta/Green"s, "Amber/Blue"s, "Custom 1"s, "Custom 2"s, "Custom 3"s, "Custom 4"s };
               string name[std::size(defaultNames)];
               for (size_t i = 0; i < std::size(defaultNames); i++)
                  if (!g_pvp->m_settings.LoadValue(Settings::Player, "Anaglyph"s.append(std::to_string(i + 1)).append("Name"s), name[i]))
                     name[i] = defaultNames[i];
               const char *glasses_items[] = { name[0].c_str(),name[1].c_str(),name[2].c_str(),name[3].c_str(),name[4].c_str(),name[5].c_str(),name[6].c_str(),name[7].c_str(),name[8].c_str(),name[9].c_str(), };
               if (ImGui::Combo("Glasses", &glassesIndex, glasses_items, IM_ARRAYSIZE(glasses_items)))
                  modeChanged = true;
               const string prefKey = "Anaglyph" + std::to_string(glassesIndex + 1);

               if (ImGui::InputText("Name", &name[glassesIndex]))
                  g_pvp->m_settings.SaveValue(Settings::Player, prefKey + "Name", name[glassesIndex]);
               static const char *filter_items[] = { "None", "Dubois", "Luminance", "Deghost" };
               int anaglyphFilter = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, prefKey + "Filter", 4);
               if (ImGui::Combo("Filter", &anaglyphFilter, filter_items, IM_ARRAYSIZE(filter_items)))
                  g_pvp->m_settings.SaveValue(Settings::Player, prefKey + "Filter", anaglyphFilter);
               float anaglyphDynDesat = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, prefKey + "DynDesat", 1.f);
               if (ImGui::InputFloat("Dyn. Desaturation", &anaglyphDynDesat, 0.01f, 0.1f))
                  g_pvp->m_settings.SaveValue(Settings::Player, prefKey + "DynDesat", anaglyphDynDesat);
               float anaglyphDeghost = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, prefKey + "Deghost", 0.f);
               if (ImGui::InputFloat("Deghost", &anaglyphDeghost, 0.01f, 0.1f))
                  g_pvp->m_settings.SaveValue(Settings::Player, prefKey + "Deghost", anaglyphDeghost);
               bool srgbDisplay = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, prefKey + "sRGB", true);
               if (ImGui::Checkbox("Calibrated sRGB Display", &srgbDisplay))
                  g_pvp->m_settings.SaveValue(Settings::Player, prefKey + "sRGB", srgbDisplay);

               Anaglyph anaglyph;
               anaglyph.LoadSetupFromRegistry(glassesIndex);
               const vec3 eyeL = anaglyph.GetLeftEyeColor(false), eyeR = anaglyph.GetRightEyeColor(false);
               Anaglyph::AnaglyphPair colors = anaglyph.GetColorPair();
               ImGui::ColorButton("LeftFilter", ImVec4(eyeL.x, eyeL.y, eyeL.z, 1.f), ImGuiColorEditFlags_NoAlpha);
               ImGui::SameLine();
               ImGui::ColorButton("RightFilter", ImVec4(eyeR.x, eyeR.y, eyeR.z, 1.f), ImGuiColorEditFlags_NoAlpha);
               ImGui::SameLine();
               ImGui::TextUnformatted(anaglyph.IsReversedColorPair() ? colors == Anaglyph::RED_CYAN ? "Cyan/Red" : colors == Anaglyph::GREEN_MAGENTA ? "Magenta/Green" : "Amber/Blue"
                                                                     : colors == Anaglyph::RED_CYAN ? "Red/Cyan" : colors == Anaglyph::GREEN_MAGENTA ? "Green/Magenta" : "Blue/Amber");
               ImGui::SameLine();
               ImGui::Text("Gamma %3.2f", anaglyph.GetDisplayGamma());
               ImGui::SameLine();
               vec3 anaglyphRatio = anaglyph.GetAnaglyphRatio();
               ImGui::Text(" Ratios R: %2d%% G: %2d%% B: %2d%%", (int)(100.f * anaglyphRatio.x), (int)(100.f * anaglyphRatio.y), (int)(100.f * anaglyphRatio.z));

               if (ImGui::Button("Calibrate Glasses"))
                  popup_anaglyph_calibration = true;
               ImGui::SameLine();
               if (ImGui::Button("Reset to default"))
               {
                  g_pvp->m_settings.DeleteValue(Settings::Player, prefKey + "Name");
                  g_pvp->m_settings.DeleteValue(Settings::Player, prefKey + "LeftRed");
                  g_pvp->m_settings.DeleteValue(Settings::Player, prefKey + "LeftGreen");
                  g_pvp->m_settings.DeleteValue(Settings::Player, prefKey + "LeftBlue");
                  g_pvp->m_settings.DeleteValue(Settings::Player, prefKey + "RightRed");
                  g_pvp->m_settings.DeleteValue(Settings::Player, prefKey + "RightGreen");
                  g_pvp->m_settings.DeleteValue(Settings::Player, prefKey + "RightBlue");
               }
            }
            if (modeChanged)
            {
               StereoMode mode = STEREO_OFF;
               if (stereo_mode == 1)
                  mode = (StereoMode)(STEREO_TB + tv_mode);
               if (stereo_mode == 2)
                  mode = (StereoMode)(STEREO_ANAGLYPH_1 + glassesIndex);
               g_pvp->m_settings.SaveValue(Settings::Player, "Stereo3D"s, (int)mode);
               if (m_renderer->m_stereo3D != STEREO_OFF && mode != STEREO_OFF) // TODO allow live switching stereo on/off
                  m_renderer->m_stereo3D = mode;
            }
         }
      }
      ImGui::EndPopup();
   }
   if (popup_anaglyph_calibration && IsAnaglyphStereoMode(g_pvp->m_settings.LoadValueInt(Settings::Player, "Stereo3D"s)))
      ImGui::OpenPopup(ID_ANAGLYPH_CALIBRATION);
}

void EditorUI::UpdateAnaglyphCalibrationModal()
{
   int glassesIndex = m_renderer->m_stereo3D - STEREO_ANAGLYPH_1;
   if (glassesIndex < 0 || glassesIndex > 9)
      return;
   static float backgroundOpacity = 1.f;
   const ImGuiIO& io = ImGui::GetIO();
   ImGui::SetNextWindowSize(io.DisplaySize);
   ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.f, 0.f, 0.f, backgroundOpacity));
   if (ImGui::BeginPopupModal(ID_ANAGLYPH_CALIBRATION, nullptr, (ImGuiWindowFlags_)((int)ImGuiWindowFlags_NoTitleBar | (int)ImGuiNextWindowDataFlags_HasBgAlpha)))
   {
      m_renderer->UpdateStereoShaderState();
      const string prefKey = "Anaglyph" + std::to_string(glassesIndex + 1);
      static int calibrationStep = -1;
      static float calibrationBrightness = 0.5f;
      static const string fields[] = { "LeftRed"s, "LeftGreen"s, "LeftBlue"s, "RightRed"s, "RightGreen"s, "RightBlue"s, };
      // use the right setting for the selected glasses (corresponding to their name)
      if (calibrationStep == -1)
      {
         calibrationStep = 0; 
         calibrationBrightness = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, prefKey + fields[calibrationStep], 0.f);
      }
      if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_LeftCtrl))
      {
         if (calibrationStep == 0)
         {
            calibrationStep = -1;
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            ImGui::OpenPopup(ID_VIDEO_SETTINGS);
            ImGui::PopStyleColor();
            return;
         }
         else
         {
            calibrationStep--;
            calibrationBrightness = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, prefKey + fields[calibrationStep], 0.f);
         }
      }
      else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_RightCtrl))
      {
         if (calibrationStep >= 5)
         {
            calibrationStep = -1;
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            ImGui::OpenPopup(ID_VIDEO_SETTINGS);
            ImGui::PopStyleColor();
            return;
         }
         else
         {
            calibrationStep++;
            calibrationBrightness = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, prefKey + fields[calibrationStep], 0.f);
         }
      }
      else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_LeftShift))
      {
         calibrationBrightness = clamp(calibrationBrightness - 0.01f, 0.f, 1.f);
         g_pvp->m_settings.SaveValue(Settings::Player, prefKey + fields[calibrationStep], calibrationBrightness);
      }
      else if (ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_RightShift))
      {
         calibrationBrightness = clamp(calibrationBrightness + 0.01f, 0.f, 1.f);
         g_pvp->m_settings.SaveValue(Settings::Player, prefKey + fields[calibrationStep], calibrationBrightness);
      }

      ImGui::PushFont(m_liveUI.GetOverlayFont(), m_liveUI.GetOverlayFont()->LegacySize);
      const ImVec2 win_size = ImGui::GetWindowSize();
      ImDrawList *draw_list = ImGui::GetWindowDrawList();
      const float s = min(win_size.x, win_size.y) / 5.f, t = 1.f * s;
      /* Initial implementation based on MBD calibration
      draw_list->AddRectFilled(ImVec2(0.5f * win_size.x - t, 0.5f * win_size.y - t), ImVec2(0.5f * win_size.x + t, 0.5f * win_size.y + t),
         ImColor(calibrationBrightness, calibrationBrightness, calibrationBrightness));
      for (int x = 0; x < 2; x++)
      {
         for (int y = 0; y < 2; y++)
         {
            if ((x & 1) != (y & 1))
            {
               ImVec2 pos(0.5f * win_size.x - t + s * x, 0.5f * win_size.y - t + s * y);
               draw_list->AddRectFilled(pos, pos + ImVec2(s, s),ImColor((calibrationStep % 3) == 0 ? 1.f : 0.f, (calibrationStep % 3) == 1 ? 1.f : 0.f, (calibrationStep % 3) == 2 ? 1.f : 0.f));
            }
         }
      }*/
      // Perform calibration using a human face, see https://people.cs.uchicago.edu/~glk/pubs/pdf/Kindlmann-FaceBasedLuminanceMatching-VIS-2002.pdf
      static constexpr int faceLength[] = {9, 4, 7, 5, 5, 4, 4, 5, 4, 4, 4, 5, 4};
      static constexpr ImVec2 face[] = {
         ImVec2( 96.5f, 86.9f), ImVec2( 17.6f,-48.1f), ImVec2(  7.5f, -1.3f), ImVec2( 13.1f, -0.8f), ImVec2( 19.8f,  0.3f), ImVec2(22.5f,  1.6f), ImVec2(-14.2f,51.9f), ImVec2(-25.7f,14.2f), ImVec2(-16.8f,1.1f),
         ImVec2(176.9f, 38.5f), ImVec2( 37.7f, 25.7f), ImVec2( -7.8f, 33.7f), ImVec2(-44.1f, -7.5f),
         ImVec2(120.3f,105.6f), ImVec2(-14.7f, 39.8f), ImVec2( -1.3f, 13.6f), ImVec2( 16.6f,  1.3f), ImVec2( 24.6f,-35.0f), ImVec2( 0.8f,-15.5f), ImVec2( -9.1f,-5.3f),
         ImVec2(120.8f,160.4f), ImVec2( 20.8f, 11.5f), ImVec2( 68.7f,-10.4f), ImVec2(-19.2f,-27.3f), ImVec2(-45.7f, -8.8f),
         ImVec2(116.8f,171.1f), ImVec2( -3.2f, 16.0f), ImVec2(-24.9f, -0.5f), ImVec2( -2.3f,-12.5f), ImVec2(  9.7f, -6.0f),
         ImVec2(116.8f,171.1f), ImVec2( 24.9f,  0.8f), ImVec2( 12.8f, 22.2f), ImVec2(-40.9f, -6.9f),
         ImVec2(141.6f,171.9f), ImVec2( 68.7f,-10.4f), ImVec2(-20.3f, 40.6f), ImVec2(-35.5f, -8.0f),
         ImVec2(154.5f,194.0f), ImVec2( -3.5f,  8.8f), ImVec2(  3.7f, 30.2f), ImVec2( 25.1f,-10.2f), ImVec2( 10.2f,-20.8f),
         ImVec2(151.0f,202.9f), ImVec2(-23.3f, -2.4f), ImVec2(  2.9f, 43.0f), ImVec2( 24.0f,-10.4f),
         ImVec2( 89.0f,194.8f), ImVec2( 38.8f,  5.6f), ImVec2(  2.9f, 43.0f), ImVec2(-38.0f, -5.3f),
         ImVec2(191.1f,134.2f), ImVec2(  5.3f,-19.0f), ImVec2( 17.1f, -1.1f), ImVec2( -3.2f, 47.3f),
         ImVec2( 74.6f,151.9f), ImVec2(  7.9f,-23.8f), ImVec2( 26.5f, -3.0f), ImVec2(-12.9f, 42.9f), ImVec2( -9.7f,  6.0f),
         ImVec2(65.5f, 148.9f), ImVec2(  6.8f,-38.6f), ImVec2( 10.6f, -0.8f), ImVec2( -0.4f, 18.5f),
      };
      ImColor backCol(calibrationBrightness, calibrationBrightness, calibrationBrightness);
      ImColor calCol((calibrationStep % 3) == 0 ? 1.f : 0.f, (calibrationStep % 3) == 1 ? 1.f : 0.f, (calibrationStep % 3) == 2 ? 1.f : 0.f);
      for (int v = 0; v < 2; v++)
      {
         ImVec2 faceTrans[10], faceOffset(win_size.x * 0.5f - 0.5f * t + (float)v * t, win_size.y * 0.5f);
         draw_list->AddRectFilled(ImVec2(0.5f * win_size.x - t + (float)v * t, 0.5f * win_size.y - t), ImVec2(0.5f * win_size.x + (float)v * t, 0.5f * win_size.y + t), v == 0 ? backCol : calCol);
         ImU32 col = ImGui::GetColorU32(v == 1 ? backCol.Value : calCol.Value);
         for (int i = 0, p = 0; i < 13; p += faceLength[i], i++)
         {
            ImVec2 pos(0.f, 0.f);
            for (int j = 0; j < faceLength[i]; j++)
            {
               pos = pos + face[p + j];
               PLOGD << pos.x << ", " << pos.y;
               faceTrans[j] = faceOffset + (pos + ImVec2(-140.f, -140.f)) * 2.f * t / 320.f;
            }
            draw_list->AddConvexPolyFilled(faceTrans, faceLength[i], col);
         }
      }

      float line_height = ImGui::CalcTextSize("A").y * 1.2f;
      #define CENTERED_TEXT(y, t) ImGui::SetCursorPos(ImVec2((win_size.x - ImGui::CalcTextSize(t).x) * 0.5f, y));ImGui::TextUnformatted(t);
      float y = win_size.y * 0.5f + t + line_height;
      string step_info = "Anaglyph glasses calibration step #"s.append(std::to_string(calibrationStep + 1)).append("/6");
      CENTERED_TEXT(y + 0 * line_height, step_info.c_str());
      step_info = (calibrationStep < 3 ? "Left eye's "s : "Right eye's "s).append((calibrationStep % 3) == 0 ? "red"s : (calibrationStep % 3) == 1 ? "green"s : "blue"s).append(" perceived luminance: "s).append(std::to_string((int)(calibrationBrightness * 100.f))).append(1,'%');
      CENTERED_TEXT(y + 1 * line_height, step_info.c_str());
      CENTERED_TEXT(y + 3 * line_height, calibrationStep < 3 ? "Close your right eye" : "Close your left eye");
      CENTERED_TEXT(y + 5 * line_height, calibrationStep == 0 ? "Use Left Control to exit calibration" : "Use Left Control to move to previous step");
      CENTERED_TEXT(y + 6 * line_height, calibrationStep == 5 ? "Use Right Control to exit calibration" : "Use Right Control to move to next step");
      CENTERED_TEXT(y + 7 * line_height, "Use Left/Right Shift to adjust face brightness until");
      CENTERED_TEXT(y + 8 * line_height, "your eye does not favor or focus one face over the other.");
      ImGui::PopFont();
      line_height = ImGui::CalcTextSize("A").y * 1.2f;
      y = win_size.y * 0.5f - t - 3.f * line_height;
      CENTERED_TEXT(y - line_height, "Background Opacity");
      ImGui::SetCursorPos(ImVec2((win_size.x - 1.5f * t) * 0.5f, y));
      ImGui::SetNextItemWidth(1.5f * t);
      ImGui::SliderFloat("##Background Opacity", &backgroundOpacity, 0.f, 1.f);
      ImGui::EndPopup();
      #undef CENTERED_TEXT
   }
   ImGui::PopStyleColor();
}

void EditorUI::UpdatePlumbWindow()
{
   // TODO table or global settings ?
   bool open = true;
   if (ImGui::BeginPopupModal(ID_PLUMB_SETTINGS, &open, ImGuiWindowFlags_AlwaysAutoResize))
   {
      // Physical accelerator settings
      bool accEnabled = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWEnabled"s, true);
      if (ImGui::Checkbox("Acc. Enabled", &accEnabled))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "PBWEnabled"s, accEnabled);
         m_player->m_pininput.ReInit();
      }
      ImGui::BeginDisabled(!accEnabled);
      int accMax[] = { m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelMaxX"s, 100), m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelMaxY"s, 100) };
      if (ImGui::InputInt2("Acc. Maximum", accMax))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "PBWAccelMaxX"s, accMax[0]);
         g_pvp->m_settings.SaveValue(Settings::Player, "PBWAccelMaxY"s, accMax[1]);
         m_player->m_pininput.ReInit();
      }
      int accGain[] = { m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelGainX"s, 150), m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelGainY"s, 150) };
      if (ImGui::InputInt2("Acc. Gain", accGain))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "PBWAccelGainX"s, accGain[0]);
         g_pvp->m_settings.SaveValue(Settings::Player, "PBWAccelGainY"s, accGain[1]);
         m_player->m_pininput.ReInit();
      }
      int accSensitivity = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "NudgeSensitivity"s, 500);
      if (ImGui::InputInt("Acc. Sensitivity", &accSensitivity))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "NudgeSensitivity"s, accSensitivity);
         m_player->m_pininput.ReInit();
      }
      bool accOrientationEnabled = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWRotationCB"s, false); // TODO Legacy stuff => remove and only keep rotation
      int accOrientation = accOrientationEnabled ? m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWRotationValue"s, 500) : 0;
      if (ImGui::InputInt("Acc. Rotation", &accOrientation))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "PBWRotationCB"s, accOrientation != 0);
         g_pvp->m_settings.SaveValue(Settings::Player, "PBWRotationValue"s, accOrientation);
         m_player->m_pininput.ReInit();
      }
      bool accFaceUp = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "PBWNormalMount"s, true);
      if (ImGui::Checkbox("Acc. Face Up", &accFaceUp))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "PBWNormalMount"s, accFaceUp);
         m_player->m_pininput.ReInit();
      }
      bool accFilter = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "EnableNudgeFilter"s, false);
      if (ImGui::Checkbox("Acc. Filter", &accFilter))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "EnableNudgeFilter"s, accFilter);
         m_player->m_pininput.ReInit();
      }
      ImGui::EndDisabled();

      ImGui::Separator();

      // Tilt plumb settings
      bool enablePlumbTilt = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "TiltSensCB"s, false);
      if (ImGui::Checkbox("Enable virtual Tilt plumb", &enablePlumbTilt))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "TiltSensCB"s, enablePlumbTilt);
         m_player->m_physics->ReadNudgeSettings(m_live_table->m_settings);
      }
      ImGui::BeginDisabled(!enablePlumbTilt);
      float plumbTiltThreshold = (float)m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "TiltSensitivity"s, 400) * (float)(45. / 1000.);
      if (ImGui::InputFloat("Tilt threshold angle", &plumbTiltThreshold, 0.1f, 1.0f, "%.1f"))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "TiltSensitivity"s, static_cast<int>(round(plumbTiltThreshold * (float)(1000. / 45.))));
         m_player->m_physics->ReadNudgeSettings(m_live_table->m_settings);
      }
      float plumbTiltMass = m_live_table->m_settings.LoadValueWithDefault(Settings::Player, "TiltInertia"s, 100.f);
      if (ImGui::InputFloat("Tilt inertia factor", &plumbTiltMass, 1.f, 10.f, "%.1f"))
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "TiltInertia"s, plumbTiltMass);
         m_player->m_physics->ReadNudgeSettings(m_live_table->m_settings);
      }
      ImGui::EndDisabled();

      ImGui::Separator();

      ImGui::TextUnformatted("Nudge & Plumb State");
      constexpr int panelSize = 100;
      if (ImGui::BeginTable("PlumbInfo", 2, ImGuiTableFlags_Borders))
      {
         ImGui::TableSetupColumn("Col1", ImGuiTableColumnFlags_WidthFixed, panelSize * m_liveUI.GetDPI());
         ImGui::TableSetupColumn("Col2", ImGuiTableColumnFlags_WidthFixed, panelSize * m_liveUI.GetDPI());
      
         ImGui::TableNextColumn();
         ImGui::TextUnformatted("Cab. Sensor");
         ImGui::TableNextColumn();
         ImGui::TextUnformatted("Plumb Position");
         ImGui::TableNextRow();

         const ImVec2 fullSize = ImVec2(panelSize * m_liveUI.GetDPI(), panelSize * m_liveUI.GetDPI());
         const ImVec2 halfSize = fullSize * 0.5f;
         ImGui::TableNextColumn();
         {
            ImGui::BeginChild("Sensor", fullSize);
            const ImVec2 &pos = ImGui::GetWindowPos();
            ImGui::GetWindowDrawList()->AddLine(pos + ImVec2(0.f, halfSize.y), pos + ImVec2(fullSize.x, halfSize.y), IM_COL32_WHITE);
            ImGui::GetWindowDrawList()->AddLine(pos + ImVec2(halfSize.x, 0.f), pos + ImVec2(halfSize.y, fullSize.y), IM_COL32_WHITE);
            const Vertex2D &acc = m_player->m_pininput.GetNudge(); // Range: -1..1
            ImVec2 accPos = pos + halfSize + ImVec2(acc.x, acc.y) * halfSize * 2.f + ImVec2(0.5f, 0.5f);
            ImGui::GetWindowDrawList()->AddCircleFilled(accPos, 5.f * m_liveUI.GetDPI(), IM_COL32(255, 0, 0, 255));
            ImGui::EndChild();
         }
         ImGui::TableNextColumn();
         {
            ImGui::BeginDisabled(!enablePlumbTilt);
            ImGui::BeginChild("PlumbPos", fullSize);
            const ImVec2 &pos = ImGui::GetWindowPos();
            ImGui::GetWindowDrawList()->AddLine(pos + ImVec2(0.f, halfSize.y), pos + ImVec2(fullSize.x, halfSize.y), IM_COL32_WHITE);
            ImGui::GetWindowDrawList()->AddLine(pos + ImVec2(halfSize.x, 0.f), pos + ImVec2(halfSize.y, fullSize.y), IM_COL32_WHITE);
            // Tilt circle
            const ImVec2 scale = halfSize * 1.5f;
            const ImVec2 radius = scale * sin(m_player->m_physics->GetPlumbTiltThreshold() * (float)(M_PI * 0.25));
            ImGui::GetWindowDrawList()->AddEllipse(pos + halfSize, radius, IM_COL32(255, 0, 0, 255));
            // Plumb position
            const Vertex3Ds &plumb = m_player->m_physics->GetPlumbPos();
            const ImVec2 plumbPos = pos + halfSize + scale * ImVec2(plumb.x, plumb.y) / m_player->m_physics->GetPlumbPoleLength() + ImVec2(0.5f, 0.5f);
            ImGui::GetWindowDrawList()->AddLine(pos + halfSize, plumbPos, IM_COL32(255, 128, 0, 255));
            ImGui::GetWindowDrawList()->AddCircleFilled(plumbPos, 5.f * m_liveUI.GetDPI(), IM_COL32(255, 0, 0, 255));
            ImGui::EndChild();
            ImGui::EndDisabled();
         }
         ImGui::TableNextRow();

         ImGui::TableNextColumn();
         ImGui::TextUnformatted("Table Acceleration");
         ImGui::TableNextColumn();
         ImGui::TextUnformatted("Plumb Angle");
         ImGui::TableNextRow();

         ImGui::TableNextColumn();
         {
            ImGui::BeginChild("Table Acceleration", fullSize);
            const ImVec2 &pos = ImGui::GetWindowPos();
            ImGui::GetWindowDrawList()->AddLine(pos + ImVec2(0.f, halfSize.y), pos + ImVec2(fullSize.x, halfSize.y), IM_COL32_WHITE);
            ImGui::GetWindowDrawList()->AddLine(pos + ImVec2(halfSize.x, 0.f), pos + ImVec2(halfSize.y, fullSize.y), IM_COL32_WHITE);
            const Vertex3Ds nudge = (float)PHYS_FACTOR * m_player->m_physics->GetNudgeAcceleration(); // Range: -1..1
            const ImVec2 nudgePos = pos + halfSize + ImVec2(nudge.x, nudge.y) * halfSize * 2.f + ImVec2(0.5f, 0.5f);
            ImGui::GetWindowDrawList()->AddCircleFilled(nudgePos, 5.f * m_liveUI.GetDPI(), IM_COL32(255, 0, 0, 255));
            ImGui::EndChild();
         }
         ImGui::TableNextColumn();
         {
            ImGui::BeginDisabled(!enablePlumbTilt);
            ImGui::BeginChild("PlumbAngle", ImVec2(panelSize * m_liveUI.GetDPI(), panelSize * m_liveUI.GetDPI()));
            const ImVec2 &pos = ImGui::GetWindowPos();
            const Vertex3Ds& plumb = m_player->m_physics->GetPlumbPos();
            float radius = min(fullSize.x, fullSize.y) * 0.9f;
            // Tilt limits
            float angle = m_player->m_physics->GetPlumbTiltThreshold() * (float)(M_PI * 0.25);
            ImVec2 plumbPos = pos + ImVec2(halfSize.x + sinf(angle) * radius, cosf(angle) * radius);
            ImGui::GetWindowDrawList()->AddLine(pos + ImVec2(halfSize.x, 0.f), plumbPos, IM_COL32(255, 0, 0, 255));
            plumbPos = pos + ImVec2(halfSize.x - sin(angle) * radius, cos(angle) * radius);
            ImGui::GetWindowDrawList()->AddLine(pos + ImVec2(halfSize.x, 0.f), plumbPos, IM_COL32(255, 0, 0, 255));
            // Plumb position
            angle = atan2f(sqrt(plumb.x * plumb.x + plumb.y * plumb.y), -plumb.z);
            const float theta = atan2(plumb.x, plumb.y);
            if (theta + (float)(M_PI/2.) < 0.f || theta + (float)(M_PI/2.) >= (float)M_PI)
               angle = -angle;
            plumbPos = pos + ImVec2(halfSize.x + sinf(angle) * radius, cosf(angle) * radius);
            ImGui::GetWindowDrawList()->AddLine(pos + ImVec2(halfSize.x, 0.f), plumbPos, IM_COL32(255, 128, 0, 255));
            ImGui::GetWindowDrawList()->AddCircleFilled(plumbPos, 5.f * m_liveUI.GetDPI(), IM_COL32(255, 0, 0, 255));
            ImGui::EndChild();
            ImGui::EndDisabled();
         }
         ImGui::EndTable();
      }
      ImGui::EndPopup();
   }
}

void EditorUI::UpdateRendererInspectionModal()
{
   // FIXME m_renderer->DisableStaticPrePass(false);
   m_useEditorCam = false;
   m_renderer->InitLayout();

   ImGui::SetNextWindowSize(ImVec2(350.f * m_liveUI.GetDPI(), 0));
   if (ImGui::Begin(ID_RENDERER_INSPECTION, &m_showRendererInspection))
   {
      ImGui::TextUnformatted("Display single render pass:");
      static int pass_selection = IF_FPS;
      ImGui::RadioButton("Disabled", &pass_selection, IF_FPS);
      #if defined(ENABLE_DX9) // No GPU profiler for OpenGL or BGFX for the time being
      ImGui::RadioButton("Profiler", &pass_selection, IF_PROFILING);
      #endif
      ImGui::RadioButton("Static prerender pass", &pass_selection, IF_STATIC_ONLY);
      ImGui::RadioButton("Dynamic render pass", &pass_selection, IF_DYNAMIC_ONLY);
      ImGui::RadioButton("Transmitted light pass", &pass_selection, IF_LIGHT_BUFFER_ONLY);
      if (m_player->m_renderer->GetAOMode() != 0)
         ImGui::RadioButton("Ambient Occlusion pass", &pass_selection, IF_AO_ONLY);
      for (size_t i = 0; i < m_table->m_vrenderprobe.size(); i++)
      {
         ImGui::RadioButton(m_table->m_vrenderprobe[i]->GetName().c_str(), &pass_selection, 100 + (int)i);
      }
      if (pass_selection < 100)
         m_player->m_infoMode = (InfoMode)pass_selection;
      else
      {
         m_player->m_infoMode = IF_RENDER_PROBES;
         m_player->m_infoProbeIndex = pass_selection - 100;
      }
      ImGui::NewLine();

      // Latency timing table
      if (ImGui::BeginTable("Latencies", 4, ImGuiTableFlags_Borders))
      {
         const uint32_t period = m_player->m_logicProfiler.GetPrev(FrameProfiler::PROFILE_FRAME);
         ImGui::TableSetupColumn("##Cat", ImGuiTableColumnFlags_WidthFixed);
         ImGui::TableSetupColumn("Min", ImGuiTableColumnFlags_WidthFixed);
         ImGui::TableSetupColumn("Max", ImGuiTableColumnFlags_WidthFixed);
         ImGui::TableSetupColumn("Avg", ImGuiTableColumnFlags_WidthFixed);
         ImGui::TableHeadersRow();
         #define PROF_ROW(name, section) \
         ImGui::TableNextColumn(); ImGui::TextUnformatted(name); \
         ImGui::TableNextColumn(); ImGui::Text("%4.1fms", m_player->m_logicProfiler.GetSlidingMin(section) * 1e-3); \
         ImGui::TableNextColumn(); ImGui::Text("%4.1fms", m_player->m_logicProfiler.GetSlidingMax(section) * 1e-3); \
         ImGui::TableNextColumn(); ImGui::Text("%4.1fms", m_player->m_logicProfiler.GetSlidingAvg(section) * 1e-3);
         PROF_ROW("Input to Script lag", FrameProfiler::PROFILE_INPUT_POLL_PERIOD)
         PROF_ROW("Input to Present lag", FrameProfiler::PROFILE_INPUT_TO_PRESENT)
         #undef PROF_ROW
         ImGui::EndTable();
         ImGui::NewLine();
      }

      ImGui::TextUnformatted("Press F11 to reset min/max/average timings");
      if (ImGui::IsKeyPressed(m_liveUI.GetImGuiKeysFromDIkeycode(m_player->m_rgKeys[eFrameCount])))
         m_player->InitFPS();

      // Other detailed information
      ImGui::TextUnformatted(m_player->GetPerfInfo().c_str());
   }
   ImGui::End();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Property panes
//

void EditorUI::TableProperties(bool is_live)
{
   HelpEditableHeader(is_live, m_table, m_live_table);
   if (ImGui::CollapsingHeader("User", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      auto reinit_lights = [this](bool is_live, float prev, float v) { m_renderer->MarkShaderDirty(); }; // Needed to update shaders with new light settings 
      PropRGB("Ambient Color", m_table, is_live, &(m_table->m_lightAmbient), m_live_table ? &(m_live_table->m_lightAmbient) : nullptr);
      
      PropSeparator();
      PropRGB("Light Em. Color", m_table, is_live, &(m_table->m_Light[0].emission), m_live_table ? &(m_live_table->m_Light[0].emission) : nullptr);
      PropFloat("Light Em. Scale", m_table, is_live, &(m_table->m_lightEmissionScale), m_live_table ? &(m_live_table->m_lightEmissionScale) : nullptr, 20000.0f, 100000.0f, "%.0f", ImGuiInputTextFlags_CharsDecimal, reinit_lights);
      PropFloat("Light Height", m_table, is_live, &(m_table->m_lightHeight), m_live_table ? &(m_live_table->m_lightHeight) : nullptr, 20.0f, 100.0f, "%.0f");
      PropFloat("Light Range", m_table, is_live, &(m_table->m_lightRange), m_live_table ? &(m_live_table->m_lightRange) : nullptr, 200.0f, 1000.0f, "%.0f");
      
      PropSeparator();
      // TODO Missing: environment texture combo
      PropFloat("Environment Em. Scale", m_table, is_live, &(m_table->m_envEmissionScale), m_live_table ? &(m_live_table->m_envEmissionScale) : nullptr, 0.1f, 0.5f, "%.3f", ImGuiInputTextFlags_CharsDecimal, reinit_lights);
      PropFloat("Ambient Occlusion Scale", m_table, is_live, &(m_table->m_AOScale), m_live_table ? &(m_live_table->m_AOScale) : nullptr, 0.1f, 1.0f);
      PropFloat("Bloom Strength", m_table, is_live, &(m_table->m_bloom_strength), m_live_table ? &(m_live_table->m_bloom_strength) : nullptr, 0.1f, 1.0f);
      PropFloat("Screen Space Reflection Scale", m_table, is_live, &(m_table->m_SSRScale), m_live_table ? &(m_live_table->m_SSRScale) : nullptr, 0.1f, 1.0f);
      
      PropSeparator();
      #ifdef ENABLE_BGFX
      static const string tonemapperLabels[] = { "Reinhard"s, "AgX"s, "Filmic"s, "Neutral"s, "AgX Punchy"s };
      #else
      static const string tonemapperLabels[] = { "Reinhard"s, "AgX"s, "Filmic"s, "Neutral"s };
      #endif
      int startup_mode = m_table ? (int)m_table->GetToneMapper() : 0;
      int live_mode = m_live_table ? (int)m_renderer->m_toneMapper : 0;
      PinTable * const table = m_table;
      Player * const player = m_player;
      auto upd_tm = [table, player](bool is_live, int prev, int v)
      {
         if (is_live)
            player->m_renderer->m_toneMapper = (ToneMapper)v;
         else
            table->SetToneMapper((ToneMapper)v);
      };
      PropCombo("Tonemapper", m_table, is_live, &startup_mode, &live_mode, std::size(tonemapperLabels), tonemapperLabels, upd_tm);
      ImGui::EndTable();
   }
}

void EditorUI::CameraProperties(bool is_live)
{
   PinTable *const table = (is_live ? m_live_table : m_table);

   switch (m_selection.camera)
   {
   case 0: ImGui::TextUnformatted("Camera: Desktop"); break;
   case 1: ImGui::TextUnformatted("Camera: Full Single Screen"); break;
   case 2: ImGui::TextUnformatted("Camera: Cabinet"); break;
   default: return; // unsupported
   }
   ImGui::Separator();

   if (ImGui::Button("Import"))
   {
      table->ImportBackdropPOV(string());
      if (is_live)
         m_renderer->MarkShaderDirty();
   }
   ImGui::SameLine();
   if (ImGui::Button("Export"))
      table->ExportBackdropPOV();
   ImGui::NewLine();
   if (BEGIN_PROP_TABLE)
   {
      const ViewSetupID vsId = (ViewSetupID) m_selection.camera;
      static const string layoutModeLabels[] = { "Relative"s, "Absolute"s };
      int startup_mode = m_table ? (int)m_table->mViewSetups[vsId].mMode : 0;
      int live_mode = m_live_table ? (int)m_live_table->mViewSetups[vsId].mMode : 0;
      auto upd_mode = [table, vsId](bool is_live, int prev, int v) { table->mViewSetups[vsId].mMode = (ViewLayoutMode)v; };
      // View
      PropCombo("Layout Mode", m_table, is_live, &startup_mode, &live_mode, std::size(layoutModeLabels), layoutModeLabels, upd_mode);
      PropFloat("Field Of View", m_table, is_live, &(m_table->mViewSetups[vsId].mFOV), m_live_table ? &(m_live_table->mViewSetups[vsId].mFOV) : nullptr, 0.2f, 1.0f);
      PropFloat("Layback", m_table, is_live, &(m_table->mViewSetups[vsId].mLayback), m_live_table ? &(m_live_table->mViewSetups[vsId].mLayback) : nullptr, 0.2f, 1.0f);
      // Player position
      PropFloat("Inclination", m_table, is_live, &(m_table->mViewSetups[vsId].mLookAt), m_live_table ? &(m_live_table->mViewSetups[vsId].mLookAt) : nullptr, 0.2f, 1.0f);
      PropFloat("X Offset", m_table, is_live, &(m_table->mViewSetups[vsId].mViewX), m_live_table ? &(m_live_table->mViewSetups[vsId].mViewX) : nullptr, 10.0f, 50.0f, "%.0f");
      PropFloat("Y Offset", m_table, is_live, &(m_table->mViewSetups[vsId].mViewY), m_live_table ? &(m_live_table->mViewSetups[vsId].mViewY) : nullptr, 10.0f, 50.0f, "%.0f");
      PropFloat("Z Offset", m_table, is_live, &(m_table->mViewSetups[vsId].mViewZ), m_live_table ? &(m_live_table->mViewSetups[vsId].mViewZ) : nullptr, 10.0f, 50.0f, "%.0f");
      // Viewport
      PropFloat("Rotation", m_table, true, &(m_table->mViewSetups[vsId].mViewportRotation), m_live_table ? &(m_live_table->mViewSetups[vsId].mViewportRotation) : nullptr, 90.f, 90.0f, "%.0f");
      // Scene scale
      PropFloat("X Scale", m_table, is_live, &(m_table->mViewSetups[vsId].mSceneScaleX), m_live_table ? &(m_live_table->mViewSetups[vsId].mSceneScaleX) : nullptr, 0.002f, 0.01f);
      PropFloat("Y Scale", m_table, is_live, &(m_table->mViewSetups[vsId].mSceneScaleY), m_live_table ? &(m_live_table->mViewSetups[vsId].mSceneScaleY) : nullptr, 0.002f, 0.01f);
      PropFloat("Z Scale", m_table, is_live, &(m_table->mViewSetups[vsId].mSceneScaleZ), m_live_table ? &(m_live_table->mViewSetups[vsId].mSceneScaleZ) : nullptr, 0.002f, 0.01f);
      ImGui::EndTable();
   }
   ImGui::Separator();
   ImGui::Text("Absolute position:\nX: %.2f  Y: %.2f  Z: %.2f", -m_renderer->GetMVP().GetView()._41,
      (m_selection.camera == 0 || m_selection.camera == 2) ? m_renderer->GetMVP().GetView()._42 : -m_renderer->GetMVP().GetView()._42, 
      m_renderer->GetMVP().GetView()._43);
}

void EditorUI::ImageProperties()
{
   m_liveUI.CenteredText("Image"s);
   string name = m_selection.image->m_name;
   ImGui::BeginDisabled(true); // Editing the name of a live item can break the script
   if (ImGui::InputText("Name", &name))
   {
   }
   ImGui::EndDisabled();
   ImGui::Separator();
   ImGui::BeginDisabled(m_selection.image->GetRawBitmap(false, 0) == nullptr || !m_selection.image->GetRawBitmap(false, 0)->HasAlpha());
   if (ImGui::InputFloat("Alpha Mask", &m_selection.image->m_alphaTestValue))
      m_table->SetNonUndoableDirty(eSaveDirty);
   ImGui::EndDisabled();
   ImGui::Separator();
   ImTextureID image = m_renderer->m_renderDevice->m_texMan.LoadTexture(m_selection.image, false);
   if (image)
   {
      const float w = ImGui::GetWindowWidth();
      ImGui::Image(image, ImVec2(w, static_cast<float>(image->GetHeight()) * w / static_cast<float>(image->GetWidth())));
   }
}

void EditorUI::RenderProbeProperties(bool is_live)
{
   RenderProbe * const live_probe = (RenderProbe *)(m_selection.is_live ? m_selection.renderprobe : m_live_table->m_startupToLive[m_selection.renderprobe]);
   RenderProbe * const startup_probe = (RenderProbe *)(m_selection.is_live ? m_live_table->m_liveToStartup[m_selection.renderprobe] : m_selection.renderprobe);
   m_liveUI.CenteredText("Render Probe"s);
   string name = m_selection.renderprobe->GetName();
   ImGui::BeginDisabled(is_live); // Editing the name of a live item can break the script
   if (ImGui::InputText("Name", &name))
   {
      // FIXME add undo
      if (startup_probe)
         startup_probe->SetName(name);
   }
   ImGui::EndDisabled();
   ImGui::Separator();
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      static const string types[] = { "Reflection"s, "Refraction"s };

      auto upd_normal = [startup_probe, live_probe](bool is_live, vec3& prev, const vec3& v)
      {
         RenderProbe * const probe = (is_live ? live_probe : startup_probe);
         if (probe)
         {
            vec4 plane;
            probe->GetReflectionPlane(plane);
            plane.x = v.x;
            plane.y = v.y;
            plane.z = v.z;
            probe->SetReflectionPlane(plane);
         }
      };
      vec4 startup_plane, live_plane;
      Vertex3Ds startup_normal, live_normal;
      if (startup_probe)
      {
         startup_probe->GetReflectionPlane(startup_plane);
         startup_normal = Vertex3Ds(startup_plane.x, startup_plane.y, startup_plane.z);
      }
      if (live_probe)
      {
         live_probe->GetReflectionPlane(live_plane);
         live_normal = Vertex3Ds(live_plane.x, live_plane.y, live_plane.z);
      }
      PropVec3("Normal", nullptr, is_live, startup_probe ? &startup_normal : nullptr, live_probe ? &live_normal : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal, upd_normal);

      auto upd_distance = [startup_probe, live_probe](bool is_live, float prev, float v)
      {
         RenderProbe * const probe = (is_live ? live_probe : startup_probe);
         if (probe)
         {
            vec4 plane;
            probe->GetReflectionPlane(plane);
            plane.w = v;
            probe->SetReflectionPlane(plane);
         }
      };
      PropFloat("Distance", nullptr, is_live, startup_probe ? &startup_plane.w : nullptr, live_probe ? &live_plane.w : nullptr, 1.f, 10.f, "%.0f", ImGuiInputTextFlags_CharsDecimal, upd_distance);

      ImGui::EndTable();
   }
   ImGui::Separator();
   if (ImGui::CollapsingHeader("Users", ImGuiTreeNodeFlags_DefaultOpen))
   {
      PinTable *const table = is_live ? m_live_table : m_table;
      RenderProbe *const probe = (is_live ? live_probe : startup_probe);
      for (size_t t = 0; t < table->m_vedit.size(); t++)
      {
         ISelect *const psel = table->m_vedit[t]->GetISelect();
         if (psel != nullptr && psel->GetItemType() == eItemPrimitive 
            && ((probe->GetType() == RenderProbe::PLANE_REFLECTION && ((Primitive *)psel)->m_d.m_szReflectionProbe == probe->GetName())
             || (probe->GetType() == RenderProbe::SCREEN_SPACE_TRANSPARENCY  && ((Primitive *)psel)->m_d.m_szRefractionProbe == probe->GetName()))
            && ImGui::Selectable(((Primitive *)psel)->GetName().c_str()))
            m_selection = Selection(is_live, table->m_vedit[t]);
      }
   }
}

void EditorUI::MaterialProperties(bool is_live)
{
   Material * const live_material = (Material *)(m_selection.is_live ? m_selection.editable : m_live_table->m_startupToLive[m_selection.editable]);
   Material * const startup_material = (Material *)(m_selection.is_live ? m_live_table->m_liveToStartup[m_selection.editable] : m_selection.editable);
   Material * const material = (is_live ? live_material : startup_material);
   m_liveUI.CenteredText("Material"s);
   string name = ((Material *)m_selection.editable)->m_name;
   ImGui::BeginDisabled(is_live); // Editing the name of a live item can break the script
   if (ImGui::InputText("Name", &name))
   {
      // FIXME add undo
      if (startup_material)
         startup_material->m_name = name;
   }
   ImGui::EndDisabled();
   ImGui::Separator();
   if (ImGui::CollapsingHeader("Visual", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      static const string matType[] = { "Default"s, "Metal"s };
      PropCombo("Type", m_table, is_live, startup_material ? (int *)&(startup_material->m_type) : nullptr, live_material ? (int *)&(live_material->m_type) : nullptr, std::size(matType), matType);
      if (material != nullptr)
      {
         PropRGB("Base Color", m_table, is_live, startup_material ? &(startup_material->m_cBase) : nullptr, live_material ? &(live_material->m_cBase) : nullptr);
         PropFloat("Wrap Lighting", m_table, is_live, startup_material ? &(startup_material->m_fWrapLighting) : nullptr, live_material ? &(live_material->m_fWrapLighting) : nullptr, 0.02f, 0.1f);
         if (material->m_type != Material::METAL)
         {
            PropRGB("Glossy Color", m_table, is_live, startup_material ? &(startup_material->m_cGlossy) : nullptr, live_material ? &(live_material->m_cGlossy) : nullptr);
            PropFloat("Glossy Image Lerp", m_table, is_live, startup_material ? &(startup_material->m_fGlossyImageLerp) : nullptr, live_material ? &(live_material->m_fGlossyImageLerp) : nullptr, 0.02f, 0.1f);
         }
         PropFloat("Shininess", m_table, is_live, startup_material ? &(startup_material->m_fRoughness) : nullptr, live_material ? &(live_material->m_fRoughness) : nullptr, 0.02f, 0.1f);
         PropRGB("Clearcoat Color", m_table, is_live, startup_material ? &(startup_material->m_cClearcoat) : nullptr, live_material ? &(live_material->m_cClearcoat) : nullptr);
         PropFloat("Edge Brightness", m_table, is_live, startup_material ? &(startup_material->m_fEdge) : nullptr, live_material ? &(live_material->m_fEdge) : nullptr, 0.02f, 0.1f);
      }
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Transparency", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropCheckbox("Enable Transparency", m_table, is_live, startup_material ? &(startup_material->m_bOpacityActive) : nullptr, live_material ? &(live_material->m_bOpacityActive) : nullptr);
      PropFloat("Opacity", m_table, is_live, startup_material ? &(startup_material->m_fOpacity) : nullptr, live_material ? &(live_material->m_fOpacity) : nullptr, 0.02f, 0.1f);
      PropFloat("Edge Opacity", m_table, is_live, startup_material ? &(startup_material->m_fEdgeAlpha) : nullptr, live_material ? &(live_material->m_fEdgeAlpha) : nullptr, 0.02f, 0.1f);
      PropFloat("Thickness", m_table, is_live, startup_material ? &(startup_material->m_fThickness) : nullptr, live_material ? &(live_material->m_fThickness) : nullptr, 0.02f, 0.1f);
      PropRGB("Refraction Tint", m_table, is_live, startup_material ? &(startup_material->m_cRefractionTint) : nullptr, live_material ? &(live_material->m_cRefractionTint) : nullptr);
      ImGui::EndTable();
   }
}

void EditorUI::BallProperties(bool is_live, Ball *startup_obj, Ball *live_obj)
{
   Ball *const ball = (is_live ? live_obj : startup_obj);
   if (ImGui::CollapsingHeader("Visual", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropCheckbox("Visible", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_visible));
      PropCheckbox("Reflection enabled", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_reflectionEnabled));
      PropCheckbox("Reflection forced", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_forceReflection));
      PropCheckbox("Use Table Settings", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_useTableRenderSettings));
      ImGui::BeginDisabled(ball->m_d.m_useTableRenderSettings);
      PropRGB("Color", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_color));
      PropImageCombo("Image", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_szImage), m_table);
      PropCheckbox("Spherical Map", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_pinballEnvSphericalMapping));
      PropImageCombo("Decal", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_imageDecal), m_table);
      PropCheckbox("Decal mode", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_decalMode));
      PropFloat("PF Reflection Strength", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_playfieldReflectionStrength), 0.02f, 0.1f);
      PropFloat("Bulb Intensity Scale", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_bulb_intensity_scale), 0.02f, 0.1f);
      ImGui::EndDisabled();
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropVec3("Position", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_hitBall.m_d.m_pos), "%.0f", ImGuiInputTextFlags_CharsDecimal);
      PropFloat("Radius", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_hitBall.m_d.m_radius), 0.02f, 0.1f);
      PropFloat("Mass", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_hitBall.m_d.m_mass), 0.02f, 0.1f);
      PropVec3("Velocity", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_hitBall.m_d.m_vel), "%.3f", ImGuiInputTextFlags_CharsDecimal);
      PropVec3("Angular Momentum", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_hitBall.m_angularmomentum), "%.3f", ImGuiInputTextFlags_CharsDecimal);
      ImGui::EndTable();
   }
}

void EditorUI::BumperProperties(bool is_live, Bumper *startup_obj, Bumper *live_obj)
{
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropMaterialCombo("Cap Material", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_szCapMaterial), m_table);
      PropMaterialCombo("Base Material", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_szBaseMaterial), m_table);
      PropMaterialCombo("Skirt Material", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_szSkirtMaterial), m_table);
      PropMaterialCombo("Ring Material", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_szRingMaterial), m_table);
      PropFloat("Radius", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_radius), 0.1f, 0.5f, "%.1f");
      PropFloat("Height Scale", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_heightScale), 0.1f, 0.5f, "%.1f");
      PropFloat("Orientation", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_orientation), 0.1f, 0.5f, "%.1f");
      PropFloat("Ring Speed", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_ringSpeed), 0.1f, 0.5f, "%.1f");
      PropFloat("Ring Drop", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_ringDropOffset), 0.1f, 0.5f, "%.1f");
      PropCheckbox("Reflection Enabled", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_reflectionEnabled));
      PropCheckbox("Cap Visible", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_capVisible));
      PropCheckbox("Base Visible", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_baseVisible));
      PropCheckbox("Skirt Visible", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_skirtVisible));
      PropCheckbox("Ring Visible", startup_obj, is_live, PROP_ACCESS(startup_obj, live_obj, m_d.m_ringVisible));
      // Missing position
      ImGui::EndTable();
   }
}

void EditorUI::FlasherProperties(bool is_live, Flasher *startup_obj, Flasher *live_obj)
{
   Flasher *const flasher = (is_live ? live_obj : startup_obj);
   if (flasher == nullptr)
      return;
   if (ImGui::CollapsingHeader("Visual", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropCheckbox("Visible", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_isVisible) : nullptr, live_obj ? &(live_obj->m_d.m_isVisible) : nullptr);
      static const string renderModes[] = { "Flasher"s, "DMD"s, "Display"s, "Alpha.Seg."s };
      PropCombo("Render Mode", m_table, is_live, startup_obj ? (int *)&(startup_obj->m_d.m_renderMode) : nullptr, live_obj ? (int *)&(live_obj->m_d.m_renderMode) : nullptr, std::size(renderModes), renderModes);
      PropRGB("Color", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_color) : nullptr, live_obj ? &(live_obj->m_d.m_color) : nullptr);
      // Missing Tex coord mode
      PropFloat("Depth bias", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_depthBias) : nullptr, live_obj ? &(live_obj->m_d.m_depthBias) : nullptr, 10.f, 100.f);

      if (flasher->m_d.m_renderMode == FlasherData::FLASHER)
      {
         PropImageCombo("Image A", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szImageA) : nullptr, live_obj ? &(live_obj->m_d.m_szImageA) : nullptr, m_table);
         PropImageCombo("Image B", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szImageB) : nullptr, live_obj ? &(live_obj->m_d.m_szImageB) : nullptr, m_table);
         // Missing Mode
         // Missing Filter Image B
         // Missing Amount
      }
      else if (flasher->m_d.m_renderMode == FlasherData::DMD)
      {
         static const string renderStyles[] = { "Legacy VPX"s, "Neon Plasma"s, "Red LED"s, "Green LED"s, "Yellow LED"s, "Generic Plasma"s, "Generic LED"s };
         PropCombo("Render Style", m_table, is_live, startup_obj ? &(startup_obj->m_d.m_renderStyle) : nullptr, live_obj ? &(live_obj->m_d.m_renderStyle) : nullptr, std::size(renderStyles), renderStyles);
         // Missing source
         PropImageCombo("Glass", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szImageA) : nullptr, live_obj ? &(live_obj->m_d.m_szImageA) : nullptr, m_table);
         PropFloat("Glass Roughness", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_glassRoughness) : nullptr, live_obj ? &(live_obj->m_d.m_glassRoughness) : nullptr, 0.f, 5.f);
         PropRGB("Glass Ambient", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_glassAmbient) : nullptr, live_obj ? &(live_obj->m_d.m_glassAmbient) : nullptr);
         PropFloat("Glass Pad Left", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_glassPadLeft) : nullptr, live_obj ? &(live_obj->m_d.m_glassPadLeft) : nullptr, 0.f, 1.f);
         PropFloat("Glass Pad Right", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_glassPadRight) : nullptr, live_obj ? &(live_obj->m_d.m_glassPadRight) : nullptr, 0.f, 1.f);
         PropFloat("Glass Pad Top", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_glassPadTop) : nullptr, live_obj ? &(live_obj->m_d.m_glassPadTop) : nullptr, 0.f, 1.f);
         PropFloat("Glass Pad Bottom", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_glassPadBottom) : nullptr, live_obj ? &(live_obj->m_d.m_glassPadBottom) : nullptr, 0.f, 1.f);
      }
      else if (flasher->m_d.m_renderMode == FlasherData::DISPLAY)
      {
         static const string renderStyles[] = { "Pixelated"s, "Smoothed"s };
         PropCombo("Render Mode", m_table, is_live, startup_obj ? &(startup_obj->m_d.m_renderStyle) : nullptr, live_obj ? &(live_obj->m_d.m_renderStyle) : nullptr, std::size(renderStyles), renderStyles);
         // Missing source
      }
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Transparency", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropInt("Opacity", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_alpha) : nullptr, live_obj ? &(live_obj->m_d.m_alpha) : nullptr);
      PropLightmapCombo("Lightmap", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szLightmap) : nullptr, live_obj ? &(live_obj->m_d.m_szLightmap) : nullptr, m_table);
      PropCheckbox("Additive Blend", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_addBlend) : nullptr, live_obj ? &(live_obj->m_d.m_addBlend) : nullptr);
      PropFloat("Modulate", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_modulate_vs_add) : nullptr, live_obj ? &(live_obj->m_d.m_modulate_vs_add) : nullptr, 0.1f, 0.5f);
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      // FIXME This allows to edit the center but does not update dragpoint coordinates accordingly => add a callback and use Translate
      // FIXME we also need to save dragpoint change when saving x/y to startup table as well as center pos => add a save callback and copy to startup table
      PropVec3("Position", startup_obj, is_live, 
         startup_obj ? &(startup_obj->m_d.m_vCenter.x) : nullptr, startup_obj ? &(startup_obj->m_d.m_vCenter.y) : nullptr, startup_obj ? &(startup_obj->m_d.m_height) : nullptr,
         live_obj    ? &(live_obj   ->m_d.m_vCenter.x) : nullptr, live_obj    ? &(live_obj   ->m_d.m_vCenter.y) : nullptr, live_obj    ? &(live_obj   ->m_d.m_height) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      PropVec3("Rotation", startup_obj, is_live, 
         startup_obj ? &(startup_obj->m_d.m_rotX) : nullptr, startup_obj ? &(startup_obj->m_d.m_rotY) : nullptr, startup_obj ? &(startup_obj->m_d.m_rotZ) : nullptr, 
         live_obj    ? &(live_obj   ->m_d.m_rotX) : nullptr, live_obj    ? &(live_obj   ->m_d.m_rotY) : nullptr, live_obj    ? &(live_obj   ->m_d.m_rotZ) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      ImGui::EndTable();
   }
}

void EditorUI::KickerProperties(bool is_live, Kicker *startup_obj, Kicker *live_obj)
{
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropMaterialCombo("Material", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szMaterial) : nullptr, live_obj ? &(live_obj->m_d.m_szMaterial) : nullptr, m_table);
      static const string shapes[] = { "Invisible"s, "Hole"s, "Cup"s, "Hole Simple"s, "Williams"s, "Gottlieb"s, "Cup 2"s };
      PropCombo("Shape", startup_obj, is_live, startup_obj ? (int *)&(startup_obj->m_d.m_kickertype) : nullptr, live_obj ? (int *)&(live_obj->m_d.m_kickertype) : nullptr, std::size(shapes), shapes);
      PropFloat("Radius", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_radius) : nullptr, live_obj ? &(live_obj->m_d.m_radius) : nullptr, 0.1f, 0.5f, "%.1f");
      PropFloat("Orientation", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_orientation) : nullptr, live_obj ? &(live_obj->m_d.m_orientation) : nullptr, 0.1f, 0.5f, "%.1f");
      // Missing position
      ImGui::EndTable();
   }
}

void EditorUI::LightProperties(bool is_live, Light *startup_light, Light *live_light)
{
   Light *const light = (is_live ? live_light : startup_light);
   if (light && ImGui::CollapsingHeader("Visual", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      auto upd_intensity = [startup_light, live_light, light](bool is_live, float prev, float v)
      {
         if (prev > 0.1f && v > 0.1f)
         {
            const float fade_up_ms = prev / light->m_d.m_fadeSpeedUp;
            light->m_d.m_fadeSpeedUp = fade_up_ms < 0.1f ? 100000.0f : v / fade_up_ms;
            const float fade_down_ms = prev / light->m_d.m_fadeSpeedDown;
            light->m_d.m_fadeSpeedDown = fade_down_ms < 0.1f ? 100000.0f : v / fade_down_ms;
         }
         startup_light->m_currentIntensity = startup_light->m_d.m_intensity * startup_light->m_d.m_intensity_scale * startup_light->m_inPlayState;
         live_light->m_currentIntensity = live_light->m_d.m_intensity * live_light->m_d.m_intensity_scale * live_light->m_inPlayState;
      };
      float startup_fadeup = startup_light ? (startup_light->m_d.m_intensity / startup_light->m_d.m_fadeSpeedUp) : 0.f;
      float live_fadeup = live_light ? (live_light->m_d.m_intensity / live_light->m_d.m_fadeSpeedUp) : 0.f;
      auto upd_fade_up = [light](bool is_live, float prev, float v) { light->m_d.m_fadeSpeedUp = v < 0.1f ? 100000.0f : light->m_d.m_intensity / v;  };
      float startup_fadedown = startup_light ? (startup_light->m_d.m_intensity / startup_light->m_d.m_fadeSpeedDown) : 0.f;
      float live_fadedown = live_light ? (live_light->m_d.m_intensity / live_light->m_d.m_fadeSpeedDown) : 0.f;
      auto upd_fade_down = [light](bool is_live, float prev, float v) { light->m_d.m_fadeSpeedDown = v < 0.1f ? 100000.0f : light->m_d.m_intensity / v; };
      bool startup_shadow = startup_light ? (startup_light->m_d.m_shadows == ShadowMode::RAYTRACED_BALL_SHADOWS) : ShadowMode::NONE;
      bool live_shadow = live_light ? (live_light->m_d.m_shadows == ShadowMode::RAYTRACED_BALL_SHADOWS) : ShadowMode::NONE;
      auto upd_shadow = [light](bool is_live, bool prev, bool v) { light->m_d.m_shadows = v ? ShadowMode::RAYTRACED_BALL_SHADOWS : ShadowMode::NONE; };

      PropSeparator("Light Settings");
      PropFloat("Intensity", startup_light, is_live, startup_light ? &(startup_light->m_d.m_intensity) : nullptr, live_light ? &(live_light->m_d.m_intensity) : nullptr, 0.1f, 1.0f, "%.1f", ImGuiInputTextFlags_CharsDecimal, upd_intensity);
      static const string faders[] = { "None"s, "Linear"s, "Incandescent"s };
      PropCombo("Fader", startup_light, is_live, startup_light ? (int *)&(startup_light->m_d.m_fader) : nullptr, live_light ? (int *)&(live_light->m_d.m_fader) : nullptr, std::size(faders), faders);
      PropFloat("Fade Up (ms)", startup_light, is_live, startup_light ? &startup_fadeup : nullptr, live_light ? &live_fadeup : nullptr, 10.0f, 50.0f, "%.0f", ImGuiInputTextFlags_CharsDecimal, upd_fade_up);
      PropFloat("Fade Down (ms)", startup_light, is_live, startup_light ? &startup_fadedown : nullptr, live_light ? &live_fadedown : nullptr, 10.0f, 50.0f, "%.0f", ImGuiInputTextFlags_CharsDecimal, upd_fade_down);
      PropRGB("Light Color", startup_light, is_live, startup_light ? &(startup_light->m_d.m_color) : nullptr, live_light ? &(live_light->m_d.m_color) : nullptr);
      PropRGB("Center Burst", startup_light, is_live, startup_light ? &(startup_light->m_d.m_color2) : nullptr, live_light ? &(live_light->m_d.m_color2) : nullptr);
      PropFloat("Falloff Range", startup_light, is_live, startup_light ? &(startup_light->m_d.m_falloff) : nullptr, live_light ? &(live_light->m_d.m_falloff) : nullptr, 10.f, 100.f, "%.0f");
      PropFloat("Falloff Power", startup_light, is_live, startup_light ? &(startup_light->m_d.m_falloff_power) : nullptr, live_light ? &(live_light->m_d.m_falloff_power) : nullptr, 0.1f, 0.5f, "%.2f");

      PropSeparator("Render Mode");
      static const string modes[] = { "Hidden"s, "Classic"s, "Halo"s };
      int startup_mode = startup_light ? startup_light->m_d.m_visible ? startup_light->m_d.m_BulbLight ? 2 : 1 : 0 : -1;
      int live_mode = live_light ? live_light->m_d.m_visible ? live_light->m_d.m_BulbLight ? 2 : 1 : 0 : -1;
      auto upd_mode = [light](bool is_live, bool prev, int v) { light->m_d.m_visible = (v != 0); light->m_d.m_BulbLight = (v != 1); };
      PropCombo("Type", startup_light, is_live, startup_mode >= 0 ? &startup_mode : nullptr, live_mode >= 0 ? &live_mode : nullptr, std::size(modes), modes, upd_mode);
      if (!light->m_d.m_visible)
      {
      }
      else if (light->m_d.m_BulbLight)
      {
         PropCheckbox("Reflection Enabled", startup_light, is_live, startup_light ? &(startup_light->m_d.m_reflectionEnabled) : nullptr, live_light ? &(live_light->m_d.m_reflectionEnabled) : nullptr);
         PropFloat("Depth Bias", startup_light, is_live, startup_light ? &(startup_light->m_d.m_depthBias) : nullptr, live_light ? &(live_light->m_d.m_depthBias) : nullptr, 10.f, 50.f, "%.0f");
         PropFloat("Halo Height", startup_light, is_live, startup_light ? &(startup_light->m_d.m_bulbHaloHeight) : nullptr, live_light ? &(live_light->m_d.m_bulbHaloHeight) : nullptr, 1.f, 5.f, "%.1f");
         PropFloat("Modulate", startup_light, is_live, startup_light ? &(startup_light->m_d.m_modulate_vs_add) : nullptr, live_light ? &(live_light->m_d.m_modulate_vs_add) : nullptr, 0.1f, 0.5f, "%.1f");
         PropFloat("Transmission", startup_light, is_live, startup_light ? &(startup_light->m_d.m_transmissionScale) : nullptr, live_light ? &(live_light->m_d.m_transmissionScale) : nullptr, 0.1f, 0.5f, "%.1f");
      }
      else
      {
         PropCheckbox("Reflection Enabled", startup_light, is_live, startup_light ? &(startup_light->m_d.m_reflectionEnabled) : nullptr, live_light ? &(live_light->m_d.m_reflectionEnabled) : nullptr);
         PropFloat("Depth Bias", startup_light, is_live, startup_light ? &(startup_light->m_d.m_depthBias) : nullptr, live_light ? &(live_light->m_d.m_depthBias) : nullptr, 10.f, 50.f, "%.0f");
         PropCheckbox("PassThrough", startup_light, is_live, startup_light ? &(startup_light->m_d.m_imageMode) : nullptr, live_light ? &(live_light->m_d.m_imageMode) : nullptr);
         PropImageCombo("Image", startup_light, is_live, startup_light ? &(startup_light->m_d.m_szImage) : nullptr, live_light ? &(live_light->m_d.m_szImage) : nullptr, m_table);
      }

      PropSeparator("Bulb");
      PropCheckbox("Render bulb", startup_light, is_live, startup_light ? &(startup_light->m_d.m_showBulbMesh) : nullptr, live_light ? &(live_light->m_d.m_showBulbMesh) : nullptr);
      PropCheckbox("Static rendering", startup_light, is_live, startup_light ? &(startup_light->m_d.m_staticBulbMesh) : nullptr, live_light ? &(live_light->m_d.m_staticBulbMesh) : nullptr);
      PropFloat("Bulb Size", startup_light, is_live, startup_light ? &(startup_light->m_d.m_meshRadius) : nullptr, live_light ? &(live_light->m_d.m_meshRadius) : nullptr, 1.0f, 5.0f, "%.0f");

      PropSeparator("Ball reflections & Shadows");
      PropCheckbox("Show Reflection on Balls", startup_light, is_live, startup_light ? &(startup_light->m_d.m_showReflectionOnBall) : nullptr, live_light ? &(live_light->m_d.m_showReflectionOnBall) : nullptr);
      PropCheckbox("Raytraced ball shadows", startup_light, is_live, startup_light ? &startup_shadow : nullptr, live_light ? &live_shadow : nullptr, upd_shadow);

      PropSeparator("Position");
      // FIXME This allows to edit the center but does not update dragpoint coordinates accordingly => add a callback and use Translate
      // FIXME we also need to save dragpoint change when saving x/y to startup table as well as center pos => add a save callback and copy to startup table
      PropFloat("X", startup_light, is_live, startup_light ? &(startup_light->m_d.m_vCenter.x) : nullptr, live_light ? &(live_light->m_d.m_vCenter.x) : nullptr, 0.1f, 0.5f, "%.1f");
      PropFloat("Y", startup_light, is_live, startup_light ? &(startup_light->m_d.m_vCenter.y) : nullptr, live_light ? &(live_light->m_d.m_vCenter.y) : nullptr, 0.1f, 0.5f, "%.1f");
      PropFloat("Z", startup_light, is_live, startup_light ? &(startup_light->m_d.m_height) : nullptr, live_light ? &(live_light->m_d.m_height) : nullptr, 0.1f, 0.5f, "%.1f");

      ImGui::EndTable();
   }
   if (light && ImGui::CollapsingHeader("States", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      auto upd_inplaystate = [startup_light, live_light](bool is_live, float prev, float v)
      {
         Light * const light = (is_live ? live_light : startup_light);
         light->setInPlayState(v > 1.f ? (float)LightStateBlinking : v);
      };
      PropFloat("State", startup_light, is_live, startup_light ? &(startup_light->m_d.m_state) : nullptr, live_light ? &(live_light->m_d.m_state) : nullptr, 0.1f, 0.5f, "%.1f", ImGuiInputTextFlags_CharsDecimal, upd_inplaystate);
      // Missing blink pattern
      PropInt("Blink interval", startup_light, is_live, startup_light ? &(startup_light->m_d.m_blinkinterval) : nullptr, live_light ? &(live_light->m_d.m_blinkinterval) : nullptr);
      ImGui::EndTable();
   }
   PROP_TIMER(is_live, startup_light, live_light)
   if (is_live && ImGui::CollapsingHeader("Live state", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PROP_TABLE_SETUP
      ImGui::BeginDisabled();
      ImGui::TableNextColumn();
      ImGui::InputFloat("Intensity", &live_light->m_currentIntensity);
      ImGui::TableNextColumn();
      ImGui::Button(ICON_SAVE "##t2");
      if (live_light->m_d.m_fader == FADER_INCANDESCENT)
      {
         ImGui::TableNextColumn();
         float temperature = (float)live_light->m_currentFilamentTemperature;
         ImGui::InputFloat("Filament Temperature", &temperature);
         ImGui::TableNextColumn();
         ImGui::Button(ICON_SAVE "##t1");
      }
      ImGui::EndDisabled();
      ImGui::EndTable();
   }
}

void EditorUI::PrimitiveProperties(bool is_live, Primitive *startup_obj, Primitive *live_obj)
{
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropSeparator("Render Options");
      PropCheckbox("Visible", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_visible) : nullptr, live_obj ? &(live_obj->m_d.m_visible) : nullptr);
      PropCheckbox("Static Rendering", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_staticRendering) : nullptr, live_obj ? &(live_obj->m_d.m_staticRendering) : nullptr);
      PropCheckbox("Reflection Enabled", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_reflectionEnabled) : nullptr, live_obj ? &(live_obj->m_d.m_reflectionEnabled) : nullptr);
      PropFloat("Depth Bias", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_depthBias) : nullptr, live_obj ? &(live_obj->m_d.m_depthBias) : nullptr, 10.f, 50.f, "%.0f");
      PropCheckbox("Depth Mask", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_useDepthMask) : nullptr, live_obj ? &(live_obj->m_d.m_useDepthMask) : nullptr);
      PropCheckbox("Render Backfaces", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_backfacesEnabled) : nullptr, live_obj ? &(live_obj->m_d.m_backfacesEnabled) : nullptr);
      PropCheckbox("Additive Blend", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_addBlend) : nullptr, live_obj ? &(live_obj->m_d.m_addBlend) : nullptr);
      PropLightmapCombo("Lightmap", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szLightmap) : nullptr, live_obj ? &(live_obj->m_d.m_szLightmap) : nullptr, m_table);

      PropSeparator("Material");
      PropMaterialCombo("Material", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szMaterial) : nullptr, live_obj ? &(live_obj->m_d.m_szMaterial) : nullptr, m_table);
      PropImageCombo("Image", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szImage) : nullptr, live_obj ? &(live_obj->m_d.m_szImage) : nullptr, m_table);
      PropImageCombo("Normal Map", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szNormalMap) : nullptr, live_obj ? &(live_obj->m_d.m_szNormalMap) : nullptr, m_table);
      PropCheckbox("Object Space NM", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_objectSpaceNormalMap) : nullptr, live_obj ? &(live_obj->m_d.m_objectSpaceNormalMap) : nullptr);
      PropFloat("Disable Spot Lights", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_disableLightingTop) : nullptr, live_obj ? &(live_obj->m_d.m_disableLightingTop) : nullptr, 0.01f, 0.05f, "%.3f");
      PropFloat("Translucency", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_disableLightingBelow) : nullptr, live_obj ? &(live_obj->m_d.m_disableLightingBelow) : nullptr, 0.01f, 0.05f, "%.3f");
      PropFloat("Modulate Opacity", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_alpha) : nullptr, live_obj ? &(live_obj->m_d.m_alpha) : nullptr, 0.01f, 0.05f, "%.3f");
      PropRGB("Modulate Color", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_color) : nullptr, live_obj ? &(live_obj->m_d.m_color) : nullptr);

      PropSeparator("Reflections");
      PropRenderProbeCombo("Reflection Probe", RenderProbe::PLANE_REFLECTION, startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szReflectionProbe) : nullptr, live_obj ? &(live_obj->m_d.m_szReflectionProbe) : nullptr, m_table);
      PropFloat("Reflection strength", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_reflectionStrength) : nullptr, live_obj ? &(live_obj->m_d.m_reflectionStrength) : nullptr, 0.01f, 0.05f, "%.3f");

      PropSeparator("Refractions");
      PropRenderProbeCombo("Refraction Probe", RenderProbe::SCREEN_SPACE_TRANSPARENCY, startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szRefractionProbe) : nullptr, live_obj ? &(live_obj->m_d.m_szRefractionProbe) : nullptr, m_table);
      PropFloat("Refraction thickness", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_refractionThickness) : nullptr, live_obj ? &(live_obj->m_d.m_refractionThickness) : nullptr, 0.01f, 0.05f, "%.3f");
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropSeparator("Position, Rotation & Size");
      PropVec3("Position", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_vPosition) : nullptr, live_obj ? &(live_obj->m_d.m_vPosition) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      PropVec3("Orientation", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_aRotAndTra[0]) : nullptr, live_obj ? &(live_obj->m_d.m_aRotAndTra[0]) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      PropVec3("Scale", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_vSize) : nullptr, live_obj ? &(live_obj->m_d.m_vSize) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      PropSeparator("Additional Transform");
      PropVec3("Translation", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_aRotAndTra[3]) : nullptr, live_obj ? &(live_obj->m_d.m_aRotAndTra[3]) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      PropVec3("Rotation", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_aRotAndTra[6]) : nullptr, live_obj ? &(live_obj->m_d.m_aRotAndTra[6]) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      ImGui::EndTable();
   }
   /* if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      ImGui::EndTable();
   }*/
}

void EditorUI::RampProperties(bool is_live, Ramp *startup_obj, Ramp *live_obj)
{
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      // Missing type
      PropImageCombo("Image", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szImage) : nullptr, live_obj ? &(live_obj->m_d.m_szImage) : nullptr, m_table);
      PropMaterialCombo("Material", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szMaterial) : nullptr, live_obj ? &(live_obj->m_d.m_szMaterial) : nullptr, m_table);
      // Missing World
      PropCheckbox("Apply Image to Wall", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_imageWalls) : nullptr, live_obj ? &(live_obj->m_d.m_imageWalls) : nullptr);
      PropFloat("Depth Bias", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_depthBias) : nullptr, live_obj ? &(live_obj->m_d.m_depthBias) : nullptr, 10.f, 50.f, "%.0f");
      PropCheckbox("Visible", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_visible) : nullptr, live_obj ? &(live_obj->m_d.m_visible) : nullptr);
      PropCheckbox("Reflection Enabled", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_reflectionEnabled) : nullptr, live_obj ? &(live_obj->m_d.m_reflectionEnabled) : nullptr);
      // Missing all dimensions
      ImGui::EndTable();
   }
   /* if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      ImGui::EndTable();
   }*/
}

void EditorUI::RubberProperties(bool is_live, Rubber *startup_obj, Rubber *live_obj)
{
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropImageCombo("Image", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szImage) : nullptr, live_obj ? &(live_obj->m_d.m_szImage) : nullptr, m_table);
      PropMaterialCombo("Material", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szMaterial) : nullptr, live_obj ? &(live_obj->m_d.m_szMaterial) : nullptr, m_table);
      PropCheckbox("Static Rendering", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_staticRendering) : nullptr, live_obj ? &(live_obj->m_d.m_staticRendering) : nullptr);
      PropCheckbox("Visible", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_visible) : nullptr, live_obj ? &(live_obj->m_d.m_visible) : nullptr);
      PropCheckbox("Reflection Enabled", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_reflectionEnabled) : nullptr, live_obj ? &(live_obj->m_d.m_reflectionEnabled) : nullptr);

      PropFloat("Height", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_height) : nullptr, live_obj ? &(live_obj->m_d.m_height) : nullptr, 0.1f, 0.5f, "%.1f");
      PropInt("Thickness", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_thickness) : nullptr, live_obj ? &(live_obj->m_d.m_thickness) : nullptr);
      PropVec3("Rotation", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_rotX) : nullptr, startup_obj ? &(startup_obj->m_d.m_rotY) : nullptr, startup_obj ? &(startup_obj->m_d.m_rotZ) : nullptr,
         live_obj ? &(live_obj->m_d.m_rotX) : nullptr, live_obj ? &(live_obj->m_d.m_rotY) : nullptr, live_obj ? &(live_obj->m_d.m_rotZ) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);

      ImGui::EndTable();
   }
   /* if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      ImGui::EndTable();
   }*/
   PROP_TIMER(is_live, startup_obj, live_obj)
}

void EditorUI::SurfaceProperties(bool is_live, Surface *startup_obj, Surface *live_obj)
{
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropCheckbox("Top Visible", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_topBottomVisible) : nullptr, live_obj ? &(live_obj->m_d.m_topBottomVisible) : nullptr);
      PropImageCombo("Top Image", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szImage) : nullptr, live_obj ? &(live_obj->m_d.m_szImage) : nullptr, m_table);
      PropMaterialCombo("Top Material", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szTopMaterial) : nullptr, live_obj ? &(live_obj->m_d.m_szTopMaterial) : nullptr, m_table);
      PropCheckbox("Side Visible", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_sideVisible) : nullptr, live_obj ? &(live_obj->m_d.m_sideVisible) : nullptr);
      PropImageCombo("Side Image", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szSideImage) : nullptr, live_obj ? &(live_obj->m_d.m_szSideImage) : nullptr, m_table);
      PropMaterialCombo("Side Material", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szSideMaterial) : nullptr, live_obj ? &(live_obj->m_d.m_szSideMaterial) : nullptr, m_table);
      // Missing animate slingshot
      // Missing flipbook
      PropFloat("Disable Spot Lights", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_disableLightingTop) : nullptr, live_obj ? &(live_obj->m_d.m_disableLightingTop) : nullptr, 0.01f, 0.05f, "%.3f");
      PropFloat("Translucency", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_disableLightingBelow) : nullptr, live_obj ? &(live_obj->m_d.m_disableLightingBelow) : nullptr, 0.01f, 0.05f, "%.3f");
      PropCheckbox("Reflection Enabled", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_reflectionEnabled) : nullptr, live_obj ? &(live_obj->m_d.m_reflectionEnabled) : nullptr);
      PropFloat("Top Height", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_heighttop) : nullptr, live_obj ? &(live_obj->m_d.m_heighttop) : nullptr, 0.1f, 0.5f, "%.1f");
      PropFloat("Bottom Height", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_heightbottom) : nullptr, live_obj ? &(live_obj->m_d.m_heightbottom) : nullptr, 0.1f, 0.5f, "%.1f");
      ImGui::EndTable();
   }
   /* if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      ImGui::EndTable();
   }*/
   PROP_TIMER(is_live, startup_obj, live_obj)
}

void EditorUI::TriggerProperties(bool is_live, Trigger *startup_obj, Trigger *live_obj)
{
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropCheckbox("Visible", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_visible) : nullptr, live_obj ? &(live_obj->m_d.m_visible) : nullptr);
      PropCheckbox("Reflection Enabled", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_reflectionEnabled) : nullptr, live_obj ? &(live_obj->m_d.m_reflectionEnabled) : nullptr);
      static const string shapes[] = { "None"s, "Wire A"s, "Star"s, "Wire B"s, "Button"s, "Wire C"s, "Wire D"s, "Inder"s };
      PropCombo("Shape", startup_obj, is_live, startup_obj ? (int *)&(startup_obj->m_d.m_shape) : nullptr, live_obj ? (int *)&(live_obj->m_d.m_shape) : nullptr, std::size(shapes), shapes);
      PropFloat("Wire Thickness", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_wireThickness) : nullptr, live_obj ? &(live_obj->m_d.m_wireThickness) : nullptr, 0.1f, 0.5f, "%.1f");
      PropFloat("Star Radius", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_radius) : nullptr, live_obj ? &(live_obj->m_d.m_radius) : nullptr, 0.1f, 0.5f, "%.1f");
      PropFloat("Rotation", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_rotation) : nullptr, live_obj ? &(live_obj->m_d.m_rotation) : nullptr, 0.1f, 0.5f, "%.1f");
      PropFloat("Anim Speed", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_animSpeed) : nullptr, live_obj ? &(live_obj->m_d.m_animSpeed) : nullptr, 0.1f, 0.5f, "%.1f");
      PropMaterialCombo("Material", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_szMaterial) : nullptr, live_obj ? &(live_obj->m_d.m_szMaterial) : nullptr, m_table);
      // Missing position
      ImGui::EndTable();
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Property field helpers
//

#define PROP_HELPER_BEGIN(type)                                                                                                                                                              \
   PROP_TABLE_SETUP                                                                                                                                                                          \
   type * const v = is_live ? live_v : startup_v;                                                                                                                                            \
   type * const ov = is_live ? startup_v : live_v;                                                                                                                                           \
   ImGui::TableNextColumn();                                                                                                                                                                 \
   if (v == nullptr)                                                                                                                                                                         \
   {                                                                                                                                                                                         \
      /* Missing value just skip */                                                                                                                                                          \
      ImGui::TableNextColumn();                                                                                                                                                              \
      return;                                                                                                                                                                                \
   }                                                                                                                                                                                         \
   ImGui::PushID(label); \
   type prev_v = *v;

#define PROP_HELPER_SYNC(type)                                                                                                                                                               \
   /* Sync button(also show if there are difference between live and startup through the enable state) */                                                                                    \
   ImGui::TableNextColumn();                                                                                                                                                                 \
   if (ov != nullptr)                                                                                                                                                                        \
   {                                                                                                                                                                                         \
      const bool synced = ((*ov) == (*v));                                                                                                                                                   \
      if (synced)                                                                                                                                                                            \
         ImGui::BeginDisabled(); \
      type prev_ov = *ov; \
      if (ImGui::Button(ICON_SAVE)) \
      { \
         *ov = *v; \

/* 
TODO update undo stack instead of SetNonUndoableDirty
psel->GetIEditable()->BeginUndo();
psel->GetIEditable()->MarkForUndo();
// Change value
psel->GetIEditable()->EndUndo();
psel->GetIEditable()->SetDirtyDraw();
*/

#define PROP_HELPER_END                                                                                                                                                                      \
      if (is_live) \
         m_table->SetNonUndoableDirty(eSaveDirty); \
   }                                                                                                                                                                                         \
   if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))                                                                                                                                   \
   {                                                                                                                                                                                         \
      ImGui::BeginTooltip();                                                                                                                                                                 \
      ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);                                                                                                                                  \
      ImGui::Text("Copy this value to the %s version", is_live ? "startup" : "live");                                                                                                        \
      ImGui::PopTextWrapPos();                                                                                                                                                               \
      ImGui::EndTooltip();                                                                                                                                                                   \
   }                                                                                                                                                                                         \
   if (synced)                                                                                                                                                                               \
      ImGui::EndDisabled();                                                                                                                                                                  \
   }                                                                                                                                                                                         \
   ImGui::PopID();

void EditorUI::PropSeparator(const char *label)
{
   PROP_TABLE_SETUP
   ImGui::TableNextColumn();
   if (label)
      ImGui::TextUnformatted(label);
   ImGui::TableNextColumn();
}

void EditorUI::PropCheckbox(const char *label, IEditable *undo_obj, bool is_live, bool *startup_v, bool *live_v, const OnBoolPropChange &chg_callback)
{
   PROP_HELPER_BEGIN(bool)
   if (ImGui::Checkbox(label, v))
   {
      if (chg_callback)
         chg_callback(is_live, prev_v, *v);
      if (!is_live)
         m_table->SetNonUndoableDirty(eSaveDirty);
   }
   PROP_HELPER_SYNC(bool)
   if (chg_callback)
      chg_callback(!is_live, prev_ov, *ov);
   PROP_HELPER_END
}

void EditorUI::PropFloat(const char *label, IEditable* undo_obj, bool is_live, float *startup_v, float *live_v, float step, float step_fast, const char *format, ImGuiInputTextFlags flags, const OnFloatPropChange &chg_callback)
{
   PROP_HELPER_BEGIN(float)
   if (ImGui::InputFloat(label, v, step, step_fast, format, flags))
   {
      if (chg_callback)
         chg_callback(is_live, prev_v, *v);
      if (!is_live)
         m_table->SetNonUndoableDirty(eSaveDirty);
   }
   PROP_HELPER_SYNC(float)
   if (chg_callback)
      chg_callback(!is_live, prev_ov, *ov);
   PROP_HELPER_END
}

void EditorUI::PropInt(const char *label, IEditable *undo_obj, bool is_live, int *startup_v, int *live_v)
{
   PROP_HELPER_BEGIN(int)
   if (ImGui::InputInt(label, v))
   {
      if (!is_live)
         m_table->SetNonUndoableDirty(eSaveDirty);
   }
   PROP_HELPER_SYNC(int)
   PROP_HELPER_END
}

void EditorUI::PropRGB(const char *label, IEditable *undo_obj, bool is_live, COLORREF *startup_v, COLORREF *live_v, ImGuiColorEditFlags flags)
{
   PROP_HELPER_BEGIN(COLORREF)
   float col[3];
   col[0] = (float)((*v) & 255) * (float)(1.0 / 255.0);
   col[1] = (float)((*v) & 65280) * (float)(1.0 / 65280.0);
   col[2] = (float)((*v) & 16711680) * (float)(1.0 / 16711680.0);
   if (ImGui::ColorEdit3(label, col, flags))
   {
      const int r = clamp((int)(col[0] * 255.f + 0.5f), 0, 255);
      const int g = clamp((int)(col[1] * 255.f + 0.5f), 0, 255);
      const int b = clamp((int)(col[2] * 255.f + 0.5f), 0, 255);
      *v = RGB(r, g, b);
      if (!is_live)
         m_table->SetNonUndoableDirty(eSaveDirty);
   }
   PROP_HELPER_SYNC(COLORREF)
   PROP_HELPER_END
}

void EditorUI::PropVec3(const char *label, IEditable *undo_obj, bool is_live, float *startup_x, float *startup_y, float *startup_z, float *live_x, float *live_y, float *live_z, const char *format, ImGuiInputTextFlags flags, const OnVec3PropChange &chg_callback)
{
   PROP_TABLE_SETUP
   ImGui::TableNextColumn();
   if ((is_live ? live_x : startup_x) == nullptr)
   { /* Missing value just skip */
      ImGui::TableNextColumn();
      return;
   }
   vec3 v = is_live ? vec3(*live_x, *live_y, *live_z) : vec3(*startup_x, *startup_y, *startup_z);
   ImGui::PushID(label);
   vec3 prev_v = v;
   if (ImGui::InputFloat3(label, &v.x, format, flags))
   {
      *(is_live ? live_x : startup_x) = v.x;
      *(is_live ? live_y : startup_y) = v.y;
      *(is_live ? live_z : startup_z) = v.z;
      if (chg_callback)
      {
         chg_callback(is_live, prev_v, v);
      }
      if (!is_live)
         m_table->SetNonUndoableDirty(eSaveDirty);
   }
   /* Sync button(also show if there are difference between live and startup through the enable state) */
   ImGui::TableNextColumn();
   if ((is_live ? startup_x : live_x) != nullptr)
   {
      const bool synced = ((*startup_x) == (*live_x)) && ((*startup_y) == (*live_y)) && ((*startup_z) == (*live_z));
      if (synced)
         ImGui::BeginDisabled();
      if (ImGui::Button(ICON_SAVE))
      {
         *(is_live ? startup_x : live_x) = v.x;
         *(is_live ? startup_y : live_y) = v.y;
         *(is_live ? startup_z : live_z) = v.z;
   PROP_HELPER_END
}

void EditorUI::PropVec3(const char *label, IEditable *undo_obj, bool is_live, float *startup_v2, float *live_v2, const char *format, ImGuiInputTextFlags flags, const OnVec3PropChange &chg_callback)
{
   Vertex3Ds startV, liveV;
   Vertex3Ds *startup_v = nullptr, *live_v = nullptr;
   if (startup_v2)
   {
      startup_v = &startV;
      startV.Set(startup_v2[0], startup_v2[1], startup_v2[2]);
   }
   if (live_v2)
   {
      live_v = &liveV;
      liveV.Set(live_v2[0], live_v2[1], live_v2[2]);
   }
   PropVec3(label, undo_obj, is_live, startup_v, live_v, format, flags, chg_callback);
   if (startup_v2)
   {
      startup_v2[0] = startV.x;
      startup_v2[1] = startV.y;
      startup_v2[2] = startV.z;
   }
   if (live_v2)
   {
      live_v2[0] = liveV.x;
      live_v2[1] = liveV.y;
      live_v2[2] = liveV.z;
   }
}

void EditorUI::PropVec3(const char *label, IEditable *undo_obj, bool is_live, Vertex3Ds *startup_v, Vertex3Ds *live_v, const char *format, ImGuiInputTextFlags flags, const OnVec3PropChange &chg_callback)
{
   PROP_HELPER_BEGIN(Vertex3Ds)
   float col[3] = { v->x, v->y, v->z };
   if (ImGui::InputFloat3(label, col, format, flags))
   {
      v->Set(col[0], col[1], col[2]);
      if (chg_callback)
      {
         vec3 v1{prev_v.x, prev_v.y, prev_v.z}, v2{v->x, v->y, v->z};
         chg_callback(is_live, v1, v2);
      }
      if (!is_live)
         m_table->SetNonUndoableDirty(eSaveDirty);
   }
   PROP_HELPER_SYNC(Vertex3Ds)
   PROP_HELPER_END
}

void EditorUI::PropCombo(const char *label, IEditable *undo_obj, bool is_live, int *startup_v, int *live_v, size_t n_values, const string labels[], const OnIntPropChange &chg_callback)
{
   PROP_HELPER_BEGIN(int)
   const char *const preview_value = labels[clamp(*v, 0, static_cast<int>(n_values) - 1)].c_str();
   if (ImGui::BeginCombo(label, preview_value))
   {
      for (int i = 0; i < (int)n_values; i++)
      {
         if (ImGui::Selectable(labels[i].c_str()))
         {
            *v = i;
            if (chg_callback)
               chg_callback(is_live, prev_v, i);
            if (!is_live)
               m_table->SetNonUndoableDirty(eSaveDirty);
         }
      }
      ImGui::EndCombo();
   }
   PROP_HELPER_SYNC(int)
   PROP_HELPER_END
}

void EditorUI::PropImageCombo(const char *label, IEditable *undo_obj, bool is_live, string *startup_v, string *live_v, PinTable *table, const OnStringPropChange &chg_callback)
{
   PROP_HELPER_BEGIN(string)
   const char *const preview_value = v->c_str();
   if (ImGui::BeginCombo(label, preview_value))
   {
      const std::function<string(Texture *)> map = [](Texture *image) -> string { return image->m_name; };
      for (Texture *texture : SortedCaseInsensitive(table->m_vimage, map))
      {
         if (ImGui::Selectable(texture->m_name.c_str()))
         {
            *v = texture->m_name;
            if (chg_callback)
               chg_callback(is_live, prev_v, *v);
            if (!is_live)
               m_table->SetNonUndoableDirty(eSaveDirty);
         }
      }
      ImGui::EndCombo();
   }
   PROP_HELPER_SYNC(string)
   if (chg_callback)
      chg_callback(!is_live, prev_ov, *ov);
   PROP_HELPER_END
}

void EditorUI::PropMaterialCombo(const char *label, IEditable *undo_obj, bool is_live, string *startup_v, string *live_v, PinTable *table, const OnStringPropChange &chg_callback)
{
   PROP_HELPER_BEGIN(string)
   const char *const preview_value = v->c_str();
   if (ImGui::BeginCombo(label, preview_value))
   {
      const std::function<string(Material *)> map = [](Material *material) -> string { return material->m_name; };
      for (Material *material : SortedCaseInsensitive(table->m_materials, map))
      {
         if (ImGui::Selectable(material->m_name.c_str()))
         {
            *v = material->m_name;
            if (chg_callback)
               chg_callback(is_live, prev_v, *v);
            if (!is_live)
               m_table->SetNonUndoableDirty(eSaveDirty);
         }
      }
      ImGui::EndCombo();
   }
   PROP_HELPER_SYNC(string)
   if (chg_callback)
      chg_callback(!is_live, prev_ov, *ov);
   PROP_HELPER_END
}

void EditorUI::PropLightmapCombo(const char *label, IEditable *undo_obj, bool is_live, string *startup_v, string *live_v, PinTable *table, const OnStringPropChange &chg_callback)
{
   PROP_HELPER_BEGIN(string)
   const char *const preview_value = v->c_str();
   if (ImGui::BeginCombo(label, preview_value))
   {
      const std::function<string(IEditable *)> map = [](IEditable *pe) -> string { return pe->GetItemType() == ItemTypeEnum::eItemLight ? pe->GetName() : string(); };
      for (IEditable *pe : SortedCaseInsensitive(table->m_vedit, map))
      {
         if (pe->GetItemType() == ItemTypeEnum::eItemLight && ImGui::Selectable(pe->GetName().c_str()))
         {
            *v = pe->GetName();
            if (chg_callback)
               chg_callback(is_live, prev_v, *v);
            if (!is_live)
               m_table->SetNonUndoableDirty(eSaveDirty);
         }
      }
      ImGui::EndCombo();
   }
   PROP_HELPER_SYNC(string)
   if (chg_callback)
      chg_callback(!is_live, prev_ov, *ov);
   PROP_HELPER_END
}

void EditorUI::PropRenderProbeCombo(const char *label, RenderProbe::ProbeType type, IEditable *undo_obj, bool is_live, string *startup_v, string *live_v, PinTable *table, const OnStringPropChange &chg_callback)
{
   PROP_HELPER_BEGIN(string)
   const char *const preview_value = v->c_str();
   if (ImGui::BeginCombo(label, preview_value))
   {
      const std::function<string(RenderProbe *)> map = [](RenderProbe *probe) -> string { return probe->GetName(); };
      for (RenderProbe *probe : SortedCaseInsensitive(table->m_vrenderprobe, map))
      {
         if (probe->GetType() == type && ImGui::Selectable(probe->GetName().c_str()))
         {
            *v = probe->GetName();
            if (chg_callback)
               chg_callback(is_live, prev_v, *v);
            if (!is_live)
               m_table->SetNonUndoableDirty(eSaveDirty);
         }
      }
      ImGui::EndCombo();
   }
   PROP_HELPER_SYNC(string)
   if (chg_callback)
      chg_callback(!is_live, prev_ov, *ov);
   PROP_HELPER_END
}

#undef PROP_HELPER_BEGIN
#undef PROP_HELPER_SYNC
#undef PROP_HELPER_END
