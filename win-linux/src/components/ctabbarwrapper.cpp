#include "components/ctabbarwrapper.h"

CTabBarWrapper::CTabBarWrapper(QWidget *parent):
    QFrame(parent)
{
    setObjectName(QString::fromUtf8("tabWrapper"));
    setFrameShape(QFrame::NoFrame);
    setFrameShadow(QFrame::Plain);

    QGridLayout *_pTabFrameLayout = new QGridLayout(this);
    _pTabFrameLayout->setSpacing(0);
    _pTabFrameLayout->setContentsMargins(0,0,0,0);
    this->setLayout(_pTabFrameLayout);

    // Bypassing the bug with tab scroller
    m_pScrollerFrame = new QFrame(this);
    m_pScrollerFrame->setObjectName("scrollerFrame");
    QHBoxLayout *_pScrollerLayout = new QHBoxLayout(m_pScrollerFrame);
    _pScrollerLayout->setSpacing(0);
    _pScrollerLayout->setContentsMargins(0,0,0,0);
    m_pScrollerFrame->setLayout(_pScrollerLayout);

    QToolButton* _pLeftButton = new QToolButton(m_pScrollerFrame);
    QToolButton* _pRightButton = new QToolButton(m_pScrollerFrame);
    _pLeftButton->setObjectName("leftButton");
    _pRightButton->setObjectName("rightButton");

    _pScrollerLayout->addWidget(_pLeftButton);
    _pScrollerLayout->addWidget(_pRightButton);
    _pLeftButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    _pRightButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    _pLeftButton->installEventFilter(this);
    _pRightButton->installEventFilter(this);
    _pLeftButton->setMouseTracking(true);
    _pRightButton->setMouseTracking(true);
    _pLeftButton->setAttribute(Qt::WA_Hover, true);
    _pRightButton->setAttribute(Qt::WA_Hover, true);   // End bypassing the bug

    m_pBar = new CTabBar(this);
    m_pBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QWidget *_pPaddingWidget = new QWidget(this);
    _pPaddingWidget->setObjectName("paddingWidget");
    _pPaddingWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QHBoxLayout *_pTabLayout = new QHBoxLayout(this);
#if (QT_VERSION < QT_VERSION_CHECK(5, 12, 2))
    _pTabLayout->setSpacing(0);
#else
    _pTabLayout->setSpacing(32);
#endif
    _pTabLayout->setContentsMargins(0,0,0,0);
    _pTabLayout->addWidget(m_pBar);
    _pTabLayout->addWidget(_pPaddingWidget);

    _pTabFrameLayout->addLayout(_pTabLayout, 0, 0, 1, 1);
    _pTabFrameLayout->addWidget(m_pScrollerFrame, 0, 0, 1, 1, Qt::AlignRight);

    m_pBar->initCustomScroll(m_pScrollerFrame, _pLeftButton, _pRightButton);

}

CTabBarWrapper::~CTabBarWrapper()
{

}

CTabBar *CTabBarWrapper::tabBar()
{
    return m_pBar;
}

void CTabBarWrapper::applyTheme(const QString &style)
{
    this->setStyleSheet(style);
    m_pScrollerFrame->setStyleSheet(style);
    m_pBar->setStyleSheet(style);
}

bool CTabBarWrapper::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type()) {
    case QEvent::HoverEnter: {
        if (object->objectName() == QString("leftButton") ||
                object->objectName() == QString("rightButton")) {
            m_pScrollerFrame->setCursor(QCursor(Qt::ArrowCursor));
        }
        break;
    }
    default:
        break;
    }
    return QWidget::eventFilter(object, event);
}
