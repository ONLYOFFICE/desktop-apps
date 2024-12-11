#include "drawingsurface.h"
#include "drawningengine.h"
#include "metrics.h"
#include "palette.h"


DrawningSurface::DrawningSurface() :
    m_metrics(new Metrics),
    m_palette(new Palette)
{
    m_engine = DrawingEngine::instance();
}

DrawningSurface::~DrawningSurface()
{
    delete m_palette, m_palette = nullptr;
    delete m_metrics, m_metrics = nullptr;
}

Metrics *DrawningSurface::metrics()
{
    return m_metrics;
}

Palette *DrawningSurface::palette()
{
    return m_palette;
}

DrawingEngine *DrawningSurface::engine()
{
    return m_engine;
}
