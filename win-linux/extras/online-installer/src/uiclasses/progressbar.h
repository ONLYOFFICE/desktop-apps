#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include "widget.h"
//#include <gdiplus.h>


class ProgressBar : public Widget
{
public:
    ProgressBar(Widget *parent = nullptr);
    virtual ~ProgressBar();

    void setProgress(int progress);
    void pulse(bool);
    void setPulseStep(int);

    /* callback */

protected:
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;

private:
    int  m_progress,
         m_pulse_pos,
         m_pulse_direction,
         m_pulse_step;
};

#endif // PROGRESSBAR_H
