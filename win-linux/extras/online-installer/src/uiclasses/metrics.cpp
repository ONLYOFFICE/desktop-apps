#include "metrics.h"


Metrics::Metrics()
{
    metrics[BorderWidth]    = 0;
    metrics[BorderRadius]   = 0;
    metrics[IconWidth]      = 16;
    metrics[IconHeight]     = 16;
    metrics[IconMarginLeft]   = 0;
    metrics[IconMarginRight]  = 0;
    metrics[IconMarginTop]    = 0;
    metrics[IconMarginBottom] = 0;
    metrics[IconAlignment]    = Alignment::AlignCenter;
    metrics[FontWidth]      = 0;
    metrics[FontHeight]     = 18;
    metrics[PrimitiveWidth] = 1;
    metrics[PrimitiveRadius] = 0;
    metrics[AlternatePrimitiveWidth] = 1;
    metrics[ShadowWidth]    = 10;
    metrics[ShadowRadius]   = 10;
    metrics[TextMarginLeft] = 0;
    metrics[TextMarginTop]  = 0;
    metrics[TextMarginRight]  = 0;
    metrics[TextMarginBottom] = 0;
    metrics[TextAlignment]    = Alignment::AlignCenter;
}

Metrics::~Metrics()
{

}

int Metrics::value(Role role)
{
    return metrics[role];
}

void Metrics::setMetrics(Role role, int value)
{
    metrics[role] = value;
}
