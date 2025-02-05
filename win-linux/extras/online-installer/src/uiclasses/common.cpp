#include "common.h"


Margins::Margins() :
    left(0), top(0), right(0), bottom(0)
{}

Margins::Margins(int l, int t, int r, int b) :
    left(l), top(t), right(r), bottom(b)
{}

Margins::Margins(const Margins &mrg)
{
    left = mrg.left;
    top = mrg.top;
    right = mrg.right;
    bottom = mrg.bottom;
}

Margins& Margins::operator=(const Margins &mrg)
{
    if (this == &mrg)
        return *this;
    left = mrg.left;
    top = mrg.top;
    right = mrg.right;
    bottom = mrg.bottom;
    return *this;
}


Rect::Rect() :
    x(0), y(0), width(0), height(0)
{}

Rect::Rect(int x, int y, int w, int h) :
    x(x), y(y), width(w), height(h)
{}

Rect::Rect(const Rect &rc)
{
    x = rc.x;
    y = rc.y;
    width = rc.width;
    height = rc.height;
}

Rect& Rect::operator=(const Rect &rc)
{
    if (this == &rc)
        return *this;
    x = rc.x;
    y = rc.y;
    width = rc.width;
    height = rc.height;
    return *this;
}


Size::Size() :
    width(0), height(0)
{}

Size::Size(int w, int h) :
    width(w), height(h)
{}

Size::Size(const Size &sz)
{
    width = sz.width;
    height = sz.height;
}

Size& Size::operator=(const Size &sz)
{
    if (this == &sz)
        return *this;
    width = sz.width;
    height = sz.height;
    return *this;
}
