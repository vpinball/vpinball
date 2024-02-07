#pragma once

#include "../b2s_i.h"
#include "../../common/RendererGraphics.h"

class Form;
class B2SSettings;

class B2SScreen
{
public:
   B2SScreen();
   ~B2SScreen();

   SDL_Rect& GetPlayfieldSize() { return m_playfieldSize; }
   void SetPlayfieldSize(const SDL_Rect& playfieldSize) { m_playfieldSize = playfieldSize; }
   string GetBackglassMonitor() const { return m_backglassMonitor; }
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
   string GetBackgroundPath() const { return m_backgroundPath; }
   void SetBackgroundPath(const string& backgroundPath) { m_backgroundPath = backgroundPath; }
   SDL_Rect& GetBackglassCutOff() { return m_backglassCutOff; }
   bool IsDMDToBeShown() const { return m_dmdToBeShown; }
   void SetDMDToBeShown(const bool dmdToBeShown) { m_dmdToBeShown = dmdToBeShown; }
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
   SDL_Surface* CutOutImage(SDL_Surface* pSourceImage, int grillheight, int smallgrillheight);
   SDL_Surface* PartFromImage(SDL_Surface* pSourceImage, SDL_Rect rect);
   SDL_Surface* ResizeImage(SDL_Surface* pSourceImage, int grillheight);
   SDL_Surface* FlipImage(SDL_Surface* pSourceImage);

   Form* m_pFormBackglass;
   Form* m_pFormDMD;
   SDL_Rect m_playfieldSize;
   string m_backglassMonitor;
   SDL_Rect m_backglassSize;
   SDL_Point m_backglassLocation;
   int m_backglassGrillHeight;
   int m_backglassSmallGrillHeight;
   SDL_Rect m_dmdSize;
   SDL_Point m_dmdLocation;
   bool m_dmdFlipY;
   bool m_dmdAtDefaultLocation;
   eDMDViewMode m_dmdViewMode;
   SDL_Rect m_backgroundSize;
   SDL_Point m_backgroundLocation;
   string m_backgroundPath;
   SDL_Rect m_backglassCutOff;
   bool m_dmdToBeShown;

   B2SData* m_pB2SData;
   B2SSettings* m_pB2SSettings;
};
