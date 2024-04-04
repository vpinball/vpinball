#pragma once

#include "PUPManager.h"

class PUPCustomPos {
public:
   ~PUPCustomPos() {}

   static PUPCustomPos* CreateFromCSVLine(string line);
   const int GetSourceScreen() const { return m_sourceScreen; }
   const float GetXPos() const { return m_xpos; }
   const float GetYPos() const { return m_ypos; }
   const float GetWidth() const { return m_width; }
   const float GetHeight() const { return m_height; }
   string ToString() const;

private:
   PUPCustomPos() {}
   
   int m_sourceScreen;
   float m_xpos;
   float m_ypos;
   float m_width;
   float m_height;
};

