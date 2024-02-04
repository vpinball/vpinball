#include "stdafx.h"

#include "FontDef.h"

FontDef::FontDef(const string& path, OLE_COLOR tint, OLE_COLOR borderTint, int borderSize)
{
   m_szPath = path;
   m_tint = tint;
   m_borderTint = borderTint;
   m_borderSize = borderSize;
}

FontDef::~FontDef()
{
}

bool FontDef::operator==(const FontDef& other) const
{
   return m_szPath == other.m_szPath &&
      m_tint == other.m_tint &&
      m_borderTint == other.m_borderTint &&
      m_borderSize == other.m_borderSize ;
}

size_t FontDef::hash() const
{
   size_t hashCode = -1876634251;
   hashCode = hashCode * -1521134295 + std::hash<string>{}(m_szPath);
   hashCode = hashCode * -1521134295 + m_tint;
   hashCode = hashCode * -1521134295 + m_borderTint;
   hashCode = hashCode * -1521134295 + m_borderSize;
   return hashCode;
}