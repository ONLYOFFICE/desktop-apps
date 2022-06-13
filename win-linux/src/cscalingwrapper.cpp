#include "cscalingwrapper.h"
#include "utils.h"

CScalingWrapper::CScalingWrapper(QWidget * parent)
    : CScalingWrapper( CScalingWrapper::parentScalingFactor(parent) )
{
}

CScalingWrapper::CScalingWrapper(double f)
{
    if ( f > 1 ) m_scaleFactor = f;
}

void CScalingWrapper::updateScalingFactor(double f)
{
    m_scaleFactor = f;
}

double CScalingWrapper::scaling() const
{
    return m_scaleFactor;
}

void CScalingWrapper::updateChildScaling(const QObject * parent, double factor)
{
    QObjectList _l = parent->children();
    if ( _l.size() ) {
        foreach ( QObject * o, _l ) {
            CScalingWrapper * _s = dynamic_cast<CScalingWrapper *>(o);
            if ( _s ) _s->updateScalingFactor(factor);

            if ( o->children().size() )
                CScalingWrapper::updateChildScaling(o, factor);
        }
    }
}

double CScalingWrapper::parentScalingFactor(const QObject * parent)
{
    if ( parent ) {
        const CScalingWrapper * _s = dynamic_cast<const CScalingWrapper *>(parent);
        if ( _s ) return _s->scaling();
        else return CScalingWrapper::parentScalingFactor(parent->parent());
    }

    return -1;
}

