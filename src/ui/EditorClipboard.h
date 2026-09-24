// license:GPLv3+

#pragma once

#include "math/vector.h"

class IEditable;

namespace VPX
{

// OS clipboard shared by the editors (Win32 editor & live editor), implemented on top of SDL's
// clipboard support. The editor payloads are offered under dedicated mime types, allowing
// copy/paste between the two editors as well as between application instances.
class EditorClipboard final
{
public:
   // Copies the given parts to the clipboard, each serialized with its item type like in a saved table
   static void CopyParts(const vector<IEditable *> &parts);
   // Returns true if the clipboard contains serialized parts
   static bool HasParts();
   // Returns the serialized parts in the clipboard (item type then data for each part, empty if none)
   static vector<vector<uint8_t>> GetParts();

   // Copies the coordinates of a drag point to the clipboard
   static void CopyPoint(const Vertex3Ds &pos);
   // Returns true if the clipboard contains drag point coordinates
   static bool HasPoint();
   // Returns the drag point coordinates in the clipboard (false if none)
   static bool GetPoint(Vertex3Ds &pos);
};

}
