#ifndef COMMONDEFINES_H
#define COMMONDEFINES_H

#include <functional>

#define SNAP_LAYOUTS_TIMER_ID 0x1f000000
#define PROGRESS_PULSE_TIMER_ID 0x2f000000

#define WM_MOUSEENTER (WM_APP + 1)
#define WM_INVOKEMETHOD (WM_APP + 2)

typedef std::function<void(void)> FnVoidVoid;
typedef std::function<void(int)> FnVoidInt;
typedef std::function<void(int, int)> FnVoidIntInt;
typedef std::function<void(bool*)> FnVoidBoolPtr;

#endif // COMMONDEFINES_H
