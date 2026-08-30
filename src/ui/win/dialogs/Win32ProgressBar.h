// license:GPLv3+

#pragma once

#include "ui/VPXFileFeedback.h"

/**
 * Displays a progress bar using Win32 API to track progress of a table being loaded.
 */
class Win32ProgressBar final : public VPXFileFeedback
{
   public:
      Win32ProgressBar(HINSTANCE app, HWND statusBar);
      ~Win32ProgressBar() override;

      void SetProgress(unsigned int progress) override;
      void SetLength(unsigned int length) override;

   private:
      void Update();

      unsigned int m_length = 0;
      unsigned int m_progress = 0;

      HINSTANCE m_app;
      HWND m_statusBar;

      HWND m_progressBar = nullptr;
};
