
#include "csinglewindowbase.h"
#include "utils.h"
#include "cwindowbase.h"
#include "ccefeventsgate.h"

#include <QVariant>
#include <QDebug>

CSingleWindowBase::CSingleWindowBase()
{

}

CSingleWindowBase::~CSingleWindowBase()
{
//    if ( m_pButtonClose ) {
//        m_pButtonClose->deleteLater();
//        m_pButtonClose = nullptr;
//    }
//    if ( m_pButtonMinimize ) {
//        m_pButtonMinimize->deleteLater();
//        m_pButtonMinimize = nullptr;
//    }
//    if ( m_pButtonMaximize ) {
//        m_pButtonMaximize->deleteLater();
//        m_pButtonMaximize = nullptr;
//    }
    qDebug() << "destroy base single window";
}

CSingleWindowBase::CSingleWindowBase(QRect& rect)
{
    m_dpiRatio = Utils::getScreenDpiRatio(rect.topLeft());
    if ( rect.isEmpty() )
        rect = QRect(100, 100, 1324 * m_dpiRatio, 800 * m_dpiRatio);

    QRect _screen_size = Utils::getScreenGeometry(rect.topLeft());
    if ( _screen_size.width() < rect.width() + 120 ||
            _screen_size.height() < rect.height() + 120 )
    {
        rect.setLeft(_screen_size.left()),
        rect.setTop(_screen_size.top());

        if ( _screen_size.width() < rect.width() ) rect.setWidth(_screen_size.width());
        if ( _screen_size.height() < rect.height() ) rect.setHeight(_screen_size.height());
    }
}

void CSingleWindowBase::setScreenScalingFactor(int f)
{
    if ( m_dpiRatio != f ) {
        QSize small_btn_size(TOOLBTN_WIDTH*f, TOOLBTN_HEIGHT*f);

        m_buttonMinimize->setFixedSize(small_btn_size);
        m_buttonMaximize->setFixedSize(small_btn_size);
        m_buttonClose->setFixedSize(small_btn_size);

        onScreenScalingFactor(f);

        m_dpiRatio = f;
    }
}

void CSingleWindowBase::setWindowTitle(const QString& title)
{
    if ( m_labelTitle ) {
        m_labelTitle->setText(title);
    }
}

//#include <QSvgRenderer>
//#include <QPainter>
QWidget * CSingleWindowBase::createMainPanel(QWidget * parent, const QString& title, bool custom, QWidget *)
{
    if ( custom ) {
        m_labelTitle = new QLabel(title);
        m_labelTitle->setObjectName("labelTitle");
        m_labelTitle->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        QSize small_btn_size(TOOLBTN_WIDTH*m_dpiRatio, TOOLBTN_HEIGHT*m_dpiRatio);

        auto _creatToolButton = [&small_btn_size](const QString& name, QWidget * parent) {
            QPushButton * btn = new QPushButton(parent);
            btn->setObjectName(name);
            btn->setProperty("class", "normal");
            btn->setProperty("act", "tool");
            btn->setFixedSize(small_btn_size);

            return btn;
        };

        // Minimize
        m_buttonMinimize = _creatToolButton("toolButtonMinimize", parent);
        QObject::connect(m_buttonMinimize, &QPushButton::clicked, [=]{onMinimizeEvent();});

        // Maximize
        m_buttonMaximize = _creatToolButton("toolButtonMaximize", parent);
        QObject::connect(m_buttonMaximize, &QPushButton::clicked, [=]{onMaximizeEvent();});

        // Close
        m_buttonClose = _creatToolButton("toolButtonClose", parent);
        QObject::connect(m_buttonClose, &QPushButton::clicked, [=]{onCloseEvent();});


//        m_pButtonMaximize = new QPushButton(parent);
//        m_pButtonMaximize->setFixedSize(small_btn_size);
//        m_pButtonMaximize->setIconSize(QSize(16,16));

//        QSvgRenderer _svg;
//        _svg.load(QString(":/tools.svg"));
//qDebug() << "def size: " << _svg.defaultSize();
//        QPixmap image(_svg.defaultSize());
//        image.fill(Qt::transparent);
//        QPainter painter( &image );
//        _svg.render(&painter, "svg-g-max");

//        m_pButtonMaximize->setIcon(QIcon(image));
    }

    return nullptr;
}

void CSingleWindowBase::onCloseEvent()
{
}

void CSingleWindowBase::onMinimizeEvent()
{

}

void CSingleWindowBase::onMaximizeEvent()
{

}

QPushButton * CSingleWindowBase::createToolButton(QWidget * parent)
{
    QPushButton * btn = new QPushButton(parent);
    btn->setProperty("class", "normal");
    btn->setProperty("act", "tool");
    btn->setFixedSize(QSize(TOOLBTN_WIDTH*m_dpiRatio, TOOLBTN_HEIGHT*m_dpiRatio));

    return btn;
}
