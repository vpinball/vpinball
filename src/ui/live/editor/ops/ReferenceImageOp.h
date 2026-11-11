// license:GPLv3+

#pragma once

#include "ReferenceImageMath.h"

#include <memory>

class Flasher;
class IEditable;
class PartGroup;
class PinTable;
class PinUndo;
class Texture;

namespace VPX::EditorUI
{

class EditorUI;
class EditorUIPart;

// Reference images: scans, blueprints or photos shown in the editor to trace the parts over them. A reference image is an ordinary
// flasher whose setup makes it an editor only visual: never rendered in play (authored not visible), but drawn in the editor views
// (display texture, Flasher render mode, a single image, and no lightmap or state source driving it). Such flashers are detected
// wherever they are (no dedicated group, no marker: the serialized fields carry the trait), and the editor adapts their property
// pane to them.
// The editor treats these flashers as images: they scale uniformly by default, their points are not edited one by one (Fit to
// Image rebuilds a clean rectangle at the image's aspect), and their UI lock also stops 3D picking and the transform gizmo.
class ReferenceImageOp final
{
public:
   explicit ReferenceImageOp(EditorUI &editor)
      : m_editor(editor)
   {
   }

   static constexpr float Height = 1.f; // Default height above the playfield (VPU), above it enough to avoid depth fighting
   static constexpr int Opacity = 60; // Default flasher opacity (%)

   // Table level helpers (no editor state: used by the editor ops below and by the tests)
   // The reference image setup, without the visibility trait (which has an authored and a display value on session parts)
   static bool HasReferenceImageSetup(const Flasher *flasher);
   // A flasher set up as a reference image and authored as invisible. For parts driven by an editor session, the visibility
   // trait is read from the part's UI part (m_d then holds a display value, not the authored one): give 'editor' to resolve it
   // (its editable map), or pass the UI part directly. Bare parts (tests, freshly loaded parts, other table instances) can be
   // checked with 'editor' == nullptr: their m_d is the authored state.
   static bool IsReferenceImage(const IEditable *part, const EditorUI *editor = nullptr);
   static bool IsReferenceImage(const EditorUIPart *uiPart);
   // A reference image with its UI lock set: neither picked nor transformed in the views
   static bool IsLocked(const IEditable *part, const EditorUI *editor = nullptr);
   static bool IsLocked(const EditorUIPart *uiPart);
   // Creates the flasher of an image of the table, centred at 'center', of the given size (VPU), flat on the playfield.
   // The caller owns the creation reference (handed to the undo record by PinUndo::MarkForCreate, or released), assigns the
   // part group and does the player side setup (render, physics).
   static Flasher *CreateFlasher(PinTable *table, const string &image, const Vertex2D &center, float width, float height);
   static ReferenceImageMath::Rect GetRect(const Flasher *flasher); // Bounds of the flasher points (the image is mapped over them)
   static void SetRect(Flasher *flasher, const ReferenceImageMath::Rect &rect); // Replaces the points by the 4 corners of the rectangle
   static void SetSize(Flasher *flasher, const Vertex2D &size); // Keeps the centre
   static bool FitToImage(Flasher *flasher); // Clean rectangle at the image's aspect, keeping the centre and width (false when the image is not found)
   static bool ReplaceImage(Flasher *flasher, const string &image); // Swaps the image, then fits to its aspect
   // Rotation about the table Z axis (win32 style Rotate command): turns the part (its Z rotation) instead of its points, which would distort
   // the image mapped over their bounds. A tilted image (X or Y rotation) is left as it is: its Z rotation is not a turn about the table Z
   // axis (use the R tool). Returns false for a part that is not a reference image (nothing done).
   static bool Rotate(IEditable *part, float angle, const Vertex2D &center, bool useElementCenter);
   static bool Rotate(const EditorUIPart *uiPart, float angle, const Vertex2D &center, bool useElementCenter);
   // Scale in the table plane (win32 style Scale command): uniform for a reference image (the larger change sets both axes), so that it is not
   // stretched. Returns false for a part that is not a reference image (nothing done).
   static bool Scale(IEditable *part, float scaleX, float scaleY, const Vertex2D &center, bool useElementCenter);
   static bool Scale(const EditorUIPart *uiPart, float scaleX, float scaleY, const Vertex2D &center, bool useElementCenter);

   // Editor ops
   bool CanAdd() const;
   void OpenAddDialog();
   void RenderModal(); // Add dialog (rendered in the overlay window scope)

private:
   EditorUI &m_editor;

   void Add(const string &image, float width, float height);
   Texture *ImportFile(const string &file); // Imports an image file into the table under a free name

   // Add dialog state
   struct Dialog
   {
      bool fromFile = true; // Image source: a file, or an image of the table
      string file; // Picked file (UTF-8)
      unsigned int fileWidth = 0, fileHeight = 0; // Pixel size of the picked file (0 when it could not be read)
      string tableImage; // Picked table image
      ReferenceImageMath::Unit unit = ReferenceImageMath::Unit::Centimeters;
      float width = 0.f, height = 0.f; // In 'unit'
      float dpi = 0.f;
      string sizedFor; // Image the size fields were prefilled for
      string error;
   } m_dialog;
   bool m_openDialog = false;
   std::shared_ptr<vector<string>> m_pendingFile; // Asynchronous file dialog result
   void PickedFile(const string &file);
   void ImageSize(unsigned int &width, unsigned int &height) const; // Pixel size of the image chosen in the dialog
};

}
