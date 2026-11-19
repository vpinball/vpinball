# Vendored ImGuiColorTextEdit

Source: <https://github.com/goossens/ImGuiColorTextEdit>
Pinned commit: `9f812a0b772e6c60c86c295a78a9fcdb2df6d59a`
License: MIT (see `LICENSE`)

This is the actively maintained rewrite of BalazsJako's ImGuiColorTextEdit,
used by the live editor's script pane (`src/ui/live/editor/ScriptPanel.cpp`).

## Vendored files

- `TextEditor.h` / `TextEditor.cpp` — the widget itself
- `LICENSE` — upstream MIT license
- `textedit.patch` — local changes vs upstream (see below)

VPX additions (not part of upstream):

- `TextEditorLanguages.h` — trimmed registry; upstream registers every
  language under `languages/`, VPX only ships VBScript
- `languages/TextEditorVbs.h` — VBScript language definition
- `VbsLex.h` — VBScript lexical helpers shared between the language
  definition and the live editor's script model

Upstream files deliberately not vendored: `TextDiff.*`, `dtl.h`, all other
`languages/*.h`, examples, docs.

## Local changes (textedit.patch)

- `#include "imgui.h"` -> `#include "imgui/imgui.h"` and
  `#include "imgui_internal.h"` -> `#include "imgui/imgui_internal.h"`
  (VPX include path is `third-party/include`, not `third-party/include/imgui`)
- `Document::getContext` bounds fix — upstream reads `line[end.index]`
  without checking `end.index < line.size()`, crashing on empty lines and
  end-of-line cursor positions
- `TextEditor::HasActiveTransient()` — exposes whether the autocomplete
  popup, find/replace window or secondary cursors are active, so the host
  pane can give Esc to the widget before defocusing it
- `TextEditor::RecolorizeAll()` (and `Colorizer::invalidate()`) — recolorizes
  every line on the next render: the script pane changes the word sets of the
  VBScript language as script symbols come and go, which `SetLanguage` with the
  same language does not pick up
- `Language::autoIndentDelta` — optional language callback used by
  `autoIndentAllCursors` to adjust the copied indentation by a block-aware
  delta (-1/0/+1); `TextEditorVbs` fills it with `VbsLex::SmartIndentDelta`

The upstream sources compile without the VPX precompiled header; they are
marked `SKIP_PRECOMPILE_HEADERS`/`NotUsing` in the build files rather than
patched to include `core/stdafx.h`.

## Updating

1. Download `TextEditor.h`, `TextEditor.cpp` and `LICENSE` at the new commit.
2. Re-apply the changes listed above (or apply `textedit.patch`).
3. Update `textedit.patch` and this file's pinned commit.
