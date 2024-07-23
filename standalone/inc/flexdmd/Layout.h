#pragma once

#include "flexdmd_i.h"

typedef enum {
   Scaling_Fit = 0,
   Scaling_Fill = 1,
   Scaling_FillX = 2,
   Scaling_FillY = 3,
   Scaling_Stretch = 4,
   Scaling_StretchX = 5,
   Scaling_StretchY = 6,
   Scaling_None = 7
} Scaling;

class Layout {
public:
   static void Scale(Scaling mode, float sourceWidth, float sourceHeight, float targetWidth, float targetHeight, float& width, float& height);
   static void Align(Alignment mode, float width, float height, float containerWidth, float containerHeight, float& x, float& y);
};