#include "palette.h"


Palette::Palette()
{
    palette[Background][Disabled] = 0x21252b;
    palette[Background][Normal]   = 0x21252b;
    palette[Background][Hover]    = 0x34383f;
    palette[Background][Pressed]  = 0x30343c;
    palette[Border][Disabled]     = 0x21252b;
    palette[Border][Normal]       = 0x21252b;
    palette[Border][Hover]        = 0x34383f;
    palette[Border][Pressed]      = 0x30343c;
    palette[Base][Disabled]       = 0x0000ff;
    palette[Base][Normal]         = 0x0000ff;
    palette[Base][Hover]          = 0x0000ff;
    palette[Base][Pressed]        = 0x0000ff;
    palette[AlternateBase][Disabled] = 0xff0000;
    palette[AlternateBase][Normal]   = 0xff0000;
    palette[AlternateBase][Hover]    = 0xff0000;
    palette[AlternateBase][Pressed]  = 0xff0000;
    palette[Text][Disabled]       = 0xeeeeee;
    palette[Text][Normal]         = 0xeeeeee;
    palette[Text][Hover]          = 0xaaaaaa;
    palette[Text][Pressed]        = 0xaaaaaa;
    palette[Primitive][Disabled]  = 0xeeeeee;
    palette[Primitive][Normal]    = 0xeeeeee;
    palette[Primitive][Hover]     = 0xeeeeee;
    palette[Primitive][Pressed]   = 0xeeeeee;

    setCurrentState(Normal);
}

Palette::~Palette()
{

}

COLORREF Palette::color(Role role)
{
    return RGB((currentColors[role] & 0xff0000) >> 16, (currentColors[role] & 0xff00) >> 8, currentColors[role] & 0xff);
}

void Palette::setColor(Role role, State state, DWORD color)
{
    palette[role][state] = color;
    currentColors[role] = palette[role][currentState];
}

void Palette::setCurrentState(State state)
{
    currentColors[Background] = palette[Background][state];
    currentColors[Border]     = palette[Border][state];
    currentColors[Base]       = palette[Base][state];
    currentColors[AlternateBase] = palette[AlternateBase][state];
    currentColors[Text]       = palette[Text][state];
    currentColors[Primitive]  = palette[Primitive][state];
    currentState = state;
}
