#include "boxlayout.h"
#include "widget.h"
#include <cmath>


BoxLayout::BoxLayout(Direction direction) :
    m_direction(direction)
{
    m_margins = Margins(6,6,6,6);
    m_spacing = 6;
}

BoxLayout::~BoxLayout()
{
    for (auto it = m_destroy_conn.begin(); it != m_destroy_conn.end(); it++)
        it->first->disconnect(it->second);
}

void BoxLayout::addWidget(Widget *wgt)
{
    m_widgets.push_back(wgt);
    // int destroy_conn = wgt->onAboutToDestroy([=]() {
    //     auto it = std::find(m_widgets.begin(), m_widgets.end(), wgt);
    //     if (it != m_widgets.end())
    //         m_widgets.erase(it);

    //     auto it_conn = m_destroy_conn.find(wgt);
    //     if (it_conn != m_destroy_conn.end())
    //         m_destroy_conn.erase(it_conn);
    // });
    // m_destroy_conn[wgt] = destroy_conn;
}

void BoxLayout::setContentMargins(int left, int top, int right, int bottom)
{
    m_margins = Margins(left, top, right, bottom);
}

void BoxLayout::setSpacing(int spacing)
{
    m_spacing = spacing;
}

void BoxLayout::onResize(int w, int h)
{
    int amount = m_widgets.size();
    if (amount > 0) {
        int x = m_margins.left;
        int y = m_margins.top;
        int sum_width = w - (m_margins.right + m_margins.left);
        int sum_height = h - (m_margins.bottom + m_margins.top);
        int num_fixed = 0;
        int sum_fixed_width_or_height = 0;
        int last_expanding = -1;
        if (m_direction == Horizontal) {
            sum_width -= (amount - 1) * m_spacing;
            for (int i = 0; i < amount; i++) {
                Widget::SizeBehavior sb = (Widget::SizeBehavior)m_widgets[i]->property(Widget::HSizeBehavior);
                if (sb == Widget::SizeBehavior::Fixed) {
                    int _w = 0, _h = 0;
                    m_widgets[i]->size(&_w, &_h);
                    sum_fixed_width_or_height += _w;
                    ++num_fixed;
                } else
                if (sb == Widget::SizeBehavior::Expanding) {
                    last_expanding = i;
                }
            }

            if (num_fixed != 0 && last_expanding != -1) {
                int sep_width = (int)std::round((float)(sum_width - sum_fixed_width_or_height)/(amount - num_fixed));
                for (int i = 0; i < amount; i++) {
                    if (i == last_expanding)
                        sep_width = (sum_width - sum_fixed_width_or_height) - (amount - num_fixed - 1)*sep_width;
                    int _w = 0, _h = 0;
                    m_widgets[i]->size(&_w, &_h);
                    Widget::SizeBehavior hsb = (Widget::SizeBehavior)m_widgets[i]->property(Widget::HSizeBehavior);
                    Widget::SizeBehavior vsb = (Widget::SizeBehavior)m_widgets[i]->property(Widget::VSizeBehavior);
                    if (hsb == Widget::SizeBehavior::Fixed) {
                        if (vsb == Widget::SizeBehavior::Fixed) {
                            m_widgets[i]->move(x, y);
                        } else
                        if (vsb == Widget::SizeBehavior::Expanding) {
                            m_widgets[i]->setGeometry(x, y, _w, sum_height);
                        }
                        x += _w + m_spacing;

                    } else
                    if (hsb == Widget::SizeBehavior::Expanding) {
                        if (vsb == Widget::SizeBehavior::Fixed) {
                            m_widgets[i]->setGeometry(x, y, sep_width, _h);
                        } else
                        if (vsb == Widget::SizeBehavior::Expanding) {
                            m_widgets[i]->setGeometry(x, y, sep_width, sum_height);
                        }
                        x += sep_width + m_spacing;
                    }
                }

            } else {
                int sep_width = (int)std::round((float)sum_width/amount);
                for (int i = 0; i < amount; i++) {
                    if (i == amount - 1)
                        sep_width = sum_width - i*sep_width;
                    Widget::SizeBehavior hsb = (Widget::SizeBehavior)m_widgets[i]->property(Widget::HSizeBehavior);
                    Widget::SizeBehavior vsb = (Widget::SizeBehavior)m_widgets[i]->property(Widget::VSizeBehavior);
                    if (hsb == Widget::SizeBehavior::Fixed) {
                        if (vsb == Widget::SizeBehavior::Fixed) {
                            m_widgets[i]->move(x, y);
                        } else
                        if (vsb == Widget::SizeBehavior::Expanding) {
                            int _w = 0, _h = 0;
                            m_widgets[i]->size(&_w, &_h);
                            m_widgets[i]->setGeometry(x, y, _w, sum_height);
                        }

                    } else
                    if (hsb == Widget::SizeBehavior::Expanding) {
                        if (vsb == Widget::SizeBehavior::Fixed) {
                            int _w = 0, _h = 0;
                            m_widgets[i]->size(&_w, &_h);
                            m_widgets[i]->setGeometry(x, y, sep_width, _h);
                        } else
                        if (vsb == Widget::SizeBehavior::Expanding) {
                            m_widgets[i]->setGeometry(x, y, sep_width, sum_height);
                        }
                    }
                    x += sep_width + m_spacing;
                }
            }

        } else {
            sum_height -= (amount - 1) * m_spacing;
            for (int i = 0; i < amount; i++) {
                Widget::SizeBehavior sb = (Widget::SizeBehavior)m_widgets[i]->property(Widget::VSizeBehavior);
                if (sb == Widget::SizeBehavior::Fixed) {
                    int _w = 0, _h = 0;
                    m_widgets[i]->size(&_w, &_h);
                    sum_fixed_width_or_height += _h;
                    ++num_fixed;
                } else
                if (sb == Widget::SizeBehavior::Expanding) {
                    last_expanding = i;
                }
            }

            if (num_fixed != 0 && last_expanding != -1) {
                int sep_height = (int)std::round((float)(sum_height - sum_fixed_width_or_height)/(amount - num_fixed));
                for (int i = 0; i < amount; i++) {
                    if (i == last_expanding)
                        sep_height = (sum_height - sum_fixed_width_or_height) - (amount - num_fixed - 1)*sep_height;
                    int _w = 0, _h = 0;
                    m_widgets[i]->size(&_w, &_h);
                    Widget::SizeBehavior hsb = (Widget::SizeBehavior)m_widgets[i]->property(Widget::HSizeBehavior);
                    Widget::SizeBehavior vsb = (Widget::SizeBehavior)m_widgets[i]->property(Widget::VSizeBehavior);
                    if (vsb == Widget::SizeBehavior::Fixed) {
                        if (hsb == Widget::SizeBehavior::Fixed) {
                            m_widgets[i]->move(x, y);
                        } else
                        if (hsb == Widget::SizeBehavior::Expanding) {
                            m_widgets[i]->setGeometry(x, y, sum_width, _h);
                        }
                        y += _h + m_spacing;

                    } else
                    if (vsb == Widget::SizeBehavior::Expanding) {
                        if (hsb == Widget::SizeBehavior::Fixed) {
                            m_widgets[i]->setGeometry(x, y, _w, sep_height);
                        } else
                        if (hsb == Widget::SizeBehavior::Expanding) {
                            m_widgets[i]->setGeometry(x, y, sum_width, sep_height);
                        }
                        y += sep_height + m_spacing;
                    }
                }

            } else {
                int sep_height = (int)std::round((float)sum_height/amount);
                for (int i = 0; i < amount; i++) {
                    if (i == amount - 1)
                        sep_height = sum_height - i*sep_height;
                    Widget::SizeBehavior hsb = (Widget::SizeBehavior)m_widgets[i]->property(Widget::HSizeBehavior);
                    Widget::SizeBehavior vsb = (Widget::SizeBehavior)m_widgets[i]->property(Widget::VSizeBehavior);
                    if (vsb == Widget::SizeBehavior::Fixed) {
                        if (hsb == Widget::SizeBehavior::Fixed) {
                            m_widgets[i]->move(x, y);
                        } else
                        if (hsb == Widget::SizeBehavior::Expanding) {
                            int _w = 0, _h = 0;
                            m_widgets[i]->size(&_w, &_h);
                            m_widgets[i]->setGeometry(x, y, sum_width, _h);
                        }

                    } else
                    if (vsb == Widget::SizeBehavior::Expanding) {
                        if (hsb == Widget::SizeBehavior::Fixed) {
                            int _w = 0, _h = 0;
                            m_widgets[i]->size(&_w, &_h);
                            m_widgets[i]->setGeometry(x, y, _w, sep_height);
                        } else
                        if (hsb == Widget::SizeBehavior::Expanding) {
                            m_widgets[i]->setGeometry(x, y, sum_width, sep_height);
                        }
                    }
                    y += sep_height + m_spacing;
                }
            }
        }
    }
}

