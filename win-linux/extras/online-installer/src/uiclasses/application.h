#ifndef APPLICATION_H
#define APPLICATION_H

#include "object.h"
#include "common.h"
#include <Windows.h>


class Widget;
class Application : public Object
{
public:
    Application(HINSTANCE hInstance, PWSTR cmdline, int cmdshow);
    Application(const Application&) = delete;
    ~Application();

    Application& operator=(const Application&) = delete;
    static Application *instance();
    HINSTANCE moduleHandle();
    void setLayoutDirection(LayoutDirection);

    int exec();
    void exit(int);

private:
    Application();
    friend class Widget;
    void registerWidget(Widget*, ObjectType, const Rect &rc);
    class ApplicationPrivate;
    ApplicationPrivate *d_ptr;
    static Application *inst;
};

#endif // APPLICATION_H
