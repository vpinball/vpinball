#pragma once


#include "SegmentList.h"
#include "SegmentStyle.h"

#include "../utils/VPXGraphics.h"

namespace B2SLegacy {

class Dream7Display;

class SegmentNumber final
{
public:
   SegmentNumber(Dream7Display* pDisplay);
   virtual ~SegmentNumber();

   void OnInvalidated();
   void Init(const SDL_FPoint& location, SegmentNumberType type, Matrix* pMatrix, float thickness);
   void AssignStyle();
   void Draw(VPXGraphics* pRenderer);
   SegmentStyle& GetStyle() { return m_pStyle; }
   const string& GetCharacter() const { return m_szCharacter; }
   void SetCharacter(const string& szCharacter);
   void InitSegments(const SegmentNumberType type, const float thickness);
   void InitMatrix(const SDL_FPoint& location, const Matrix* pMatrix);
   static bool SetSegmentState(Segment* pSegment, const bool isOn);
   void DisplayCharacter(const string& szCharacter);
   void DisplayBitCode(int value);

private:
   Dream7Display* m_pDisplay = nullptr;
   SegmentList m_segments;
   SegmentStyle m_pStyle;
   SegmentNumberType m_type = (SegmentNumberType)-1;
   float m_thickness = 16.0f;
   Matrix m_numberMatrix;
   string m_szCharacter;
};

}
