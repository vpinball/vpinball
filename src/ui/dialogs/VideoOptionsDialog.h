// license:GPLv3+

#pragma once

#include <wxx_propertysheet.h>  // Add CPropertyPage, CPropertySheet

class VideoOptionProperties : public CPropertySheet
{
public:
   VideoOptionProperties(HWND hParent /* = nullptr*/);

private:
   Settings m_appSettings, m_tableSettings;
};
