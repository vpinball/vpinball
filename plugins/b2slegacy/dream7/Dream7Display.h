#pragma once


#include "../controls/Control.h"
#include "SegmentNumber.h"
#include "../utils/Matrix.h"
#include "../utils/VPXGraphics.h"

#include <map>



namespace B2SLegacy {

class Dream7Display final : public Control
{
public:
   Dream7Display(VPXPluginAPI* vpxApi);
   virtual ~Dream7Display();

   void OnPaint(VPXRenderContext2D* const ctx) override;
   void OnHandleCreated() override;

   bool IsHidden() const { return m_hidden; }
   void SetHidden(const bool hidden) { m_hidden = hidden; }
   bool IsMirrored() const { return m_mirrored; }
   void SetMirrored(const bool mirrored) { if (m_mirrored != mirrored) { m_mirrored = mirrored; InitSegmentsStyle(); } }
   const string& GetText() const { return m_szText; }
   void SetText(const string& szText);
   int GetDigits() const { return m_digits; }
   void SetDigits(const int digits) { if (m_digits != digits) { m_digits = digits; InitSegments(); } }
   ScaleMode GetScaleMode() const { return m_scaleMode; }
   void SetScaleMode(const ScaleMode scaleMode) { if (m_scaleMode != scaleMode) { m_scaleMode = scaleMode; InitSegmentsStyle(); } }
   float GetSpacing() const { return m_spacing; }
   void SetSpacing(const float spacing) { if (m_spacing != spacing) { m_spacing = spacing; InitSegmentsStyle(); } }
   bool IsTransparentBackground() const { return m_transparentBackground; }
   void SetTransparentBackground(const bool transparentBackground);
   SegmentNumberType GetType() const { return m_type; }
   void SetType(const SegmentNumberType segmentNumberType) {if (m_type != segmentNumberType) { m_type = segmentNumberType; InitSegmentsStyle(); } }
   uint32_t GetOffColor() const { return m_offColor; }
   void SetOffColor(const uint32_t offColor);
   uint32_t GetLightColor() const { return m_lightColor; }
   void SetLightColor(const uint32_t lightColor);
   uint32_t GetGlassColor() const { return m_glassColor; }
   void SetGlassColor(const uint32_t glassColor);
   uint32_t GetGlassColorCenter() const { return m_glassColorCenter; }
   void SetGlassColorCenter(const uint32_t glassColorCenter);
   int GetGlassAlpha() const { return m_glassAlpha; }
   void SetGlassAlpha(const int glassAlpha);
   int GetGlassAlphaCenter() const { return m_glassAlphaCenter; }
   void SetGlassAlphaCenter(const int glassAlphaCenter);
   float GetGlow() const { return m_glow; }
   void SetGlow(const float glow);
   const SDL_FRect& GetBulbSize() { return m_bulbSize; }
   void SetBulbSize(const SDL_FRect& bulbSize);
   bool IsWireFrame() const { return m_wireFrame; }
   void SetWireFrame(const bool wireFrame);
   float GetShear() const { return m_shear; }
   void SetShear(const float shear) { if (m_shear != shear) { m_shear = shear; InitSegmentsStyle(); } }
   float GetThickness() const { return m_thickness; }
   void SetThickness(const float thickness) { if (m_thickness != thickness) { m_thickness = thickness; InitSegmentsStyle(); } }
   float GetScaleFactor() const { return m_scaleFactor; }
   void SetScaleFactor(const float scaleFactor) { if (m_scaleFactor != scaleFactor) { m_scaleFactor = scaleFactor; InitSegmentsStyle(); } }
   int GetOffsetWidth() const { return m_offsetWidth; }
   void SetOffsetWidth(const int offsetWidth) { if (m_offsetWidth != offsetWidth) { m_offsetWidth = offsetWidth; InitSegmentsStyle(); } }
   float GetAngle() const { return m_angle; }
   void SetAngle(const float angle) { if (m_angle != angle) { m_angle = angle; InitSegmentsStyle(); } }
   void SetValue(int segment, const string& value);
   void SetValue(int segment, long value);
   void SetExtraSpacing(int segment, float value);
   void SegmentNumberInvalidated(SegmentNumber* pNumber);
   void SegmentDisplayHandleCreated();
   void SegmentDisplaySizeChanged();

private:
   void InitMatrix(float shear, float scaleFactor, bool mirrored);
   SDL_FRect GetBounds(Matrix* pMatrix);
   void InitSegments();
   void InitSegments(int digits, SegmentNumberType type, float shear);
   void InitSegmentsStyle();

   vector<SegmentNumber*> m_segmentNumbers;
   std::map<int, float> m_extraSpacings;
   bool m_hidden;
   bool m_mirrored;
   string m_szText;
   int m_digits;
   ScaleMode m_scaleMode;
   float m_spacing;
   bool m_transparentBackground;
   SegmentNumberType m_type;
   uint32_t m_offColor;
   uint32_t m_lightColor;
   uint32_t m_glassColor;
   uint32_t m_glassColorCenter;
   int m_glassAlpha;
   int m_glassAlphaCenter;
   float m_glow;
   SDL_FRect m_bulbSize;
   bool m_wireFrame;
   float m_shear;
   float m_thickness;
   float m_scaleFactor;
   int m_offsetWidth;
   float m_angle;
   Matrix* m_pMatrix;
   SDL_FRect m_bounds;
   std::unique_ptr<VPXGraphics> m_pGraphics;
};

}
