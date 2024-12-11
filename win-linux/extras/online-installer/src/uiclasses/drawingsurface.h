#ifndef DRAWNINGSURFACE_H
#define DRAWNINGSURFACE_H


class Metrics;
class Palette;
class DrawingEngine;
class DrawningSurface
{
public:
    DrawningSurface();
    virtual ~DrawningSurface();

    Metrics *metrics();
    Palette *palette();

protected:
    DrawingEngine *engine();

private:
    Metrics *m_metrics;
    Palette *m_palette;
    DrawingEngine *m_engine;
};

#endif // DRAWNINGSURFACE_H
