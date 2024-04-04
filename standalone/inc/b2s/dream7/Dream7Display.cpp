#include "stdafx.h"

#include "Dream7Display.h"

Dream7Display::Dream7Display() : Control()
{

   m_hidden = false;
   m_mirrored = false;
   m_szText = "";
   m_scaleMode = ScaleMode_Stretch;
   m_spacing = 20.0f;
   m_transparentBackground = false;
   m_type = SegmentNumberType_SevenSegment;
   m_offColor = RGB(20, 20, 20);
   m_shear = 0.1f;
   m_thickness = 16.0f;
   m_scaleFactor = 0.5f;
   m_offsetWidth = 0;
   m_angle = 0.0f;
   m_pMatrix = NULL;
}

Dream7Display::~Dream7Display()
{
   if (m_pMatrix)
      delete m_pMatrix;
}

void Dream7Display::OnPaint(VP::RendererGraphics* pGraphics)
{
   if (IsVisible()) {
      pGraphics->SetColor(RGB(0, 0, 0));
      pGraphics->FillRectangle(GetRect());

      pGraphics->TranslateTransform(GetLeft(), GetTop());

      for (auto& pSegmentNumber : m_segmentNumbers)
         pSegmentNumber->Draw(pGraphics);

      pGraphics->TranslateTransform(-GetLeft(), -GetTop());
   }

   Control::OnPaint(pGraphics);
}

void Dream7Display::OnHandleCreated()
{
   SegmentDisplayHandleCreated();
}

void Dream7Display::SetText(const string& szText)
{
   m_szText = szText;
   int nLen = 0;
   if (!m_szText.empty())
      nLen = m_szText.length() - 1;
   int nIndex = 0;
   for (int nSegment = 0; nSegment <= nLen; nSegment++) {
      if (nIndex >= m_segmentNumbers.size())
         break;
      char sChar = ' ';
      if (!m_szText.empty() && m_szText.length() > nSegment)
         sChar = m_szText.substr(nSegment, 1)[0];
      if (sChar == '.' && nIndex > 0 && !m_segmentNumbers[nIndex - 1]->GetCharacter().ends_with("."))
         m_segmentNumbers[nIndex - 1]->SetCharacter(m_segmentNumbers[nIndex - 1]->GetCharacter() + ".");
      else {
         m_segmentNumbers[nIndex]->SetCharacter(string() + sChar);
         nIndex++;
      }
   }
}

void Dream7Display::SetTransparentBackground(const bool transparentBackground)
{
   m_transparentBackground = transparentBackground;
   if (!transparentBackground) {
      // Me.DoubleBuffered = True
      // Me.SetStyle(ControlStyles.Opaque, False)
      // Me.SetStyle(ControlStyles.OptimizedDoubleBuffer, True)    
   }
   else {
      // Me.DoubleBuffered = False
      // Me.SetStyle(ControlStyles.Opaque, True)
      // Me.SetStyle(ControlStyles.OptimizedDoubleBuffer, False)
   }
}

void Dream7Display::SetOffColor(const OLE_COLOR offColor)
{
   for (auto& pSegmentNumber : m_segmentNumbers) {
      pSegmentNumber->GetStyle()->SetOffColor(offColor);
      pSegmentNumber->AssignStyle();
   }
}

void Dream7Display::SetLightColor(const OLE_COLOR lightColor)
{
   for (auto& pSegmentNumber : m_segmentNumbers) {
      pSegmentNumber->GetStyle()->SetLightColor(lightColor);
      pSegmentNumber->AssignStyle();
   }
}

void Dream7Display::SetGlassColor(const OLE_COLOR glassColor)
{
   for (auto& pSegmentNumber : m_segmentNumbers) {
      pSegmentNumber->GetStyle()->SetGlassColor(glassColor);
      pSegmentNumber->AssignStyle();
   }
}

void Dream7Display::SetGlassColorCenter(const OLE_COLOR glassColorCenter)
{
   for (auto& pSegmentNumber : m_segmentNumbers) {
      pSegmentNumber->GetStyle()->SetGlassColorCenter(glassColorCenter);
      pSegmentNumber->AssignStyle();
   }
}

void Dream7Display::SetGlassAlpha(const int glassAlpha)
{
   for (auto& pSegmentNumber : m_segmentNumbers) {
      pSegmentNumber->GetStyle()->SetGlassAlpha(glassAlpha);
      pSegmentNumber->AssignStyle();
   }
}

void Dream7Display::SetGlassAlphaCenter(const int glassAlphaCenter)
{
   for (auto& pSegmentNumber : m_segmentNumbers) {
      pSegmentNumber->GetStyle()->SetGlassAlphaCenter(glassAlphaCenter);
      pSegmentNumber->AssignStyle();
   }
}

void Dream7Display::SetGlow(const float glow)
{
   for (auto& pSegmentNumber : m_segmentNumbers) {
      pSegmentNumber->GetStyle()->SetGlow(glow);
      pSegmentNumber->AssignStyle();
   }
}

void Dream7Display::SetBulbSize(const SDL_FRect& sizeF)
{
   for (auto& pSegmentNumber : m_segmentNumbers) {
      pSegmentNumber->GetStyle()->SetBulbSize(sizeF);
      pSegmentNumber->AssignStyle();
   }
}

void Dream7Display::SetWireFrame(const bool wireFrame)
{
   for (auto& pSegmentNumber : m_segmentNumbers) {
      pSegmentNumber->GetStyle()->SetWireFrame(wireFrame);
      pSegmentNumber->AssignStyle();
   }
}

void Dream7Display::SetValue(int segment, string value)
{
   if (m_segmentNumbers.size() <= segment)
      return;

   m_segmentNumbers[segment]->DisplayCharacter(value);
}

void Dream7Display::SetValue(int segment, long value)
{
   if (m_segmentNumbers.size() <= segment)
      return;

   m_segmentNumbers[segment]->DisplayBitCode(value);
}

void Dream7Display::SetExtraSpacing(int segment, long value)
{
   if (m_extraSpacings.find(segment) != m_extraSpacings.end())
      m_extraSpacings.erase(segment);
   if (value > 0)
       m_extraSpacings[segment] = value;
    InitSegmentsStyle();
}

void Dream7Display::InitMatrix(float shear, float scaleFactor, bool mirrored)
{
   if (m_pMatrix)
      delete m_pMatrix;
   m_pMatrix = new VP::Matrix();
   if (shear < 0.0f)
       shear = 0.0f;
   if (shear > 2.0f)
      shear = 2.0f;
   if (scaleFactor <= 0.01f)
      scaleFactor = 0.01f;
   if (scaleFactor > 10.0f)
      scaleFactor = 10.0f;
   VP::Matrix styleMatrix;
   if (mirrored) {
      VP::Matrix InvertMatrix(1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
      styleMatrix.Multiply(InvertMatrix);
   }
   styleMatrix.Shear(-shear, 0.0f);
   styleMatrix.Rotate(m_angle);
   styleMatrix.Scale(scaleFactor, scaleFactor);
   styleMatrix.Translate(10.0f, 10.0f);
   if (IsHandleCreated()) {
      SDL_FRect bounds = GetBounds(&styleMatrix);
      if (m_scaleMode != ScaleMode_Manual) {
         float scaleX = (this->GetWidth() + 3.0f - m_offsetWidth) / bounds.w;
         float scaleY = (this->GetHeight() - 1.0f) / bounds.h;
         if (m_scaleMode == ScaleMode_Zoom) {
            scaleY = std::min(scaleX, scaleY);
            scaleX = scaleY;
         }
         if (scaleX > 0.0f && scaleY > 0.0f)
            m_pMatrix->Scale(scaleX, scaleY);
      }
      m_pMatrix->Translate(-bounds.x, bounds.y);
   }
   m_pMatrix->Multiply(styleMatrix);
}

SDL_FRect Dream7Display::GetBounds(VP::Matrix* pMatrix)
{
   // determine the bounds of the whole display
   vector<SDL_FPoint> points;
   float extraSpacings = 0.0f;
   for (auto& [key, spacing] : m_extraSpacings)
      extraSpacings += spacing;
   m_bounds = { -10.0f, -10.0f, (float)m_segmentNumbers.size() * (154.0f + m_spacing) + 15.0f - m_spacing + extraSpacings, 264.0f };
   points.push_back({ m_bounds.x, m_bounds.y });
   points.push_back({ m_bounds.x + m_bounds.w, m_bounds.y });
   points.push_back({ m_bounds.x + m_bounds.w, m_bounds.y + m_bounds.h });
   points.push_back({ m_bounds.x, m_bounds.y + m_bounds.h });
   pMatrix->TransformPoints(points);
   float minX = std::numeric_limits<float>::max();
   float minY = std::numeric_limits<float>::max();
   float maxX = -std::numeric_limits<float>::max();
   float maxY = -std::numeric_limits<float>::max();
   for (auto& point : points) {
      minX = std::min(minX, point.x);
      minY = std::min(minY, point.y);
      maxX = std::max(maxX, point.x);
      maxY = std::max(maxY, point.y);
   }
   return { minX, minY, maxX - minX, maxY - minY };
}

void Dream7Display::InitSegments()
{
   InitSegments(m_digits, m_type, m_shear);
}

void Dream7Display::SegmentNumberInvalidated(SegmentNumber* pNumber)
{
   GraphicsPath* pPath = pNumber->GetBounds();
   delete pPath;

   Invalidate();
}

void Dream7Display::InitSegments(int digits, SegmentNumberType type, float shear)
{
   if (digits >= 0 && digits <= 80) {
      for (int nNumber = m_segmentNumbers.size(); nNumber < digits; nNumber++) {
         SegmentNumber* pNumber = new SegmentNumber(this);
         m_segmentNumbers.push_back(pNumber);
      }
      while(m_segmentNumbers.size() > digits) {
         SegmentNumber* pNumber = m_segmentNumbers.back();
         m_segmentNumbers.pop_back();
         delete pNumber;
      }
      m_digits = m_segmentNumbers.size();
      InitSegmentsStyle();
   }
}

void Dream7Display::InitSegmentsStyle()
{
   InitMatrix(m_shear, m_scaleFactor, m_mirrored);
   int number = 0;
   float distance = 154.0f + m_spacing;
   float xPos = 0.0f;
   for (auto& pNumber : m_segmentNumbers) {
      pNumber->Init( { xPos, 0.0f }, m_type, m_pMatrix, m_thickness);
      xPos += distance;
      if (m_extraSpacings.find(number) != m_extraSpacings.end())
         xPos += m_extraSpacings[number];
      number++;
   }
   Invalidate();
}

void Dream7Display::SegmentDisplayHandleCreated()
{
   InitSegmentsStyle();
}

void Dream7Display::SegmentDisplaySizeChanged()
{
   InitSegmentsStyle();
}