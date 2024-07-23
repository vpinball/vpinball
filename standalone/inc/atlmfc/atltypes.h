/*
 * Portions of this code was derived from MFC reference material:
 *
 * https://learn.microsoft.com/en-us/cpp/atl-mfc-shared/reference/crect-class
 */

// 130
class CRect {
public:
   CRect(int l, int t, int r, int b)
   {
      left = l;
      top = t;
      right = r;
      bottom = b;
   }

   int left;
   int top;
   int right;
   int bottom;
};
