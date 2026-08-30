// license:GPLv3+

#pragma once

#include "core/stdafx.h"
#include "ui/VPXFileFeedback.h"


class VPXProgress: public VPXFileFeedback
{
public:
   void SetProgress(unsigned int progress) override;
   void SetLength(unsigned int length) override;
   
private:
   unsigned int m_progress = 0;
   unsigned int m_total = 1;
};
