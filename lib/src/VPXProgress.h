// license:GPLv3+

#pragma once

#include "core/stdafx.h"
#include "ui/VPXFileFeedback.h"


class VPXProgress: public VPXFileFeedback
{
public:
   void LoadingProgressUpdated(int loaded, int total) override;
};
