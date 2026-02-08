// license:GPLv3+

#pragma once


namespace VPX::WinUI
{

class TranslatePointsDialog
{
public:
   explicit TranslatePointsDialog(ISelect *psel);

private:
   static int m_applyCount;
   static INT_PTR CALLBACK TranslateProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};


class ScalePointsDialog
{
public:
   explicit ScalePointsDialog(ISelect *psel);

private:
   static int m_applyCount;
   static INT_PTR CALLBACK ScaleProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};


class RotatePointsDialog
{
public:
   explicit RotatePointsDialog(ISelect *psel);

private:
   static int m_applyCount;
   static INT_PTR CALLBACK RotateProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

}
