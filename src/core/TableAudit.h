// license:GPLv3+

#pragma once

#include <functional>
#include <memory>
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
   // Fixes that may change nothing (e.g. a re-encode that would not be smaller) set it to the reason, or clear it when applied.
   // The caller then discards the empty undo record and reports the reason.
   std::shared_ptr<string> fixFailure;
};

// Table audit: checks a table for inconsistencies, deprecated features and invalid data
// (see PinTable::AuditTable for the textual report used by the Win32 editor and the CLI).
// Run collects the findings as items; Format reproduces that report.
class TableAudit
{
public:
   vector<TableAuditItem> items;

   // Collects the findings of a table audit. Non const: items hold mutable handles for fixing.
   // 'soundInformations' gives a sound's decoded specification (see AudioPlayer::GetSoundInformations), e.g. from the running audio
   // player or a cache. Without it, the audit creates its own audio player, which opens the output devices.
   static TableAudit Run(PinTable &table, const std::function<VPX::SoundSpec(const VPX::Sound &)> &soundInformations = nullptr);

   // The re-encode fixes the audit offers for 'sound' given its decoded specification
   // (from AudioPlayer::GetSoundInformations, zeroed when the file cannot be decoded):
   // a stereo-to-mono fix for playfield sounds and/or an Ogg Opus re-encode for non-Opus
   // data. Exposed for the Live Editor property pane, which offers the same actions.
   static vector<TableAuditItem> SoundFixes(VPX::Sound &sound, const VPX::SoundSpec &specs);

   // The re-encode fix the audit offers for 'image': re-encode when the stored format is
   // not the compact one for its range (WebP for LDR, EXR for HDR). Empty for linked images.
   static vector<TableAuditItem> ImageFixes(PinTable &table, Texture &image);

   // Compression of a sound's file data, from its content: lossless (PCM/float WAV, FLAC, Ogg FLAC), lossy (Ogg Vorbis/Opus, MP3)
   // with the average audio bitrate (bit/s, without headers and tags, 0 if the length is unknown), or unknown (e.g. ADPCM WAV)
   enum class SoundFileCodec
   {
      Unknown,
      Lossless,
      OggLossy,
      MP3
   };
   struct SoundFileInfo
   {
      SoundFileCodec codec = SoundFileCodec::Unknown;
      int bitrate = 0;
      int bitsPerSample = 0; // Of a WAV or FLAC file, 0 otherwise
   };
   static SoundFileInfo AnalyzeSoundFile(const VPX::Sound &sound, float lengthInSeconds);

   // Encoding quality (1..100, libjpeg scale) of JPEG file data, estimated from its quantization tables, 0 if not a readable JPEG.
   // 'exact': the tables are the JPEG spec (Annex K) ones scaled for that quality, as libjpeg based encoders write them. Otherwise
   // (e.g. camera or Photoshop tables), the quality with the same average scaling.
   static int EstimateJpegQuality(const uint8_t *data, size_t size, bool &exact);
   // Whether JPEG file data stores colors at full resolution (4:4:4, no chroma subsampling)
   static bool JpegHasFullColorResolution(const uint8_t *data, size_t size);

   // The findings in the legacy textual form (". Error: ..." / ". Warning: ..." / stats lines)
   string Format() const;
   bool HasErrors() const;
};
