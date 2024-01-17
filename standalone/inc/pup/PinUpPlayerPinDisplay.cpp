#include "stdafx.h"
#include "PinUpPlayerPinDisplay.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <filesystem>
#include <stdlib.h>

#include "PinUpPlayerUtils.h"
#include "RSJparser.tcc"
#include "PinUpPlayerLabel.h"
#include "../b2s/classes/B2SSettings.h"
#include "../vpinmame/VPinMAMEController.h"

struct Coord {
   float x;
   float y;
};

static std::set<string> ext_image = {"jpg", "png"};
static std::set<string> ext_video = {"mp4"};
static std::set<string> ext_music = {"mp3"};

PinUpPlayerPinDisplay::PinUpPlayerPinDisplay()
{
   if (TTF_Init() == -1) {
      PLOGI << "Error initializing SDL_TTF " << TTF_GetError();
      exit(2);
   }

   m_running = true;
}

PinUpPlayerPinDisplay::~PinUpPlayerPinDisplay()
{
}

PinUpPlayerScreen* PinUpPlayerPinDisplay::getScreen(int ScreenNum) 
{
   for (auto& screen : m_screens)
      if (screen->m_screenNum == ScreenNum) return screen;

   PLOGI << "Screen not found " << ScreenNum;
   return NULL;
}

// PUP uses coordinates in percentages of another screen
static Coord fromPupDMD(PinUpPlayerScreen* screen, float x, float y) 
{
   auto pos = screen->m_window->GetPosition();

   Coord coord;
   coord.x = pos.x + (x / 100.0 * screen->m_window->GetWidth());
   coord.y = pos.y + (y / 100.0 * screen->m_window->GetHeight());

   return coord;
}

STDMETHODIMP PinUpPlayerPinDisplay::Init(LONG ScreenNum, BSTR RootDir)
{
   PLOGI << "Initializing screen " << ScreenNum;

   // Remove when Topper(0) is added to VPX Standalone
   if (ScreenNum == 0) {
      PLOGI << "Topper not implemented";
      return S_OK;
   }

   auto scrn = getScreen(ScreenNum);
   if (scrn && scrn->m_window) {
      PLOGI << "Screen already initialized " << ScreenNum;
      return 1;
   }

   if (scrn->m_active == PinUpPlayerScreen::WindowPreference::MusicOnly || 
       scrn->m_active == PinUpPlayerScreen::WindowPreference::Off) {
      PLOGI << "No need to create a window for screen " << ScreenNum;
      return S_OK;
   }

   // The table has no screens.pup file. The script initializes everything
   // directly
   if (scrn == NULL) {
      scrn = new PinUpPlayerScreen();

      scrn->m_screenNum = ScreenNum;
      scrn->m_screenDes = "PupScreen" + std::to_string(ScreenNum);
      scrn->m_playList = "";
      scrn->m_playFile = "";
      scrn->m_loopit = "";
      scrn->m_active = PinUpPlayerScreen::WindowPreference::Show;
      scrn->m_priority = 0;
      scrn->m_customPos = "";

      m_screens.push_back(scrn);
   }

   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   // Hardcoding screen #2 as backglass and #1 as the DMD. This is not always
   // true in PUP configurations. In the future there could be overrides in
   // the table INI file

   if (ScreenNum == 2) {
      if (!pSettings->LoadValueWithDefault(Settings::Standalone, "B2SWindows"s, true)) {
         PLOGI.printf("B2S Backglass window disabled, it can't be used as Pup Background");
         return 1;
      }

      scrn->m_window = VP::Window::Create("PupBackglass",
         pSettings->LoadValueWithDefault(Settings::Standalone, "B2SBackglassX"s, SETTINGS_B2S_BACKGLASSX),
         pSettings->LoadValueWithDefault(Settings::Standalone, "B2SBackglassY"s, SETTINGS_B2S_BACKGLASSY),
         pSettings->LoadValueWithDefault(Settings::Standalone, "B2SBackglassWidth"s, SETTINGS_B2S_BACKGLASSWIDTH),
         pSettings->LoadValueWithDefault(Settings::Standalone, "B2SBackglassHeight"s, SETTINGS_B2S_BACKGLASSHEIGHT));

      PLOGI.printf("Created PupBackglass window");
   } else {
      vector<string> parts = PinUpPlayerUtils::ParseCSVLine(scrn->m_customPos);

      // Full screen DMD
      if ((ScreenNum == 1) && 
            (scrn->m_customPos.empty() || 
            (parts[1] == "0" && parts[2] == "0" && parts[3] == "100" && parts[4] == "100"))) {

         if (!pSettings->LoadValueWithDefault(Settings::Standalone, "PinMAMEWindow"s, true)) {
            PLOGI.printf("PinMAME window disabled, it can't be used as Pup DMD");
            return 1;
         }

         scrn->m_window = VP::Window::Create("PupDMD",
            pSettings->LoadValueWithDefault(Settings::Standalone, "PinMAMEWindowX"s, SETTINGS_PINMAME_WINDOW_X),
            pSettings->LoadValueWithDefault(Settings::Standalone, "PinMAMEWindowY"s, SETTINGS_PINMAME_WINDOW_Y),
            pSettings->LoadValueWithDefault(Settings::Standalone, "PinMAMEWindowWidth"s, SETTINGS_PINMAME_WINDOW_WIDTH),
            pSettings->LoadValueWithDefault(Settings::Standalone, "PinMAMEWindowHeight"s, SETTINGS_PINMAME_WINDOW_HEIGHT));

         PLOGI.printf("Created PupDMD window");
      } else {
         if (parts.size() != 5) { // malformed or empty customPos field
            PLOGI << "Invalid custom position " << ScreenNum << " " << scrn->m_customPos;
            return -1;
         }

         // Screen relative to another window
         auto baseScreen = getScreen(string_to_int(parts[0]));
         auto xyCoord = fromPupDMD(baseScreen, std::stof(parts[1]), std::stof(parts[2]));
         auto whCoord = fromPupDMD(baseScreen, std::stof(parts[3]), std::stof(parts[4]));

         scrn->m_window = VP::Window::Create(scrn->m_screenDes, xyCoord.x, xyCoord.y, whCoord.x, whCoord.y);
         PLOGI.printf("Created custom Pup window at %fx%f %fx%f", xyCoord.x, xyCoord.y, whCoord.x, whCoord.y);

         // z-order not implemented. For now just hide any window that is supposed 
         // to be presented programmatically
         if ((scrn->m_active == PinUpPlayerScreen::WindowPreference::ForcePoP) ||
            (scrn->m_active == PinUpPlayerScreen::WindowPreference::ForceBack) ||
            (scrn->m_active == PinUpPlayerScreen::WindowPreference::ForcePopBack)) {
               if (scrn->m_screenNum > 3) {
                  scrn->m_window->Hide();
                  return S_OK;
               }
         }
      }
   }

   scrn->m_window->Show();

   return S_OK;
}

void PinUpPlayerPinDisplay::RenderLoop()
{
    const UINT32 targetFrameTime = 1000 / 60;

    while (m_running) {
        Uint32 frameStartTime = SDL_GetTicks();

         for (auto const& screen : m_screens) {
            if (screen->m_window)
               screen->Render(screen->m_window->GetGraphics());
         }

        Uint32 frameEndTime = SDL_GetTicks();
        Uint32 frameDuration = frameEndTime - frameStartTime;

        if (frameDuration < targetFrameTime)
           SDL_Delay(targetFrameTime - frameDuration);
    }
}

STDMETHODIMP PinUpPlayerPinDisplay::playlistadd(LONG ScreenNum, BSTR folder, LONG sort, LONG restSeconds)
{
   PLOGW << "Not implemented " << ScreenNum << " - " << MakeString(folder) << " - " << sort << " - " << restSeconds;

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::playlistplay(LONG ScreenNum, BSTR playlist){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::playlistplayex(LONG ScreenNum, BSTR playlist, BSTR playfilename, LONG volume, LONG forceplay)
{
   auto pl = MakeString(playlist);
   auto filename = MakeString(playfilename);
   PLOGI << ScreenNum << " - " << pl << "/" << filename << " - VL:" << volume << " - " << forceplay;
   PlaylistPlayInternal(ScreenNum, pl, filename, volume, forceplay);

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::play(LONG ScreenNum, BSTR playlist, BSTR playfilename){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::setWidth(LONG ScreenNum, LONG width){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::setHeight(LONG ScreenNum, LONG Height){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::setPosX(LONG ScreenNum, LONG Posx){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::setPosY(LONG ScreenNum, LONG PosY){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::setAspect(LONG ScreenNum, LONG aspectWide, LONG aspectHigh){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::setVolume(LONG ScreenNum, LONG vol){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::playpause(LONG ScreenNum){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::playresume(LONG ScreenNum){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::playstop(LONG ScreenNum){
   PLOGW << "Not implemented " << ScreenNum;

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::CloseApp(){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::get_isPlaying(LONG ScreenNum, LONG *Value){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::put_isPlaying(LONG ScreenNum, LONG Value){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::SetLength(LONG ScreenNum, LONG StopSecs){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::SetLoop(LONG ScreenNum, LONG LoopState){
   PLOGW << "Not implemented " << ScreenNum << " - " << LoopState;

   return S_OK;
}

// Mode - 1=Start, 0=Stop, 2=? (stop also?)
STDMETHODIMP PinUpPlayerPinDisplay::SetBackGround(LONG ScreenNum, LONG Mode)
{
   PLOGW << "Not implemented " << ScreenNum << " - " << Mode;

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::BlockPlay(LONG ScreenNum, LONG Mode){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::SetScreen(LONG ScreenNum){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::SetScreenEx(LONG ScreenNum, LONG xpos, LONG ypos, LONG swidth, LONG sheight, LONG popup) 
{
   PLOGW << "Not implemented " << ScreenNum << " - " << xpos << "x" << ypos << " " << swidth << "x" << sheight << " - " << popup;

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::get_SN(LONG *Value){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::put_SN(LONG Value){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::B2SData(BSTR tIndex, int Value)
{
   auto index = MakeString(tIndex);

   // 'E' events are triggered by the table's logic
   if (index[0] == 'E') {
      TriggerEvent(index);
      return S_OK;
   }

   PLOGW << "Not implemented " << " - " << MakeString(tIndex) << " - " << Value;

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::get_B2SFilter(BSTR *Value){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::put_B2SFilter(BSTR Value){
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::Show(LONG ScreenNum)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::Hide(LONG ScreenNum)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::B2SInit(BSTR TName, BSTR RomName)
{
   m_romname = MakeString(RomName);

   string path = g_pvp->m_currentTablePath + m_romname;

   if (!DirExists(path)) {
      PLOGE.printf("Directory does not exist: path=%s", path.c_str());
      return S_OK;
   }

   string playlistsPath = path + PATH_SEPARATOR_CHAR + "playlists.pup";
   std::ifstream playlistsFile;
   playlistsFile.open(playlistsPath, std::ifstream::in);
   if (playlistsFile.is_open()) {
      string line;
      int i = 0;
      while (std::getline(playlistsFile, line)) {
         if (++i == 1)
            continue;

         PinUpPlayerPlaylist* pPlaylist = PinUpPlayerPlaylist::CreateFromCSVLine(line);

         if (pPlaylist)
            m_playlists.push_back(pPlaylist);
      }
      PLOGI.printf("Playlists loaded: file=%s, size=%d", playlistsPath.c_str(), m_playlists.size());
   }
   else {
      PLOGE.printf("Unable to load %s", playlistsPath.c_str());
   }

   string triggersPath = path + PATH_SEPARATOR_CHAR + "triggers.pup";
   std::ifstream triggersFile;
   triggersFile.open(triggersPath, std::ifstream::in);
   if (triggersFile.is_open()) {
      string line;
      int i = 0;
      while (std::getline(triggersFile, line)) {
         if (++i == 1)
            continue;

         PinUpPlayerTrigger* pTrigger = PinUpPlayerTrigger::CreateFromCSVLine(line);

         if (pTrigger)
            m_triggers.push_back(pTrigger);
      }
      PLOGI.printf("Triggers loaded: file=%s, size=%d", triggersPath.c_str(), m_triggers.size());
   }
   else {
      PLOGE.printf("Unable to load %s", triggersPath.c_str());
   }

   string screensPath = path + PATH_SEPARATOR_CHAR + "screens.pup";
   std::ifstream screensFile;
   screensFile.open(screensPath, std::ifstream::in);
   if (screensFile.is_open()) {
      string line;
      int i = 0;
      while (std::getline(screensFile, line)) {
         if (++i == 1)
            continue;

         PinUpPlayerScreen* pScreen = PinUpPlayerScreen::CreateFromCSVLine(line);

         if (pScreen)
            m_screens.push_back(pScreen);
      }
      PLOGI.printf("Screens loaded: file=%s, size=%d", screensPath.c_str(), m_screens.size());
   }
   else {
      PLOGE.printf("Unable to load %s", screensPath.c_str());
   }

   // Load all fonts
   try {
      auto fontsPath = g_pvp->m_currentTablePath + m_romname + PATH_SEPARATOR_CHAR + "FONTS";
      for (const auto& entry : std::filesystem::directory_iterator(fontsPath)) {
         auto path = entry.path();
         auto extension = path.extension().string();

         if (extension == ".TTF" || extension == ".ttf") {
            PLOGI << "Loading font file " << path.filename();

            auto ttf = TTF_OpenFont(path.string().c_str(), 8);
            if (!ttf) {
               PLOGW << "Error loading font file " << path.filename();
               continue;
            }

            auto familyName = std::string(TTF_FontFaceFamilyName(ttf));
            m_fonts[familyName] = ttf;

            // Also add the filename to the map         
            m_fonts[path.stem()] = ttf;
         }
      }
   } catch (...) {
      PLOGW << "No PUP FONTS directory";
   }

   // Initialize screens
   Init(2, NULL); // Backglass
   Init(1, NULL); // DMD
   for (auto& screen : m_screens) {
      if (screen->m_screenNum < 3) continue;
      Init(screen->m_screenNum, NULL);
   }

   // It seems like the D0 event triggers after the table is initialized
   // Some tables use it as a signal to put the table in attract mode
   TriggerEvent("D0");

   // Using a single render thread for all Pup screens
   PLOGI.printf("Starting pup render thread");
   m_pThread = new std::thread([this]() {
      RenderLoop();
      PLOGI.printf("Pup render thread finished");
   });

   return S_OK;
}

void PinUpPlayerPinDisplay::TriggerEvent(string name) 
{
   PLOGI << "Triggering Event " << name;

   for (auto& trigger : m_triggers) { 
      if (trigger->m_active && trigger->m_trigger == name) {
         if (trigger->m_screenNum == 0) continue; // Ignore Topper for now
         
         PlaylistPlayInternal(trigger->m_screenNum, trigger->m_playList, trigger->m_playFile, trigger->m_volume, 0);
      }
   }
}

void PinUpPlayerPinDisplay::PlaylistPlayInternal(LONG ScreenNum, string playlist, string playfilename, LONG volume, LONG forceplay) 
{
   PLOGI << "Playing " << playfilename;

   if (playlist.empty()) {
      PLOGW << "A directory under the pup media is required";
      return;
   }

   auto screen = getScreen(ScreenNum);
   if (!screen) {
      PLOGW << "Screen not found " << ScreenNum;
      return;
   }

   string path = g_pvp->m_currentTablePath + m_romname + PATH_SEPARATOR_CHAR + playlist;
   // No idea what the Pup library does when a file is not provided
   // Let's grab a random one from the playlist directory
   if (playfilename.empty()) {
      srand (time(NULL));

      std:vector<std::string> filenames;
      for (const auto & entry : std::filesystem::directory_iterator(path))
         filenames.push_back(entry.path());

      if (filenames.size() == 0) {
         PLOGW << "No file in the playlist " << playlist;
         return;
      }

      auto index = rand() % filenames.size();
      path = filenames[index];
      playfilename = path;
      PLOGI << "Randomly selected " << path;
   } else {
      path += PATH_SEPARATOR_CHAR + playfilename;
   }

   // Determine the type of action for this event
   auto extension = playfilename.substr(playfilename.find_last_of(".") + 1);
   std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

   if (ext_image.count(extension)) { screen->ShowImage(path, forceplay); }
   else if (ext_video.count(extension)) { screen->PlayVideo(path, volume, forceplay); }
   else if (ext_music.count(extension)) { screen->PlayMusic(path, volume, forceplay); } 
   else { PLOGW << "Unknown media file with extension " << extension; }
}

STDMETHODIMP PinUpPlayerPinDisplay::SendMSG(BSTR cMSG)
{
   PLOGW << "Not implemented " << MakeString(cMSG);

   return S_OK;
}

// ScreenNum - in standard we'd set this to pDMD ( or 1)
// LabelName - your name of the label. keep it short no spaces (like 8 chars) although you can call it
//             anything really. When setting the label you will use this labelname to access the label
// FontName  - Windows font name, this must be exact match of OS front name. if you are using custom
//             TTF fonts then double check the name of font names
// Size      - Height as a percent of display height. 20=20% of screen height
// Color     - integer value of windows color
// Angle     - not used yet (future)
// xAlign    - 0 = horizontal left align, 1 = center horizontal, 2 = right horizontal
// yAlign    - 0 = top, 1 = center, 2 = bottom vertical alignment
// xPos      - This should be 0, but if you want to 'force' a position you can set this. it is a % of horizontal
//             width. 20=20% of screen width
// yPos      - same as xpos
// PageNum   - IMPORTANT... this will assign this label to this 'page' or group
// Visible   - initial state of label. visible = 1 show, 0 = off
STDMETHODIMP PinUpPlayerPinDisplay::LabelNew(LONG ScreenNum, BSTR LabelName, BSTR FontName, LONG Size, LONG Color, LONG Angle, LONG xAlign, LONG yAlign, LONG xPos, LONG yPos, LONG PageNum, LONG Visible)
{
   auto screen = getScreen(ScreenNum);
   if (!screen) {
      PLOGW << "Invallid target screen " << ScreenNum << ". Cannot create label";
      return 1;
   }

   // TODO: This is likely wrong. Need to move all logic dependent on the window
   // to rendering time to allow creating labels before the window is visible.
   if (!screen->m_window) {
      PLOGW << "Window not created, skipping labels. " << ScreenNum;
      return 1;
   }

   auto fontName = MakeString(FontName);
   if (auto search = m_fonts.find(fontName); search == m_fonts.end()) {
      PLOGW << "Font not found - " << fontName << ". Not creating label " << MakeString(LabelName);
      return S_OK;
   }

   auto screenHeight = screen->m_window->GetHeight();
   auto screenWidth = screen->m_window->GetWidth();

   auto fontHeight = Size / 100.0 * screenHeight;
   auto pupLabel = new PupLabel(m_fonts[fontName], fontHeight, Color, Visible);
   pupLabel->m_xalign = xAlign;
   pupLabel->m_yalign = yAlign;
   pupLabel->m_x = xPos / 100.0 * screenWidth;
   pupLabel->m_y = yPos / 100.0 * screenHeight;

   // Update the map with all labels by name
   // Label names are not case sensitive
   auto labelName = MakeString(LabelName);
   std::transform(labelName.begin(), labelName.end(), labelName.begin(), ::tolower);
   screen->m_labels.insert ( std::pair<string,PupLabel*>(labelName, pupLabel) );

   // Update the map with labels by page
   if (auto search = screen->m_labelPages.find(PageNum); search == screen->m_labelPages.end()) {
      screen->m_labelPages[PageNum] = new std::set<PupLabel*>();
   }
   screen->m_labelPages[PageNum]->insert(pupLabel);

   pupLabel->render(screen->m_window->GetGraphics()->GetRenderer());
   screen->LabelUpdated(pupLabel);

   return S_OK;
}

// ScreenNum - same as when adding new label.
// LabelName - same as labelname when you added new label.
// Caption -  value to assign label. Note you can add a ~ to insert a line break....see notes on that.
// Visible -  set visible state of label.
// Special -  this is a special crafter JSON string that can have unlimited values set. we'll have to see
//            examples and things like animations need many different variables and settings. This allows use to
//            future proof the text driver using a JSON string. Note: to make it easier to read in VBS its a json where
//            you normally see "field", is 'field'.... we replace all ' with " before processing json.
//            ----new in v1.3 Special can adjust many label properties AFTER created already.
//            special allows lots of modification of existing label. you should not change this too often because of
//            performance penalty like changing a fontname etc. for other items like position, there won't be a
//            performance issue.
STDMETHODIMP PinUpPlayerPinDisplay::LabelSet(LONG ScreenNum, BSTR LabelName, BSTR Caption, LONG Visible, BSTR Special)
{
   auto screen = getScreen(ScreenNum);
   if (!screen) {
      PLOGW << "Invallid target screen " << ScreenNum;
      return 1;
   }

   auto labelName = MakeString(LabelName);
   std::transform(labelName.begin(), labelName.end(), labelName.begin(), ::tolower);
   if (auto search = screen->m_labels.find(labelName); search == screen->m_labels.end()) {
      PLOGW << "Label not found - '" << labelName << "'";
      return S_OK;
   }

   auto renderLabel = false;
   auto label = screen->m_labels[labelName];

   auto oldText = label->m_text;
   auto newText = MakeString(Caption);
   if (oldText != newText) {
      label->m_text = newText;
      renderLabel = true;
   }

   label->m_visible = Visible;

   // possible keys: size,xpos,ypos,fname,color,xalign,yalign,pagenum
   auto jsonStr = MakeString(Special);
   if (!jsonStr.empty()) {
      RSJresource json(jsonStr);
      auto screenWidth = screen->m_window->GetWidth();
      auto screenHeight = screen->m_window->GetHeight();

      switch (json["mt"].as<int>(0))
      {
         case 2: // Label change
            if (json["size"].exists()) {
               float height = std::stof(json["size"].as_str());
               auto fontHeight = height / 100.0 * screenHeight;;
               // PLOGI << "Set size from " << label->height << " to " << fontHeight;
               label->m_height = fontHeight;
               renderLabel = true;
            }

            if (json["xpos"].exists()) {
               label->m_x = std::stof(json["xpos"].as_str()) / 100.0 * screenWidth;
               // PLOGI << "Set X=" << label->x;
            }

            if (json["ypos"].exists()) {
               label->m_y = std::stof(json["ypos"].as_str()) / 100.0 * screenHeight;
               // PLOGI << "Set Y=" << label->y;
            }

            if (json["fname"].exists()) {
               auto fontName = json["fname"].as_str();
               if (auto search = m_fonts.find(fontName); search == m_fonts.end()) {
                  PLOGW << "Font not found - " << fontName << ". Cannot change font face on label " << labelName;
               } else {
                  label->m_ftFace = m_fonts[fontName];
                  renderLabel = true;
               }
            }

            if (json["color"].exists()) {
               label->m_color = json["color"].as<int>();
               // PLOGI << "Set color to " << label->color;
               renderLabel = true;
            }

            if (json["xalign"].exists()) {
               label->m_xalign = json["xalign"].as<int>();
               // PLOGI << "Set xaliign=" << label->xalign;
               renderLabel = true;
            }

            if (json["yalign"].exists()) {
               label->m_yalign = json["yalign"].as<int>();
               // PLOGI << "Set yaliign=" << label->yalign;
               renderLabel = true;
            }

            if (json["pagenum"].exists()) {
               auto newPage = json["pagenum"].as<int>();
               PLOGW << "Changing page not implemented. New page " << newPage;
            }

            break;
         
         case 1: // Animate
            // at  = animate type (1=flashing, 2=motion)
            // fq  = when flashing its the frequency of flashing
            // len = length in ms of animation
            // fc  = foreground color of text during animation
            PLOGW << "Label animation not implemented";
            break;

         default:
            PLOGW << "Unknown message type " << json["mt"].as_str();
            break;
      }
   }

   if (renderLabel) label->render(screen->m_window->GetGraphics()->GetRenderer());
   screen->LabelUpdated(label);

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::LabelSetEx()
{
   PLOGW << "Not implemented";

   return S_OK;
}

// ScreenNum
// PageNum - page to view
// Seconds - seconds to show page...this is useful for 'splash' pages. If your page is a splash page then
//           you would set this to 3 for 3 second splash. It will auto-return to the default page. The
//           default page is the one that was last set with seconds=0; So we set seconds to 0 to tell driver 
//           that this is the current active page.
// Special - future.
STDMETHODIMP PinUpPlayerPinDisplay::LabelShowPage(LONG ScreenNum, LONG PageNum, LONG Seconds, BSTR Special)
{
   PLOGI << "Show label page " << PageNum << " on screen " << ScreenNum << " for " << Seconds;

   auto screen = getScreen(ScreenNum);
   if (!screen) {
      PLOGW << "Invallid screen " << ScreenNum;
      return 1;
   }

   screen->m_currentPage = PageNum;
   if (Seconds == 0) {
      screen->m_defaultPageNum = PageNum;
   }

   screen->m_labelPageSeconds = Seconds;

   screen->MarkDirty();

   return S_OK;
}

// According to the Pup DMD v1.3 doc this method is for initializing a DMD.
// It may not be needed in this implementation.
STDMETHODIMP PinUpPlayerPinDisplay::LabelInit(LONG ScreenNum)
{
   PLOGI << "Ignored for screen " << ScreenNum;

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::get_GetGame(BSTR *Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::put_GetGame(BSTR Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::get_GetRoot(BSTR *Value)
{
   string path = g_pvp->m_currentTablePath;
   #ifndef _WIN32
   // Assuming tables are written for Windows and expect \ as path separator
   std::replace(path.begin(), path.end(), '/', '\\');
   #endif

   PLOGI << "get_GetRoot " << path;

   CComBSTR Val(path.c_str());
   *Value = Val.Detach();

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::put_GetRoot(BSTR Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::SoundAdd(BSTR sname, BSTR fname, LONG svol, double sX, double sy, BSTR SP)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::SoundPlay(BSTR sname)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::PuPSound(BSTR sname, LONG sX, LONG sy, LONG sz, LONG vol, BSTR SP)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::InitPuPMenu(LONG Param1)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::get_B2SDisplays(BSTR *Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::setVolumeCurrent(LONG ScreenNum, LONG vol)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::get_GameUpdate(BSTR GameTitle, LONG Func, LONG FuncData, BSTR Extra, LONG *Value)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::GrabDC(LONG pWidth, LONG pHeight, BSTR wintitle, VARIANT *pixels)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::GetVersion(BSTR *pver)
{
   int nVersionNo0 = 1;
   int nVersionNo1 = 5;
   int nVersionNo2 = 99;
   int nVersionNo3 = 99;

   char szVersion[MAXTOKEN];
   sprintf_s(szVersion, sizeof(szVersion), "%d.%d.%d.%d", nVersionNo0, nVersionNo1, nVersionNo2, nVersionNo3);

   WCHAR wzVersion[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, szVersion, -1, wzVersion, MAXTOKEN);

   *pver = SysAllocString(wzVersion);

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::GrabDC2(LONG pWidth, LONG pHeight, BSTR wintitle, SAFEARRAY **pixels)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::playevent(LONG ScreenNum, BSTR playlist, BSTR playfilename, LONG volume, LONG priority, LONG playtype, LONG Seconds, BSTR Special)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::SetPosVideo(LONG ScreenNum, LONG StartPos, LONG EndPos, LONG Mode, BSTR Special)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP PinUpPlayerPinDisplay::PuPClose()
{
   PLOGW << "Not implemented";

   return S_OK;
}
