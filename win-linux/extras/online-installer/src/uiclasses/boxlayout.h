#ifndef BOXLAYOUT_H
#define BOXLAYOUT_H

#include "layout.h"
#include <unordered_map>


class BoxLayout : public Layout
{
public:
    enum Direction : unsigned char {
        Horizontal,
        Vertical
    };
    BoxLayout(Direction);
    ~BoxLayout();

    virtual void addWidget(Widget *wgt) override;
    virtual void setContentMargins(int, int, int, int);
    virtual void setSpacing(int);

protected:

private:
    virtual void onResize(int w, int h) override;
    std::unordered_map<Widget*, int> m_destroy_conn;
    std::vector<Widget*> m_widgets;
    Direction m_direction;
    int m_spacing;
    int m_total_fixed_size;
};

#endif // BOXLAYOUT_H
