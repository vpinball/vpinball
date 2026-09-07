// Win32++   Version 10.3.0
// Release Date: 4th September 2026
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//           https://github.com/DavidNash2024/Win32xx
//
//
// Copyright (c) 2005-2026  David Nash
//
// Permission is hereby granted, free of charge, to
// any person obtaining a copy of this software and
// associated documentation files (the "Software"),
// to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify,
// merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom
// the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice
// shall be included in all copies or substantial portions
// of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.
//
////////////////////////////////////////////////////////


///////////////////////////////////////////////////////
// wxx_rect.h
//  Definitions of the CSize, CPoint and CRect classes.


#ifndef WIN32XX_RECT_H_
#define WIN32XX_RECT_H_


namespace Win32xx
{
    // Forward declarations
    class CRect;
    class CSize;


    ////////////////////////////////////////////////////////////////
    // The CPoint class can be used in place of the POINT structure.
    class CPoint : public POINT
    {
    public:
        CPoint();
        CPoint(int x, int y);
        CPoint(POINT pt);
        CPoint(POINTS pts);
        CPoint(SIZE sz);
        CPoint(LPARAM dwPos);

        void Offset(int dx, int dy);
        void Offset(POINT pt);
        void Offset(SIZE sz);
        void SetPoint(int px, int py);

        // Operators
        operator LPPOINT();
        bool operator==(CPoint pt) const;
        bool operator!=(CPoint pt) const;
        void operator+=(SIZE sz);
        void operator-=(SIZE sz);
        void operator+=(POINT pt);
        void operator-=(POINT pt);

        // Operators returning CPoint
        CPoint operator-() const;
        CPoint operator+(SIZE sz) const;
        CPoint operator-(SIZE sz) const;
        CPoint operator+(POINT pt) const;
        CPoint operator-(POINT pt) const;

        // Operators returning CRect
        CRect operator+(LPCRECT prc) const;
        CRect operator-(LPCRECT prc) const;
    };


    //////////////////////////////////////////////////////////////
    // The CRect class can be used in place of the RECT structure.
    class CRect : public RECT
    {
    public:
        CRect();
        CRect(int l, int t, int r, int b);
        CRect(RECT rc);
        CRect(POINT pt, SIZE sz);
        CRect(POINT topLeft, POINT bottomRight);

        BOOL CopyRect(LPCRECT prc);
        BOOL DeflateRect(int x, int y);
        BOOL DeflateRect(SIZE size);
        void DeflateRect(LPCRECT prc);
        void DeflateRect(int l, int t, int r, int b);
        BOOL EqualRect(LPRECT prc) const;
        BOOL InflateRect(int dx, int dy);
        BOOL InflateRect(SIZE sz);
        void InflateRect(LPCRECT prc);
        void InflateRect(int l, int t, int r, int b);
        BOOL IntersectRect(LPCRECT prc1, LPCRECT prc2);
        BOOL IsRectEmpty() const;
        BOOL IsRectNull() const;
        CRect MulDiv(int mult, int div) const;
        void NormalizeRect();
        BOOL OffsetRect(int dx, int dy);
        BOOL OffsetRect(POINT pt);
        BOOL OffsetRect(SIZE size);
        BOOL PtInRect(POINT pt) const;
        BOOL SetRect(int l, int t, int r, int b);
        BOOL SetRect(POINT topLeft, POINT bottomRight);
        BOOL SetRectEmpty();
        BOOL SubtractRect(LPCRECT prc1, LPCRECT prc2);
        BOOL UnionRect(LPCRECT prc1, LPCRECT prc2);

        // Reposition rectangle
        void MoveToX(int x);
        void MoveToY(int y);
        void MoveToXY(int x, int y);
        void MoveToXY(POINT pt);

        // Attributes
        int Height() const;
        int Width() const;
        CSize Size() const;
        CPoint CenterPoint() const;
        CPoint TopLeft() const;
        CPoint BottomRight() const;

        // operators
        operator LPRECT();
        operator LPCRECT() const;
        bool operator==(CRect rc) const;
        bool operator!=(CRect rc) const;
        void operator+=(POINT pt);
        void operator+=(SIZE size);
        void operator+=(LPCRECT prc);
        void operator-=(LPCRECT prc);
        void operator-=(POINT pt);
        void operator-=(SIZE sz);
        void operator&=(RECT rc);
        void operator|=(RECT rc);

        // Operators returning CRect
        CRect operator+(POINT pt) const;
        CRect operator-(POINT pt) const;
        CRect operator+(SIZE sz) const;
        CRect operator-(SIZE sz) const;
        CRect operator+(LPRECT prc) const;
        CRect operator-(LPRECT prc) const;
        CRect operator&(RECT rc) const;
        CRect operator|(RECT rc) const;
    };


    //////////////////////////////////////////////////////////////
    // The CSize class can be used in place of the SIZE structure.
    class CSize : public SIZE
    {
    public:
        CSize();
        CSize(int px, int py);
        CSize(SIZE sz);
        CSize(POINT pt);
        CSize(DWORD dw);
        void SetSize(int sx, int sy);

        // Operators
        operator LPSIZE();
        bool operator==(CSize sz) const;
        bool operator!=(CSize sz) const;
        void operator+=(SIZE sz);
        void operator-=(SIZE sz);

        // Operators returning CSize
        CSize operator-() const;
        CSize operator+(SIZE sz) const;
        CSize operator-(SIZE sz) const;

        // Operators returning CPoint
        CPoint operator+(POINT point) const;
        CPoint operator-(POINT point) const;

        // Operators returning CRect
        CRect operator+(LPCRECT prc) const;
        CRect operator-(LPCRECT prc) const;
    };


    ////////////////////////////////////
    // Definitions for the CPoint class.
    //

    // CPoint constructor.
    inline CPoint::CPoint()
    {
        x = 0;
        y = 0;
    }

    // CPoint constructor.
    inline CPoint::CPoint(int px, int py)
    {
        x = px;
        y = py;
    }

    // CPoint constructor.
    inline CPoint::CPoint(POINT pt)
    {
        x = pt.x;
        y = pt.y;
    }

    // CPoint constructor.
    inline CPoint::CPoint(POINTS pts)
    {
        x = pts.x;
        y = pts.y;
    }

    // CPoint constructor.
    inline CPoint::CPoint(SIZE sz)
    {
        x = sz.cx;
        y = sz.cy;
    }

    // CPoint constructor.
    inline CPoint::CPoint(LPARAM dwPos)
    {
        x = GET_X_LPARAM(dwPos);
        y = GET_Y_LPARAM(dwPos);
    }

    // Moves the CPoint by the specified offsets.
    inline void CPoint::Offset(int dx, int dy)
    {
        x += dx;
        y += dy;
    }

    // Moves the CPoint by the specified offsets.
    inline void CPoint::Offset(POINT pt)
    {
        x += pt.x;
        y += pt.y;
    }

    // Moves the CPoint by the specified offsets.
    inline void CPoint::Offset(SIZE sz)
    {
        x += sz.cx;
        y += sz.cy;
    }

    // Sets the coordinates of the CPoint.
    inline void CPoint::SetPoint(int px, int py)
    {
        x = px;
        y = py;
    }

    // Returns a pointer to the POINT associated with this object.
    inline CPoint::operator LPPOINT()
    {
        return this;
    }

    // Returns true if the co-ordinates of the source point and the CPoint are
    // equal.
    inline bool CPoint::operator==(CPoint pt) const
    {
        return ((x == pt.x) && (y == pt.y));
    }

    // Returns true if the co-ordinates of the source point and the CPoint are
    // not equal.
    inline bool CPoint::operator!=(CPoint pt) const
    {
        return ((x != pt.x) || (y != pt.y));
    }

    // Adds the specified SIZE to the point.
    inline void CPoint::operator+=(SIZE sz)
    {
        x += sz.cx;
        y += sz.cy;
    }

    // Subtracts the specified SIZE from this point.
    inline void CPoint::operator-=(SIZE sz)
    {
        x -= sz.cx;
        y -= sz.cy;
    }

    // Adds the specified POINT to this point.
    inline void CPoint::operator+=(POINT pt)
    {
        x += pt.x;
        y += pt.y;
    }

    // Subtracts the specified POINT from this point.
    inline void CPoint::operator-=(POINT pt)
    {
        x -= pt.x;
        y -= pt.y;
    }

    // Returns the unary minus (additive inverse).
    inline CPoint CPoint::operator-() const
    {
        return CPoint(-x, -y);
    }

    // Adds the specified SIZE and returns the value.
    inline CPoint CPoint::operator+(SIZE sz) const
    {
        return CPoint(x + sz.cx, y + sz.cy);
    }

    // Subtracts the specified SIZE and returns the value.
    inline CPoint CPoint::operator-(SIZE sz) const
    {
        return CPoint(x - sz.cx, y - sz.cy);
    }

    // Adds the specified POINT and returns the value.
    inline CPoint CPoint::operator+(POINT pt) const
    {
        return CPoint(x + pt.x, y + pt.y);
    }

    // Subtracts the specified POINT and returns the value.
    inline CPoint CPoint::operator-(POINT pt) const
    {
        return CPoint(x - pt.x, y - pt.y);
    }

    // Adds the specified RECT and returns the value.
    inline CRect CPoint::operator+(LPCRECT prc) const
    {
        return CRect(*prc) + *this;
    }

    // Subtracts the specified RECT and returns the value.
    inline CRect CPoint::operator-(LPCRECT prc) const
    {
        return CRect(*prc) - *this;
    }


    ///////////////////////////////////
    // Definitions for the CRect class.
    //

    // CRect constructor.
    inline CRect::CRect()
    {
        left = 0;
        top = 0;
        right = 0;
        bottom = 0;
    }

    // CRect constructor.
    inline CRect::CRect(int l, int t, int r, int b)
    {
        left = l;
        top = t;
        right = r;
        bottom = b;
    }

    // CRect constructor.
    inline CRect::CRect(RECT rc)
    {
        left = rc.left;
        top = rc.top;
        right = rc.right;
        bottom = rc.bottom;
    }

    // CRect constructor.
    inline CRect::CRect(POINT pt, SIZE sz)
    {
        left = pt.x;
        top = pt.y;
        right = pt.x + sz.cx;
        bottom = pt.y + sz.cy;
    }

    // CRect constructor.
    inline CRect::CRect(POINT topLeft, POINT bottomRight)
    {
        left = topLeft.x;
        top = topLeft.y;
        right = bottomRight.x;
        bottom = bottomRight.y;
    }

    // Copies the coordinates of the source rectangle to the CRect.
    inline BOOL CRect::CopyRect(LPCRECT prc)
    {
        return ::CopyRect(this, prc);
    }

    // Decreases the width and height of the CRect.
    inline BOOL CRect::DeflateRect(int x, int y)
    {
        return ::InflateRect(this, -x, -y);
    }

    // Decreases the width and height of the CRect.
    inline BOOL CRect::DeflateRect(SIZE size)
    {
        return ::InflateRect(this, -size.cx, -size.cy);
    }

    // Decreases the width and height of the CRect.
    inline void CRect::DeflateRect(LPCRECT prc)
    {
        left += prc->left;
        top += prc->top;
        right -= prc->right;
        bottom -= prc->bottom;
    }

    // Decreases the width and height of the CRect.
    inline void CRect::DeflateRect(int l, int t, int r, int b)
    {
        left += l;
        top += t;
        right -= r;
        bottom -= b;
    }

    // Determines whether the source rectangle and the CRect are equal by
    // comparing the coordinates of their upper-left and lower-right corners.
    inline BOOL CRect::EqualRect(LPRECT prc) const
    {
        return ::EqualRect(prc, this);
    }

    // Increases the width and height of the CRect.
    inline BOOL CRect::InflateRect(int dx, int dy)
    {
        return ::InflateRect(this, dx, dy);
    }

    // Increases the width and height of the CRect.
    inline BOOL CRect::InflateRect(SIZE sz)
    {
        return ::InflateRect(this, sz.cx, sz.cy);
    }

    // Increases the width and height of the CRect.
    inline void CRect::InflateRect(LPCRECT prc)
    {
        left -= prc->left;
        top -= prc->top;
        right += prc->right;
        bottom += prc->bottom;
    }

    // Increases the width and height of the CRect.
    inline void CRect::InflateRect(int l, int t, int r, int b)
    {
        left -= l;
        top -= t;
        right += r;
        bottom += b;
    }

    // Calculates the intersection of two source rectangles and places the
    // coordinates of the intersection rectangle into the CRect.
    inline BOOL CRect::IntersectRect(LPCRECT prc1, LPCRECT prc2)
    {
        return ::IntersectRect(this, prc1, prc2);
    }

    // Determines whether the CRect is empty.
    inline BOOL CRect::IsRectEmpty() const
    {
        return ::IsRectEmpty(this);
    }

    // Determines whether the CRect is null.
    inline BOOL CRect::IsRectNull() const
    {
        return (left == 0 && right == 0 && top == 0 && bottom == 0);
    }

    // Multiplies the CRect values by mult, and then divides the result by div.
    inline CRect CRect::MulDiv(int mult, int div) const
    {
        return CRect((left * mult) / div, (top * mult) / div,
        (right * mult) / div, (bottom * mult) / div);
    }

    // Normalizes CRect so that both the height and width are positive.
    inline void CRect::NormalizeRect()
    {
        int temp;
        if (left > right)
        {
            temp = left;
            left = right;
            right = temp;
        }

        if (top > bottom)
        {
            temp = top;
            top = bottom;
            bottom = temp;
        }
    }

    // Moves the CRect by the specified offsets.
    inline BOOL CRect::OffsetRect(int dx, int dy)
    {
        return ::OffsetRect(this, dx, dy);
    }

    // Moves the CRect by the specified offsets.
    inline BOOL CRect::OffsetRect(POINT pt)
    {
        return ::OffsetRect(this, pt.x, pt.y);
    }

    // Moves the CRect by the specified offsets.
    inline BOOL CRect::OffsetRect(SIZE size)
    {
        return ::OffsetRect(this, size.cx, size.cy);
    }

    // Determines whether the specified point lies within the CRect.
    inline BOOL CRect::PtInRect(POINT pt) const
    {
        return ::PtInRect(this, pt);
    }

    // Sets the coordinates of the CRect.
    inline BOOL CRect::SetRect(int l, int t, int r, int b)
    {
        return ::SetRect(this, l, t, r, b);
    }

    // Sets the coordinates of the CRect.
    inline BOOL CRect::SetRect(POINT topLeft, POINT bottomRight)
    {
        return ::SetRect(this, topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
    }

    // Sets all the coordinates of the CRect to zero.
    inline BOOL CRect::SetRectEmpty()
    {
        return ::SetRectEmpty(this);
    }

    // Sets the coordinates of the CRect to those formed by subtracting one
    // rectangle from another.
    inline BOOL CRect::SubtractRect(LPCRECT prc1, LPCRECT prc2)
    {
        return ::SubtractRect(this, prc1, prc2);
    }

    // Creates the union of two rectangles.
    inline BOOL CRect::UnionRect(LPCRECT prc1, LPCRECT prc2)
    {
        return ::UnionRect(this, prc1, prc2);
    }

    // Moves the CRect to the specified left position.
    inline void CRect::MoveToX(int x)
    {
        right = Width() + x;
        left = x;
    }

    // Moves the CRect to the specified top position.
    inline void CRect::MoveToY(int y)
    {
        bottom = Height() + y;
        top = y;
    }

    // Moves to CRect to the specified left and top positions.
    inline void CRect::MoveToXY(int x, int y)
    {
        MoveToX(x);
        MoveToY(y);
    }

    // Moves to CRect to the specified left and top positions.
    inline void CRect::MoveToXY(POINT pt)
    {
        MoveToX(pt.x);
        MoveToY(pt.y);
    }

    // Returns the height of the CRect.
    inline int CRect::Height() const
    {
        return bottom - top;
    }

    // Returns the width of the CRect.
    inline int CRect::Width() const
    {
        return right - left;
    }

    // Returns the size (width and height) of the CRect.
    inline CSize CRect::Size() const
    {
        return CSize(Width(), Height());
    }

    // Returns the point at the center of the CRect.
    inline CPoint CRect::CenterPoint() const
    {
        return CPoint((left + right) / 2, (top + bottom) / 2);
    }

    // Returns the top left point of the CRect.
    inline CPoint CRect::TopLeft() const
    {
        return CPoint(left, top);
    }

    // Returns the bottom right point of the CRect.
    inline CPoint CRect::BottomRight() const
    {
        return CPoint(right, bottom);
    }

    // Returns a pointer to the RECT associated with this CRect.
    inline CRect::operator LPRECT()
    {
        return this;
    }

    // Returns a const pointer to the RECT associated with this CRect.
    inline CRect::operator LPCRECT() const
    {
        return this;
    }

    // Returns true if the co-ordinates of the source rectangle and the CRect
    // are equal.
    inline bool CRect::operator==(CRect rc) const
    {
        return (::EqualRect(this, &rc) != 0);
    }

    // Returns true if the co-ordinates of the source rectangle and the CRect
    // are not equal.
    inline bool CRect::operator!=(CRect rc) const
    {
        return (::EqualRect(this, &rc) == 0);
    }

    // Adds the specified value to the CRect.
    inline void CRect::operator+=(POINT pt)
    {
        ::OffsetRect(this, pt.x, pt.y);
    }

    // Adds the specified value to the CRect.
    inline void CRect::operator+=(SIZE size)
    {
        ::OffsetRect(this, size.cx, size.cy);
    }

    // Adds the specified value to the CRect.
    inline void CRect::operator+=(LPCRECT prc)
    {
        ::InflateRect(this, prc->right - prc->left, prc->bottom - prc->top);
    }

    // Subtracts the specified value from the CRect.
    inline void CRect::operator-=(LPCRECT prc)
    {
        ::InflateRect(this, prc->left - prc->right, prc->top - prc->bottom);
    }

    // Subtracts the specified value from the CRect.
    inline void CRect::operator-=(POINT pt)
    {
        ::OffsetRect(this, -pt.x, -pt.y);
    }

    // Subtracts the specified value from the CRect.
    inline void CRect::operator-=(SIZE sz)
    {
        ::OffsetRect(this, -sz.cx, -sz.cy);
    }

    // Determines the intersection with the specified RECT.
    inline void CRect::operator&=(RECT rc)
    {
        ::IntersectRect(this, this, &rc);
    }

    // Determines the union with the specified RECT.
    inline void CRect::operator|=(RECT rc)
    {
        ::UnionRect(this, this, &rc);
    }

    // Offsets the CRect and returns the result.
    inline CRect CRect::operator+(POINT pt) const
    {
        CRect rc(*this);
        ::OffsetRect(&rc, pt.x, pt.y);
        return rc;
    }

    // Offsets the CRect and returns the result.
    inline CRect CRect::operator-(POINT pt) const
    {
        CRect rc(*this);
        ::OffsetRect(&rc, -pt.x, -pt.y);
        return rc;
    }

    // Offsets the CRect and returns the result.
    inline CRect CRect::operator+(SIZE sz) const
    {
        CRect rc(*this);
        ::OffsetRect(&rc, sz.cx, sz.cy);
        return rc;
    }

    // Offsets the CRect and returns the result.
    inline CRect CRect::operator-(SIZE sz) const
    {
        CRect rc(*this);
        ::OffsetRect(&rc, -sz.cx, -sz.cy);
        return rc;
    }

    // Offsets the CRect and returns the result.
    inline CRect CRect::operator+(LPRECT prc) const
    {
        CRect rc1(*this);
        rc1.InflateRect(prc);
        return rc1;
    }

    // Offsets the CRect and returns the result.
    inline CRect CRect::operator-(LPRECT prc) const
    {
        CRect rc1(*this);
        rc1.DeflateRect(prc);
        return rc1;
    }

    // Returns the intersection with the specified RECT.
    inline CRect CRect::operator&(RECT rc) const
    {
        CRect rc1;
        ::IntersectRect(&rc1, this, &rc);
        return rc1;
    }

    // Returns the union with the specified RECT.
    inline CRect CRect::operator|(RECT rc) const
    {
        CRect rc1;
        ::UnionRect(&rc1, this, &rc);
        return rc1;
    }


    ///////////////////////////////////
    // Definitions for the CSize class.
    //

    // CSize constructor.
    inline CSize::CSize()
    {
        cx = 0;
        cy = 0;
    }

    // CSize constructor.
    inline CSize::CSize(int x, int y)
    {
        cx = x;
        cy = y;
    }

    // CSize constructor.
    inline CSize::CSize(SIZE sz)
    {
        cx = sz.cx;
        cy = sz.cy;
    }

    // CSize constructor.
    inline CSize::CSize(POINT pt)
    {
        cx = pt.x;
        cy = pt.y;
    }

    // CSize constructor.
    inline CSize::CSize(DWORD dw)
    {
        cx = LOWORD(dw);
        cy = HIWORD(dw);
    }

    // Sets the coordinates of the CSize.
    inline void CSize::SetSize(int sx, int sy)
    {
        cx = sx;
        cy = sy;
    }

    // Returns the pointer to the SIZE associated with this object.
    inline CSize::operator LPSIZE()
    {
        return this;
    }

    // Returns true if the co-ordinates of the source size and the CSize are
    // equal.
    inline bool CSize::operator==(CSize sz) const
    {
        return (cx == sz.cx && cy == sz.cy);
    }

    // Returns true if the co-ordinates of the source size and the CSize are
    // not equal.
    inline bool CSize::operator!=(CSize sz) const
    {
        return (cx != sz.cx || cy != sz.cy);
    }

    // Adds the specified SIZE.
    inline void CSize::operator+=(SIZE sz)
    {
        cx += sz.cx;
        cy += sz.cy;
    }

    // Subtracts the specified SIZE.
    inline void CSize::operator-=(SIZE sz)
    {
        cx -= sz.cx;
        cy -= sz.cy;
    }

    // Returns the unary minus (additive inverse).
    inline CSize CSize::operator-() const
    {
        return CSize(-cx, -cy);
    }

    // Adds the specified SIZE and returns the value.
    inline CSize CSize::operator+(SIZE sz) const
    {
        return CSize(cx + sz.cx, cy + sz.cy);
    }

    // Subtracts the specified SIZE and returns the value.
    inline CSize CSize::operator-(SIZE sz) const
    {
        return CSize(cx - sz.cx, cy - sz.cy);
    }

    // Adds the specified POINT and returns the value.
    inline CPoint CSize::operator+(POINT pt) const
    {
        return CPoint(cx + pt.x, cy + pt.y);
    }

    // Subtracts the specified POINT and returns the value.
    inline CPoint CSize::operator-(POINT pt) const
    {
        return CPoint(cx - pt.x, cy - pt.y);
    }

    // Adds the specified RECT and returns the value.
    inline CRect CSize::operator+(LPCRECT prc) const
    {
        return CRect(*prc) + *this;
    }

    // Subtracts the specified RECT and returns the value.
    inline CRect CSize::operator-(LPCRECT prc) const
    {
        return CRect(*prc) - *this;
    }

    ////////////////////
    // Global Functions.
    //

    // Returns a CPoint holding the current cursor position.
    inline CPoint GetCursorPos()
    {
        CPoint pt;
        if (!::GetCursorPos(&pt))
        {
            TRACE("GetCursorPos failed\n");
            pt.x = 0;
            pt.y = 0;
        }
        return pt;
    }

} // namespace Win32xx

#endif // WIN32XX_RECT_H_
