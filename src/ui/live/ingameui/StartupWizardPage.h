// license:GPLv3+

#pragma once

#include "CabinetRender.h"
#include "InGameUIPage.h"
#include "StartupWizardModel.h"

namespace VPX::InGameUI
{

class StartupWizardPage final : public InGameUIPage
{
public:
   StartupWizardPage();

   bool UsesStandardActionItems() const override { return false; }
   void BuildPage() override;
   const string& GetInfo() const override;
   void CollectPanes(vector<InGameUIPane>& panes) override;
   void AdjustItem(float direction, bool isInitialPress) override;

   StartupWizardModel& GetModel() { return *m_model; }
   int GetStepCount() const;
   int GetStepIndex() const { return m_step; }
   string GetStepTitle(int step) const;
   void GoToStep(int step);
   void PreviousStep();
   void NextStep();
   void BackOrCancel();
   void Cancel();
   void Finish();
   void RestartNow();
   void CloseAfterFinish();
   bool IsFooterAction(int index) const;

private:
   enum class StepKind
   {
      Welcome,
      PlayMode,
      ActiveDisplays,
      Output,
      Cabinet,
      TableFolder
   };
   vector<StepKind> GetSteps() const;
   StepKind GetStepKind(int step) const;
   VPXWindowId GetOutputStepOutput(int step) const;
   int CabinetStepIndex() const;
   int TableFolderStepIndex() const { return GetStepCount() - 1; }
   void AddFooterItems();
   void AddDisplayModeStep();
   void AddActiveDisplaysStep();
   void AddOutputSettingsStep(VPXWindowId output);
   void AddCabinetStep();
   void AddTableFolderStep();
   void AddFloatSetting(const VPX::Properties::PropertyRegistry::PropId propId, const char* format);
   void ApplyLiveAncillaryChanges();
   void CloseOrBack();
   void RenderDisplayPreview(const ImVec4& rect) const;
   void RenderOutputPreview(VPXWindowId output, const ImVec4& rect) const;

   std::unique_ptr<StartupWizardModel> m_model;
   CabinetRender m_cabinetRender;
   int m_step = 0;
   vector<int> m_footerItems;
};

}
