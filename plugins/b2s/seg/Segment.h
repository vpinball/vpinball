#pragma once

#include "../common.h"
#include "SegmentStyle.h"
#include "Matrix.h"
#include "SegGraphics.h"
#include "GraphicsPath.h"

namespace B2S::seg
{

enum SegmentCap
{
   SegmentCap_Standard = 0,
   SegmentCap_Flat = 1,
   SegmentCap_MoreLeft = 2,
   SegmentCap_Left = 3,
   SegmentCap_MoreRight = 4,
   SegmentCap_Right = 5
};

class Segment final
{
public:
   Segment(const string& szName, float x, float y, float width, float height, float angle);
   Segment(const string& szName, float x, float y, float width, float height, float angle, SegmentCap topcap, SegmentCap bottomcap);
   Segment(const string& szName, float x, float y, float width, float height, float angle, SegmentCap topcap, SegmentCap bottomcap, float capangle);
   Segment(float x, float y, float radius);
   virtual ~Segment();

   void SetStyle(SegmentStyle* pStyle) { m_pStyle = pStyle; }
   bool IsOn() const { return m_on; }
   void SetOn(bool on) { m_on = on; }
   const string& GetName() const { return m_szName; }
   GraphicsPath* GetGlassPath();
   void AssignStyle();
   void Draw(SegGraphics* pRenderer);
   void DrawLight(SegGraphics* pRenderer);
   void Transform(Matrix* pMatrix);

private:
   void InitSegmentDot(float x, float y, float radius);
   void InitSegment(const string& szName, float x, float y, float width, float height, float angle, SegmentCap topcap, SegmentCap bottomcap, float capangle);
   void CreateLightData();
   void SetBulbSize();
   static void LeftRightFromCap(SegmentCap nCap, float nWidth, float nCapangle, float& nLeft, float& nRight, float& nDelta);
   void PaintSegment(SegGraphics* pRenderer, const Brush& pBrush, uint32_t penColor, const GraphicsPath* const __restrict pPath);
   void GetGlassData();
   void GetLightData();
   void ResetCacheData();
   void SetTransform(SegGraphics* pRenderer);

   FPoint m_focusScales = { 0.0f, 0.0f };
   vector<FPoint> m_lights;
   FRect m_lightDot = { 0.0f, 0.0f, 0.0f, 0.0f };
   vector<FPoint> m_points;
   SegmentStyle* m_pStyle = nullptr;
   bool m_on = false;
   string m_szName;
   GraphicsPath* m_pGlassPath = nullptr;
   float m_glow = -1.0f;
   float m_angle = 0.0f;
   float m_radius = 0.0f;
   GraphicsPath* m_pLightPath = nullptr;
   PathGradientBrush* m_pLightBrush = nullptr;
   Matrix m_ownMatrix;
   Matrix m_externMatrix;
};

}
