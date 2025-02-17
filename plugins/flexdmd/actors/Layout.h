#pragma once

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
   static void Scale(Scaling mode, float sourceWidth, float sourceHeight, float targetWidth, float targetHeight, float& width, float& height)
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

   static void Align(Alignment mode, float width, float height, float containerWidth, float containerHeight, float& x, float& y)
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

   static void ApplyAlign(Alignment mode, float width, float height, float& x, float& y)
   {
      switch (mode)
      {
      case Alignment_BottomLeft:
      case Alignment_Left:
      case Alignment_TopLeft: break;
      case Alignment_Bottom:
      case Alignment_Center:
      case Alignment_Top: x -= width * 0.5f; break;
      case Alignment_BottomRight:
      case Alignment_Right:
      case Alignment_TopRight: x -= width; break;
      }
      switch (mode)
      {
      case Alignment_BottomLeft:
      case Alignment_Bottom:
      case Alignment_BottomRight: y -= height; break;
      case Alignment_Left:
      case Alignment_Center:
      case Alignment_Right: y -= height * 0.5f; break;
      case Alignment_TopLeft:
      case Alignment_Top:
      case Alignment_TopRight: break;
      }
   }
};
