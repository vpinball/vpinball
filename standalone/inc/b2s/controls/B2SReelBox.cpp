#include "core/stdafx.h"

#include "B2SReelBox.h"

B2SReelBox::B2SReelBox(B2SData* pB2SData) : B2SBaseBox(pB2SData)
{
   m_led = false;
   m_length = 1;
   m_initValue = "0";
   m_szReelIndex.clear();
   m_intermediates = -1;
   m_intermediates2go = 0;
   m_szSoundName.clear();
   m_pSound = NULL;
   m_scoreType = eScoreType_NotUsed;
   m_szGroupName.clear();
   m_illuminated = false;
   m_value = 0;
   m_currentText = 0;
   m_text = 0;
   m_rollingInterval = cTimerInterval;
   m_pTimer = new VP::Timer(m_rollingInterval, std::bind(&B2SReelBox::ReelAnimationTimerTick, this, std::placeholders::_1));
   m_firstintermediatecount = 1;
}

B2SReelBox::~B2SReelBox()
{
   delete m_pTimer;
}

void B2SReelBox::OnPaint(VP::RendererGraphics* pGraphics)
{
   if (IsVisible()) {
      if (!m_szReelIndex.empty()) {
         B2SData* pB2SData = GetB2SData();

         GenericDictionaryIgnoreCase<SDL_Surface*>* pImages = (m_illuminated ? pB2SData->GetReelIlluImages() : pB2SData->GetReelImages());
         GenericDictionaryIgnoreCase<SDL_Surface*>* pIntImages = (m_illuminated ? pB2SData->GetReelIntermediateIlluImages() : pB2SData->GetReelIntermediateImages());
         string name;
         SDL_Rect rect = GetRect();
         if (m_intermediates == -1 && m_pTimer->IsEnabled()) {
            name = m_szReelType + '_' + m_szReelIndex + (m_setID > 0 && m_illuminated ? '_' + std::to_string(m_setID) : string()) + '_' + std::to_string(m_firstintermediatecount);
            if (pIntImages->contains(name)) {
               pGraphics->DrawImage((*pIntImages)[name], NULL, &rect);
               m_firstintermediatecount++;
               m_intermediates2go = 2;
            }
            else {
               name = m_szReelType + '_' + ConvertText(m_currentText + 1) + (m_setID > 0 && m_illuminated ? '_' + std::to_string(m_setID) : string());
               if (pImages->contains(name))
                  pGraphics->DrawImage((*pImages)[name], NULL, &rect);
               m_intermediates = m_firstintermediatecount - 1;
               m_intermediates2go = 1;
            }
         }
         else if (m_intermediates2go > 0) {
            name = m_szReelType + '_' + m_szReelIndex + (m_setID > 0 && m_illuminated ? '_' + std::to_string(m_setID) : string()) + '_' + std::to_string(m_intermediates - m_intermediates2go + 1);
            if (pIntImages->contains(name))
               pGraphics->DrawImage((*pIntImages)[name], NULL, &rect);
         }
         else { 
            if (m_intermediates2go == 0 && m_intermediates > 0)
               name = m_szReelType + '_' + ConvertText(m_currentText + 1) + (m_setID > 0 && m_illuminated ? '_' + std::to_string(m_setID) : string());
            else
               name = m_szReelType + '_' + m_szReelIndex + (m_setID > 0 && m_illuminated ? '_' + std::to_string(m_setID) : string());
            if (pImages->contains(name)) 
               pGraphics->DrawImage((*pImages)[name], NULL, &rect);
         }
      }
   }

   Control::OnPaint(pGraphics);
}

void B2SReelBox::ReelAnimationTimerTick(VP::Timer* pTimer)
{
   if (m_intermediates2go > 0 || m_intermediates == -1) {
      Refresh();
      m_intermediates2go--;
   }
   else {
      if (m_intermediates2go == 0) {
         // add one reel step
         m_currentText++;
         if (m_currentText > 9) {
            m_currentText = 0;
         }

         m_szReelIndex = ConvertText(m_currentText);

         // play sound and redraw reel
         if (m_pSound != NULL) {
            //My.Computer.Audio.Play(Sound(), AudioPlayMode.Background)
         }
         else if (m_szSoundName == "stille") {
            // no sound
         }
         else {
            //My.Computer.Audio.Play(My.Resources.EMReel, AudioPlayMode.Background)
         }

         Refresh();
         m_intermediates2go--;
      }
      else if (m_intermediates2go == -1)
         m_intermediates2go--;
      else {
         // maybe stop timer
         m_intermediates2go = m_intermediates;
         if (m_currentText == m_text || m_text >= 10) {
             m_pTimer->Stop();
             m_pTimer->SetInterval(cTimerInterval);
             m_intermediates2go = -1;
         }
      }
   }
}

void B2SReelBox::SetRollingInterval(int rollingInterval)
{
   if (m_rollingInterval != rollingInterval) {
      m_rollingInterval = rollingInterval;
      if (m_rollingInterval < 10)
         m_rollingInterval = cTimerInterval;
   }
}

void B2SReelBox::SetReelType(const string& szReelType)
{
   string value = szReelType;

   m_szReelIndex = "0";
   if (szReelType.substr(value.length() - 1, 1) == "_") {
      m_length = 2;
       m_szReelIndex = "00"s;
       value = value.substr(0, value.length() - 1);
   }
   if (string_starts_with_case_insensitive(szReelType, "led"s) || string_starts_with_case_insensitive(szReelType, "importedled"s)) {
      m_led = true;
      m_szReelIndex = "Empty"s;
      m_initValue = "Empty"s;
      SetText(-1);
   }
    m_szReelType = value;
}

void B2SReelBox::SetIlluminated(bool illuminated)
{
   if (m_illuminated != illuminated) {
      m_illuminated = illuminated;
      m_intermediates2go = 0;
      Refresh();
   }
}

void B2SReelBox::SetValue(int value, bool refresh)
{
   if (m_value != value || refresh) {
      m_value = value;
      m_szReelIndex = ConvertValue(m_value);
      Refresh();
   }
}

void B2SReelBox::SetText(int text, bool animateReelChange)
{
   if (text >= 0) {
      if (m_text != text) {
         m_text = text;
         if (animateReelChange && !m_led) {
            m_pTimer->Stop();
            m_intermediates2go = m_intermediates;
            m_pTimer->Start();
         }
         else {
            m_szReelIndex = ConvertText(m_text);
            Refresh();
         }
      }
   }
}

string B2SReelBox::ConvertValue(int value)
{
   string ret;
   // remove the "," from the 7-segmenter
   if (value >= 128 && value <= 255)
      value -= 128;
   // map value
      switch (value) {
         // 7-segment stuff
         case 63: ret = "0"; break;
         case 6:  ret = "1"; break;
         case 91: ret = "2"; break;
         case 79: ret = "3"; break;
         case 102:ret = "4"; break;
         case 109:ret = "5"; break;
         case 125:ret = "6"; break;
         case 7:  ret = "7"; break;
         case 127:ret = "8"; break;
         case 111:ret = "9"; break;
         //additional 10-segment stuff
         case 768:ret = "1"; break;
         case 124:ret = "6"; break;
         case 103:ret = "9"; break;
         default: ret = m_initValue; break;
      }
   return (m_length == 2 ? "0" : string()) + ret;
}

string B2SReelBox::ConvertText(int text)
{
   const string ret = "00" + std::to_string(text);
   return ret.substr(ret.length() - m_length, m_length);
}