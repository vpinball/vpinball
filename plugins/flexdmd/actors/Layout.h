#pragma once

namespace Flex {

enum Alignment
{
   Alignment_TopLeft = 0,
   Alignment_Top = 1,
   Alignment_TopRight = 2,
   Alignment_Left = 3,
   Alignment_Center = 4,
   Alignment_Right = 5,
   Alignment_BottomLeft = 6,
   Alignment_Bottom = 7,
   Alignment_BottomRight = 8
};

enum Scaling
{
   Scaling_Fit = 0,
   Scaling_Fill = 1,
   Scaling_FillX = 2,
   Scaling_FillY = 3,
   Scaling_Stretch = 4,
   Scaling_StretchX = 5,
   Scaling_StretchY = 6,
   Scaling_None = 7
};

class Layout final
{
public:
   static void Scale(Scaling mode, int sourceWidth, int sourceHeight, int targetWidth, int targetHeight, float& width, float& height)
   {
      switch (mode) {
         case Scaling_Fit: {
            float targetRatio = (float)targetHeight / (float)targetWidth;
            float sourceRatio = (float)sourceHeight / (float)sourceWidth;
            if (targetRatio > sourceRatio)
            {
               width = (float)targetWidth;
               height = (float)(sourceHeight * targetWidth) / (float)sourceWidth;
            }
            else
            {
               width = (float)(sourceWidth * targetHeight) / (float)sourceHeight;
               height = (float)targetHeight;
            }
            break;
         }
         case Scaling_Fill: {
            float targetRatio = (float)targetHeight / (float)targetWidth;
            float sourceRatio = (float)sourceHeight / (float)sourceWidth;
            if (targetRatio < sourceRatio)
            {
               width = (float)targetWidth;
               height = (float)(sourceHeight * targetWidth) / (float)sourceWidth;
            }
            else
            {
               width = (float)(sourceWidth * targetHeight) / (float)sourceHeight;
               height = (float)targetHeight;
            }
            break;
         }
         case Scaling_FillX: {
            width = (float)targetWidth;
            height = (float)(sourceHeight * targetWidth) / (float)sourceWidth;
            break;
         }
         case Scaling_FillY: {
            width = (float)(sourceWidth * targetHeight) / (float)sourceHeight;
            height = (float)targetHeight;
            break;
         }
         case Scaling_Stretch:
            width = (float)targetWidth;
            height = (float)targetHeight;
            break;
         case Scaling_StretchX:
            width = (float)targetWidth;
            height = (float)sourceHeight;
            break;
         case Scaling_StretchY:
            width = (float)sourceWidth;
            height = (float)targetHeight;
            break;
         case Scaling_None:
            width = (float)sourceWidth;
            height = (float)sourceHeight;
            break;
         default:
            width = 0.f;
            height = 0.f;
            break;
      }
   }

   static void Align(Alignment mode, float width, float height, int containerWidth, int containerHeight, float& x, float& y)
   {
      switch (mode) {
         default:
         case Alignment_TopLeft:
         case Alignment_Left:
         case Alignment_BottomLeft:
            x = 0.0f;
            break;
         case Alignment_Top:
         case Alignment_Center:
         case Alignment_Bottom:
            x = ((float)containerWidth - width) * 0.5f;
            break;
         case Alignment_TopRight:
         case Alignment_Right:
         case Alignment_BottomRight:
            x = (float)containerWidth - width;
            break;
      }
      switch (mode) {
         default:
         case Alignment_TopLeft:
         case Alignment_Top:
         case Alignment_TopRight:
            y = 0.0f;
            break;
         case Alignment_Left:
         case Alignment_Center:
         case Alignment_Right:
            y = ((float)containerHeight - height) * 0.5f;
            break;
         case Alignment_BottomLeft:
         case Alignment_Bottom:
         case Alignment_BottomRight:
            y = (float)containerHeight - height;
            break;
      }
   }

   static void ApplyAlign(Alignment mode, int width, int height, float& x, float& y)
   {
      switch (mode)
      {
      case Alignment_BottomLeft:
      case Alignment_Left:
      case Alignment_TopLeft: break;
      case Alignment_Bottom:
      case Alignment_Center:
      case Alignment_Top: x -= (float)width * 0.5f; break;
      case Alignment_BottomRight:
      case Alignment_Right:
      case Alignment_TopRight: x -= (float)width; break;
      }
      switch (mode)
      {
      case Alignment_BottomLeft:
      case Alignment_Bottom:
      case Alignment_BottomRight: y -= (float)height; break;
      case Alignment_Left:
      case Alignment_Center:
      case Alignment_Right: y -= (float)height * 0.5f; break;
      case Alignment_TopLeft:
      case Alignment_Top:
      case Alignment_TopRight: break;
      }
   }
};

}
