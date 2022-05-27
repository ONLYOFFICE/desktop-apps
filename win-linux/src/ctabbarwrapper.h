#ifndef CTABBARWRAPPER_H
#define CTABBARWRAPPER_H

#include <QFrame>
#include <QGridLayout>
#include <QToolButton>
#include "ctabbar.h"


class CTabBarWrapper: public QFrame
{
    Q_OBJECT

public:

    explicit CTabBarWrapper(QWidget *parent = nullptr);

    ~CTabBarWrapper();

    CTabBar *tabBar();

    void applyTheme(const QString &style);

private:

    QFrame  *m_pScrollerFrame;

    CTabBar *m_pBar;

    bool eventFilter(QObject *, QEvent *);

};

#endif // CTABBARWRAPPER_H
