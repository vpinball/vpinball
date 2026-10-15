// license:GPLv3+

#pragma once

#include <string>

#include "unordered_dense.h"

class PinFont;

// Registers a table's embedded fonts with GDI (writes each to a temp file and calls
// AddFontResource) so the editor's GDI font pickers can resolve them by face name.
// Owned by the table editor: destruction removes every resource and deletes the temp files.
class PinFontRegistrar final
{
public:
   PinFontRegistrar() = default;
   ~PinFontRegistrar() { UnregisterAll(); }
   PinFontRegistrar(const PinFontRegistrar&) = delete;
   PinFontRegistrar& operator=(const PinFontRegistrar&) = delete;

   void Register(PinFont* font); // kept in sync via ITableEditorSink::OnFontAdded
   void Unregister(PinFont* font); // remove a font's registration (called via ITableEditorSink::OnFontRemoved, before the font is deleted)
   void UnregisterAll();

private:
   ankerl::unordered_dense::map<const PinFont*, string> m_registered; // font -> temp file it was registered from
};
