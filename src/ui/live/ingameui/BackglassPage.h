// license:GPLv3+

#pragma once

namespace VPX::InGameUI
{

class BackglassPage final : public InGameUIPage
{
public:
   BackglassPage();

   bool IsDefaults() const override;
   bool IsModified() const override;
   void ResetToDefaults() override;
   void ResetToInitialValues() override;
   void SaveGlobally() override;
   void SaveTableOverride() override;

private:
   enum class PresetPosition { None, UpperLeft, UpperCenter, UpperRight, LowerLeft, LowerCenter, LowerRight };

   bool ShouldShow() const;
   void ApplySize(int width, int height);
   void ApplyPosition(PresetPosition position);
   float GetSafeAspectRatio() const;
   int GetAllowedMaxWidth(float ar, int screenWidth, int screenHeight, int x, int y) const;
   int GetAllowedMaxHeight(float ar, int screenWidth, int screenHeight, int x, int y) const;
   void ApplyFromWidth(int desiredW, float ar, int screenWidth, int screenHeight, int& w, int& h) const;
   void ApplyFromHeight(int desiredH, float ar, int screenWidth, int screenHeight, int& w, int& h) const;

   bool m_lockAspectRatio = true;
   float m_aspectRatio = 0.0f;
   float m_initialAspectRatio = 0.0f;
   string m_activeControl = "";
   bool m_lastSizeZero = false;
   int m_initialX;
   int m_initialY;
   int m_initialWidth;
   int m_initialHeight;
   std::shared_ptr<BaseTexture> m_posNone;
   std::shared_ptr<BaseTexture> m_posUpperLeft;
   std::shared_ptr<BaseTexture> m_posUpperCenter;
   std::shared_ptr<BaseTexture> m_posUpperRight;
   std::shared_ptr<BaseTexture> m_posLowerLeft;
   std::shared_ptr<BaseTexture> m_posLowerCenter;
   std::shared_ptr<BaseTexture> m_posLowerRight;
};

}