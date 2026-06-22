#pragma once

namespace B2SLegacy {

class Form;
class B2SSettings;
class B2SData;
class B2SBaseBox;
class Dream7Display;

class B2SScreen final
{
public:
   B2SScreen(B2SData* pB2SData, MsgPluginAPI* msgApi, VPXPluginAPI* vpxApi, unsigned int endpointId);
   ~B2SScreen();

   SDL_Rect& GetPlayfieldSize() { return m_playfieldSize; }
   void SetPlayfieldSize(const SDL_Rect& playfieldSize) { m_playfieldSize = playfieldSize; }
   const string& GetBackglassMonitor() const { return m_backglassMonitor; }
   void SetBackglassMonitor(const string& backglassMonitor) { m_backglassMonitor = backglassMonitor; }
   SDL_Rect& GetBackglassSize() { return m_backglassSize; }
   void SetBackglassSize(const SDL_Rect& backglassSize) { m_backglassSize = backglassSize; }
   SDL_Point& GetBackglassLocation() { return m_backglassLocation; }
   void SetBackglassLocation(const SDL_Point& backglassLocation) { m_backglassLocation = backglassLocation; }
   int GetBackglassGrillHeight() const { return m_backglassGrillHeight; }
   void SetBackglassGrillHeight(const int backglassGrillHeight) { m_backglassGrillHeight = backglassGrillHeight; }
   int GetBackglassSmallGrillHeight() const { return m_backglassSmallGrillHeight; }
   void SetBackglassSmallGrillHeight(const int backglassSmallGrillHeight) { m_backglassSmallGrillHeight = backglassSmallGrillHeight; }
   SDL_Rect& GetDMDSize() { return m_dmdSize; }
   void SetDMDSize(const SDL_Rect& dmdSize) { m_dmdSize = dmdSize; }
   SDL_Point& GetDMDLocation() { return m_dmdLocation; }
   void SetDMDLocation(const SDL_Point& dmdLocation) { m_dmdLocation = dmdLocation; }
   bool IsDMDFlipY() const { return m_dmdFlipY; }
   void SetDMDFlipY(const bool dmdFlipY) { m_dmdFlipY = dmdFlipY; }
   bool IsDMDAtDefaultLocation() const { return m_dmdAtDefaultLocation; }
   void SetDMDAtDefaultLocation(const bool dmdAtDefaultLocation) { m_dmdAtDefaultLocation = dmdAtDefaultLocation; }
   eDMDViewMode GetDMDViewMode() const { return m_dmdViewMode; }
   void SetDMDViewMode(const eDMDViewMode dmdViewMode) { m_dmdViewMode = dmdViewMode; }
   SDL_Rect& GetBackgroundSize() { return m_backgroundSize; }
   void SetBackgroundSize(const SDL_Rect& backgroundSize) { m_backgroundSize = backgroundSize; }
   SDL_Point& GetBackgroundLocation() { return m_backgroundLocation; }
   void SetBackgroundLocation(const SDL_Point& backgroundLocation) { m_backgroundLocation = backgroundLocation; }
   const string& GetBackgroundPath() const { return m_backgroundPath; }
   void SetBackgroundPath(const string& backgroundPath) { m_backgroundPath = backgroundPath; }
   SDL_Rect& GetBackglassCutOff() { return m_backglassCutOff; }
   bool IsDMDToBeShown() const { return m_dmdToBeShown; }
   void SetDMDToBeShown(const bool dmdToBeShown) { m_dmdToBeShown = dmdToBeShown; }
   const SDL_FRect& GetRescaleBackglass() const { return m_rescaleBackglass; }
   void Start(Form* pFormBackglass);
   void Start(Form* pFormBackglass, int backglassGrillHeight, int smallBackglassGrillHeight);
   void Start(Form* pFormBackglass, Form* pFormDMD, SDL_Point defaultDMDLocation);
   void Start(Form* pFormBackglass, Form* pFormDMD, SDL_Point defaultDMDLocation, eDMDViewMode dmdViewMode, int backglassGrillHeight, int smallBackglassGrillHeight);

private:
   void ReadB2SSettingsFromFile();
   void GetB2SSettings(SDL_Point defaultDMDLocation, eDMDViewMode dmdViewMode, int backglassGrillHeight, int backglassSmallGrillHeight);
   void Show();
   void ScaleAllControls(float rescaleX, float rescaleY, float rescaleDMDX, float rescaleDMDY);
   void ScaleControl(B2SBaseBox* pControl, float rescaleX, float rescaleY, bool isOnDMD, bool flipY = false);
   void ScaleControl(Dream7Display* pControl, float rescaleX, float rescaleY, bool isOnDMD, bool flipY = false);
   VPXTexture CutOutImage(VPXTexture pSourceTexture, int grillheight, int smallgrillheight);
   VPXTexture PartFromImage(VPXTexture pSourceTexture, SDL_Rect rect);
   VPXTexture ResizeImage(VPXTexture pSourceTexture, int grillheight);
   VPXTexture FlipImage(VPXTexture pSourceTexture);

   B2SData* m_pB2SData = nullptr;
   B2SSettings* m_pB2SSettings = nullptr;
   MsgPluginAPI* m_msgApi = nullptr;
   VPXPluginAPI* m_vpxApi = nullptr;
   unsigned int m_endpointId = 0;
   Form* m_pFormBackglass = nullptr;
   Form* m_pFormDMD = nullptr;
   SDL_Rect m_playfieldSize = { 0, 0, 0, 0 };
   string m_backglassMonitor;
   SDL_Rect m_backglassSize = { 0, 0, 0, 0 };
   SDL_Point m_backglassLocation = { 0, 0 };
   int m_backglassGrillHeight = 0;
   int m_backglassSmallGrillHeight = 0;
   SDL_Rect m_dmdSize = { 0, 0, 0, 0 };
   SDL_Point m_dmdLocation = { 0, 0 };
   bool m_dmdFlipY = false;
   bool m_dmdAtDefaultLocation = true;
   eDMDViewMode m_dmdViewMode = eDMDViewMode_NotDefined;
   SDL_Rect m_backgroundSize = { 0, 0, 0, 0 };
   SDL_Point m_backgroundLocation = { 0, 0 };
   string m_backgroundPath;
   SDL_Rect m_backglassCutOff = { 0, 0, 0, 0 };
   bool m_dmdToBeShown = false;
   SDL_FRect m_rescaleBackglass = { 0, 0, 1.0f, 1.0f };
};

}
