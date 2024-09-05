#ifndef PALETTE_H
#define PALETTE_H

#include <Windows.h>


class Palette
{
public:
    Palette();
    ~Palette();

    enum Role : BYTE {
        Background = 0,
        Border,
        Base,
        AlternateBase,
        Text,
        Primitive,
        PALETTE_ROLE_COUNT
    };

    enum State : BYTE {
        Disabled = 0,
        Normal,
        Hover,
        Pressed,
        PALETTE_STATE_COUNT
    };

    COLORREF color(Role);
    void setColor(Role, State, DWORD);
    void setCurrentState(State);

private:
    DWORD palette[PALETTE_ROLE_COUNT][PALETTE_STATE_COUNT];
    DWORD currentColors[PALETTE_ROLE_COUNT];
    State currentState;
};

#endif // PALETTE_H
