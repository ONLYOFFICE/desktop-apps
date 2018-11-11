#include "cmainwindowbase.h"
#include "cwindowbase.h"
#include "ctabbar.h"

CMainWindowBase::CMainWindowBase()
{

}

int CMainWindowBase::attachEditor(QWidget * panel, int index)
{
    CMainPanel * _pMainPanel = mainPanel();
    int _index = _pMainPanel->tabWidget()->insertPanel(panel, index);
    if ( !(_index < 0) ) {
        _pMainPanel->toggleButtonMain(false);

        QTabBar * tabs = _pMainPanel->tabWidget()->tabBar();
        tabs->setCurrentIndex(_index);

//        if ( false ) {
//            QApplication::sendEvent( tabs,
//                &QMouseEvent(QEvent::MouseButtonPress,
//                    tabs->tabRect(_index).topLeft() + (QPoint(10, 65)*m_dpiRatio),
//                    Qt::LeftButton, Qt::LeftButton, Qt::NoModifier) );
//        }
    }

    return _index;
}

int CMainWindowBase::attachEditor(QWidget * panel, const QPoint& pt)
{
    CMainPanel * _pMainPanel = mainPanel();
    QPoint _pt_local = _pMainPanel->tabWidget()->tabBar()->mapFromGlobal(pt);
    int _index = _pMainPanel->tabWidget()->tabBar()->tabAt(_pt_local);

    if ( !(_index < 0) ) {
        QRect _rc_tab = _pMainPanel->tabWidget()->tabBar()->tabRect(_index);
        if ( _pt_local.x() > _rc_tab.left() + (_rc_tab.width() / 2) ) ++_index;
    }

    return attachEditor(panel, _index);
}

bool CMainWindowBase::pointInTabs(const QPoint& pt)
{
    QRect _rc_title(mainPanel()->geometry());
    _rc_title.setHeight(mainPanel()->tabWidget()->tabBar()->height());

    return _rc_title.contains(mainPanel()->mapFromGlobal(pt));
}

bool CMainWindowBase::movedByTab()
{
    return mainPanel()->tabWidget()->count() == 1 &&
            ((CTabBar *)mainPanel()->tabWidget()->tabBar())->draggedTabIndex() == 0;
}

QWidget * CMainWindowBase::editorPanel(int index)
{
    return mainPanel()->tabWidget()->releaseEditor(index);
}

bool CMainWindowBase::holdView(int id) const
{
    return mainPanel()->holdUid(id);
}

int CMainWindowBase::editorsCount() const
{
    return mainPanel()->tabWidget()->count(cvwtEditor);
}
