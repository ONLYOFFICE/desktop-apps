#ifndef METRICS_H
#define METRICS_H


class Metrics
{
public:
    Metrics();
    ~Metrics();

    enum Alignment : unsigned char {
        AlignHLeft   = 1,
        AlignHCenter = 2,
        AlignHRight  = 4,
        AlignVTop    = 8,
        AlignVCenter = 16,
        AlignVBottom = 32,
        AlignCenter  = AlignHCenter | AlignVCenter
    };

    enum Role : unsigned char {
        BorderWidth,
        BorderRadius,
        IconWidth,
        IconHeight,
        IconMarginLeft,
        IconMarginTop,
        IconMarginRight,
        IconMarginBottom,
        IconAlignment,
        FontWidth,
        FontHeight,
        PrimitiveWidth,
        AlternatePrimitiveWidth,
        PrimitiveRadius,
        ShadowWidth,
        ShadowRadius,
        TextMarginLeft,
        TextMarginTop,
        TextMarginRight,
        TextMarginBottom,
        TextAlignment,
        METRICS_COUNT
    };

    void setMetrics(Role, int);
    int  value(Role);

protected:

private:
    int metrics[METRICS_COUNT];
};

#endif // METRICS_H
