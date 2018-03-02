#include "cscalingwrapper.h"
#include "utils.h"

CScalingWrapper::CScalingWrapper(QWidget * parent)
    : CScalingWrapper( CScalingWrapper::parentScalingFactor(parent) )
{
}

CScalingWrapper::CScalingWrapper(int f)
{
    if ( f > 1 ) m_scaleFactor = f;
}

void CScalingWrapper::updateScaling(int f)
{
    m_scaleFactor = f;
}

int CScalingWrapper::scaling() const
{
    return m_scaleFactor;
}

void CScalingWrapper::updateChildScaling(const QObject * parent, int factor)
{
    QObjectList _l = parent->children();
    if ( _l.size() ) {
        foreach ( QObject * o, _l ) {
            CScalingWrapper * _s = dynamic_cast<CScalingWrapper *>(o);
            if ( _s ) _s->updateScaling(factor);

            if ( o->children().size() )
                CScalingWrapper::updateChildScaling(o, factor);
        }
    }
}

int CScalingWrapper::parentScalingFactor(const QObject * parent)
{
    if ( parent ) {
        const CScalingWrapper * _s = dynamic_cast<const CScalingWrapper *>(parent);
        if ( _s ) return _s->scaling();
        else return CScalingWrapper::parentScalingFactor(parent->parent());
    }

    return -1;
}

