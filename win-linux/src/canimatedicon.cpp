#include "canimatedicon.h"

#include <QPainter>
#include <QDebug>

#define RELEASE_OBJECT(obj) if (obj) delete obj, obj = nullptr;

CAnimatedIcon::CAnimatedIcon(QWidget * parent)
    : QLabel(parent)
{

}

void CAnimatedIcon::setPixmap(const QPixmap & pixmap, bool forcestop)
{
    if ( !forcestop && isStarted() ) {
        RELEASE_OBJECT(m_static);
        m_static = new QPixmap(pixmap);
    } else {
        RELEASE_OBJECT(m_svg);
        RELEASE_OBJECT(m_image);

        QLabel::setPixmap(pixmap);
    }
}

void CAnimatedIcon::startSvg(const QString& source, const QString& id)
{
    if ( !m_svg ) m_svg = new QSvgRenderer(this);
    else disconnect(m_svg);

    if ( m_svg->load(source) ) {
        setFixedSize( m_svg->defaultSize() );

        if ( !m_static && pixmap() ) {
            m_static = new QPixmap(*pixmap());
        }

        if ( m_svg->animated() ) {
            m_svgElemId = id;

            if ( m_image ) delete m_image;
            m_image = new QPixmap(m_svg->defaultSize());

            connect(m_svg, &QSvgRenderer::repaintNeeded, this, &CAnimatedIcon::onSvgRepaint);
        } else {
            QPixmap image( m_svg->defaultSize() );
            QPainter painter( &image );

            if ( id.isEmpty() )
                m_svg->render( &painter );
            else m_svg->render( &painter, id );

            RELEASE_OBJECT(m_svg);
            QLabel::setPixmap( image );
        }
    }
}

void CAnimatedIcon::stop()
{
    RELEASE_OBJECT(m_svg);
    if ( m_static ) {
        QLabel::setPixmap( *m_static );
        setFixedSize( m_static->size() );
    }
}

void CAnimatedIcon::onSvgRepaint()
{
    m_image->fill(Qt::transparent);
    QPainter painter( m_image );

    if ( m_svgElemId.isEmpty() )
        m_svg->render( &painter );
    else m_svg->render( &painter, m_svgElemId );

    QLabel::setPixmap( *m_image );
}

void CAnimatedIcon::setSvgElement(const QString& id)
{
    m_svgElemId = id;
}

bool CAnimatedIcon::isStarted()
{
    if ( m_svg && m_svg->animated() )
        return true;

    return false;
}
