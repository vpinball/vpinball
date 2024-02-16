#include "stdafx.h"
#include "PinUpPlayerScreen.h"
#include "PinUpPlayerUtils.h"
#include "freeimage.h"

PinUpPlayerScreen::PinUpPlayerScreen()
{
}

PinUpPlayerScreen::~PinUpPlayerScreen()
{
   if (m_imageTexture) SDL_DestroyTexture(m_imageTexture);
}

static PinUpPlayerScreen::WindowPreference fromString(string name) {
   std::transform(name.begin(), name.end(), name.begin(), ::tolower);

   if (name == "show") return PinUpPlayerScreen::WindowPreference::Show;
   if (name == "forceon") return PinUpPlayerScreen::WindowPreference::ForceON;
   if (name == "forcepop") return PinUpPlayerScreen::WindowPreference::ForcePoP;
   if (name == "forceback") return PinUpPlayerScreen::WindowPreference::ForceBack;
   if (name == "forcepopback") return PinUpPlayerScreen::WindowPreference::ForcePopBack;
   if (name == "musiconly") return PinUpPlayerScreen::WindowPreference::MusicOnly;
   if (name == "off") return PinUpPlayerScreen::WindowPreference::Off;

   PLOGW << "Unknown Window Preference " << name;
   return PinUpPlayerScreen::WindowPreference::Off;
}

PinUpPlayerScreen* PinUpPlayerScreen::CreateFromCSVLine(string line)
{
   vector<string> parts = PinUpPlayerUtils::ParseCSVLine(line);
   if (parts.size() != 8)
      return NULL;

   PinUpPlayerScreen* screen = new PinUpPlayerScreen();

   screen->m_screenNum = string_to_int(parts[0], 0);
   screen->m_screenDes = parts[1];
   screen->m_playList = parts[2];
   screen->m_playFile = parts[3];
   screen->m_loopit = parts[4];
   screen->m_active = fromString(parts[5]);
   screen->m_priority = string_to_int(parts[6], 0);
   screen->m_customPos = parts[7];

   return screen;
}

void PinUpPlayerScreen::PlayMusic(string path, LONG volume, LONG forceplay) 
{
   PLOGI << "Playing " << path << " on " << m_screenNum;

   if (!m_audioPlayer) {
      m_audioPlayer = new AudioPlayer();
   }

   // Is this the right way to handle forceplay?
   if (m_audioPlayer->MusicActive() && !forceplay) {
      PLOGI << "Music playing on " << m_screenNum << ". Skipping " << path;
   }

   const float MusicVolume = g_pplayer->m_MusicVolume*volume*0.01 * (float)(1.0/100.0);

   m_audioPlayer->MusicInit(path, MusicVolume);
   m_audioPlayer->MusicPlay();
}

void PinUpPlayerScreen::ClearBackground() {
   if (m_imageTexture) {
      SDL_DestroyTexture(m_imageTexture);
      m_imageTexture = NULL;
      MarkDirty();
   }
}

void PinUpPlayerScreen::ShowImage(string path, LONG forceplay) 
{
   if (!m_window) {
      PLOGI << "Window not initialized. Cannot show " << path;
      return;
   }

   PLOGI << "Showing Image " << path << " on " << m_screenNum;

   ClearBackground();

   auto filetype = FreeImage_GetFileType(path.c_str(), 0);
   auto freeimage_bitmap = FreeImage_Load(filetype, path.c_str(), 0);
   FreeImage_FlipVertical(freeimage_bitmap);

   auto sdl_surface = SDL_CreateRGBSurfaceFrom(
      FreeImage_GetBits(freeimage_bitmap),
      FreeImage_GetWidth(freeimage_bitmap),
      FreeImage_GetHeight(freeimage_bitmap),
      FreeImage_GetBPP(freeimage_bitmap),
      FreeImage_GetPitch(freeimage_bitmap),
      FreeImage_GetRedMask(freeimage_bitmap),
      FreeImage_GetGreenMask(freeimage_bitmap),
      FreeImage_GetBlueMask(freeimage_bitmap),
      0 
   );

   m_imageTexture = SDL_CreateTextureFromSurface(m_window->GetGraphics()->GetRenderer(), sdl_surface);

   FreeImage_Unload(freeimage_bitmap);
   SDL_FreeSurface(sdl_surface);

   MarkDirty();
}

void PinUpPlayerScreen::LabelUpdated(PupLabel* label, int pageNumber) {
   if (m_currentPage == 0 || !label->m_visible) return;

   if (pageNumber == -1) {
      // Find in which page this label is in
      // Not computationally efficient but there should be just
      // a few pages. Optimize later if needed
      for (auto const& [page, labels] : m_labelPages) {
         if (labels->find(label) != labels->end()) {
            pageNumber = page;
            break;
         }
      }
   }

   // Update the screen if the label is on the current page
   if (pageNumber == m_currentPage) {
         MarkDirty();
   }
}


void PinUpPlayerScreen::PlayVideo(string path, LONG volume, LONG forceplay) 
{
   PLOGI << "Playing Video WIP, coming soon " << path << " on " << m_screenNum;
}


void PinUpPlayerScreen::MarkDirty() {
   m_isDirty = true;
}


void PinUpPlayerScreen::Render(VP::Graphics* graphics)
{
   if (!m_isDirty) return;

   //
   // Background image
   //

   if (m_imageTexture) {
      graphics->DrawTexture(m_imageTexture, NULL, NULL);
   } else {
      graphics->SetColor(RGB(0, 0, 0));
      graphics->Clear();
   }

   //
   // Labels
   //

   // Page 0 is used to hide all labels from the screen
   if (m_currentPage) {
      if (auto search = m_labelPages.find(m_currentPage); search == m_labelPages.end()) {
         PLOGW << "Label page " << m_currentPage << " not found in " << m_screenNum;
         return;
      }

      auto labels = m_labelPages.at(m_currentPage);
      for (auto it=labels->begin(); it!=labels->end(); ++it) {
         auto label = *it;
         if (!label->m_visible || !label->m_texture) continue;

         int w, h;
         SDL_QueryTexture(label->m_texture, NULL, NULL, &w, &h);

         SDL_Rect dest = { static_cast<int>(label->m_x)/*+ftFace->glyph->bitmap_left*/, 
                           static_cast<int>(label->m_y)/*-ftFace->glyph->bitmap_top*/, 
                           w, h };
         SDL_Rect src = { 0, 0, w, h };
         // PLOGI << "Rendering label " << label->m_text << " " << dest.x << "x" << dest.y << " " << w << "x" << h;

         graphics->DrawTexture(label->m_texture, &src, &dest);
      } 
   }

   graphics->Present();
   m_isDirty = false;
}
