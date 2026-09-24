// license:GPLv3+

#include "core/stdafx.h"
#include "OutlinerPanel.h"

#include "fonts/IconsForkAwesome.h"
#include "parts/Material.h"
#include "parts/PartGroup.h"
#include "parts/Sound.h"
#include "renderer/Texture.h"
#include "ui/live/EditorUI.h"
#include "ui/live/LiveUI.h"

#include "imgui/imgui_stdlib.h"

namespace VPX::EditorUI
{

template <class T> static std::vector<T> SortedCaseInsensitive(std::vector<T> &list, const std::function<string(T)> &map)
{
   std::vector<T> sorted(list.begin(), list.end());
   std::ranges::sort(sorted,
      [map](const T &a, const T &b) -> bool
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

bool OutlinerPanel::MatchesFilter(const string &name) const
{
   if (m_filter.empty())
      return true;
   const string name_lcase = lowerCase(name);
   const string filter_lcase = lowerCase(m_filter);
   return name_lcase.find(filter_lcase) != std::string::npos;
}

void OutlinerPanel::Render(float topBarHeight)
{
   EditorUI &editor = m_editor;
   if (editor.m_table->IsLocked())
      return;

   const ImGuiViewport *const viewport = ImGui::GetMainViewport();
   const float pane_width = PaneWidth * editor.m_liveUI.GetDPI();
   ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + topBarHeight));
   ImGui::SetNextWindowSize(ImVec2(pane_width, viewport->Size.y - topBarHeight));
   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
      | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f * editor.m_liveUI.GetDPI(), 4.0f * editor.m_liveUI.GetDPI()));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
   ImGui::Begin("OUTLINER", nullptr, window_flags);
   {
      const bool popStyle = m_syncToSelection;
      if (popStyle)
         ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
      if (ImGui::Button(ICON_FK_LINK "##SyncTree"))
      {
         m_syncToSelection = !m_syncToSelection;
         m_lastSyncedPart = nullptr; // Reveal the current selection on next render
      }
      if (popStyle)
         ImGui::PopStyleColor();
      if (ImGui::IsItemHovered())
         ImGui::SetTooltip("Sync tree with selection");
   }
   {
      ImGui::SameLine();
      const float clearFilterWidth = ImGui::CalcTextSize(ICON_FK_TIMES).x + ImGui::GetStyle().FramePadding.x * 2.0f;
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - clearFilterWidth - ImGui::GetStyle().ItemSpacing.x);
      ImGui::InputTextWithHint("##OutlinerFilter", "Name part filter", &m_filter);
   }
   {
      ImGui::SameLine();
      ImGui::BeginDisabled(m_filter.empty());
      if (ImGui::Button(ICON_FK_TIMES "##ClearOutlinerFilter"))
         m_filter.clear();
      ImGui::EndDisabled();
   }
   ImGui::Separator();

   if (ImGui::TreeNodeEx("View Setups"))
   {
      if (ImGui::Selectable("Editor Camera"))
      {
         editor.ClearSelection();
         editor.m_table->SetViewSetupOverride(BG_DESKTOP);
         editor.m_table->SetViewSetupOverride(BG_INVALID);
         editor.m_camMode = ViewMode::EditorCam;
      }
      const Selection cam0 = Selection::Camera(0);
      if (ImGui::Selectable("Preview: Desktop", editor.m_selection == cam0))
      {
         editor.SetSelection(cam0);
         editor.m_camMode = ViewMode::PreviewCam;
         editor.m_table->SetViewSetupOverride(BG_DESKTOP);
      }
      const Selection cam1 = Selection::Camera(1);
      if (ImGui::Selectable("Preview: Cabinet", editor.m_selection == cam1))
      {
         editor.SetSelection(cam1);
         editor.m_camMode = ViewMode::PreviewCam;
         editor.m_table->SetViewSetupOverride(BG_FULLSCREEN);
      }
      const Selection cam2 = Selection::Camera(2);
      if (ImGui::Selectable("Preview: Full Single Screen", editor.m_selection == cam2))
      {
         editor.SetSelection(cam2);
         editor.m_camMode = ViewMode::PreviewCam;
         editor.m_table->SetViewSetupOverride(BG_FSS);
      }
      ImGui::TreePop();
   }
   const std::function<string(Material *)> materialName = [](Material *material) -> string { return material->m_name; };
   RenderResourceList("Materials", editor.m_table->m_materials, editor.m_multiSelMaterials, editor.m_outlinerMaterialAnchor, materialName);
   const std::function<string(Texture *)> imageName = [](Texture *image) -> string { return image->m_name; };
   RenderResourceList("Images", editor.m_table->m_vimage, editor.m_multiSelImages, editor.m_outlinerImageAnchor, imageName);
   const std::function<string(VPX::Sound *)> soundName = [](VPX::Sound *sound) -> string { return sound->GetName(); };
   RenderResourceList("Sounds", editor.m_table->m_vsound, editor.m_multiSelSounds, editor.m_outlinerSoundAnchor, soundName);
   if (ImGui::TreeNode("Render Probes"))
   {
      for (RenderProbe *probe : editor.m_table->m_vrenderprobe)
      {
         Selection sel(probe);
         if (ImGui::Selectable(probe->GetName().c_str(), editor.m_selection == sel))
            editor.SetSelection(sel);
      }
      ImGui::TreePop();
   }
   if (ImGui::TreeNodeEx("Scene Parts", ImGuiTreeNodeFlags_DefaultOpen))
   {
      // Table definition parts
      struct Node
      {
         PartGroup *group;
         bool opened;
      };
      // When a filter is applied, only display groups that match it or contain a matching part in their subtree
      ankerl::unordered_dense::set<const PartGroup *> matchedGroups;
      ankerl::unordered_dense::set<const PartGroup *> visibleGroups;
      if (!m_filter.empty())
      {
         vector<PartGroup *> groupStack;
         for (const auto &edit : editor.m_editables)
         {
            IEditable *const editable = edit->GetEditable();
            while (!groupStack.empty() && (editable->GetPartGroup() == nullptr || !editable->IsChild(groupStack.back())))
               groupStack.pop_back();
            if (MatchesFilter(editable->GetName())
               || (editable->GetItemType() != eItemPartGroup && std::ranges::any_of(groupStack, [&matchedGroups](const PartGroup *group) { return matchedGroups.contains(group); })))
            {
               for (PartGroup *ancestor : groupStack)
                  visibleGroups.insert(ancestor);
               if (editable->GetItemType() == eItemPartGroup)
               {
                  matchedGroups.insert(static_cast<PartGroup *>(editable));
                  visibleGroups.insert(static_cast<PartGroup *>(editable));
               }
            }
            if (editable->GetItemType() == eItemPartGroup)
               groupStack.push_back(static_cast<PartGroup *>(editable));
         }
      }
      vector<Node> stack;
      int outlinerItem = 0;
      const float eyeX = ImGui::GetContentRegionAvail().x;
      // When 'sync tree with selection' is enabled, reveal the newly selected part in the tree
      const std::shared_ptr<EditorUIPart> revealPart = (m_syncToSelection && editor.m_selection.GetPart() != m_lastSyncedPart) ? editor.m_selection.GetPart() : nullptr;
      m_lastSyncedPart = editor.m_selection.GetPart();
      for (const auto &edit : editor.m_editables)
      {
         const PartGroup *parent = edit->GetEditable()->GetPartGroup();
         while (!stack.empty()
            && ((parent == nullptr && (edit->GetEditable()->GetItemType() == eItemPartGroup)) // Root partgroup: pop all
               || (parent == nullptr && (edit->GetEditable()->GetItemType() != eItemPartGroup) && (stack.back().group != nullptr)) // Live object: pop all unless in 'Live Object' group
               || (parent != nullptr && (!edit->GetEditable()->IsChild(stack.back().group))))) // Child object: pop up to the parent group
         {
            if (stack.back().opened)
               ImGui::TreePop();
            stack.pop_back();
         }
         // TODO allow selection => ImGuiTreeNodeFlags_Selected
         // TODO support empty nodes => ImGuiTreeNodeFlags_Leaf
         if (edit->GetEditable()->GetItemType() == eItemPartGroup && !m_filter.empty() && !visibleGroups.contains(static_cast<PartGroup *>(edit->GetEditable())))
            continue;
         // In desktop backdrop mode, only display backdrop parts, and conversely in the other view modes
         if (edit->GetEditable()->GetItemType() != eItemPartGroup && (edit->GetEditable()->m_desktopBackdrop != (editor.m_camMode == ViewMode::DesktopBackdrop)))
            continue;
         ImGui::AlignTextToFramePadding();
         if (edit->GetEditable()->GetItemType() == eItemPartGroup)
         {
            PartGroup *group = static_cast<PartGroup *>(edit->GetEditable());
            if (revealPart && revealPart->GetEditable()->IsChild(group))
               ImGui::SetNextItemOpen(true);
            const bool opened = ImGui::TreeNodeEx(edit->GetEditable()->GetName().c_str(), ImGuiTreeNodeFlags_AllowOverlap | (editor.IsPartSelected(edit) ? ImGuiTreeNodeFlags_Selected : 0));
            if (edit == revealPart && !ImGui::IsItemVisible())
               ImGui::SetScrollHereY();
            if (ImGui::BeginPopupContextItem())
            {
               if (ImGui::MenuItem("Select"))
                  editor.SetSelection(Selection(edit));
               if (ImGui::MenuItem("Select Contents"))
                  editor.SelectPartsInGroup(group);
               ImGui::EndPopup();
            }
            if (editor.m_table->m_liveBaseTable == nullptr)
            {
               ImGui::SameLine(eyeX);
               ImGui::PushStyleColor(ImGuiCol_Text, group->IsUIVisible(false) ? IM_COL32_WHITE : IM_COL32(128, 128, 128, 255));
               if (ImGui::SmallButton(((group->IsUIVisible(false) ? ICON_FK_EYE : ICON_FK_EYE_SLASH) + "##Eye__"s + edit->GetEditable()->GetName()).c_str()))
                  group->SetUIVisible(!group->IsUIVisible(false));
               ImGui::PopStyleColor();
            }
            stack.emplace_back(static_cast<PartGroup *>(edit->GetEditable()), (stack.empty() || stack.back().opened) ? opened : false);
         }
         else
         {
            const bool show = MatchesFilter(edit->GetEditable()->GetName())
               || std::ranges::any_of(stack, [&matchedGroups](const Node &node) { return node.group != nullptr && matchedGroups.contains(node.group); });
            if (parent == nullptr && stack.empty() && show)
            {
               if (revealPart && revealPart->GetEditable()->GetPartGroup() == nullptr)
                  ImGui::SetNextItemOpen(true);
               stack.push_back({ nullptr, ImGui::TreeNodeEx("[Live Objects]", ImGuiTreeNodeFlags_AllowOverlap) });
            }
            if (!stack.empty() && stack.back().opened)
            {
               Selection sel(edit);
               if (show)
               {
                  if (ImGui::Selectable(
                         (edit->GetEditable()->GetName() + "##Outliner"s + std::to_string(outlinerItem++)).c_str(), editor.IsPartSelected(edit), ImGuiSelectableFlags_AllowOverlap))
                  {
                     const ImGuiIO &selIO = ImGui::GetIO();
                     if (selIO.KeyCtrl)
                     {
                        editor.TogglePartSelection(edit);
                        editor.m_outlinerAnchor = edit;
                     }
                     else if (selIO.KeyShift)
                        editor.SelectOutlinerRange(edit);
                     else
                     {
                        editor.SetSelection(sel);
                        editor.m_outlinerAnchor = edit;
                     }
                  }
                  if (edit == revealPart && !ImGui::IsItemVisible())
                     ImGui::SetScrollHereY();
                  IEditable *editable = edit->GetEditable();
                  if (editable && editor.m_table->m_liveBaseTable == nullptr)
                  {
                     ImGui::SameLine(eyeX);
                     ImGui::PushStyleColor(ImGuiCol_Text, editable->IsUIVisible(false) ? IM_COL32_WHITE : IM_COL32(128, 128, 128, 255));
                     if (ImGui::SmallButton(((editable->IsUIVisible(false) ? ICON_FK_EYE : ICON_FK_EYE_SLASH) + "##Eye__"s + edit->GetEditable()->GetName()).c_str()))
                        editable->SetUIVisible(!editable->IsUIVisible(false));
                     ImGui::PopStyleColor();
                  }
               }
            }
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

   ImGui::End();
   ImGui::PopStyleVar(3);
}

template <class T> void OutlinerPanel::RenderResourceList(const char *label, vector<T *> &items, vector<T *> &multiSel, T *&anchor, const std::function<string(T *)> &nameOf)
{
   if (!ImGui::TreeNode(label))
      return;
   EditorUI &editor = m_editor;
   const std::vector<T *> sorted = SortedCaseInsensitive(items, nameOf);
   int pos = 0;
   for (T *const item : sorted)
   {
      if (!MatchesFilter(nameOf(item)))
         continue;
      ImGui::PushID(pos++);
      const bool selected = std::ranges::find(multiSel, item) != multiSel.end();
      const bool clicked = ImGui::Selectable(nameOf(item).c_str(), selected);
      ImGui::PopID();
      if (!clicked)
         continue;
      const ImGuiIO &selIO = ImGui::GetIO();
      if (selIO.KeyCtrl)
      {
         const auto it = std::ranges::find(multiSel, item);
         if (it == multiSel.end())
         {
            multiSel.push_back(item);
            editor.m_selection = Selection(item);
         }
         else
         {
            const bool wasActive = editor.m_selection == Selection(item);
            multiSel.erase(it);
            if (multiSel.empty())
               editor.m_selection = Selection();
            else if (wasActive)
               editor.m_selection = Selection(multiSel.back());
         }
         anchor = item;
      }
      else if (selIO.KeyShift && anchor != nullptr)
      {
         // Range selection between the anchor and the clicked item, restricted to the visible items
         vector<T *> visible;
         for (T *const v : sorted)
            if (MatchesFilter(nameOf(v)))
               visible.push_back(v);
         const auto anchorIt = std::ranges::find(visible, anchor);
         const auto itemIt = std::ranges::find(visible, item);
         if (anchorIt == visible.end() || itemIt == visible.end())
         {
            editor.SetSelection(Selection(item));
            anchor = item;
         }
         else
         {
            multiSel.clear();
            for (auto it = std::min(anchorIt, itemIt); it != std::max(anchorIt, itemIt) + 1; ++it)
               multiSel.push_back(*it);
            editor.m_selection = Selection(item);
         }
      }
      else
      {
         editor.SetSelection(Selection(item));
         anchor = item;
      }
   }
   ImGui::TreePop();
}
}
