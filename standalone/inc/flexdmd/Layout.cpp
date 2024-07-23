#include "stdafx.h"
#include "Layout.h"

void Layout::Scale(Scaling mode, float sourceWidth, float sourceHeight, float targetWidth, float targetHeight, float& width, float& height)
{
   switch (mode) {
      case Scaling_Fit: {
         float targetRatio = targetHeight / targetWidth;
         float sourceRatio = sourceHeight / sourceWidth;
         float scale = targetRatio > sourceRatio ? targetWidth / sourceWidth : targetHeight / sourceHeight;
         width = sourceWidth * scale;
         height = sourceHeight * scale;
         break;
      }
      case Scaling_Fill: {
         float targetRatio = targetHeight / targetWidth;
         float sourceRatio = sourceHeight / sourceWidth;
         float scale = targetRatio < sourceRatio ? targetWidth / sourceWidth : targetHeight / sourceHeight;
         width = sourceWidth * scale;
         height = sourceHeight * scale;
         break;
      }
      case Scaling_FillX: {
         float scale = targetWidth / sourceWidth;
         width = sourceWidth * scale;
         height = sourceHeight * scale;
         break;
      }
      case Scaling_FillY: {
         float scale = targetHeight / sourceHeight;
         width = sourceWidth * scale;
         height = sourceHeight * scale;
         break;
      }
      case Scaling_Stretch:
         width = targetWidth;
         height = targetHeight;
         break;
      case Scaling_StretchX:
         width = targetWidth;
         height = sourceHeight;
         break;
      case Scaling_StretchY:
         width = sourceWidth;
         height = targetHeight;
         break;
      case Scaling_None:
         width = sourceWidth;
         height = sourceHeight;
         break;
      default:
         width = 0;
         height = 0;
         break;
   }
}

void Layout::Align(Alignment mode, float width, float height, float containerWidth, float containerHeight, float& x, float& y)
{
   switch (mode) {
      case Alignment_TopLeft:
      case Alignment_Left:
      case Alignment_BottomLeft:
         x = 0.0f;
         break;
      case Alignment_Top:
      case Alignment_Center:
      case Alignment_Bottom:
         x = (containerWidth - width) * 0.5f;
         break;
      case Alignment_TopRight:
      case Alignment_Right:
      case Alignment_BottomRight:
         x = containerWidth - width;
         break;
      default:
         x = 0.0f;
         break;
   }
   switch (mode) {
      case Alignment_TopLeft:
      case Alignment_Top:
      case Alignment_TopRight:
         y = 0.0f;
         break;
      case Alignment_Left:
      case Alignment_Center:
      case Alignment_Right:
         y = (containerHeight - height) * 0.5f;
         break;
      case Alignment_BottomLeft:
      case Alignment_Bottom:
      case Alignment_BottomRight:
         y = containerHeight - height;
         break;
      default:
         y = 0.0f;
         break;
   }
}