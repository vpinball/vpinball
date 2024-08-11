// license:GPLv3+

#pragma once

class RenderProbe;

class RenderProbeDialog : public CDialog
{
public:
   RenderProbeDialog();

protected:
   BOOL OnInitDialog();
   virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   virtual void OnOK();
   virtual void OnClose();

private:
   void UpdateList();
   void LoadProbeToUI(RenderProbe *const pb);
   void SaveProbeFromUI(RenderProbe *const pb);
   void LoadPosition();
   void SavePosition();

   HWND hListHwnd;
};
