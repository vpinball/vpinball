// license:GPLv3+

#include "core/stdafx.h"
#include "SystemInfoPage.h"

#include "core/VPApp.h"
#include "audio/AudioPlayer.h"
#include "parts/Sound.h"
#include "renderer/Renderer.h"

#include <map>
#include <tuple>


namespace VPX::InGameUI
{

namespace
{
   string FormatKHz(unsigned int sampleRate) { return std::format("{:g} kHz", sampleRate / 1000.0); }

   // imgui_markdown has no escape character: names are inserted after a no-break space, whitespace before a '*' or '_' becomes a
   // no-break space (an emphasis needs whitespace before it), so do line breaks (a line start allows headings and lists), and square
   // brackets are shown as parentheses (a '[' followed by a ']' forms a link with any later '(' of the line)
   constexpr const char* kNoBreakSpace = "\xC2\xA0";
   string MarkdownSafe(const string& text)
   {
      string out;
      out.reserve(text.size() + 8);
      for (size_t i = 0; i < text.size(); i++)
      {
         const char c = text[i];
         const char next = i + 1 < text.size() ? text[i + 1] : '\0';
         if ((c == ' ' || c == '\t') && (next == '*' || next == '_'))
            out += kNoBreakSpace;
         else if (c == '\n' || c == '\r')
            out += kNoBreakSpace;
         else if (c == '[')
            out += '(';
         else if (c == ']')
            out += ')';
         else
            out += c;
      }
      return out;
   }

   // " *Label*: device - format, mode" line of an audio output
   string FormatAudioOutput(const VPX::AudioPlayer& audio, const char* label, VPX::SoundOutTypes out, const char* sameAsLabel, const string& mode)
   {
      const VPX::AudioPlayer::OutputInfo info = audio.GetOutputInfo(out);
      string line = std::format(" *{}*:{}", label, kNoBreakSpace);
      if (sameAsLabel != nullptr)
         line += std::format("same device as {}", sameAsLabel);
      else
      {
         line += info.deviceName.empty() ? "?"s : MarkdownSafe(info.deviceName);
         if (info.opened)
         {
            const double periodMs = info.sampleRate > 0 ? std::round(10.0 * info.periodFrames * 1000.0 / info.sampleRate) / 10.0 : 0.0;
            line += std::format(" - {} Hz, {} ch, {}, {} frames ({:g} ms)", info.sampleRate, info.channels, info.sampleFormat, info.periodFrames, periodMs);
         }
      }
      if (!info.opened)
         line += " - failed to open";
      else if (info.spatial) // The output mode only applies when spatial audio is disabled
         line += info.spatialReady ? ", spatial (binaural)"s : ", spatial (binaural), binaural renderer failed to initialize: silent"s;
      else
         line += ", " + mode;
      return line + '\n';
   }
}

SystemInfoPage::SystemInfoPage()
   : InGameUIPage("System Info"s, ""s, SaveMode::None)
{
}

void SystemInfoPage::BuildPage()
{
   std::ostringstream info;
   info << std::format(" *Visual Pinball*: {} \n", VP_VERSION_STRING_FULL_LITERAL);
   info << std::format(" *Logical CPU cores*: {}\n", g_app->GetLogicalNumberOfProcessors());
   info << std::format(" *GPU*: {} ({})\n", g_pplayer->m_renderer->m_renderDevice->m_GPU_name, g_pplayer->m_renderer->m_renderDevice->m_driver_name);
   // 2 reasons HDR can be off: display is not in HDR mode so no HDR10 backbuffer (see allowHDR10ColorSpace),
   // or it is, and something else declined (= video capture, anaglyph stereo, VR, or a backend without HDR10 swap chains)
   const VPX::Window* const outputWnd = g_pplayer->m_renderer->m_renderDevice->m_outputWnd[0];
   const char* const hdrState = outputWnd->IsWCGBackBuffer() ? "enabled"
                              : outputWnd->IsWCGDisplay()    ? "disabled (display is in HDR mode)"
                                                             : "disabled (display is not in HDR mode)";
   info << std::format(" *Display*: HDR {}, Refresh Rate: {} Hz, Resolution: {}x{}, Touch {}\n\n",
      hdrState, g_pplayer->m_playfieldWnd->GetRefreshRate(),
      outputWnd->GetPixelWidth(), outputWnd->GetPixelHeight(),
      (g_pplayer->m_pininput.HasTouchInput() ? "enabled" : "disabled"));

   // Audio outputs, plugin audio streams and table sounds
   if (const VPX::AudioPlayer* const audio = g_pplayer->m_audioPlayer.get(); audio)
   {
      using VPX::SoundOutTypes;
      info << std::format(" *Audio*: Driver {}\n", VPX::AudioPlayer::GetDriverName());
      info << FormatAudioOutput(*audio, "Backglass", SoundOutTypes::SNDOUT_BACKGLASS, nullptr,
         Settings::GetPlayer_Sound3DBackglass_Property()->GetEnum(static_cast<int>(audio->GetBackglassMode())));
      info << FormatAudioOutput(*audio, "Playfield", SoundOutTypes::SNDOUT_TABLE,
         audio->IsSameDevice(SoundOutTypes::SNDOUT_TABLE, SoundOutTypes::SNDOUT_BACKGLASS) ? "backglass" : nullptr,
         Settings::GetPlayer_Sound3D_Property()->GetEnum(static_cast<int>(audio->GetSoundMode3D())));
      if (audio->IsHapticEnabled())
         info << FormatAudioOutput(*audio, "Haptic", SoundOutTypes::SNDOUT_HAPTIC,
            audio->IsSameDevice(SoundOutTypes::SNDOUT_HAPTIC, SoundOutTypes::SNDOUT_BACKGLASS)  ? "backglass"
               : audio->IsSameDevice(SoundOutTypes::SNDOUT_HAPTIC, SoundOutTypes::SNDOUT_TABLE) ? "playfield"
                                                                                                 : nullptr,
            Settings::GetPlayer_Sound3DHaptic_Property()->GetEnum(static_cast<int>(audio->GetHapticMode())));
      else
         info << " *Haptic*: disabled\n";

      // Plugin audio sources, their streams grouped by format (all are played on the backglass output)
      const vector<Player::AudioLaneInfo> lanes = g_pplayer->GetAudioLaneInfos();
      if (lanes.empty())
         info << " *Plugin Audio*: none\n";
      else
      {
         info << " *Plugin Audio*:\n";
         const unsigned int streamRate = audio->GetStreamOutputSampleRate();
         for (const Player::AudioLaneInfo& lane : lanes)
         {
            string line = std::format("  -{}{} {:.0f}%", kNoBreakSpace, MarkdownSafe(lane.name), 100.f * lane.mixerVolume);
            if (lane.overridden)
               line += " - overridden by another source (muted)";
            else if (lane.streams.empty())
               line += " - no open stream";
            else
            {
               std::map<std::tuple<unsigned int, unsigned int, bool>, int> formats;
               for (const Player::AudioLaneInfo::Stream& stream : lane.streams)
                  formats[{ stream.sampleRate, stream.channels, stream.isFloat }]++;
               bool first = true;
               for (const auto& [format, count] : formats)
               {
                  const auto& [rate, channels, isFloat] = format;
                  line += std::format("{}{} stream{} {} Hz {} {} ({})", first ? " - " : ", ", count, count > 1 ? "s" : "", rate,
                     channels == 1 ? "mono" : channels == 2 ? "stereo" : std::format("{} ch", channels), isFloat ? "float" : "int16",
                     rate == streamRate ? "native" : "resampled");
                  first = false;
               }
            }
            info << line << '\n';
         }
      }

      // Table sounds, grouped by the sample rate of their file, resampled when it differs from their output's rate
      const VPX::AudioPlayer::OutputInfo backglassOutput = audio->GetOutputInfo(SoundOutTypes::SNDOUT_BACKGLASS);
      const VPX::AudioPlayer::OutputInfo playfieldOutput = audio->GetOutputInfo(SoundOutTypes::SNDOUT_TABLE);
      std::map<unsigned int, std::pair<int, bool>> soundRates; // rate -> count, resampled
      for (const VPX::Sound* const sound : g_pplayer->m_ptable->m_vsound)
      {
         const unsigned int rate = audio->GetSoundSampleRate(sound);
         auto& [count, resampled] = soundRates[rate];
         count++;
         const VPX::AudioPlayer::OutputInfo& output = sound->GetOutputTarget() == SoundOutTypes::SNDOUT_BACKGLASS ? backglassOutput : playfieldOutput;
         resampled |= rate != 0 && output.opened && rate != output.sampleRate;
      }
      string sounds = std::format(" *Table Sounds*: {} loaded", g_pplayer->m_ptable->m_vsound.size());
      bool first = true;
      for (const auto& [rate, entry] : soundRates)
      {
         sounds += std::format("{}{} x{}{}", first ? " - " : ", ", rate == 0 ? "unreadable"s : FormatKHz(rate), entry.first, entry.second ? " (resampled)" : "");
         first = false;
      }
      info << sounds << "\n\n";
   }
   // ImGui displays UTF-8
   info << std::format(" *App Root*: {}\n", PathToUTF8(g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Root)));
   info << std::format(" *Settings*: {}\n", PathToUTF8(g_settingsService.GetAppSettings().GetIniPath()));
   info << std::format(" *Scripts*: {}\n", PathToUTF8(g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Scripts, "core.vbs").parent_path()));
   info << std::format(" *Table Path*: {}\n", PathToUTF8(g_pplayer->m_ptable->m_filename));
   info << std::format(" *Table Data Path*: {}\n", PathToUTF8(g_app->m_fileLocator.GetTablePath(g_pplayer->m_ptable, FileLocator::TableSubFolder::Root, false)));

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Markdown, info.str()));
}

}
