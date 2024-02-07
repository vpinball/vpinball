#include "stdafx.h"

#include "SegmentNumber.h"
#include "Dream7Display.h"

SegmentNumber::SegmentNumber(Dream7Display* pDisplay)
{
   m_pDisplay = pDisplay;

   m_thickness = 16.0f;
   m_pStyle = new SegmentStyle();
   m_pNumberMatrix = NULL;
   m_pDisplay = pDisplay;
}

SegmentNumber::~SegmentNumber()
{
   if (m_pStyle)
      delete m_pStyle;

   if (m_pNumberMatrix)
      delete m_pNumberMatrix;
}

void SegmentNumber::OnInvalidated()
{
   m_pDisplay->SegmentNumberInvalidated(this);
}

void SegmentNumber::Init(const SDL_FPoint& location, SegmentNumberType type, VP::Matrix* pMatrix, float thickness)
{
   if (m_type != type || m_thickness != thickness) {
      InitSegments(type, thickness);
      DisplayCharacter(m_szCharacter);
   }
   InitMatrix(location, pMatrix);
}

void SegmentNumber::AssignStyle()
{
   for (auto& pSegment : m_segments)
      pSegment->AssignStyle();
   OnInvalidated();
}

void SegmentNumber::Draw(VP::RendererGraphics* pGraphics)
{
   pGraphics->ResetTransform();

   SDL_BlendMode blendMode = pGraphics->GetBlendMode();

   pGraphics->SetBlendMode(SDL_BLENDMODE_NONE);
   for (auto& pSegment : m_segments) {
      if (!pSegment->IsOn())
         pSegment->Draw(pGraphics);
   }
   if (m_pStyle->GetGlow() > 0.0f) {
      pGraphics->SetBlendMode(SDL_BLENDMODE_BLEND);
      for (auto& pSegment : m_segments) {
         if (pSegment->IsOn())
            pSegment->DrawLight(pGraphics);
      }
   }
   else
      pGraphics->SetBlendMode(SDL_BLENDMODE_NONE);
   for (auto& pSegment : m_segments) {
      if (pSegment->IsOn())
         pSegment->Draw(pGraphics);
   }

   pGraphics->SetBlendMode(blendMode);

   pGraphics->ResetTransform();
}

GraphicsPath* SegmentNumber::GetBounds()
{
   SDL_FRect bounds = {-14.0f, -14.0f, 173.0f, 272.f};
   GraphicsPath* pRegion = new GraphicsPath();
   pRegion->AddRectangle(bounds);
   pRegion->Transform(m_pNumberMatrix);
   return pRegion;
}

void SegmentNumber::SetCharacter(const string& szCharacter)
{
   m_szCharacter = szCharacter;
   DisplayCharacter(m_szCharacter);
}

void SegmentNumber::InitSegments(const SegmentNumberType type, const float thickness)
{
   m_segments.clear();
   float TH = thickness;
   float T4 = TH / 4.0f;
   float T2 = TH / 2.0f;
   m_type = type;
   m_thickness = TH;
   switch (type) {
      case SegmentNumberType_SevenSegment:
         m_segments.push_back(new Segment("a", T4 + 2, TH, TH, 120 - T2, -90, SegmentCap_MoreRight, SegmentCap_MoreRight));
         m_segments.push_back(new Segment("b", 124 - TH, T4 + 2, TH, 110 - T4, 0, SegmentCap_MoreRight, SegmentCap_Standard));
         m_segments.push_back(new Segment("c", 124 - TH, 116, TH, 110 - T4, 0, SegmentCap_Standard, SegmentCap_MoreRight));
         m_segments.push_back(new Segment("d", T4 + 2, 228, TH, 120 - T2, -90, SegmentCap_MoreLeft, SegmentCap_MoreLeft));
         m_segments.push_back(new Segment("e", 0, 116, TH, 110 - T4, 0, SegmentCap_Standard, SegmentCap_MoreLeft));
         m_segments.push_back(new Segment("f", 0, T4 + 2, TH, 110 - T4, 0, SegmentCap_MoreLeft, SegmentCap_Standard));
         m_segments.push_back(new Segment("g", T2 + 2, 114 + T2, TH, 120 - TH, -90));
         m_segments.push_back(new Segment(135 - T4 * 5, 228 - T4 * 5, TH));
         break;
      case SegmentNumberType_FourteenSegment:
      {
         float angleDiag = 26.5f + T4;
         float MT = TH - T4;
         float sinA = sin(angleDiag / 180.0f * M_PI);
         float cosA = cos(angleDiag / 180.0f * M_PI);
         float nTanA = tan(angleDiag / 180.0f * M_PI);
         float diagX = cosA * MT;
         float diagY = sinA * MT;
         float diagHeight = (58 - TH - MT / 2) / sinA + MT / nTanA;
         m_segments.push_back(new Segment("a", T4 + 2, TH, TH, 120 - T2, -90, SegmentCap_MoreRight, SegmentCap_MoreRight));
         m_segments.push_back(new Segment("b", 124 - TH, T4 + 2, TH, 110 - T4, 0, SegmentCap_MoreRight, SegmentCap_Standard));
         m_segments.push_back(new Segment("c", 124 - TH, 116, TH, 110 - T4, 0, SegmentCap_Standard, SegmentCap_MoreRight));
         m_segments.push_back(new Segment("d", T4 + 2, 228, TH, 120 - T2, -90, SegmentCap_MoreLeft, SegmentCap_MoreLeft));
         m_segments.push_back(new Segment("e", 0, 116, TH, 110 - T4, 0, SegmentCap_Standard, SegmentCap_MoreLeft));
         m_segments.push_back(new Segment("f", 0, T4 + 2, TH, 110 - T4, 0, SegmentCap_MoreLeft, SegmentCap_Standard));
         m_segments.push_back(new Segment("g1", T2 + 2, 114 + T2, TH, 59 - T2, -90, SegmentCap_Standard, SegmentCap_Flat));
         m_segments.push_back(new Segment(135 - T4 * 5, 224 - T4 * 6, TH));
         m_segments.push_back(new Segment("h", TH - diagX + 2, TH + 2 + diagY, MT, diagHeight, -angleDiag, SegmentCap_Right, SegmentCap_Left, 90 - angleDiag));
         m_segments.push_back(new Segment("i", 62 - MT / 2, TH + 2, MT, 110 - TH - T2, 0, SegmentCap_Flat, SegmentCap_Flat));
         m_segments.push_back(new Segment("j", 122 - TH, TH + 2, MT, diagHeight, angleDiag, SegmentCap_Left, SegmentCap_Right, 90 - angleDiag));
         m_segments.push_back(new Segment("g2", 63, 114 + T2, TH, 59 - T2, -90, SegmentCap_Flat, SegmentCap_Standard));
         m_segments.push_back(new Segment("m", MT / 2.0f - diagX + 64, 116 + T2 + diagY, MT, diagHeight, -angleDiag, SegmentCap_Right, SegmentCap_Left, 90 - angleDiag));
         m_segments.push_back(new Segment("l", 62 - MT / 2, 116 + T2, MT, 110 - TH - T2, 0, SegmentCap_Flat, SegmentCap_Flat));
         m_segments.push_back(new Segment("k", 60 - MT / 2, 116 + TH / 2, MT, diagHeight, angleDiag, SegmentCap_Left, SegmentCap_Right, 90 - angleDiag));
         m_segments.push_back(new Segment(131 - T4 * 5, 230 - T4 * 4, TH));
         break;
      }
      case SegmentNumberType_TenSegment:
         m_segments.push_back(new Segment("a", T4 + 2, TH, TH, 120 - T2, -90, SegmentCap_MoreRight, SegmentCap_MoreRight));
         m_segments.push_back(new Segment("b", 124 - TH, T4 + 2, TH, 110 - T4, 0, SegmentCap_MoreRight, SegmentCap_Standard));
         m_segments.push_back(new Segment("c", 124 - TH, 116, TH, 110 - T4, 0, SegmentCap_Standard, SegmentCap_MoreRight));
         m_segments.push_back(new Segment("d", T4 + 2, 228, TH, 120 - T2, -90, SegmentCap_MoreLeft, SegmentCap_MoreLeft));
         m_segments.push_back(new Segment("e", 0, 116, TH, 110 - T4, 0, SegmentCap_Standard, SegmentCap_MoreLeft));
         m_segments.push_back(new Segment("f", 0, T4 + 2, TH, 110 - T4, 0, SegmentCap_MoreLeft, SegmentCap_Standard));
         m_segments.push_back(new Segment("g1", T2 + 2, 114 + T2, TH, 69 - T2, -90, SegmentCap_Standard, SegmentCap_Standard));
         m_segments.push_back(new Segment("g2", 73, 114 + T2, TH, 49 - T2, -90, SegmentCap_Standard, SegmentCap_Standard));
         m_segments.push_back(new Segment("i", 72 - T2, T2 + 2, TH, 110 - T2, 0, SegmentCap_Standard, SegmentCap_Standard));
         m_segments.push_back(new Segment("l", 72 - T2, 116, TH, 110 - T2, 0, SegmentCap_Standard, SegmentCap_Standard));
         break;
   }
   for (auto& pSegment : m_segments)
      pSegment->SetStyle(m_pStyle);
}

void SegmentNumber::InitMatrix(const SDL_FPoint& location, VP::Matrix* pMatrix)
{
   m_pNumberMatrix = pMatrix->Clone();
   m_pNumberMatrix->Translate(location.x, location.y);
   m_segments.Transform(m_pNumberMatrix);
}
   
bool SegmentNumber::SetSegmentState(Segment* pSegment, const bool isOn)
{
   if (pSegment->IsOn() == isOn) 
      return false;

   pSegment->SetOn(isOn);

   return true;
}

void SegmentNumber::GetSegmentRegions()
{
   for (auto& pSegment : m_segments) {
      GraphicsPath* pPath = pSegment->GetGlassPathTransformed();
      pPath->Transform(m_pNumberMatrix);
      delete pPath;
   }
}
   
void SegmentNumber::DisplayCharacter(const string& szCharacter)
{
   string szSegments = "";
   if (!szCharacter.empty()) {
      switch (m_type) {
         case SegmentNumberType_SevenSegment:
            switch (szCharacter[0]) {
               case '0': szSegments = "abcdef"; break;
               case '1': szSegments = "bc"; break;
               case '2': szSegments = "abdeg"; break;
               case '3': szSegments = "abcdg"; break;
               case '4': szSegments = "bcfg"; break;
               case '5': szSegments = "acdfg"; break;
               case '6': szSegments = "acdefg"; break;
               case '7': szSegments = "abc"; break;
               case '8': szSegments = "abcdefg"; break;
               case '9': szSegments = "abcdfg"; break;
               case 'A':
               case 'a': szSegments = "abcefg"; break;
               case 'B':
               case 'b': szSegments = "cdefg"; break;
               case 'C':
               case 'c': szSegments = "adef"; break;
               case 'D':
               case 'd': szSegments = "bcdeg"; break;
               case 'E':
               case 'e': szSegments = "adefg"; break;
               case 'F':
               case 'f': szSegments = "aefg"; break;
            }
            break;
         case SegmentNumberType_TenSegment:
            switch (szCharacter[0]) {
               case '0': szSegments = "abcdefjk"; break;
               case '1': szSegments = "il"; break;
               case '2': szSegments = "abdeg1g2"; break;
               case '3': szSegments = "abcdg2"; break;
               case '4': szSegments = "bcfg1g2"; break;
               case '5': szSegments = "acdfg1g2"; break;
               case '6': szSegments = "acdefg1g2"; break;
               case '7': szSegments = "abc"; break;
               case '8': szSegments = "abcdefg1g2"; break;
               case '9': szSegments = "abcdfg1g2"; break;
               case 'A':
               case 'a': szSegments = "abcefg"; break;
               case 'B':
               case 'b': szSegments = "cdefg"; break;
               case 'C':
               case 'c': szSegments = "adef"; break;
               case 'D':
               case 'd': szSegments = "bcdeg"; break;
               case 'E':
               case 'e': szSegments = "adefg"; break;
               case 'F':
               case 'f': szSegments = "aefg"; break;  
            }
            break;
         case SegmentNumberType_FourteenSegment:
             switch (szCharacter[0]) {
                case '0': szSegments = "abcdefjk"; break;
                case '1': szSegments = "bcj"; break;
                case '2': szSegments = "abdeg1g2"; break;
                case '3': szSegments = "abcdg2"; break;
                case '4': szSegments = "bcfg1g2"; break;
                case '5': szSegments = "acdfg1g2"; break;
                case '6': szSegments = "acdefg1g2"; break;
                case '7': szSegments = "abc"; break;
                case '8': szSegments = "abcdefg1g2"; break;
                case '9': szSegments = "abcdfg1g2"; break;
                case 'A':
                case 'a': szSegments = "abcefg1g2"; break;
                case 'B':
                case 'b': szSegments = "abcdg2il"; break;
                case 'C':
                case 'c': szSegments = "adef"; break;
                case 'D':
                case 'd': szSegments = "abcdil"; break;
                case 'E':
                case 'e': szSegments = "adefg1"; break;
                case 'F':
                case 'f': szSegments = "aefg1"; break;
                case 'G':
                case 'g': szSegments = "acdefg2"; break;
                case 'H':
                case 'h': szSegments = "bcefg1g2"; break;
                case 'I':
                case 'i': szSegments = "adil"; break;
                case 'J':
                case 'j': szSegments = "bcde"; break;
                case 'K':
                case 'k': szSegments = "efg1jm"; break;
                case 'L':
                case 'l': szSegments = "def"; break;
                case 'M':
                case 'm': szSegments = "bcefhj"; break;
                case 'N':
                case 'n': szSegments = "bcefhm"; break;
                case 'O':
                case 'o': szSegments = "abcdef"; break;
                case 'P':
                case 'p': szSegments = "abefg1g2"; break;
                case 'Q':
                case 'q': szSegments = "abcdefm"; break;
                case 'R':
                case 'r': szSegments = "abefg1g2m"; break;
                case 'S':
                case 's': szSegments = "acdfg1g2"; break;
                case 'T':
                case 't': szSegments = "ail"; break;
                case 'U':
                case 'u': szSegments = "bcdef"; break;
                case 'V':
                case 'v': szSegments = "efjk"; break;
                case 'W':
                case 'w': szSegments = "bcefkm"; break;
                case 'X':
                case 'x': szSegments = "hjkm"; break;
                case 'Y':
                case 'y': szSegments = "hjl"; break;
                case 'Z':
                case 'z': szSegments = "adjk"; break;
             }
             break;
      }
      if (szCharacter.ends_with("."))
         szSegments += ".";
   }
   bool anyChange = false;
   for (auto& pSegment : m_segments) {
      if (SetSegmentState(pSegment, szSegments.find(pSegment->GetName()) != string::npos))
         anyChange = true;
   }
   if (anyChange)
      OnInvalidated();
}

void SegmentNumber::DisplayBitCode(long value)
{
   long segment = 0;
   bool anyChange = false;
   for (auto& pSegment : m_segments) {
      if (m_type == SegmentNumberType_TenSegment && pSegment->GetName() == "g2") {
         if (SetSegmentState(pSegment, (value & (1 << (segment - 1))) > 0))
            anyChange = true;
      }
      else {
         if (SetSegmentState(pSegment, (value & (1 << segment)) > 0))
            anyChange = true;
      }
      segment++;
   }
   if (anyChange)
      OnInvalidated();
}
    
