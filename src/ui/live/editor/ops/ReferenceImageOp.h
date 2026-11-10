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

// Reference images: scans, blueprints or photos shown in the editor to trace the parts over them. A reference image is an ordinary flasher
// (Flasher render mode, Wrap image mode, 4 corner points sized at the image's real size) inside a part group carrying the 'refimg' part
// extension, whose player mode visibility mask is cleared so that no play mode renders it (editor views do not apply that mask). Feature
// created groups and flashers are marked with the same extension so that renames do not detach them from the feature.
// The editor treats the flashers of this group as images: they scale uniformly by default, their points are not edited one by one (Fit to
// Image rebuilds a clean rectangle at the image's aspect), and their UI lock also stops 3D picking and the transform gizmo.
class ReferenceImageOp final
{
public:
   explicit ReferenceImageOp(EditorUI &editor)
      : m_editor(editor)
   {
   }

   static constexpr const char *Key = "refimg"; // Part extension key marking feature created groups and images (survives renames)
   static constexpr float Height = 1.f; // Default height above the playfield (VPU), above it enough to avoid depth fighting
   static constexpr int Opacity = 60; // Default flasher opacity (%)

   // Table level helpers (no editor state: used by the editor ops below and by the tests)
   static bool IsReferenceImage(const IEditable *part); // A flasher marked as a reference image, or inside a marked group (directly or nested)
   static bool IsReferenceGroup(const PartGroup *group); // A marked reference images group or a group nested in one (never a target for new parts)
   static bool IsLocked(const IEditable *part); // A reference image with its UI lock set: neither picked nor transformed in the views
   static PartGroup *FindGroup(const PinTable *table);
   // Creates the root level ReferenceImages group with a cleared player mode visibility mask.
   // The caller owns the creation reference (handed to the undo record by PinUndo::MarkForCreate, or released).
   static PartGroup *CreateGroup(PinTable *table);
   // Creates the flasher of an image of the table, centred at 'center', of the given size (VPU), flat on the playfield. Same ownership as CreateGroup.
   // The player side setup (render, physics) is left to the caller.
   static Flasher *CreateFlasher(PinTable *table, PartGroup *group, const string &image, const Vertex2D &center, float width, float height);
   static ReferenceImageMath::Rect GetRect(const Flasher *flasher); // Bounds of the flasher points (the image is mapped over them)
   static void SetRect(Flasher *flasher, const ReferenceImageMath::Rect &rect); // Replaces the points by the 4 corners of the rectangle
   static void SetSize(Flasher *flasher, const Vertex2D &size); // Keeps the centre
   static bool FitToImage(Flasher *flasher); // Clean rectangle at the image's aspect, keeping the centre and width (false when the image is not found)
   static bool ReplaceImage(Flasher *flasher, const string &image); // Swaps the image, then fits to its aspect
   // Group of a pasted or duplicated part: a marked reference image stays in a reference group (hidden in play), whatever the target of
   // new parts. When the group named by the pasted data is still a reference group it is reused (same table paste), otherwise the table's
   // reference group is found or created (marked for creation in the undo record being built, when undo is given). Any other part goes to 'target'.
   static PartGroup *PasteGroup(PinTable *table, PinUndo *undo, const IEditable *pasted, PartGroup *target);
   // Rotation about the table Z axis (win32 style Rotate command): turns the part (its Z rotation) instead of its points, which would distort
   // the image mapped over their bounds. A tilted image (X or Y rotation) is left as it is: its Z rotation is not a turn about the table Z
   // axis (use the R tool). Returns false for a part that is not a reference image (nothing done).
   static bool Rotate(IEditable *part, float angle, const Vertex2D &center, bool useElementCenter);
   // Scale in the table plane (win32 style Scale command): uniform for a reference image (the larger change sets both axes), so that it is not
   // stretched. Returns false for a part that is not a reference image (nothing done).
   static bool Scale(IEditable *part, float scaleX, float scaleY, const Vertex2D &center, bool useElementCenter);

   // Editor ops
   bool CanAdd() const;
   void OpenAddDialog();
   bool CanApplyToSelection() const; // Some selected parts are reference images
   void OpenReplaceDialog(); // Replace Image: swaps the image of the selected reference images, then fits them
   void FitSelection(); // Fit to Image of the selected reference images
   void RenderModal(); // Add / Replace dialog (rendered in the overlay window scope)

private:
   EditorUI &m_editor;

   vector<Flasher *> SelectedReferenceImages() const;
   void Add(const string &image, float width, float height);
   void Replace(const string &image);
   Texture *ImportFile(const string &file); // Imports an image file into the table under a free name

   // Add / Replace dialog state
   struct Dialog
   {
      bool replace = false;
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
