#ifndef CSCALINGWRAPPER_H
#define CSCALINGWRAPPER_H

#include <QWidget>

class CScalingWrapper
{
public:
    explicit CScalingWrapper(QWidget *);
    explicit CScalingWrapper(int);

    virtual void updateScaling(int f);
    int scaling() const;

    static void updateChildScaling(const QObject * parent, int factor);
    static int parentScalingFactor(const QObject * parent);

private:
    int m_scaleFactor = 1;
};

#endif // CSCALINGWRAPPER_H
