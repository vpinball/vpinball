// license:GPLv3+

#pragma once

#include <memory>
#include <variant>

class Material;
class Texture;
class RenderProbe;

namespace VPX::EditorUI
{

class EditorUIPart;

// Current editor selection: either nothing, a camera view setup, a material, an image,
// an editable part (the active part of the multi selection) or a render probe.
struct Selection
{
   enum SelectionType
   {
      S_NONE,
      S_CAMERA,
      S_MATERIAL,
      S_IMAGE,
      S_EDITABLE,
      S_RENDERPROBE
   };
   struct CameraSel
   {
      int viewSetup;
      bool operator==(const CameraSel &) const = default;
   };

   // Alternative order must match SelectionType (GetType relies on it)
   using Payload = std::variant<std::monostate, CameraSel, Material *, Texture *, std::shared_ptr<EditorUIPart>, RenderProbe *>;
   static_assert(std::variant_size_v<Payload> == S_RENDERPROBE + 1);
   Payload payload;

   Selection() = default;
   Selection(Material *material)
      : payload(material)
   {
   }
   Selection(Texture *image)
      : payload(image)
   {
   }
   Selection(const std::shared_ptr<EditorUIPart> &uiPart)
      : payload(uiPart)
   {
   }
   Selection(RenderProbe *probe)
      : payload(probe)
   {
   }
   static Selection Camera(int viewSetup)
   {
      Selection sel;
      sel.payload = CameraSel { viewSetup };
      return sel;
   }

   SelectionType GetType() const { return static_cast<SelectionType>(payload.index()); }
   int GetCamera() const
   {
      const CameraSel *cam = std::get_if<CameraSel>(&payload);
      return cam ? cam->viewSetup : -1;
   }
   Material *GetMaterial() const
   {
      const auto *p = std::get_if<Material *>(&payload);
      return p ? *p : nullptr;
   }
   Texture *GetImage() const
   {
      const auto *p = std::get_if<Texture *>(&payload);
      return p ? *p : nullptr;
   }
   std::shared_ptr<EditorUIPart> GetPart() const
   {
      const auto *p = std::get_if<std::shared_ptr<EditorUIPart>>(&payload);
      return p ? *p : nullptr;
   }
   RenderProbe *GetProbe() const
   {
      const auto *p = std::get_if<RenderProbe *>(&payload);
      return p ? *p : nullptr;
   }

   bool operator==(const Selection &) const = default;
};

}
