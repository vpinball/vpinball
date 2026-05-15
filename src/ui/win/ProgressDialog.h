#pragma once

#ifndef __STANDALONE__
#include <wxx_controls.h>
#include <wxx_dialog.h>
#endif


class ProgressDialog final : public CDialog
{
public:
   ProgressDialog();

   void SetProgress(const string &text, const int value = -1);

protected:
   BOOL OnInitDialog() override;

private:
#ifdef __STANDALONE__
   int m_progress = -1;
#else
   CProgressBar m_progressBar;
   CStatic m_progressName;
#endif
};
