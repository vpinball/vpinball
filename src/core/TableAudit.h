// license:GPLv3+

#pragma once

#include <functional>
#include <string>
#include <vector>

class IEditable;
class Material;
class PinTable;
class PinUndo;
class Texture;
namespace VPX
{
class Sound;
struct SoundSpec;
}

// One finding of a table audit: a severity, a one line summary and a longer explanation,
// an optional subject (part, asset or script line) used to navigate to the offending
// element, and an optional fix action. Produced by TableAudit::Run.
struct TableAuditItem
{
   enum class Severity
   {
      Info,
      Warning,
      Error
   };
   Severity severity;
   string summary; // One line message (item lists and Format() output)
   string details; // Longer explanation for a details pane (may be the same as summary)
   // Subject for navigation/selection (at most one set):
   IEditable *part = nullptr;
   Texture *image = nullptr;
   VPX::Sound *sound = nullptr;
   Material *material = nullptr;
   int scriptLine = -1; // 1-based line in the table script, -1 = none
   bool bulletInReport = true; // Whether Format() prefixes the line with the ". " list bullet
   // Optional fix. Invoked inside a PinUndo Begin/End scope by the caller; implementations mark
   // their own edits (MarkForUndo / MarkForCreate / MarkForAction). Returns the created part
   // when the fix creates one (the caller performs its player side setup), nullptr otherwise.
   std::function<IEditable *(PinTable &table, PinUndo &undo)> fix;
   string fixLabel; // Undo label / button text, e.g. "Create 'PinMAMETimer' timer"
};

// Table audit: checks a table for inconsistencies, deprecated features and invalid data
// (see PinTable::AuditTable for the textual report used by the Win32 editor and the CLI).
// Run collects the findings as items; Format reproduces that report.
class TableAudit
{
public:
   vector<TableAuditItem> items;

   // Collects the findings of a table audit. Non const: items hold mutable handles for fixing.
   static TableAudit Run(PinTable &table);

   // The re-encode fixes the audit offers for 'sound' given its decoded specification
   // (from AudioPlayer::GetSoundInformations, zeroed when the file cannot be decoded):
   // a stereo-to-mono fix for playfield sounds and/or an Ogg Opus re-encode for non-Opus
   // data. Exposed for the Live Editor property pane, which offers the same actions.
   static vector<TableAuditItem> SoundFixes(VPX::Sound &sound, const VPX::SoundSpec &specs);

   // The re-encode fix the audit offers for 'image': re-encode when the stored format is
   // not the compact one for its range (WebP for LDR, EXR for HDR). Empty for linked images.
   static vector<TableAuditItem> ImageFixes(PinTable &table, Texture &image);

   // The findings in the legacy textual form (". Error: ..." / ". Warning: ..." / stats lines)
   string Format() const;
   bool HasErrors() const;
};
