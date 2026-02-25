// license:GPLv3+

#pragma once

class RenderProbe;

class RenderProbeDialog final : public CDialog
{
public:
   RenderProbeDialog();

protected:
   BOOL OnInitDialog() override;
   INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   void OnOK() override;
   void OnClose() override;

private:
   void UpdateList();
   void LoadProbeToUI(RenderProbe *const pb);
   void SaveProbeFromUI(RenderProbe *const pb);
   void LoadPosition();
   void SavePosition();

   HWND hListHwnd;
};
