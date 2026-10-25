#pragma once

#include "Segment.h"
#include "SegmentStyle.h"

#include "SegGraphics.h"

namespace B2S::seg
{

typedef enum
{
   SegmentNumberType_SevenSegment = 7,
   SegmentNumberType_TenSegment = 10,
   SegmentNumberType_FourteenSegment = 14,
   SegmentNumberType_SixteenSegment = 16
} SegmentNumberType;

class Dream7Display;

class SegmentList final : public vector<Segment*>
{
public:
   void Transform(Matrix* pMatrix)
   {
      for (auto& pSegment : *this)
         pSegment->Transform(pMatrix);
   }
};

class SegmentNumber final
{
public:
   SegmentNumber(Dream7Display* pDisplay);
   virtual ~SegmentNumber();

   void OnInvalidated();
   void Init(const FPoint& location, SegmentNumberType type, Matrix* pMatrix, float thickness);
   void AssignStyle();
   void Draw(SegGraphics* pRenderer);
   SegmentStyle& GetStyle() { return m_pStyle; }
   const string& GetCharacter() const { return m_szCharacter; }
   void SetCharacter(const string& szCharacter);
   void InitSegments(const SegmentNumberType type, const float thickness);
   void InitMatrix(const FPoint& location, const Matrix* pMatrix);
   static bool SetSegmentState(Segment* pSegment, const bool isOn);
   void DisplayCharacter(const string& szCharacter);
   void DisplayBitCode(int value);

private:
   Dream7Display* m_pDisplay = nullptr;
   SegmentList m_segments;
   SegmentStyle m_pStyle;
   SegmentNumberType m_type = (SegmentNumberType)0;
   float m_thickness = 16.0f;
   Matrix m_numberMatrix;
   string m_szCharacter;
};

}
