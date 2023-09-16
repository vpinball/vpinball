#pragma once

#include "../b2s_i.h"

#include "../controls/Control.h"

#include <map>

class Dream7Display : public Control {
public:
   Dream7Display();
   virtual ~Dream7Display();

   bool IsHidden() const { return m_hidden; }
   void SetHidden(const bool hidden) { m_hidden = hidden; }
   bool IsMirrored() const { return m_mirrored; }
   void SetMirrored(const bool mirrored) { if (m_mirrored != mirrored) { m_mirrored = mirrored; /* InitSegmentsStyle(); */} }
   string GetText() const { return m_szText; }
   void SetText(const string& szText);
   int GetDigits() const { return m_digits; }
   void SetDigits(const int digits) { if (m_digits != digits) { m_digits = digits; /* InitSegements(); */} }
   ScaleMode GetScaleMode() const { return m_scaleMode; }
   void SetScaleMode(const ScaleMode scaleMode) { if (m_scaleMode != scaleMode) { m_scaleMode = scaleMode; /* InitSegements(); */ } }
   float GetSpacing() const { return m_spacing; }
   void SetSpacing(const float spacing) { if (m_spacing != spacing) { m_spacing = spacing; /* InitSegements(); */ } }
   bool IsTransparentBackground() const { return m_transparentBackground; }
   void SetTransparentBackground(const bool transparentBackground);
   SegmentNumberType GetType() const { return m_segmentNumberType; }
   void SetType(const SegmentNumberType segmentNumberType) {if (m_segmentNumberType != segmentNumberType) { m_segmentNumberType = segmentNumberType; /* InitSegmentsStyle(); */} }
   OLE_COLOR GetOffColor() const { return m_offColor; }
   void SetOffColor(const OLE_COLOR offColor);
   OLE_COLOR GetLightColor() const { return m_lightColor; }
   void SetLightColor(const OLE_COLOR lightColor);
   OLE_COLOR GetGlassColor() const { return m_glassColor; }
   void SetGlassColor(const OLE_COLOR glassColor);
   OLE_COLOR GetGlassColorCenter() const { return m_glassColorCenter; }
   void SetGlassColorCenter(const OLE_COLOR glassColorCenter);
   int GetGlassAlpha() const { return m_glassAlpha; }
   void SetGlassAlpha(const int glassAlpha);
   int GetGlassAlphaCenter() const { return m_glassAlphaCenter; }
   void SetGlassAlphaCenter(const int glassAlphaCenter);
   float GetGlow() const { return m_glow; }
   void SetGlow(const float glow);
   SDL_FRect& GetBulbSize() { return m_bulbSize; }
   void SetBulbSize(const SDL_FRect& bulbSize);
   bool IsWireFrame() const { return m_wireFrame; }
   void SetWireFrame(const bool wireFrame);
   float GetShear() const { return m_shear; }
   void SetShear(const float shear) { if (m_shear != shear) { m_shear = shear; /* InitSegementStyle(); */ } }
   float GetThickness() const { return m_thickness; }
   void SetThickness(const float thickness) { if (m_thickness != thickness) { m_thickness = thickness; /* InitSegementStyle(); */ } }
   float GetScaleFactor() const { return m_scaleFactor; }
   void SetScaleFactor(const float scaleFactor) { if (m_scaleFactor != scaleFactor) { m_scaleFactor = scaleFactor; /* InitSegementStyle(); */ } }
   int GetOffsetWidth() const { return m_offsetWidth; }
   void SetOffsetWidth(const int offsetWidth) { if (m_offsetWidth != offsetWidth) { m_offsetWidth = offsetWidth; /* InitSegementStyle(); */ } }
   float GetAngle() const { return m_angle; }
   void SetAngle(const float angle) { if (m_angle != angle) { m_angle = angle; /* InitSegementStyle(); */ } }

   void SetValue(int segment, string value);
   void SetValue(int segment, int value);

private:
   vector<string> m_segmentNumbers;
   std::map<int, float> m_extraSpacings;
   long m_segmentPaintCounter;
   bool m_hidden;
   bool m_mirrored;
   string m_szText;
   ScaleMode m_scaleMode;
   int m_digits;
   float m_spacing;
   bool m_transparentBackground;
   SegmentNumberType m_segmentNumberType;
   OLE_COLOR m_offColor;
   OLE_COLOR m_lightColor;
   OLE_COLOR m_glassColor;
   OLE_COLOR m_glassColorCenter;
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
};