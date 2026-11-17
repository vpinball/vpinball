// Trimmed language registry for the VPX vendored ImGuiColorTextEdit.
// Upstream registers every language under languages/; VPX only needs VBScript,
// so the other upstream language headers are not vendored at all.
#pragma once

#include "TextEditor.h"
#include "languages/TextEditorVbs.h"

struct TextEditorLanguages {
   static void registerLanguages()
   {
      TextEditor::RegisterLanguage("VBScript", TextEditorVbs::getDefinition, { ".vbs", ".vpx" });
   }
};
