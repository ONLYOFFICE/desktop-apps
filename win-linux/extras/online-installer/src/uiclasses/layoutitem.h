#ifndef LAYOUTITEM_H
#define LAYOUTITEM_H


class Widget;
class Layout;

class LayoutItem
{
public:
    LayoutItem();
    ~LayoutItem();

    virtual Widget *widget();
    virtual Layout *layout();

protected:
};

#endif // LAYOUTITEM_H
