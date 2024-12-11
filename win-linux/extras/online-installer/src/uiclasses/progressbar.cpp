#include "progressbar.h"
#include "drawningengine.h"

#define DEFAULT_PULSE_STEP 1


ProgressBar::ProgressBar(Widget *parent) :
    Widget(parent, ObjectType::WidgetType),
    m_progress(0),
    m_pulse_pos(-1),
    m_pulse_direction(1),
    m_pulse_step(DEFAULT_PULSE_STEP)
{

}

ProgressBar::~ProgressBar()
{

}

void ProgressBar::setProgress(int progress)
{
    m_progress = progress;
    update();
}

void ProgressBar::pulse(bool enable)
{
    m_pulse_pos = enable ? 0 : -1;
    m_pulse_direction = 1;
    if (enable) {
        timeBeginPeriod(1);
        SetTimer(m_hWnd, PROGRESS_PULSE_TIMER_ID, 17, NULL);
    } else {
        KillTimer(m_hWnd, PROGRESS_PULSE_TIMER_ID);
        timeEndPeriod(1);
    }
}

void ProgressBar::setPulseStep(int step)
{
    if (step < 1)
        step = 1;
    else
    if (step > 50)
        step = 50;
    m_pulse_step = step;
}

bool ProgressBar::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    // case WM_LBUTTONDOWN: {
    //     palette()->setCurrentState(Palette::Pressed);
    //     repaint();
    //     return false;
    // }

    // case WM_LBUTTONUP: {
    //     palette()->setCurrentState(Palette::Hover);
    //     repaint();
    //     break;
    // }
    // case WM_MOUSEENTER: {
    //     palette()->setCurrentState(Palette::Hover);
    //     repaint();
    //     break;
    // }

    // case WM_MOUSELEAVE:
    // case WM_NCMOUSELEAVE: {
    //     palette()->setCurrentState(Palette::Normal);
    //     repaint();
    //     break;
    // }

    case WM_PAINT: {
        RECT rc;
        GetClientRect(m_hWnd, &rc);

        engine()->Begin(this, m_hWnd, &rc);
        engine()->DrawProgressBar(m_progress, m_pulse_pos);
        engine()->End();

        *result = FALSE;
        return true;
    }

    case WM_TIMER: {
        if (wParam == PROGRESS_PULSE_TIMER_ID) {
            m_pulse_pos += m_pulse_direction * m_pulse_step;
            if (m_pulse_pos >= 100) {
                m_pulse_pos = 100;
                m_pulse_direction = -1;
            } else
            if (m_pulse_pos <= 0) {
                m_pulse_pos = 0;
                m_pulse_direction = 1;
            }
            update();
        }
        break;
    }

    default:
        break;
    }
    return Widget::event(msg, wParam, lParam, result);
}
