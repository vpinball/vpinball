// license:GPLv3+

#include "core/stdafx.h"
#include "VPXLoadFileProgressBar.h"


VPXLoadFileProgressBar::VPXLoadFileProgressBar(HINSTANCE app, HWND statusBar)
   : m_app{app},
     m_statusBar{statusBar}
{
}


VPXLoadFileProgressBar::~VPXLoadFileProgressBar()
{
   if (m_progressBar != nullptr) {
      ::DestroyWindow(m_progressBar);
   }
}


void VPXLoadFileProgressBar::OperationStarted()
{
   ::SendMessage(m_statusBar, SB_SETTEXT, 3 | 0, (LPARAM)LocalString(IDS_LOADING).m_szbuffer);

   const HCURSOR cursor = ::LoadCursor(nullptr, IDC_WAIT);
   ::SetCursor(cursor);

   if (m_progressBar != nullptr) {
      return;
   }

   RECT rc;
   ::SendMessage(m_statusBar, SB_GETRECT, 2, (LPARAM)&rc);

   m_progressBar = ::CreateWindowEx(
      0,
      PROGRESS_CLASS,
      (LPSTR)nullptr,
      WS_CHILD | WS_VISIBLE,
      rc.left,
      rc.top,
      rc.right - rc.left,
      rc.bottom - rc.top,
      m_statusBar,
      (HMENU)nullptr,
      m_app, 
      nullptr
   );

   ::SendMessage(m_progressBar, PBM_SETPOS, 1, 0);
};


void VPXLoadFileProgressBar::AboutToProcessTable(int elementCount)
{
}


void VPXLoadFileProgressBar::Update()
{
   if (m_progressBar) {
      ::SendMessage(m_progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, m_totalItems + m_totalSounds + m_totalImages + m_totalFonts + m_totalCollections));
      ::SendMessage(m_progressBar, PBM_SETPOS, m_itemsCount + m_soundCount + m_imageCount + m_fontCount + m_collectionCount, 0);
   }
}


void VPXLoadFileProgressBar::ItemHasBeenProcessed(int itemsCount, int totalItems)
{
   m_itemsCount = itemsCount;
   m_totalItems = totalItems;
   Update();
}


void VPXLoadFileProgressBar::SoundHasBeenProcessed(int soundCount, int totalSounds)
{
   m_soundCount = soundCount;
   m_totalSounds = totalSounds;
   Update();
}


void VPXLoadFileProgressBar::ImageHasBeenProcessed(int imageCount, int totalImages)
{
   m_imageCount = imageCount;
   m_totalImages = totalImages;
   Update();
}


void VPXLoadFileProgressBar::FontHasBeenProcessed(int fontCount, int totalFonts)
{
   m_fontCount = fontCount;
   m_totalFonts = totalFonts;
   Update();
}


void VPXLoadFileProgressBar::CollectionHasBeenProcessed(int collectionCount, int totalCollection)
{
   m_collectionCount = collectionCount;
   m_totalCollections = totalCollection;
   Update();
}


void VPXLoadFileProgressBar::Done()
{
   ::SendMessage(m_statusBar, SB_SETTEXT, 3 | 0, (LPARAM)L"");

   const HCURSOR cursor = ::LoadCursor(nullptr, IDC_ARROW);
   ::SetCursor(cursor);

   if (m_progressBar == nullptr) {
      return;
   }

   ::DestroyWindow(m_progressBar);
   m_progressBar = nullptr;
}
