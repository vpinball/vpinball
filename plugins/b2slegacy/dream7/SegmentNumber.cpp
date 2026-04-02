#include "../common.h"
#include <cmath>

#include "SegmentNumber.h"
#include "Dream7Display.h"

namespace B2SLegacy {

SegmentNumber::SegmentNumber(Dream7Display* pDisplay)
   : m_pDisplay(pDisplay)
   , m_thickness(16.0f)
   , m_pNumberMatrix(nullptr)
{
}

SegmentNumber::~SegmentNumber()
{
   delete m_pNumberMatrix;
}

void SegmentNumber::OnInvalidated()
{
   m_pDisplay->SegmentNumberInvalidated(this);
}

void SegmentNumber::Init(const SDL_FPoint& location, SegmentNumberType type, Matrix* pMatrix, float thickness)
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

void SegmentNumber::Draw(VPXGraphics* pRenderer)
{
   int onCount = 0, offCount = 0;
   for (const auto& pSegment : m_segments) {
      if (pSegment->IsOn()) onCount++; else offCount++;
   }

   // First draw OFF segments (if any)
   for (auto& pSegment : m_segments) {
      if (!pSegment->IsOn())
         pSegment->Draw(pRenderer);
   }

   // Handle glow effect for ON segments
   if (m_pStyle.GetGlow() > 0.0f) {
      for (auto& pSegment : m_segments) {
         if (pSegment->IsOn())
            pSegment->DrawLight(pRenderer);
      }
   }

   // Draw ON segments
   for (auto& pSegment : m_segments) {
      if (pSegment->IsOn())
         pSegment->Draw(pRenderer);
   }
}

GraphicsPath* SegmentNumber::GetBounds()
{
   constexpr SDL_FRect bounds = {-14.0f, -14.0f, 173.0f, 272.f};
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
   for (Segment* pSegment : m_segments)
      delete pSegment;
   m_segments.clear();

   const float TH = thickness;
   const float T4 = TH / 4.0f;
   const float T2 = TH / 2.0f;
   m_type = type;
   m_thickness = TH;
   switch (type) {
      case SegmentNumberType_SevenSegment:
         m_segments.push_back(new Segment("a"s, T4 + 2, TH, TH, 120 - T2, -90, SegmentCap_MoreRight, SegmentCap_MoreRight));
         m_segments.push_back(new Segment("b"s, 124 - TH, T4 + 2, TH, 110 - T4, 0, SegmentCap_MoreRight, SegmentCap_Standard));
         m_segments.push_back(new Segment("c"s, 124 - TH, 116, TH, 110 - T4, 0, SegmentCap_Standard, SegmentCap_MoreRight));
         m_segments.push_back(new Segment("d"s, T4 + 2, 228, TH, 120 - T2, -90, SegmentCap_MoreLeft, SegmentCap_MoreLeft));
         m_segments.push_back(new Segment("e"s, 0, 116, TH, 110 - T4, 0, SegmentCap_Standard, SegmentCap_MoreLeft));
         m_segments.push_back(new Segment("f"s, 0, T4 + 2, TH, 110 - T4, 0, SegmentCap_MoreLeft, SegmentCap_Standard));
         m_segments.push_back(new Segment("g"s, T2 + 2, 114 + T2, TH, 120 - TH, -90));
         m_segments.push_back(new Segment(135 - T4 * 5, 228 - T4 * 5, TH));
         break;
      case SegmentNumberType_FourteenSegment:
      {
         const float angleDiag = 26.5f + T4;
         const float MT = TH - T4;
         const float sinA = sinf(angleDiag * (float)(M_PI / 180.0));
         const float cosA = cosf(angleDiag * (float)(M_PI / 180.0));
         const float nTanA = tanf(angleDiag * (float)(M_PI / 180.0));
         const float diagX = cosA * MT;
         const float diagY = sinA * MT;
         const float diagHeight = (58.f - TH - MT / 2.f) / sinA + MT / nTanA;
         m_segments.push_back(new Segment("a"s, T4 + 2.f, TH, TH, 120.f - T2, -90, SegmentCap_MoreRight, SegmentCap_MoreRight));
         m_segments.push_back(new Segment("b"s, 124.f - TH, T4 + 2.f, TH, 110.f - T4, 0, SegmentCap_MoreRight, SegmentCap_Standard));
         m_segments.push_back(new Segment("c"s, 124.f - TH, 116, TH, 110.f - T4, 0, SegmentCap_Standard, SegmentCap_MoreRight));
         m_segments.push_back(new Segment("d"s, T4 + 2.f, 228, TH, 120.f - T2, -90, SegmentCap_MoreLeft, SegmentCap_MoreLeft));
         m_segments.push_back(new Segment("e"s, 0, 116, TH, 110.f - T4, 0, SegmentCap_Standard, SegmentCap_MoreLeft));
         m_segments.push_back(new Segment("f"s, 0, T4 + 2.f, TH, 110.f - T4, 0, SegmentCap_MoreLeft, SegmentCap_Standard));
         m_segments.push_back(new Segment("g1"s, T2 + 2.f, 114.f + T2, TH, 59.f - T2, -90, SegmentCap_Standard, SegmentCap_Flat));
         m_segments.push_back(new Segment(135.f - T4 * 5.f, 224.f - T4 * 6.f, TH));
         m_segments.push_back(new Segment("h"s, TH - diagX + 2.f, TH + 2.f + diagY, MT, diagHeight, -angleDiag, SegmentCap_Right, SegmentCap_Left, 90 - angleDiag));
         m_segments.push_back(new Segment("i"s, 62.f - MT / 2.f, TH + 2.f, MT, 110.f - TH - T2, 0, SegmentCap_Flat, SegmentCap_Flat));
         m_segments.push_back(new Segment("j"s, 122.f - TH, TH + 2.f, MT, diagHeight, angleDiag, SegmentCap_Left, SegmentCap_Right, 90 - angleDiag));
         m_segments.push_back(new Segment("g2"s, 63, 114.f + T2, TH, 59.f - T2, -90, SegmentCap_Flat, SegmentCap_Standard));
         m_segments.push_back(new Segment("m"s, MT / 2.0f - diagX + 64.f, 116.f + T2 + diagY, MT, diagHeight, -angleDiag, SegmentCap_Right, SegmentCap_Left, 90 - angleDiag));
         m_segments.push_back(new Segment("l"s, 62.f - MT / 2.f, 116.f + T2, MT, 110.f - TH - T2, 0, SegmentCap_Flat, SegmentCap_Flat));
         m_segments.push_back(new Segment("k"s, 60.f - MT / 2.f, 116.f + TH / 2.f, MT, diagHeight, angleDiag, SegmentCap_Left, SegmentCap_Right, 90 - angleDiag));
         m_segments.push_back(new Segment(131.f - T4 * 5.f, 230.f - T4 * 4.f, TH));
         break;
      }
      case SegmentNumberType_TenSegment:
         m_segments.push_back(new Segment("a"s, T4 + 2.f, TH, TH, 120.f - T2, -90, SegmentCap_MoreRight, SegmentCap_MoreRight));
         m_segments.push_back(new Segment("b"s, 124.f - TH, T4 + 2.f, TH, 110.f - T4, 0, SegmentCap_MoreRight, SegmentCap_Standard));
         m_segments.push_back(new Segment("c"s, 124.f - TH, 116, TH, 110.f - T4, 0, SegmentCap_Standard, SegmentCap_MoreRight));
         m_segments.push_back(new Segment("d"s, T4 + 2.f, 228, TH, 120.f - T2, -90, SegmentCap_MoreLeft, SegmentCap_MoreLeft));
         m_segments.push_back(new Segment("e"s, 0, 116, TH, 110.f - T4, 0, SegmentCap_Standard, SegmentCap_MoreLeft));
         m_segments.push_back(new Segment("f"s, 0, T4 + 2.f, TH, 110.f - T4, 0, SegmentCap_MoreLeft, SegmentCap_Standard));
         m_segments.push_back(new Segment("g1"s, T2 + 2.f, 114.f + T2, TH, 69.f - T2, -90, SegmentCap_Standard, SegmentCap_Standard));
         m_segments.push_back(new Segment("g2"s, 73, 114.f + T2, TH, 49.f - T2, -90, SegmentCap_Standard, SegmentCap_Standard));
         m_segments.push_back(new Segment("i"s, 72.f - T2, T2 + 2.f, TH, 110.f - T2, 0, SegmentCap_Standard, SegmentCap_Standard));
         m_segments.push_back(new Segment("l"s, 72.f - T2, 116, TH, 110.f - T2, 0, SegmentCap_Standard, SegmentCap_Standard));
         break;
      default: break;
   }
   for (auto& pSegment : m_segments)
      pSegment->SetStyle(&m_pStyle);
}

void SegmentNumber::InitMatrix(const SDL_FPoint& location, const Matrix* pMatrix)
{
   delete m_pNumberMatrix;
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
   string szSegments;
   if (!szCharacter.empty()) {
      switch (m_type) {
         case SegmentNumberType_SevenSegment:
            switch (szCharacter[0]) {
               case '0': szSegments = "abcdef"sv; break;
               case '1': szSegments = "bc"sv; break;
               case '2': szSegments = "abdeg"sv; break;
               case '3': szSegments = "abcdg"sv; break;
               case '4': szSegments = "bcfg"sv; break;
               case '5': szSegments = "acdfg"sv; break;
               case '6': szSegments = "acdefg"sv; break;
               case '7': szSegments = "abc"sv; break;
               case '8': szSegments = "abcdefg"sv; break;
               case '9': szSegments = "abcdfg"sv; break;
               case 'A':
               case 'a': szSegments = "abcefg"sv; break;
               case 'B':
               case 'b': szSegments = "cdefg"sv; break;
               case 'C':
               case 'c': szSegments = "adef"sv; break;
               case 'D':
               case 'd': szSegments = "bcdeg"sv; break;
               case 'E':
               case 'e': szSegments = "adefg"sv; break;
               case 'F':
               case 'f': szSegments = "aefg"sv; break;
            }
            break;
         case SegmentNumberType_TenSegment:
            switch (szCharacter[0]) {
               case '0': szSegments = "abcdefjk"sv; break;
               case '1': szSegments = "il"sv; break;
               case '2': szSegments = "abdeg1g2"sv; break;
               case '3': szSegments = "abcdg2"sv; break;
               case '4': szSegments = "bcfg1g2"sv; break;
               case '5': szSegments = "acdfg1g2"sv; break;
               case '6': szSegments = "acdefg1g2"sv; break;
               case '7': szSegments = "abc"sv; break;
               case '8': szSegments = "abcdefg1g2"sv; break;
               case '9': szSegments = "abcdfg1g2"sv; break;
               case 'A':
               case 'a': szSegments = "abcefg"sv; break;
               case 'B':
               case 'b': szSegments = "cdefg"sv; break;
               case 'C':
               case 'c': szSegments = "adef"sv; break;
               case 'D':
               case 'd': szSegments = "bcdeg"sv; break;
               case 'E':
               case 'e': szSegments = "adefg"sv; break;
               case 'F':
               case 'f': szSegments = "aefg"sv; break;
            }
            break;
         case SegmentNumberType_FourteenSegment:
             switch (szCharacter[0]) {
                case '0': szSegments = "abcdefjk"sv; break;
                case '1': szSegments = "bcj"sv; break;
                case '2': szSegments = "abdeg1g2"sv; break;
                case '3': szSegments = "abcdg2"sv; break;
                case '4': szSegments = "bcfg1g2"sv; break;
                case '5': szSegments = "acdfg1g2"sv; break;
                case '6': szSegments = "acdefg1g2"sv; break;
                case '7': szSegments = "abc"sv; break;
                case '8': szSegments = "abcdefg1g2"sv; break;
                case '9': szSegments = "abcdfg1g2"sv; break;
                case 'A':
                case 'a': szSegments = "abcefg1g2"sv; break;
                case 'B':
                case 'b': szSegments = "abcdg2il"sv; break;
                case 'C':
                case 'c': szSegments = "adef"sv; break;
                case 'D':
                case 'd': szSegments = "abcdil"sv; break;
                case 'E':
                case 'e': szSegments = "adefg1"sv; break;
                case 'F':
                case 'f': szSegments = "aefg1"sv; break;
                case 'G':
                case 'g': szSegments = "acdefg2"sv; break;
                case 'H':
                case 'h': szSegments = "bcefg1g2"sv; break;
                case 'I':
                case 'i': szSegments = "adil"sv; break;
                case 'J':
                case 'j': szSegments = "bcde"sv; break;
                case 'K':
                case 'k': szSegments = "efg1jm"sv; break;
                case 'L':
                case 'l': szSegments = "def"sv; break;
                case 'M':
                case 'm': szSegments = "bcefhj"sv; break;
                case 'N':
                case 'n': szSegments = "bcefhm"sv; break;
                case 'O':
                case 'o': szSegments = "abcdef"sv; break;
                case 'P':
                case 'p': szSegments = "abefg1g2"sv; break;
                case 'Q':
                case 'q': szSegments = "abcdefm"sv; break;
                case 'R':
                case 'r': szSegments = "abefg1g2m"sv; break;
                case 'S':
                case 's': szSegments = "acdfg1g2"sv; break;
                case 'T':
                case 't': szSegments = "ail"sv; break;
                case 'U':
                case 'u': szSegments = "bcdef"sv; break;
                case 'V':
                case 'v': szSegments = "efjk"sv; break;
                case 'W':
                case 'w': szSegments = "bcefkm"sv; break;
                case 'X':
                case 'x': szSegments = "hjkm"sv; break;
                case 'Y':
                case 'y': szSegments = "hjl"sv; break;
                case 'Z':
                case 'z': szSegments = "adjk"sv; break;
             }
             break;
         default: break;
      }
      if (szCharacter.ends_with('.'))
         szSegments += '.';
   }
   bool anyChange = false;
   for (auto& pSegment : m_segments) {
      if (SetSegmentState(pSegment, szSegments.find(pSegment->GetName()) != string::npos))
         anyChange = true;
   }
   if (anyChange)
      OnInvalidated();
}

void SegmentNumber::DisplayBitCode(int value)
{
   int segment = 0;
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

}
