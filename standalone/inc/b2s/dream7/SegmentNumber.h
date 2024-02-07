#pragma once

#include "../b2s_i.h"

#include "SegmentList.h"
#include "SegmentStyle.h"

#include "../../common/RendererGraphics.h"

class Dream7Display;

class SegmentNumber 
{
public:
   SegmentNumber(Dream7Display* pDisplay);
   virtual ~SegmentNumber();

   void OnInvalidated();
   void Init(const SDL_FPoint& location, SegmentNumberType type, VP::Matrix* pMatrix, float thickness);
   void AssignStyle();
   void Draw(VP::RendererGraphics* pGraphics);
   GraphicsPath* GetBounds();
   SegmentStyle* GetStyle() const { return m_pStyle; }
   const string& GetCharacter() const { return m_szCharacter; }
   void SetCharacter(const string& szCharacter);
   void InitSegments(const SegmentNumberType type, const float thickness);
   void InitMatrix(const SDL_FPoint& location, VP::Matrix* pMatrix);
   bool SetSegmentState(Segment* pSegment, const bool isOn);
   void GetSegmentRegions();
   void DisplayCharacter(const string& szCharacter);
   void DisplayBitCode(long value);

private:
   Dream7Display* m_pDisplay;
   SegmentList m_segments;
   SegmentStyle* m_pStyle;
   SegmentNumberType m_type;
   float m_thickness;
   VP::Matrix* m_pNumberMatrix;
   string m_szCharacter;
};