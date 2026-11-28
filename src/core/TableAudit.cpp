// license:GPLv3+

#include "core/stdafx.h"
#include "core/TableAudit.h"

#include "audio/AudioPlayer.h"
#include "miniaudio/miniaudio.h" // Must precede OpusDecoder.h
#include "audio/OpusDecoder.h"
#include "core/editablereg.h"
#include "core/pinundo.h"
#include "parts/light.h"
#include "parts/Material.h"
#include "parts/pintable.h"
#include "parts/primitive.h"
#include "parts/Sound.h"
#include "parts/textbox.h"
#include "parts/timer.h"
#include "renderer/Texture.h"

#ifdef __STANDALONE__
#define _WINDOWS_
#endif
#include "FreeImage.h"
#include "ogg/ogg.h"
#include "opus.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <sstream>

namespace
{

// Undoable change of a set of fields of 'table', capturing the pre-fix values in the undo
// restorer and the post-fix values in the redo restorer (see PinUndo::MarkForAction)
template <typename Set> std::function<IEditable *(PinTable &table, PinUndo &undo)> TableFieldsFix(Set &&set, const std::function<std::vector<std::function<void()>>()> &snapshot)
{
   return [set = std::move(set), snapshot](PinTable &table, PinUndo &undo) -> IEditable *
   {
      undo.MarkForAction(
         [restore = snapshot()]()
         {
            for (const auto &restoreField : restore)
               restoreField();
         }, //
         [snapshot]() -> std::function<void()>
         {
            return [restore = snapshot()]()
            {
               for (const auto &restoreField : restore)
                  restoreField();
            };
         });
      set(table);
      return nullptr;
   };
}

// Creates a Timer part with the given name (the audit only offers this fix when the name
// is available). The returned part is registered to the player by the fix caller.
std::function<IEditable *(PinTable &table, PinUndo &undo)> CreateTimerFix(const string &timerName)
{
   return [timerName](PinTable &table, PinUndo &undo) -> IEditable *
   {
      IEditable *const timer = EditableRegistry::CreateAndInit(eItemTimer, &table, 0.f, 0.f);
      if (timer == nullptr)
         return nullptr;
      timer->GetIScriptable()->m_name = timerName; // Set before AddPart which registers it (and asserts it is set and unique)
      timer->m_timerEnabled = true;
      table.AddPart(timer);
      undo.MarkForCreate(timer);
      return timer;
   };
}

// Fix of a single field of a part's data (the part is serialized by MarkForUndo)
template <typename TPart, typename TData, typename TField> std::function<IEditable *(PinTable &table, PinUndo &undo)> PartFieldFix(TPart *part, TField TData::*field, TField value)
{
   return [part, field, value](PinTable &table, PinUndo &undo) -> IEditable *
   {
      undo.MarkForUndo(part);
      part->m_d.*field = value;
      return nullptr;
   };
}

// Opus codec rate for a source sample rate: the codec only accepts 8/12/16/24/48kHz input, so the smallest covering the source
// bandwidth (e.g. a 22kHz sound is encoded at 24kHz without wasting bits on ultrasonics)
int OpusEncodeRate(const int sourceRate) { return sourceRate <= 8000 ? 8000 : sourceRate <= 12000 ? 12000 : sourceRate <= 16000 ? 16000 : sourceRate <= 24000 ? 24000 : 48000; }

// Re-encodes a sound's file data as an Ogg Opus stream. 'channels' is the output channel
// count: 1 to downmix to mono, 0 to keep the source count. The data is decoded to float
// by the same miniaudio + Opus decoder stack as the audio player, resampled to OpusEncodeRate,
// downmixing with its
// 'simple' (averaging) channel mixer when 'channels' is below the source count. When
// downmixing to mono, samples are scaled by the source channel count: this is the audio
// player's own loudness compensation (averaging loses energy, see
// SoundPlayer::m_monoCompensation), so the mono stream stays as loud as the stereo source.
// 'bitrate' is the Opus bitrate in bit/s.
vector<uint8_t> EncodeSoundToOggOpus(const VPX::Sound &sound, const int targetChannels, const int bitrate)
{
   // Probe the source channel count and sample rate with a native decoder
   ma_decoder_config probeConfig = ma_decoder_config_init(ma_format_f32, 0, 0);
   VPX::AddOpusDecoder(probeConfig);
   ma_decoder probe;
   if (ma_decoder_init_memory(sound.GetFileRaw(), sound.GetFileSize(), &probeConfig, &probe) != MA_SUCCESS)
      return {};
   const int sourceChannels = static_cast<int>(probe.outputChannels);
   const int sourceRate = static_cast<int>(probe.outputSampleRate);
   ma_decoder_uninit(&probe);

   // Opus' simple channel mapping only supports mono/stereo: downmix anything above
   const int channels = std::clamp(targetChannels > 0 ? targetChannels : sourceChannels, 1, 2);
   const int rate = OpusEncodeRate(sourceRate);
   ma_decoder_config decoderConfig = ma_decoder_config_init(ma_format_f32, channels, rate);
   decoderConfig.channelMixMode = ma_channel_mix_mode_simple;
   VPX::AddOpusDecoder(decoderConfig);
   ma_decoder decoder;
   if (ma_decoder_init_memory(sound.GetFileRaw(), sound.GetFileSize(), &decoderConfig, &decoder) != MA_SUCCESS)
      return {};
   vector<float> pcm;
   float frames[960 * 2];
   for (;;)
   {
      ma_uint64 read = 0;
      const ma_result result = ma_decoder_read_pcm_frames(&decoder, frames, std::size(frames) / channels, &read);
      pcm.insert(pcm.end(), frames, frames + read * channels);
      if (result != MA_SUCCESS || read == 0)
         break;
   }
   ma_decoder_uninit(&decoder);
   if (pcm.empty())
      return {};
   // Note: the clamp clips loud sources with in-phase channels (|L+R| > 1), which the player (float mix, volume applied after) does not
   if (channels == 1 && sourceChannels > 1)
      for (float &sample : pcm)
         sample = std::clamp(sample * static_cast<float>(sourceChannels), -1.f, 1.f);

   int err = 0;
   OpusEncoder *const encoder = opus_encoder_create(rate, channels, OPUS_APPLICATION_AUDIO, &err);
   if (encoder == nullptr || err != OPUS_OK)
      return {};
   opus_encoder_ctl(encoder, OPUS_SET_BITRATE(bitrate));
   int lookahead = 0;
   opus_encoder_ctl(encoder, OPUS_GET_LOOKAHEAD(&lookahead));
   // Pre-skip and granulepos are counted in 48kHz samples, the fixed Opus decode rate (RFC 7845):
   // converting the encoder lookahead (in 'rate' units) is exact for every supported rate
   const int to48k = 48000 / rate;

   ogg_stream_state stream;
   ogg_stream_init(&stream, 0x56505853); // Arbitrary stream serial
   vector<uint8_t> out;
   const auto writePages = [&stream, &out](const bool flush)
   {
      ogg_page page;
      while ((flush ? ogg_stream_flush : ogg_stream_pageout)(&stream, &page))
      {
         out.insert(out.end(), page.header, page.header + page.header_len);
         out.insert(out.end(), page.body, page.body + page.body_len);
      }
   };

   // Opus identification header (RFC 7845), on its own BOS page
   {
      uint8_t head[19];
      memcpy(head, "OpusHead", 8);
      head[8] = 1; // Version
      head[9] = static_cast<uint8_t>(channels);
      head[10] = static_cast<uint8_t>((lookahead * to48k) & 0xFF);
      head[11] = static_cast<uint8_t>(((lookahead * to48k) >> 8) & 0xFF);
      head[12] = static_cast<uint8_t>(sourceRate & 0xFF); // Original rate (informational only, playback is always 48kHz)
      head[13] = static_cast<uint8_t>((sourceRate >> 8) & 0xFF);
      head[14] = static_cast<uint8_t>((sourceRate >> 16) & 0xFF);
      head[15] = static_cast<uint8_t>((sourceRate >> 24) & 0xFF);
      head[16] = head[17] = 0; // Output gain
      head[18] = 0; // Channel mapping family 0 (mono/stereo)
      ogg_packet packet { head, 19, 1, 0, 0, 0 };
      ogg_stream_packetin(&stream, &packet);
   }
   // Comment header (OpusTags), on its own page
   {
      static constexpr char vendor[] = "VPinball";
      uint8_t tags[8 + 4 + sizeof(vendor) - 1 + 4] = {};
      memcpy(tags, "OpusTags", 8);
      constexpr uint32_t vendorLength = sizeof(vendor) - 1;
      tags[8] = vendorLength & 0xFF;
      tags[9] = (vendorLength >> 8) & 0xFF;
      tags[10] = (vendorLength >> 16) & 0xFF;
      tags[11] = (vendorLength >> 24) & 0xFF;
      memcpy(tags + 12, vendor, vendorLength); // Zero user comments
      ogg_packet packet { tags, static_cast<long>(sizeof(tags)), 0, 0, 0, 1 };
      ogg_stream_packetin(&stream, &packet);
   }
   writePages(true);

   // Audio packets, 20ms frames (960 samples @48kHz, down to 160 @8kHz). granulepos counts
   // the decoded samples in 48kHz units plus the pre-skip lookahead: decoders drop pre-skip
   // samples and report the real length.
   const int frameSamples = rate / 50;
   std::array<uint8_t, 8192> packetData;
   std::array<float, 960 * 2> input;
   int64_t granulepos = lookahead * to48k;
   int64_t packetNo = 2;
   for (size_t pos = 0; pos < pcm.size();)
   {
      const size_t count = std::min(pcm.size() - pos, static_cast<size_t>(frameSamples * channels));
      const int frameSize = static_cast<int>(count / channels);
      input.fill(0.f);
      memcpy(input.data(), pcm.data() + pos, count * sizeof(float));
      pos += count;
      granulepos += static_cast<int64_t>(frameSize) * to48k;
      const int length = opus_encode_float(encoder, input.data(), frameSamples, packetData.data(), static_cast<opus_int32>(packetData.size()));
      if (length <= 0)
      {
         ogg_stream_clear(&stream);
         opus_encoder_destroy(encoder);
         return {};
      }
      ogg_packet packet { packetData.data(), length, 0, pos >= pcm.size() ? 1 : 0, granulepos, packetNo++ };
      ogg_stream_packetin(&stream, &packet);
      writePages(false);
   }
   writePages(true);
   ogg_stream_clear(&stream);
   opus_encoder_destroy(encoder);

   // Reject what the playback decoder cannot open instead of storing an invalid stream
   ma_decoder_config checkConfig = ma_decoder_config_init(ma_format_f32, 0, 0);
   VPX::AddOpusDecoder(checkConfig);
   ma_decoder check;
   const bool valid = ma_decoder_init_memory(out.data(), out.size(), &checkConfig, &check) == MA_SUCCESS;
   if (valid)
      ma_decoder_uninit(&check);
   return valid ? out : vector<uint8_t> {};
}

bool Is16BitImageType(const FREE_IMAGE_TYPE type) { return type == FIT_UINT16 || type == FIT_RGB16 || type == FIT_RGBA16; }

// Formats that are (usually) lossy compressed: re-encoding them lossless would mostly make them larger
bool IsLossyImageFormat(const FREE_IMAGE_FORMAT fif) { return fif == FIF_JPEG || fif == FIF_J2K || fif == FIF_JP2 || fif == FIF_JXR; }

FREE_IMAGE_FORMAT GetImageFileFormat(const uint8_t *data, const size_t size)
{
   FIMEMORY *const mem = FreeImage_OpenMemory(const_cast<BYTE *>(data), static_cast<DWORD>(size));
   if (mem == nullptr)
      return FIF_UNKNOWN;
   const FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(mem);
   FreeImage_CloseMemory(mem);
   return fif;
}

FIBITMAP *DecodeImageFile(const uint8_t *data, const size_t size)
{
   FIMEMORY *const mem = FreeImage_OpenMemory(const_cast<BYTE *>(data), static_cast<DWORD>(size));
   if (mem == nullptr)
      return nullptr;
   const FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(mem);
   FIBITMAP *const bitmap = fif != FIF_UNKNOWN && FreeImage_FIFSupportsReading(fif) ? FreeImage_LoadFromMemory(fif, mem, fif == FIF_PNG ? PNG_IGNOREGAMMA : 0) : nullptr;
   FreeImage_CloseMemory(mem);
   return bitmap;
}

// Decoded pixels of an image file, as 32 bit BGRA or as RGBA floats, to compare files across formats. Empty if not decodable.
struct ComparablePixels
{
   unsigned int width = 0, height = 0;
   vector<uint8_t> bytes;
   bool operator==(const ComparablePixels &) const = default;
};
ComparablePixels DecodeComparablePixels(const uint8_t *data, const size_t size, const bool asFloat)
{
   ComparablePixels pixels;
   FIBITMAP *const bitmap = DecodeImageFile(data, size);
   if (bitmap == nullptr)
      return pixels;
   FIBITMAP *const normalized = asFloat ? FreeImage_ConvertToRGBAF(bitmap) : FreeImage_ConvertTo32Bits(bitmap);
   FreeImage_Unload(bitmap);
   if (normalized == nullptr)
      return pixels;
   pixels.width = FreeImage_GetWidth(normalized);
   pixels.height = FreeImage_GetHeight(normalized);
   for (unsigned int y = 0; y < pixels.height; y++)
      pixels.bytes.insert(pixels.bytes.end(), FreeImage_GetScanLine(normalized, y), FreeImage_GetScanLine(normalized, y) + FreeImage_GetLine(normalized));
   FreeImage_Unload(normalized);
   return pixels;
}

// WebP quality (1..100) for re-encoding a lossy image: the JPEG's estimated quality, else 75 (libwebp's default)
int LossyWebPQuality(const uint8_t *data, const size_t size)
{
   bool exact;
   const int quality = TableAudit::EstimateJpegQuality(data, size, exact);
   return quality > 0 ? quality : 75;
}

// Re-encodes an image's file data to WebP (LDR) or EXR (HDR). Returns the new data, or nothing with the reason in 'failure'. The
// result must be smaller than the current data and, unless lossy as listed below, decode to exactly the same pixels:
// - WebP: lossless (a 16 bit per channel source reduced to 8 bit first, as VPX renders it), or lossy for a lossy source (JPEG,
//   JPEG 2000, JPEG XR) at LossyWebPQuality. FreeImage leaves libwebp's 'exact' mode off, so a lossless encode may change the
//   color of fully transparent pixels (seen through filtering at alpha edges): the comparison rejects it.
// - EXR (PIZ compression): half floats from a 32 bit float source (lower precision accepted) unless a value would be clamped to the
//   half float range, else 32 bit floats, exactly.
vector<uint8_t> EncodeImage(const Texture &image, const FREE_IMAGE_FORMAT format, string &failure)
{
   const FREE_IMAGE_FORMAT fif = GetImageFileFormat(image.GetFileRaw(), image.GetFileSize());
   FIBITMAP *bitmap = DecodeImageFile(image.GetFileRaw(), image.GetFileSize());
   if (bitmap == nullptr)
   {
      failure = "the image file could not be decoded";
      return {};
   }
   const bool lossySource = IsLossyImageFormat(fif);

   // Convert to a type the target format can store, without loss
   const FREE_IMAGE_TYPE type = FreeImage_GetImageType(bitmap);
   FIBITMAP *converted = nullptr;
   if (format == FIF_WEBP)
   {
      // 16 bit per channel images are reduced to 8 bit, as VPX renders them (see Texture's loader): the comparison below is made
      // against that 8 bit version of the source
      if (type != FIT_BITMAP && !Is16BitImageType(type))
      {
         FreeImage_Unload(bitmap);
         failure = "the image is not 8 or 16 bit per channel";
         return {};
      }
      if (type != FIT_BITMAP || (FreeImage_GetBPP(bitmap) != 24 && FreeImage_GetBPP(bitmap) != 32)) // FreeImage's WebP encoder only takes 24/32 bit
         converted = FreeImage_IsTransparent(bitmap) ? FreeImage_ConvertTo32Bits(bitmap) : FreeImage_ConvertTo24Bits(bitmap);
   }
   else if (type != FIT_RGBF && type != FIT_RGBAF)
      converted = type == FIT_RGBA16 || (type == FIT_BITMAP && FreeImage_IsTransparent(bitmap)) ? FreeImage_ConvertToRGBAF(bitmap) : FreeImage_ConvertToRGBF(bitmap);
   if (converted != nullptr)
   {
      FreeImage_Unload(bitmap);
      bitmap = converted;
   }

   // Half floats only from a 32 bit float source (not e.g. 16 bit integers), and only if no value would be clamped
   bool toHalf = false;
   if (format == FIF_EXR && (type == FIT_RGBF || type == FIT_RGBAF))
   {
      toHalf = true;
      const unsigned int nFloats = FreeImage_GetWidth(bitmap) * (type == FIT_RGBAF ? 4 : 3);
      for (unsigned int y = 0; y < FreeImage_GetHeight(bitmap) && toHalf; y++)
      {
         const float *const values = reinterpret_cast<const float *>(FreeImage_GetScanLine(bitmap, y));
         for (unsigned int i = 0; i < nFloats && toHalf; i++)
            toHalf = !std::isfinite(values[i]) || std::abs(values[i]) <= 65504.f; // Largest half float
      }
   }
   const bool exact = !lossySource && !toHalf;
   // FreeImage takes the lossy WebP quality (1..100) in the low bits of the save flags
   const int saveFlags = format == FIF_WEBP ? (lossySource ? LossyWebPQuality(image.GetFileRaw(), image.GetFileSize()) : WEBP_LOSSLESS) : toHalf ? EXR_DEFAULT : EXR_FLOAT;

   vector<uint8_t> out;
   FIMEMORY *const dst = FreeImage_OpenMemory();
   if (dst != nullptr)
   {
      if (FreeImage_SaveToMemory(format, bitmap, dst, saveFlags))
      {
         out.resize(FreeImage_TellMemory(dst));
         FreeImage_SeekMemory(dst, 0, SEEK_SET);
         FreeImage_ReadMemory(out.data(), 1, static_cast<unsigned int>(out.size()), dst);
      }
      FreeImage_CloseMemory(dst);
   }
   FreeImage_Unload(bitmap);
   if (out.empty())
   {
      failure = "the image could not be encoded";
      return {};
   }
   if (exact)
   {
      const bool asFloat = format == FIF_EXR;
      if (DecodeComparablePixels(out.data(), out.size(), asFloat) != DecodeComparablePixels(image.GetFileRaw(), image.GetFileSize(), asFloat))
      {
         failure = format == FIF_WEBP ? "the lossless WebP encoder would alter pixels (likely the colors of fully transparent pixels)" : "EXR can not store the image values exactly";
         return {};
      }
   }
   else if (FIBITMAP *const check = DecodeImageFile(out.data(), out.size()); check != nullptr)
      FreeImage_Unload(check);
   else
   {
      failure = "the re-encoded image could not be decoded";
      return {};
   }
   if (out.size() >= image.GetFileSize())
   {
      failure = "the re-encoded file would not be smaller (" + SizeToReadable(out.size()) + " instead of " + SizeToReadable(image.GetFileSize()) + ')';
      return {};
   }
   failure.clear();
   return out;
}

// 'path' with its extension replaced, or built from the asset name if empty
std::filesystem::path PathWithExtension(const std::filesystem::path &path, const string &name, const char *extension)
{
   if (path.empty())
      return PathFromUTF8(name + extension);
   std::filesystem::path renamed = path;
   renamed.replace_extension(extension);
   return renamed;
}

std::filesystem::path StoredPath(const VPX::Sound &sound) { return sound.GetImportPath(); }
std::filesystem::path StoredPath(const Texture &image) { return image.GetFilePath(); }

// Undo/redo action restoring an asset's current file data. Asset deletes are not undoable, so the asset may be gone when the record
// is replayed: the snapshot and the restore then check that it is still in 'assets' (a vector of the table, which outlives its undo).
template <typename TAsset> std::function<void()> FileDataRestorer(const vector<TAsset *> &assets, TAsset *const asset)
{
   if (std::ranges::find(assets, asset) == assets.end())
      return [] { };
   return [&assets, asset, path = StoredPath(*asset), data = vector<uint8_t>(asset->GetFileRaw(), asset->GetFileRaw() + asset->GetFileSize())]()
   {
      if (std::ranges::find(assets, asset) != assets.end())
         asset->SetFromFileData(path, data);
   };
}

// Opus bitrate (bit/s) of a re-encode to 'channels' channels at 'encodeRate' (see OpusEncodeRate) for a lossless source, and the
// maximum for a lossy one. At 48kHz, 96 kbit/s mono and 128 kbit/s stereo (the upper part of Xiph's transparent range for music).
// A lower rate has less bandwidth to code, the bitrate is scaled down less than in proportion (quality first): 0.75x at 24kHz,
// 0.625x at 16kHz, 0.5x at 12/8kHz.
int OpusMaxBitrate(const int channels, const int encodeRate)
{
   const int bitrate = channels == 1 ? 96000 : 128000;
   return encodeRate >= 48000 ? bitrate : encodeRate >= 24000 ? bitrate * 3 / 4 : encodeRate >= 16000 ? bitrate * 5 / 8 : bitrate / 2;
}

// Opus bitrate (bit/s) of a re-encode to 'channels' channels. A lossy source of known bitrate is matched in quality, after listening
// tests (Opus is about as good as MP3 at 0.7x its bitrate, as Vorbis at the same bitrate): 0.8x an MP3 (a margin for the second
// generation of loss), 1x Vorbis/Opus, 0.75x more for a mono downmix of a multichannel source, within 24 kbit/s .. OpusMaxBitrate.
int OpusBitrate(const int channels, const int sourceChannels, const int encodeRate, const TableAudit::SoundFileInfo &source)
{
   const int maxBitrate = OpusMaxBitrate(channels, encodeRate);
   if (source.bitrate <= 0)
      return maxBitrate;
   double bitrate = source.bitrate * (source.codec == TableAudit::SoundFileCodec::MP3 ? 0.8 : 1.0);
   if (channels == 1 && sourceChannels > 1)
      bitrate *= 0.75;
   return std::clamp(static_cast<int>(bitrate), std::min(24000, maxBitrate), maxBitrate);
}

// Undoable re-encoding of a sound's file data as Ogg Opus (see EncodeSoundToOggOpus). 'onlyIfSmaller' is for re-encodes meant to
// save space (not to fix the channel count).
std::function<IEditable *(PinTable &table, PinUndo &undo)> ReencodeSoundFix(
   VPX::Sound *sound, const int channels, const int bitrate, const bool onlyIfSmaller, const std::shared_ptr<string> &failure)
{
   return [sound, channels, bitrate, onlyIfSmaller, failure](PinTable &table, PinUndo &undo) -> IEditable *
   {
      vector<uint8_t> data = EncodeSoundToOggOpus(*sound, channels, bitrate);
      if (data.empty())
      {
         *failure = "Sound '" + sound->GetName() + "' kept as is: it could not be re-encoded to Ogg Opus";
         PLOGE << *failure;
         return nullptr;
      }
      if (onlyIfSmaller && data.size() >= sound->GetFileSize())
      {
         *failure = "Sound '" + sound->GetName() + "' kept as is: the re-encoded file would not be smaller (" + SizeToReadable(data.size()) + " instead of "
            + SizeToReadable(sound->GetFileSize()) + ')';
         PLOGI << *failure;
         return nullptr;
      }
      failure->clear();
      undo.MarkForAction(FileDataRestorer(table.m_vsound, sound), [&table, sound]() { return FileDataRestorer(table.m_vsound, sound); });
      sound->SetFromFileData(PathWithExtension(sound->GetImportPath(), sound->GetName(), ".opus"), std::move(data));
      return nullptr;
   };
}

// Undoable re-encoding of an image's file data to another format (see EncodeImage)
std::function<IEditable *(PinTable &table, PinUndo &undo)> ReencodeImageFix(Texture *image, const FREE_IMAGE_FORMAT format, const char *extension, const std::shared_ptr<string> &failure)
{
   return [image, format, extension, failure](PinTable &table, PinUndo &undo) -> IEditable *
   {
      string reason;
      vector<uint8_t> data = EncodeImage(*image, format, reason);
      if (data.empty())
      {
         *failure = "Image '" + image->m_name + "' kept as is: " + reason;
         PLOGI << *failure;
         return nullptr;
      }
      failure->clear();
      undo.MarkForAction(FileDataRestorer(table.m_vimage, image), [&table, image]() { return FileDataRestorer(table.m_vimage, image); });
      image->SetFromFileData(PathWithExtension(image->GetFilePath(), image->m_name, extension), std::move(data));
      return nullptr;
   };
}
}

vector<TableAuditItem> TableAudit::SoundFixes(VPX::Sound &sound, const VPX::SoundSpec &specs)
{
   vector<TableAuditItem> fixes;
   const int channels = static_cast<int>(specs.nChannels);
   if (channels <= 0)
      return fixes;
   const SoundFileInfo source = AnalyzeSoundFile(sound, specs.lengthInSeconds);
   const int encodeRate = OpusEncodeRate(static_cast<int>(specs.sampleFrequency));
   // Bitrate part of the details text
   const auto bitrateText = [&source](const int bitrate)
   { return "at " + std::to_string(bitrate / 1000) + " kbit/s" + (source.bitrate > 0 ? " (the source has " + std::to_string(source.bitrate / 1000) + " kbit/s)" : ""s); };
   // Mono fix: a correctness fix, always offered
   if (channels > 1 && sound.GetOutputTarget() == VPX::SNDOUT_TABLE)
   {
      const int bitrate = OpusBitrate(1, channels, encodeRate, source);
      TableAuditItem &item = fixes.emplace_back();
      item.severity = TableAuditItem::Severity::Error;
      item.summary = "sound '" + sound.GetName() + "' is used for playfield physical sound but has multiple channels (not mono).";
      item.details = item.summary + "\nThe fix downmixes it to mono, keeping its loudness, and stores it as Ogg Opus " + bitrateText(bitrate)
         + ", which is lossy: a sound that was already lossy compressed gets a second generation of compression loss.";
      item.sound = &sound;
      item.fixFailure = std::make_shared<string>();
      item.fix = ReencodeSoundFix(&sound, 1, bitrate, false, item.fixFailure);
      item.fixLabel = "Re-encode as mono Ogg Opus (lossy)";
   }
   // Space saving fix: for a lossless source, or a lossy one of at least 1.5x OpusMaxBitrate (encoded at OpusMaxBitrate). Below, a
   // re-encode matching the source quality saves less than a third (or nothing), at the cost of a second generation of loss. Not
   // for unknown formats or bitrates, nor for low quality sounds (11kHz or less, 8 bit): already small, and the lossy codec would
   // add its artifacts to their noise.
   const int outChannels = std::min(channels, 2); // Opus' simple channel mapping only supports mono/stereo
   const bool lossless = source.codec == SoundFileCodec::Lossless;
   const bool lowQuality = specs.sampleFrequency <= 11025 || (source.bitsPerSample > 0 && source.bitsPerSample <= 8);
   if (!specs.isOpus && !lowQuality
      && (lossless || (source.codec != SoundFileCodec::Unknown && source.bitrate >= OpusMaxBitrate(outChannels, encodeRate) * 3 / 2)))
   {
      const int bitrate = OpusBitrate(outChannels, channels, encodeRate, source);
      TableAuditItem &item = fixes.emplace_back();
      item.severity = TableAuditItem::Severity::Info;
      item.summary = "sound '" + sound.GetName() + "' is not stored in the 'Ogg Opus' format, which is " + (lossless ? "much more compact"s : "more compact at this quality"s);
      item.details = item.summary + ".\nIt is re-encoded as Ogg Opus " + bitrateText(bitrate)
         + (lossless ? ", which is lossy."s : ", which is lossy: being already lossy compressed, the sound gets a second generation of compression loss."s)
         + " The sound is kept as is if the re-encoded file would not be smaller.";
      item.sound = &sound;
      item.fixFailure = std::make_shared<string>();
      item.fix = ReencodeSoundFix(&sound, outChannels, bitrate, true, item.fixFailure);
      item.fixLabel = "Re-encode as Ogg Opus (lossy)";
   }
   return fixes;
}

vector<TableAuditItem> TableAudit::ImageFixes(PinTable &table, Texture &image)
{
   vector<TableAuditItem> fixes;
   if (table.GetImageLink(&image))
      return fixes; // Linked (shared) images are managed outside of the table: do not re-encode them
   const string extension = lowerCase(PathToUTF8(image.GetFilePath().extension()));
   if (image.IsHDR() ? extension != ".exr" : extension != ".webp")
   {
      const FREE_IMAGE_FORMAT fif = GetImageFileFormat(image.GetFileRaw(), image.GetFileSize());
      const bool lossy = !image.IsHDR() && IsLossyImageFormat(fif);
      string lossyQuality;
      if (lossy)
      {
         bool exact;
         const int jpegQuality = EstimateJpegQuality(image.GetFileRaw(), image.GetFileSize(), exact);
         lossyQuality = jpegQuality == 0 ? "quality 75"s
            : "quality " + std::to_string(jpegQuality) + (exact ? ", the quality of the JPEG"s : ", an estimate of the JPEG's quality from its quantization tables"s);
         if (JpegHasFullColorResolution(image.GetFileRaw(), image.GetFileSize()))
            lossyQuality += "; this JPEG stores colors at full resolution (4:4:4) while lossy WebP halves it in both directions (4:2:0): fine colored details soften";
      }
      // 16 bit per channel (only checked for PNG/TIFF, whose loaders can read the header alone)
      bool is16Bit = false;
      if (!image.IsHDR() && (fif == FIF_PNG || fif == FIF_TIFF))
         if (FIMEMORY *const mem = FreeImage_OpenMemory(const_cast<BYTE *>(image.GetFileRaw()), static_cast<DWORD>(image.GetFileSize())); mem != nullptr)
         {
            if (FIBITMAP *const header = FreeImage_LoadFromMemory(fif, mem, FIF_LOAD_NOPIXELS); header != nullptr)
            {
               is16Bit = Is16BitImageType(FreeImage_GetImageType(header));
               FreeImage_Unload(header);
            }
            FreeImage_CloseMemory(mem);
         }
      const string target = image.IsHDR() ? "EXR"s : "WebP"s;
      TableAuditItem &item = fixes.emplace_back();
      item.severity = TableAuditItem::Severity::Info;
      item.summary = (image.IsHDR() ? "HDR image '"s : "Image '"s) + image.m_name + "' is stored in the '" + extension + "' format instead of '" + target + "', which is usually more compact";
      item.details = item.summary + ".\n"
         + (lossy ? "The image is lossy compressed: it is re-encoded as lossy WebP (" + lossyQuality + "), which adds a second generation of compression loss. "
            : image.IsHDR() ? "A 32 bit float image is stored with half floats (lower precision) unless some of its values are beyond the half float range, "
                              "then with 32 bit floats, exactly. Other images are stored exactly. "s
            : is16Bit ? "The image has 16 bits per channel but VPX renders it with 8 bits: it is reduced to these 8 bits (the extra precision is dropped), "
                        "then stored lossless, checked to have exactly the pixels VPX renders. "s
                            : "The re-encode is lossless: it is only applied if the re-encoded image has exactly the same pixels. "s)
         + "The image is kept as is if the re-encoded file would not be smaller.";
      item.image = &image;
      item.fixFailure = std::make_shared<string>();
      item.fix = ReencodeImageFix(&image, image.IsHDR() ? FIF_EXR : FIF_WEBP, image.IsHDR() ? ".exr" : ".webp", item.fixFailure);
      // HDR: no lossless/lossy in the label, half floats are only chosen once the image is decoded
      item.fixLabel = "Re-encode as " + target + (lossy ? " (lossy)" : image.IsHDR() ? "" : is16Bit ? " (as rendered, 8 bit)" : " (lossless)");
   }
   return fixes;
}

TableAudit::SoundFileInfo TableAudit::AnalyzeSoundFile(const VPX::Sound &sound, const float lengthInSeconds)
{
   SoundFileInfo info;
   const uint8_t *const data = sound.GetFileRaw();
   const size_t size = sound.GetFileSize();
   if (size < 16)
      return info;
   const auto readLE16 = [data](const size_t pos) { return static_cast<unsigned int>(data[pos] | (data[pos + 1] << 8)); };
   const auto readLE32 = [data](const size_t pos) { return static_cast<size_t>(data[pos] | (data[pos + 1] << 8) | (data[pos + 2] << 16)) | (static_cast<size_t>(data[pos + 3]) << 24); };
   size_t payload = 0;
   if (memcmp(data, "RIFF", 4) == 0 && memcmp(data + 8, "WAVE", 4) == 0)
   {
      // WAV: lossless if PCM or float (also as WAVE_FORMAT_EXTENSIBLE sub format), unknown otherwise (ADPCM, MP3, ...)
      for (size_t pos = 12; pos + 8 <= size;)
      {
         const size_t chunkSize = readLE32(pos + 4);
         if (memcmp(data + pos, "fmt ", 4) == 0 && pos + 10 <= size)
         {
            unsigned int format = readLE16(pos + 8);
            if (format == 0xFFFE && chunkSize >= 26 && pos + 34 <= size)
               format = readLE16(pos + 32);
            if (format == 1 || format == 3)
               info.codec = SoundFileCodec::Lossless;
            if (pos + 24 <= size)
               info.bitsPerSample = static_cast<int>(readLE16(pos + 22));
            return info;
         }
         pos += 8 + chunkSize + (chunkSize & 1);
      }
      return info;
   }
   if (memcmp(data, "fLaC", 4) == 0)
   {
      info.codec = SoundFileCodec::Lossless;
      if (size >= 22)
         info.bitsPerSample = (((data[20] & 1) << 4) | (data[21] >> 4)) + 1; // STREAMINFO, the first metadata block
      return info;
   }
   if (memcmp(data, "OggS", 4) == 0)
   {
      // Ogg: the codec from the first packet. Lossy: the bodies of the audio pages. Header pages (granule position 0) are left
      // out, as the Vorbis setup header alone (several KB) would make a short sound look like a high bitrate one.
      const size_t firstPacket = 27 + data[26];
      if (firstPacket + 8 <= size && memcmp(data + firstPacket, "\x7F" "FLAC", 5) == 0)
         info.codec = SoundFileCodec::Lossless;
      else if (firstPacket + 8 <= size && (memcmp(data + firstPacket, "\x01vorbis", 7) == 0 || memcmp(data + firstPacket, "OpusHead", 8) == 0))
         info.codec = SoundFileCodec::OggLossy;
      if (info.codec != SoundFileCodec::OggLossy)
         return info;
      for (size_t pos = 0; pos + 27 <= size && memcmp(data + pos, "OggS", 4) == 0;)
      {
         const size_t nSegments = data[pos + 26];
         if (pos + 27 + nSegments > size)
            break;
         size_t bodySize = 0;
         for (size_t i = 0; i < nSegments; i++)
            bodySize += data[pos + 27 + i];
         if (std::any_of(data + pos + 6, data + pos + 14, [](const uint8_t b) { return b != 0; }))
            payload += bodySize;
         pos += 27 + nSegments + bodySize;
      }
   }
   else
   {
      // MP3: the frames, without the ID3 tags (which may hold large pictures)
      size_t start = 0, end = size;
      if (memcmp(data, "ID3", 3) == 0)
         start = 10 + ((data[6] & 0x7F) << 21 | (data[7] & 0x7F) << 14 | (data[8] & 0x7F) << 7 | (data[9] & 0x7F)) + ((data[5] & 0x10) ? 10 : 0); // Syncsafe size, footer
      if (size >= 128 && memcmp(data + size - 128, "TAG", 3) == 0)
         end = size - 128; // ID3v1
      if (start + 2 > end || data[start] != 0xFF || (data[start + 1] & 0xE0) != 0xE0)
         return info; // Unknown format
      info.codec = SoundFileCodec::MP3;
      payload = end - start;
   }
   if (lengthInSeconds > 0.f)
      info.bitrate = static_cast<int>(static_cast<double>(payload) * 8.0 / lengthInSeconds);
   return info;
}

bool TableAudit::JpegHasFullColorResolution(const uint8_t *data, const size_t size)
{
   if (size < 4 || data[0] != 0xFF || data[1] != 0xD8)
      return false;
   for (size_t pos = 2; pos + 4 <= size;)
   {
      if (data[pos] != 0xFF)
         return false;
      const uint8_t marker = data[pos + 1];
      if (marker == 0xFF)
      {
         pos++; // Fill byte
         continue;
      }
      if (marker == 0x01 || (marker >= 0xD0 && marker <= 0xD7))
      {
         pos += 2; // Markers without a segment
         continue;
      }
      if (marker == 0xDA || marker == 0xD9)
         return false; // No frame header before the scan data
      const size_t length = (static_cast<size_t>(data[pos + 2]) << 8) | data[pos + 3];
      if (length < 2 || pos + 2 + length > size)
         return false;
      // Start of frame (SOF0..SOF15, except DHT C4, JPG C8 and DAC CC): the components' sampling factors
      if (marker >= 0xC0 && marker <= 0xCF && marker != 0xC4 && marker != 0xC8 && marker != 0xCC)
      {
         const size_t nComponents = length >= 8 ? data[pos + 9] : 0;
         if (nComponents < 3 || length < 8 + 3 * nComponents)
            return false; // Grayscale
         for (size_t i = 1; i < nComponents; i++)
            if (data[pos + 11 + 3 * i] != data[pos + 11])
               return false;
         return true;
      }
      pos += 2 + length;
   }
   return false;
}

int TableAudit::EstimateJpegQuality(const uint8_t *data, const size_t size, bool &exact)
{
   exact = false;
   // Standard luminance and chrominance quantization tables (JPEG spec Annex K), in natural (row) order.
   // IJG libjpeg v9e+ uses a chrominance DC base coefficient of 16 instead of the spec's 17, for lossless support (as does the one bundled
   // in FreeImage): both variants are accepted below.
   static constexpr std::array<int, 64> standardTables[2] = {
      { 16, 11, 10, 16, 24, 40, 51, 61, 12, 12, 14, 19, 26, 58, 60, 55, 14, 13, 16, 24, 40, 57, 69, 56, 14, 17, 22, 29, 51, 87, 80, 62, //
         18, 22, 37, 56, 68, 109, 103, 77, 24, 35, 55, 64, 81, 104, 113, 92, 49, 64, 78, 87, 103, 121, 120, 101, 72, 92, 95, 98, 112, 100, 103, 99 },
      { 17, 18, 24, 47, 99, 99, 99, 99, 18, 21, 26, 66, 99, 99, 99, 99, 24, 26, 56, 99, 99, 99, 99, 99, 47, 66, 99, 99, 99, 99, 99, 99, //
         99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 } };
   // Natural position of the coefficients in the zigzag order used by the DQT segments
   static constexpr std::array<int, 64> naturalOrder = { 0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5, 12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28,
      35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51, 58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63 };

   // Quantization tables 0 (luminance) and 1 (chrominance), in natural order, from the DQT segments before the scan data
   if (size < 4 || data[0] != 0xFF || data[1] != 0xD8)
      return 0;
   std::array<int, 64> tables[2];
   bool hasTable[2] = { false, false }, is16bit[2] = { false, false };
   for (size_t pos = 2; pos + 4 <= size;)
   {
      if (data[pos] != 0xFF)
         return 0; // Not a marker: broken file
      const uint8_t marker = data[pos + 1];
      if (marker == 0xFF)
      {
         pos++; // Fill byte
         continue;
      }
      if (marker == 0x01 || (marker >= 0xD0 && marker <= 0xD7))
      {
         pos += 2; // Markers without a segment
         continue;
      }
      if (marker == 0xDA || marker == 0xD9)
         break; // Start of scan data or end of image: no more tables
      const size_t length = (static_cast<size_t>(data[pos + 2]) << 8) | data[pos + 3];
      if (length < 2 || pos + 2 + length > size)
         return 0;
      if (marker == 0xDB)
      {
         for (size_t p = pos + 4, end = pos + 2 + length; p < end;)
         {
            const bool wide = (data[p] >> 4) != 0;
            const int id = data[p] & 0x0F;
            p++;
            if (p + (wide ? 128 : 64) > end)
               return 0;
            for (int k = 0; k < 64; k++, p += wide ? 2 : 1)
               if (id < 2)
                  tables[id][naturalOrder[k]] = wide ? (data[p] << 8) | data[p + 1] : data[p];
            if (id < 2)
            {
               hasTable[id] = true;
               is16bit[id] = wide;
            }
         }
      }
      pos += 2 + length;
   }
   if (!hasTable[0])
      return 0;

   // Same tables as libjpeg writes for 'quality' (jpeg_quality_scaling, jpeg_add_quant_table: 8 bit tables are clamped to 255)
   const auto matches = [&](const int quality)
   {
      const int scale = quality < 50 ? 5000 / quality : 200 - quality * 2;
      const auto scaledValue = [scale](const int base, const int maxValue) { return std::clamp((base * scale + 50) / 100, 1, maxValue); };
      for (int t = 0; t < 2; t++)
         if (hasTable[t])
            for (int i = 0; i < 64; i++)
            {
               const int maxValue = is16bit[t] ? 32767 : 255;
               const bool matchesStandard = tables[t][i] == scaledValue(standardTables[t][i], maxValue);
               const bool matchesLibjpeg9eDc = t == 1 && i == 0 && tables[t][i] == scaledValue(16, maxValue);
               if (!matchesStandard && !matchesLibjpeg9eDc)
                  return false;
            }
      return true;
   };
   for (int quality = 100; quality >= 1; quality--)
      if (matches(quality))
      {
         exact = true;
         return quality;
      }

   // Other tables: the quality whose scaling gives the same luminance table sum
   int sum = 0, standardSum = 0;
   for (int i = 0; i < 64; i++)
   {
      sum += tables[0][i];
      standardSum += standardTables[0][i];
   }
   const float scale = 100.f * static_cast<float>(sum) / static_cast<float>(standardSum);
   const float quality = scale <= 100.f ? (200.f - scale) * 0.5f : 5000.f / scale;
   return std::clamp(static_cast<int>(std::lround(quality)), 1, 100);
}

TableAudit TableAudit::Run(PinTable &table, const std::function<VPX::SoundSpec(const VPX::Sound &)> &soundInformations)
{
   // Perform a simple table audit (disable lighting vs static, script reference of static parts, png vs webp, hdr vs exr,...)
   TableAudit audit;
   auto &items = audit.items;
   const auto addItem = [&items](TableAuditItem::Severity severity, const string &summary, const string &details = {}) -> TableAuditItem &
   {
      items.push_back({ .severity = severity, .summary = summary, .details = details.empty() ? summary : details });
      return items.back();
   };

   vector<string> functions;
   vector<string> identifiers;
   table.ParseScript(table.m_script_text, functions, identifiers,
      [&addItem](const string &functionName, int line)
      {
         TableAuditItem &item = addItem(TableAuditItem::Severity::Error, "Duplicate declaration of '" + functionName + "' in script at line " + std::to_string(line));
         item.scriptLine = line + 1; // 'line' counts lines from 0, the script editor is 1-based
      });

   if (FindIndexOf(identifiers, "execute"s) != -1)
      addItem(TableAuditItem::Severity::Warning, "Scripts seems to use the 'Execute' command. This command triggers computer security checks and will likely cause stutters during play.");

   // Glass height fixes: restore the default height of a new table (see PinTable::m_glassBottomHeight)
   const auto fixGlassHeight = TableFieldsFix([](PinTable &table) { table.m_glassBottomHeight = table.m_glassTopHeight = 210.f; }, [&table]() -> std::vector<std::function<void()>>
      { return { [&table, value = table.m_glassBottomHeight]() { table.m_glassBottomHeight = value; }, [&table, value = table.m_glassTopHeight]() { table.m_glassTopHeight = value; } }; });
   if (table.m_glassBottomHeight > table.m_glassTopHeight)
   {
      TableAuditItem &item = addItem(TableAuditItem::Severity::Warning, "Glass height seems invalid: bottom is higher than top");
      item.part = &table;
      item.fix = fixGlassHeight;
      item.fixLabel = "Reset glass height to default";
   }
   if (table.m_glassBottomHeight < INCHESTOVPU(2) || table.m_glassTopHeight < INCHESTOVPU(2))
   {
      TableAuditItem &item = addItem(TableAuditItem::Severity::Warning, "Glass height seems invalid: glass is below 2\"");
      item.part = &table;
      item.fix = fixGlassHeight;
      item.fixLabel = "Reset glass height to default";
   }

   if (table.m_ballSphericalMapping)
   {
      TableAuditItem &item
         = addItem(TableAuditItem::Severity::Warning, "Ball uses legacy 'spherical mapping', it will be rendered like a 2D object and therefore will look bad in VR, stereo or headtracking");
      item.part = &table;
      item.fix = TableFieldsFix([](PinTable &table) { table.m_ballSphericalMapping = false; },
         [&table]() -> std::vector<std::function<void()>> { return { [&table, value = table.m_ballSphericalMapping]() { table.m_ballSphericalMapping = value; } }; });
      item.fixLabel = "Disable legacy spherical mapping";
   }

   for (const string &fix : table.m_loadFixes)
      addItem(TableAuditItem::Severity::Warning, fix);

   // Search for inconsistencies in the table parts
   bool hasPulseTimer = false, hasPinMameTimer = false;
   for (const auto part : table.GetParts())
   {
      auto type = part->GetItemType();
      Primitive *const prim = type == eItemPrimitive ? (Primitive *)part : nullptr;
      Light *const light = type == eItemLight ? (Light *)part : nullptr;
      Textbox *const textbox = type == eItemTextbox ? (Textbox *)part : nullptr;

      // Referencing a static object from script (ok if it is for reading properties, not for writing)
      if (type == eItemPrimitive && prim->m_d.m_staticRendering && FindIndexOf(identifiers, prim->GetName()) != -1)
         addItem(TableAuditItem::Severity::Warning,
            "Primitive '" + prim->GetName() + "' seems to be referenced from the script while it is marked as static (most properties of a static object may not be modified at runtime).")
            .part
            = part;

      if (type == eItemTextbox && (textbox->m_d.m_isDMD || StrFindNoCase(textbox->m_d.m_text, "DMD"s) != string::npos))
         addItem(TableAuditItem::Severity::Warning, "legacy Textbox '" + textbox->GetName() + "' is used for DMD rendering. It should be replaced by a flasher to get better rendering.").part
            = part;

      if (type == eItemTimer)
      {
         const string name = lowerCase(((Timer *)part)->GetName());
         hasPulseTimer |= name == "pulsetimer";
         hasPinMameTimer |= name == "pinmametimer";
      }

      // Warning on very fast timers (lower than 5ms)
      switch (type)
      {
      case eItemBall:
      // eItemDecal
      case eItemPartGroup:
      // eItemPrimitive
      case eItemTextbox:
      case eItemDispReel:
      case eItemSurface:
      case eItemTimer:
      case eItemLight:
      case eItemRamp:
      case eItemPlunger:
      case eItemSpinner:
      case eItemTrigger:
      case eItemKicker:
      case eItemRubber:
      case eItemFlasher:
      case eItemLightSeq:
      case eItemHitTarget:
      case eItemBumper:
      case eItemFlipper:
      case eItemGate:
         if (part->m_timerEnabled && part->m_timerInterval != -1 && part->m_timerInterval != -2 && part->m_timerInterval < 17)
            addItem(TableAuditItem::Severity::Warning,
               "Part '" + part->GetName() + "' uses a timer with a very short period of " + std::to_string(part->m_timerInterval)
                  + "ms, below a 60FPS framerate. This will likely cause stutters and the table will not support 'frame pacing'.")
               .part
               = part;
         break;
      // eItemTable
      // eItemLightCenter
      // eItemDragPoint
      // eItemCollection
      default: break;
      }

      if (type == eItemPrimitive && prim->m_d.m_visible && prim->m_d.m_disableLightingBelow != 1.f && !prim->m_d.m_staticRendering
         && (!table.GetMaterial(prim->m_d.m_szMaterial)->m_bOpacityActive || table.GetMaterial(prim->m_d.m_szMaterial)->m_fOpacity == 1.f)
         && (table.GetImage(prim->m_d.m_szImage) == nullptr || table.GetImage(prim->m_d.m_szImage)->IsOpaque()))
      {
         TableAuditItem &item = addItem(
            TableAuditItem::Severity::Warning, "Primitive '" + prim->GetName() + "' uses translucency (lighting from below) while it is fully opaque. Translucency will be discarded.");
         item.part = part;
         item.fix = PartFieldFix(prim, &PrimitiveData::m_disableLightingBelow, 1.f);
         item.fixLabel = "Disable translucency";
      }

      if (type == eItemLight && light->m_d.m_intensity < 0.f)
      {
         TableAuditItem &item = addItem(TableAuditItem::Severity::Error, "Light '" + light->GetName() + "' has a negative intensity.");
         item.part = part;
         item.fix = PartFieldFix(light, &LightData::m_intensity, 0.f);
         item.fixLabel = "Clamp intensity to 0";
      }
      if (type == eItemLight && light->m_d.m_intensity_scale < 0.f)
      {
         TableAuditItem &item = addItem(TableAuditItem::Severity::Error, "Light '" + light->GetName() + "' has a negative intensity scale.");
         item.part = part;
         item.fix = PartFieldFix(light, &LightData::m_intensity_scale, 0.f);
         item.fixLabel = "Clamp intensity scale to 0";
      }

      // Disabled as this is now enforced in the rendering
      // Enabling translucency (light from below) won't work with static parts: otherwise the rendering will be different in VR/Headtracked vs desktop modes. It also needs a non opaque alpha.
      //if (type == eItemPrimitive && prim->m_d.m_disableLightingBelow != 1.f && prim->m_d.m_staticRendering)
      //   ss << ". Warning: Primitive '" << prim->GetName() << "' has translucency enabled but is also marked as static. Translucency will not be applied on desktop, and it will look different between VR/headtracked and desktop.\r\n";
      //if (type == eItemSurface && surf->m_d.m_disableLightingBelow != 1.f && surf->StaticRendering())
      //   ss << ". Warning: Wall '" << surf->GetName() << "' has translucency enabled but will be statically rendered (not droppable with opaque materials). Translucency will not be applied on desktop, and it will look different between VR/headtracked and desktop.\r\n";
   }

   // The flasher VideoCap API (capturing the window of an external application) is deprecated in favor of plugins and may be removed in a future version
   if (FindIndexOf(identifiers, "videocapwidth"s) != -1 || FindIndexOf(identifiers, "videocapheight"s) != -1 || FindIndexOf(identifiers, "videocapupdate"s) != -1)
   {
      // Name the flashers referenced as '<name>.VideoCap...' (whole name, case insensitive like VBScript), or the script for an indirect use
      const string script = lowerCase(table.m_script_text);
      bool attributed = false;
      for (const auto part : table.GetParts())
      {
         if (part->GetItemType() != eItemFlasher)
            continue;
         const string ref = lowerCase(string(part->GetName())) + ".videocap";
         for (size_t pos = script.find(ref); pos != string::npos; pos = script.find(ref, pos + 1))
         {
            if (pos > 0 && (isalnum(static_cast<unsigned char>(script[pos - 1])) || script[pos - 1] == '_'))
               continue; // Part of a longer name
            addItem(TableAuditItem::Severity::Error,
               "Flasher '" + part->GetName()
                  + "' uses the deprecated 'VideoCap' API (VideoCapWidth/VideoCapHeight/VideoCapUpdate). It is superseded by plugins and may be removed in a future version.")
               .part
               = part;
            attributed = true;
            break;
         }
      }
      if (!attributed)
         addItem(TableAuditItem::Severity::Error,
            "script uses the deprecated Flasher 'VideoCap' API (VideoCapWidth/VideoCapHeight/VideoCapUpdate). It is superseded by plugins and may be removed in a future version.");
   }

   if ((FindIndexOf(identifiers, "loadvpm"s) != -1) || (FindIndexOf(identifiers, "loadvpmalt"s) != -1))
   {
      if (!hasPinMameTimer)
      {
         TableAuditItem &item = addItem(TableAuditItem::Severity::Warning, "VPM controller is used but table is missing a Timer object named 'PinMAMETimer'.");
         if (table.IsNameUnique("PinMAMETimer"s))
         {
            item.fix = CreateTimerFix("PinMAMETimer"s);
            item.fixLabel = "Create 'PinMAMETimer' timer";
         }
      }
      if (FindIndexOf(identifiers, "vpminit"s) == -1)
         addItem(TableAuditItem::Severity::Warning, "VPM controller is used but vpmInit is not called. pause/resume/exit will likely exhibit bugs and physic outputs won't be supported.");
   }

   if (!hasPulseTimer && (FindIndexOf(identifiers, "vpmTimer"s) != -1))
   {
      TableAuditItem &item = addItem(TableAuditItem::Severity::Warning, "script uses 'vpmTimer' but table is missing a Timer object named 'PulseTimer'. vpmTimer will not work as expected.");
      if (table.IsNameUnique("PulseTimer"s))
      {
         item.fix = CreateTimerFix("PulseTimer"s);
         item.fixLabel = "Create 'PulseTimer' timer";
      }
   }

   // Without a provider, a private audio player (no haptic output: only used to query sound informations)
   std::unique_ptr<VPX::AudioPlayer> audioPlayer;
   if (soundInformations == nullptr && !table.m_vsound.empty())
      audioPlayer = std::make_unique<VPX::AudioPlayer>(table.GetSettings().GetPlayer_SoundDeviceBG(), table.GetSettings().GetPlayer_SoundDevice(),
         static_cast<VPX::SoundConfigTypes>(table.GetSettings().GetPlayer_Sound3D()), static_cast<VPX::BackglassConfigTypes>(table.GetSettings().GetPlayer_Sound3DBackglass()), ""s,
         VPX::SNDCFG_SND3D2CH);
   for (const auto sound : table.m_vsound)
   {
      const VPX::SoundSpec specs = audioPlayer ? audioPlayer->GetSoundInformations(sound) : soundInformations(*sound);
      if (specs.nChannels == 0)
      {
         addItem(TableAuditItem::Severity::Error, "sound '" + sound->GetName() + "' could not be decoded (invalid or unsupported file)").sound = sound;
         continue; // An undecodable sound cannot be checked or re-encoded
      }
      for (TableAuditItem &fix : SoundFixes(*sound, specs))
         items.push_back(std::move(fix));
   }
   audioPlayer = nullptr;

   for (Texture *const image : table.m_vimage)
      for (TableAuditItem &fix : ImageFixes(table, *image))
         items.push_back(std::move(fix));

   const bool issueFound = std::ranges::any_of(items, [](const TableAuditItem &item) { return item.severity != TableAuditItem::Severity::Info; });
   if (!issueFound)
      items.push_back({ .severity = TableAuditItem::Severity::Info, .summary = "No issue identified.", .details = "No issue identified.", .bulletInReport = false });

   // Also output a log of the table file content to allow easier size optimization
   size_t totalSize = 0, totalGpuSize = 0;
   for (const auto sound : table.m_vsound)
   {
      //ss << "  . Sound: '" << sound->m_name << "', size: " << SizeToReadable(sound->m_cdata) << "\r\n";
      totalSize += sound->GetFileSize();
   }
   addItem(TableAuditItem::Severity::Info, "Total sound size: "s + SizeToReadable(totalSize));

   totalSize = 0;
   for (const auto image : table.m_vimage)
   {
      size_t imageSize = image->GetFileSize();
      size_t gpuSize = image->GetEstimatedGPUSize();
      //ss << "  . Image: '" << image->m_name << "', size: " << SizeToReadable(imageSize) << ", GPU mem size: " << SizeToReadable(gpuSize) << "\r\n";
      totalSize += imageSize;
      totalGpuSize += gpuSize;
   }
   addItem(TableAuditItem::Severity::Info, "Total image size: "s + SizeToReadable(totalSize) + " in VPX file, at least "s + SizeToReadable(totalGpuSize) + " in GPU memory when played");

   // Backdrop images referenced per view mode but not present in the table would render black.
   // An empty name or the "<None>" sentinel means no backdrop is set.
   static const char *const bgSetNames[NUM_BG_SETS] = { "Desktop", "Cabinet", "Full Single Screen" };
   for (unsigned int i = 0; i < NUM_BG_SETS; ++i)
      if (const string &bg = table.m_BG_image[i]; !bg.empty() && !StrCompareNoCase(bg, g_szNoneSelection) && table.GetImage(bg) == nullptr)
      {
         TableAuditItem &item = addItem(TableAuditItem::Severity::Warning, string(bgSetNames[i]) + " backdrop image '" + bg + "' is not present in the table (renders black)");
         item.part = &table;
         item.fix = TableFieldsFix([i](PinTable &table) { table.m_BG_image[i].clear(); },
            [&table, i]() -> std::vector<std::function<void()>> { return { [&table, i, value = table.m_BG_image[i]]() { table.m_BG_image[i] = value; } }; });
         item.fixLabel = "Clear missing backdrop reference";
      }

   int nPrimTris = 0, primMemSize = 0;
   for (const auto part : table.GetParts())
      if (part->GetItemType() == eItemPrimitive && ((Primitive *)part)->m_d.m_use3DMesh /* && ((Primitive *)part)->m_d.m_visible */)
      {
         primMemSize += (((Primitive *)part)->m_mesh.NumIndices() > 65536 ? 4 : 2) * (int)((Primitive *)part)->m_mesh.NumIndices();
         primMemSize += (int)((Primitive *)part)->m_mesh.NumVertices() * sizeof(Vertex3D_NoTex2);
         nPrimTris += (int)((Primitive *)part)->m_mesh.NumIndices() / 3;
      }
   addItem(
      TableAuditItem::Severity::Info, "Total number of faces used in primitives: "s + std::to_string(nPrimTris) + ", needing "s + SizeToReadable(primMemSize) + " in GPU memory when played");

   return audit;
}

string TableAudit::Format() const
{
   std::stringstream ss;
   for (const TableAuditItem &item : items)
   {
      if (item.bulletInReport)
      {
         ss << ". ";
         switch (item.severity)
         {
         case TableAuditItem::Severity::Error: ss << "Error: "; break;
         case TableAuditItem::Severity::Warning: ss << "Warning: "; break;
         case TableAuditItem::Severity::Info: break;
         }
      }
      ss << item.summary << "\r\n";
   }
   return ss.str();
}

bool TableAudit::HasErrors() const
{
   return std::ranges::any_of(items, [](const TableAuditItem &item) { return item.severity == TableAuditItem::Severity::Error; });
}
