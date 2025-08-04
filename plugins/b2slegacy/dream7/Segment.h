#pragma once


#include "SegmentStyle.h"
#include "../utils/Matrix.h"
#include "../utils/VPXGraphics.h"
#include "../utils/GraphicsPath.h"

namespace B2SLegacy {

enum SegmentCap {
   SegmentCap_Standard = 0,
   SegmentCap_Flat = 1,
   SegmentCap_MoreLeft = 2,
   SegmentCap_Left = 3,
   SegmentCap_MoreRight = 4,
   SegmentCap_Right = 5
};

class Segment
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
   GraphicsPath* GetGlassPathTransformed();
   void AssignStyle();
   void Draw(VPXGraphics* pRenderer);
   void DrawLight(VPXGraphics* pRenderer);
   void Transform(Matrix* pMatrix);

private:
   void InitSegmentDot(float x, float y, float radius);
   void InitSegment(const string& szName, float x, float y, float width, float height, float angle, SegmentCap topcap, SegmentCap bottomcap, float capangle);
   void CreateLightData();
   void SetBulbSize();
   static void LeftRightFromCap(SegmentCap nCap, float nWidth, float nCapangle, float& nLeft, float& nRight, float& nDelta);
   void PaintSegment(VPXGraphics* pRenderer, GraphicsPath* pPath);
   void GetGlassData();
   void GetLightData();
   void ResetCacheData();
   void SetTransform(VPXGraphics* pRenderer);

   SDL_FPoint m_focusScales;
   vector<SDL_FPoint> m_lights;
   SDL_FRect m_lightDot;
   vector<SDL_FPoint> m_points;
   SegmentStyle* m_pStyle;
   bool m_on;
   string m_szName;
   GraphicsPath* m_pGlassPath;
   float m_glow;
   float m_angle;
   float m_radius;
   GraphicsPath* m_pLightPath;
   Matrix* m_pOwnMatrix;
   Matrix* m_pExternMatrix;
};

}
