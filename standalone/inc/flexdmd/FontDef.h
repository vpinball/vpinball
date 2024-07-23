#pragma once

#include "BaseDef.h"

class FontDef : public BaseDef
{
public:
   FontDef(const string& path, OLE_COLOR tint, OLE_COLOR borderTint, int borderSize = 0);
   ~FontDef();

   bool operator==(const FontDef& other) const;
   size_t hash() const;

   const string& GetPath() const { return m_szPath; }
   OLE_COLOR GetTint() const { return m_tint; }
   OLE_COLOR GetBorderTint() const { return m_borderTint; }
   int GetBorderSize() const { return m_borderSize; }

private:
   string m_szPath;
   OLE_COLOR m_tint;
   OLE_COLOR m_borderTint;
   int m_borderSize;
};
