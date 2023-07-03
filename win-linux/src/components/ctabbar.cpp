/*
 * (c) Copyright Ascensio System SIA 2010-2019
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
*/

#include "components/ctabbar.h"
#include "components/canimatedicon.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QResizeEvent>
#include <QStylePainter>
#include <QStyle>
#include <QTimer>

#define ANIMATION_DEFAULT_MS  0
#define ANIMATION_SCROLL_MS   60
#define ANIMATION_MOVE_TAB_MS 350
#define TABSPACING 0
#define DEFAULT_ICON_SIZE QSize(16,16)
#define _tabRect(i) tabList[i]->geometry()
#define tabIndex(i) tabList[i]->index
#define signum(a) (a ? 1 : -1);


class Tab : public QFrame
{
    Q_OBJECT
public:
    Tab(QWidget *parent = nullptr);
    ~Tab();

    void setText(const QString &text, Qt::TextElideMode mode = Qt::ElideRight);
    void elideText(Qt::TextElideMode mode = Qt::ElideRight);
    void setIcon(const QIcon &icon);
    void polish();

    QIcon  *tab_icon = nullptr;
    CAnimatedIcon *icon_label = nullptr;
    QLabel *text_label = nullptr;
    QToolButton *close_btn = nullptr;
    QString text;
    QString tabcolor;
    int tab_width = -1;
    int index = -1;

signals:
    void onTabWidthChanged(int width);

protected:
    virtual void resizeEvent(QResizeEvent*) final;
    virtual void paintEvent(QPaintEvent*) override;
    virtual bool eventFilter(QObject*, QEvent*) override;

private:
    CTabBar *tabBar = nullptr;
};

Tab::Tab(QWidget *parent) :
    QFrame(parent),
    tabcolor("none")
{
    setAttribute(Qt::WA_Hover);
    installEventFilter(this);
    setMinimumWidth(20);
    tabBar = dynamic_cast<CTabBar*>(parent->parent());

    QHBoxLayout *lut = new QHBoxLayout(this);
    lut->setContentsMargins(6, 6, 6, 6);
    lut->setSpacing(6);
    setLayout(lut);

    icon_label = new CAnimatedIcon(this);
    icon_label->setObjectName("tabIcon");
    icon_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    icon_label->setMaximumSize(DEFAULT_ICON_SIZE);
    lut->addWidget(icon_label);

    text_label = new QLabel(this);
    text_label->setObjectName("tabText");
    text_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    lut->addWidget(text_label);

    close_btn = new QToolButton(this);
    close_btn->setObjectName("tabButton");
    close_btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    close_btn->setMaximumSize(DEFAULT_ICON_SIZE);
    lut->addWidget(close_btn);
    show();
}

Tab::~Tab()
{
    if (tab_icon)
        delete tab_icon, tab_icon = nullptr;
}

void Tab::setText(const QString &text, Qt::TextElideMode mode)
{
    this->text = text;
    elideText(mode);
}

void Tab::elideText(Qt::TextElideMode mode)
{
    const QMargins mrg = text_label->contentsMargins();
    const int padding = mrg.left() + mrg.right();
    const int width = text_label->maximumWidth() != QWIDGETSIZE_MAX ? text_label->maximumWidth() : text_label->width();
    QFontMetrics mtr(text_label->font());
    text_label->setText(mtr.elidedText(text, mode, width - padding - 1));
}

void Tab::setIcon(const QIcon &icon)
{
    if (tab_icon)
        delete tab_icon, tab_icon = nullptr;
    tab_icon = new QIcon(icon);
    icon_label->setPixmap(tab_icon->pixmap(icon_label->size()));
}

void Tab::polish()
{
    style()->polish(this);
    icon_label->style()->polish(icon_label);
    text_label->style()->polish(text_label);
    close_btn->style()->polish(close_btn);
}

void Tab::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    int new_width = event->size().width();
    if (tab_width < 0) {
        tab_width = new_width;
    } else {
        if (tab_width != new_width) {
            tab_width = new_width;
            QTimer::singleShot(0, this, [=]() {
                emit onTabWidthChanged(tab_width);
            });
        }
    }
}

void Tab::paintEvent(QPaintEvent *ev)
{
    QFrame::paintEvent(ev);
    if (!tabcolor.isEmpty() && tabcolor != "none" && property("selected").toBool()) {
        if (tabBar && tabBar->property("active").toBool() && !tabBar->ignoreActiveTabColor()) {
            QStylePainter p(this);
            p.fillRect(rect(), QBrush(QColor(tabcolor)));
        }
    }
}

bool Tab::eventFilter(QObject *obj, QEvent *ev)
{
    switch (ev->type()) {
    case QEvent::HoverEnter:
        setProperty("hovered", true);
        polish();
        break;
    case QEvent::HoverLeave:
        setProperty("hovered", false);
        polish();
        break;
    default:
        break;
    }
    return QFrame::eventFilter(obj, ev);
}


class CTabBar::CTabBarPrivate
{
public:
    CTabBarPrivate(CTabBar* owner);
    ~CTabBarPrivate();

    enum Direction {
        Left, Right
    };

    int getIntersectedOffset(int index);
    int getIntersectedIndex(int direction, int &offsetX);
    int getLayoutsIntersectedIndex(Tab *tab, int &offsetX);
    int cellWidth();
    int nextTabPosByPrev(int index);
//    int prevTabPosByNext(int index);
    void slide(int from, int to, int offset, int animation_ms);
    void scrollToDirection(int direction);
    void scrollTo(int index);
    void onCurrentChanged(int index);
    void onTabWidthChanged(int width);
    void changeScrollerState();
    void reorderIndexes();
    void recalcWidth();
    bool indexIsValid(int index);

    CTabBar*     owner = nullptr;
    QFrame*      tabArea = nullptr;
    QFrame*      scrollFrame = nullptr;
    QToolButton* leftButton = nullptr;
    QToolButton* rightButton = nullptr;
    QVector<Tab*>  tabList;
    QVector<QRect> tabLayouts;
    Qt::TextElideMode elideMode;
    Tab* movedTab = nullptr;
    bool lock = false;
    bool isUIThemeDark = false;
    bool ignore_tabcolor = false;
    int animationInProgress = 0;
    int movedTabPosX = 0;
    int movedTabPressPosX = 0;
    int movedTabIndex = -1;
    int tab_width = -1;
    int currentIndex = -1;
    QSize iconSize;
};

CTabBar::CTabBarPrivate::CTabBarPrivate(CTabBar* owner) :
    owner(owner),
    elideMode(Qt::ElideRight),
    iconSize(DEFAULT_ICON_SIZE)
{

}

CTabBar::CTabBarPrivate::~CTabBarPrivate()
{}

int CTabBar::CTabBarPrivate::getIntersectedOffset(int index)
{
    if (indexIsValid(index)) {
        QRect tabRect = _tabRect(index);
        if (!tabArea->rect().contains(tabRect) && tabArea->rect().intersects(tabRect)) {
            QRect interRect = tabArea->rect().intersected(tabRect);
            return (cellWidth() - interRect.width()) * signum(interRect.x() != 0);
        }
    }
    return 0;
}

int CTabBar::CTabBarPrivate::getIntersectedIndex(int direction, int &offsetX)
{
    offsetX = 0;
    for (int i = 0; i < tabList.size(); i++) {
        QRect tabRect = _tabRect(i);
        if (!tabArea->rect().contains(tabRect) && tabArea->rect().intersects(tabRect)) {
            QRect interRect = tabArea->rect().intersected(tabRect);
            if ((direction == Direction::Left && interRect.x() != 0)
                    || (direction == Direction::Right && interRect.x() == 0)) {
                offsetX = cellWidth() - interRect.width();
                return i;
            }
        }
    }
    return -1;
}

int CTabBar::CTabBarPrivate::getLayoutsIntersectedIndex(Tab *tab, int &offsetX)
{
    offsetX = 0;
    QRect tabRect = tab->geometry();
    for (int i = 0; i < tabLayouts.size(); i++) {
        if (i != tab->index && tabRect.intersects(tabLayouts[i])) {
            QRect interRect = tabRect.intersected(tabLayouts[i]);
            offsetX = interRect.width() * signum(tabRect.x() >= tabLayouts[i].x());
            return i;
        }
    }
    return -1;
}

int CTabBar::CTabBarPrivate::cellWidth()
{
    Q_ASSERT(!tabList.isEmpty());
    return tabList[0]->width() + TABSPACING;
}

//int CTabBar::CTabBarPrivate::prevTabPosByNext(int index)
//{
//    Q_ASSERT(index > -1 && index < tabList.size());
//    return tabRect(index).left() - cellWidth() - 1;
//}

int CTabBar::CTabBarPrivate::nextTabPosByPrev(int index)
{
    Q_ASSERT(index > -1 && index < tabList.size());
    return _tabRect(index).right() + TABSPACING + 1;
}

void CTabBar::CTabBarPrivate::slide(int from, int to, int offset, int animation_ms)
{
    animationInProgress++;
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    for (int i = from; i <= to; i++) {
        QPropertyAnimation* animation = new QPropertyAnimation(tabList[i], "pos");
        animation->setDuration(animation_ms);
        animation->setStartValue(tabList[i]->pos());
        animation->setEndValue(tabList[i]->pos() + QPoint(offset, 0));
        animation->setEasingCurve(QEasingCurve::InOutQuad);
        group->addAnimation(animation);
    }
    QObject::connect(group, &QParallelAnimationGroup::finished, qApp, [=]() {
        changeScrollerState();
    });
    QObject::connect(group, &QParallelAnimationGroup::stateChanged, qApp,
                     [=](QAbstractAnimation::State newState, QAbstractAnimation::State) {
        if (newState == QAbstractAnimation::Stopped)
            animationInProgress--;
    });
    group->start(QParallelAnimationGroup::DeleteWhenStopped);
}

void CTabBar::CTabBarPrivate::scrollToDirection(int direction)
{
    while (animationInProgress)
        qApp->processEvents();

    if (tabList.isEmpty())
        return;

    const int supIndex = (direction == Direction::Left) ? tabList.size() - 1 : 0;
    if (!tabArea->rect().contains(_tabRect(supIndex))) {
        int offsetX;
        int ind = getIntersectedIndex(direction, offsetX);
        int offset = (ind != -1) ? offsetX : cellWidth();
        if (direction == Direction::Left)
            offset *= -1;
        slide(0, tabList.size() - 1, offset, ANIMATION_SCROLL_MS);
    }
}

void CTabBar::CTabBarPrivate::scrollTo(int index)
{
    while (animationInProgress)
        qApp->processEvents();

    if (!indexIsValid(index))
        return;

    if (!tabArea->rect().contains(_tabRect(index))) {
        int x = _tabRect(index).x();
        int offsetX = (x > 0) ? x - tabArea->width() + cellWidth() : x;
        slide(0, tabList.size() - 1, -1 * offsetX, ANIMATION_DEFAULT_MS);
    } else {
        changeScrollerState();
    }
}

void CTabBar::CTabBarPrivate::onCurrentChanged(int index)
{
    while (animationInProgress)
        qApp->processEvents();
    recalcWidth();
    scrollTo(index);
    currentIndex = index;
    for (int i = 0; i < tabList.size(); i++) {
        tabList[i]->setProperty("selected", i == index);
        tabList[i]->polish();
    }
    emit owner->currentChanged(index);
}

void CTabBar::CTabBarPrivate::onTabWidthChanged(int width)
{
    Q_UNUSED(width)
    while (animationInProgress)
        qApp->processEvents();

    if (!tabList.isEmpty())
        iconSize = tabList[0]->icon_label->size();

    for (int i = 0; i < tabList.size(); i++) {
        int posX = i * cellWidth();
        tabList[i]->move(posX, 0);

        if (QIcon *icon = tabList[i]->tab_icon)
            tabList[i]->icon_label->setPixmap(icon->pixmap(iconSize));

        tabList[i]->elideText(elideMode);
    }
    recalcWidth();
    scrollTo(currentIndex);
}

void CTabBar::CTabBarPrivate::changeScrollerState()
{
    bool allowScroll = false;
    for (int i = 0; i < tabList.size(); i++) {
        if (_tabRect(i).x() + 1 < 0) {
            allowScroll = true;
            break;
        }
    }
    leftButton->setEnabled(allowScroll);

    allowScroll = false;
    for (int i = 0; i < tabList.size(); i++) {
        if (_tabRect(i).right() + TABSPACING > tabArea->rect().right()) {
            allowScroll = true;
            break;
        }
    }
    rightButton->setEnabled(allowScroll);
}

void CTabBar::CTabBarPrivate::reorderIndexes()
{
    const int size = tabList.size();
    QVector<Tab*> dupTabList(size, nullptr);
    for (int i = 0; i < size; i++) {
        Q_ASSERT(tabIndex(i) > -1 && tabIndex(i) < size);
        dupTabList[tabIndex(i)] = tabList[i];
    }
    tabList = std::move(dupTabList);
}

void CTabBar::CTabBarPrivate::recalcWidth()
{
    int minWidth = (tabList.isEmpty()) ? 0 : cellWidth();
    tabArea->setMinimumWidth(minWidth);
    tabArea->setMaximumWidth(minWidth * tabList.size());
    owner->setMaximumWidth(tabArea->maximumWidth() + scrollFrame->maximumWidth());
    qApp->processEvents();
}

bool CTabBar::CTabBarPrivate::indexIsValid(int index)
{
    return index > -1 && index < tabList.size();
}


CTabBar::CTabBar(QWidget *parent) :
    QFrame(parent),
    d(new CTabBarPrivate(this))
{
    installEventFilter(this);
    QHBoxLayout *main_layout = new QHBoxLayout(this);
    main_layout->setAlignment(Qt::AlignLeft);
    main_layout->setContentsMargins(0,0,0,0);
    main_layout->setSpacing(0);
    d->tabArea = new QFrame(this);
    d->tabArea->setObjectName("tabArea");
    d->tabArea->installEventFilter(this);
    d->tabArea->setMaximumWidth(0);
    d->tabArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    main_layout->addWidget(d->tabArea);
    setMinimumWidth(0);
    setMaximumWidth(0);

    d->scrollFrame = new QFrame(this);
    d->scrollFrame->setObjectName("tabScroll");
    QHBoxLayout *scrollLayout = new QHBoxLayout(d->scrollFrame);
    scrollLayout->setSpacing(0);
    scrollLayout->setContentsMargins(0,0,0,0);
    d->scrollFrame->setLayout(scrollLayout);

    d->leftButton = new QToolButton(d->scrollFrame);
    d->rightButton = new QToolButton(d->scrollFrame);
    d->leftButton->setObjectName("leftButton");
    d->rightButton->setObjectName("rightButton");

    scrollLayout->addWidget(d->leftButton);
    scrollLayout->addWidget(d->rightButton);
    d->leftButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    d->rightButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    main_layout->addWidget(d->scrollFrame);
    connect(d->leftButton, &QToolButton::clicked, this, [=]() {
        d->scrollToDirection(d->Direction::Right);
    });
    connect(d->rightButton, &QToolButton::clicked, this, [=]() {
        d->scrollToDirection(d->Direction::Left);
    });

    show();
    d->leftButton->hide();
    d->rightButton->hide();
    d->leftButton->setEnabled(false);
    d->rightButton->setEnabled(false);
}

CTabBar::~CTabBar()
{
    delete d, d = nullptr;
}

int CTabBar::addTab(const QString &text)
{
    while (d->animationInProgress)
        qApp->processEvents();

    const int lastIndex = d->tabList.size() - 1;
    const int posX = (lastIndex == -1) ? 0 : d->nextTabPosByPrev(lastIndex);
    Tab *tab = new Tab(d->tabArea);
    tab->move(posX, 0);
    tab->setFixedHeight(d->tabArea->height());
    tab->setText(text, d->elideMode);
    if (tab->icon_label->minimumSize().isNull()) {
        tab->icon_label->setBaseSize(d->iconSize);
        tab->icon_label->setFixedSize(d->iconSize);
    }
    tab->index = lastIndex + 1;
    d->tabList.append(tab);
    connect(tab->close_btn, &QToolButton::clicked, this, [=]() {
        emit tabCloseRequested(tab->index);
    });
    connect(tab, &Tab::onTabWidthChanged, this, [=](int width) {
        if (d->tab_width != width) {
            d->tab_width = width;
            d->onTabWidthChanged(width);
        }
    });
    tabInserted(lastIndex + 1);
    d->onCurrentChanged(lastIndex + 1);
    return d->currentIndex;
}

int CTabBar::addTab(const QIcon &icon, const QString &text)
{
    const int index = addTab(text);
    setTabIcon(index, icon);
    return index;
}

int CTabBar::count() const
{
    return d->tabList.size();
}

int CTabBar::currentIndex() const
{
    return d->currentIndex;
}

Qt::TextElideMode CTabBar::elideMode() const
{
    return d->elideMode;
}

QSize CTabBar::iconSize() const
{
    return d->iconSize;
}

int CTabBar::insertTab(int index, const QString &text)
{
    while (d->animationInProgress)
        qApp->processEvents();

    if (!d->indexIsValid(index))
        return addTab(text);

    while (d->animationInProgress)
        qApp->processEvents();

    int posX = d->_tabRect(index).left();
    d->slide(index, d->tabList.size() - 1, d->cellWidth(), ANIMATION_DEFAULT_MS);
    while (d->animationInProgress)
        qApp->processEvents();

    Tab *tab = new Tab(d->tabArea);
    tab->move(posX, 0);
    tab->setFixedHeight(d->tabArea->height());
    tab->setText(text, d->elideMode);
    if (tab->icon_label->minimumSize().isNull()) {
        tab->icon_label->setBaseSize(d->iconSize);
        tab->icon_label->setFixedSize(d->iconSize);
    }
    d->tabList.insert(index, tab);
    for (int i = index; i < d->tabList.size(); i++)
        d->tabIndex(i) = i;

    connect(tab->close_btn, &QToolButton::clicked, this, [=]() {
        emit tabCloseRequested(tab->index);
    });
    connect(tab, &Tab::onTabWidthChanged, this, [=](int width) {
        if (d->tab_width != width) {
            d->tab_width = width;
            d->onTabWidthChanged(width);
        }
    });
    tabInserted(index);
    d->onCurrentChanged(index);
    return d->currentIndex;
}

int CTabBar::insertTab(int index, const QIcon &icon, const QString &text)
{
    const int actual_index = insertTab(index, text);
    setTabIcon(actual_index, icon);
    return actual_index;
}

//void CTabBar::moveTab(int from, int to)
//{
//    while (d->animationInProgress)
//        qApp->processEvents();

//    if (from == to || !d->indexIsValid(from) || !d->indexIsValid(to))
//        return;

//    int posX = d->_tabRect(from).x();
//    d->tabList[from]->move(d->_tabRect(to).x(), 0);
//    d->tabList[to]->move(posX, 0);
//    int from_index = d->tabIndex(from);
//    d->tabIndex(from) = d->tabIndex(to);
//    d->tabIndex(to) = from_index;
//    std::swap(d->tabList[from], d->tabList[to]);
//    d->scrollTo(to);
//    emit tabMoved(from, to);
//}

void CTabBar::removeTab(int index)
{
    while (d->animationInProgress)
        qApp->processEvents();

    if (!d->indexIsValid(index))
        return;

    d->tabList[index]->hide();
    if (d->_tabRect(0).x() <= d->tabArea->x() - d->cellWidth()) {
        const int prevIndex = index - 1;
        if (prevIndex > -1) {
            d->slide(0, prevIndex, d->cellWidth(), ANIMATION_DEFAULT_MS);
            while (d->animationInProgress)
                qApp->processEvents();
        }
    } else {
        const int nextIndex = index + 1;
        if (nextIndex < d->tabList.size()) {
            d->slide(nextIndex, d->tabList.size() - 1, -1 * d->cellWidth(), ANIMATION_DEFAULT_MS);
            while (d->animationInProgress)
                qApp->processEvents();
        }
    }

    delete d->tabList[index];
    d->tabList.remove(index);
    for (int i = index; i < d->tabList.size(); i++)
        d->tabIndex(i) = i;

    if (d->currentIndex > 0) {
        if (index > 0) {
            if (d->currentIndex > index) {
                d->onCurrentChanged(d->currentIndex - 1);
            } else
            if (d->currentIndex < index) {
                d->recalcWidth();
            } else {
                const int initialMaxIndex = d->tabList.size(); // max index before deletion
                d->onCurrentChanged(index < initialMaxIndex ? index : initialMaxIndex - 1);
            }
        } else {
            d->onCurrentChanged(d->currentIndex - 1);
        }
    } else {
        if (index > 0) {
            d->recalcWidth();
        } else {
            const int initialMaxIndex = d->tabList.size(); // max index before deletion
            d->onCurrentChanged(index < initialMaxIndex ? index : -1);
        }
    }
}

void CTabBar::setElideMode(Qt::TextElideMode mode)
{
    d->elideMode = mode;
}

void CTabBar::setIconSize(const QSize &size)
{
    d->iconSize = size;
    for (int i = 0; i < d->tabList.size(); i++) {
        if (d->tabList[i]->icon_label->baseSize().isNull())
            break;
        d->tabList[i]->icon_label->setBaseSize(size);
        d->tabList[i]->icon_label->setFixedSize(size);
        if (QIcon *icon = d->tabList[i]->tab_icon)
            d->tabList[i]->icon_label->setPixmap(icon->pixmap(d->iconSize));
    }
}

void CTabBar::setTabIconLabel(int index, QWidget *widget)
{
    if (!d->indexIsValid(index))
        return;
    if (CAnimatedIcon *icon_label = dynamic_cast<CAnimatedIcon*>(widget)) {
        Tab *tab = d->tabList[index];
        if (tab->icon_label)
            delete tab->icon_label;
        tab->icon_label = icon_label;
        icon_label->setParent(tab);
        icon_label->setObjectName("tabIcon");
        icon_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        icon_label->setMaximumSize(DEFAULT_ICON_SIZE);
        if (QHBoxLayout *lut = dynamic_cast<QHBoxLayout*>(tab->layout()))
            lut->insertWidget(0, icon_label);
        icon_label->style()->polish(icon_label);
    }
}

void CTabBar::setTabButton(int index, QWidget *widget)
{
    if (!d->indexIsValid(index))
        return;
    if (QToolButton *close_btn = dynamic_cast<QToolButton*>(widget)) {
        Tab *tab = d->tabList[index];
        if (tab->close_btn)
            delete tab->close_btn;
        tab->close_btn = close_btn;
        close_btn->setParent(tab);
        close_btn->setObjectName("tabButton");
        close_btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        close_btn->setMaximumSize(DEFAULT_ICON_SIZE);
        connect(close_btn, &QToolButton::clicked, this, [=]() {
            emit tabCloseRequested(tab->index);
        });
        if (QHBoxLayout *lut = dynamic_cast<QHBoxLayout*>(tab->layout()))
            lut->insertWidget(2, close_btn);
        close_btn->style()->polish(close_btn);
    }
}

//void CTabBar::setTabData(int index, const QVariant &data)
//{
//    if (d->indexIsValid(index))
//        d->tabList[index]->setProperty("TabData", data);
//}

void CTabBar::setTabIcon(int index, const QIcon &icon)
{
    if (d->indexIsValid(index))
        d->tabList[index]->setIcon(icon);
}

void CTabBar::setTabText(int index, const QString &text)
{
    if (d->indexIsValid(index))
        d->tabList[index]->setText(text);
}

void CTabBar::setTabToolTip(int index, const QString &text)
{
    if (d->indexIsValid(index)) {
        d->tabList[index]->setProperty("ToolTip", text);
        d->tabList[index]->icon_label->setProperty("ToolTip", text);
        d->tabList[index]->text_label->setProperty("ToolTip", text);
        d->tabList[index]->close_btn->setProperty("ToolTip", text);
    }
}

void CTabBar::setCurrentIndex(int index)
{
    while (d->animationInProgress)
        qApp->processEvents();
    if (!d->indexIsValid(index) || index == d->currentIndex)
        return;
    d->onCurrentChanged(index);
}

void CTabBar::setActiveTabColor(int index, const QString& color)
{
    if (!d->indexIsValid(index) || color == "none")
        return;
    if (d->tabList[index]->tabcolor != color) {
        d->tabList[index]->tabcolor = color;
        d->tabList[index]->polish();
    }
}

void CTabBar::setUseTabCustomPalette(int index, bool use)
{
    if (d->indexIsValid(index)) {
        if (d->tabList[index]->property("custom").toBool() != use) {
            d->tabList[index]->setProperty("custom", use);
            polish();
        }
    }
}

void CTabBar::setTabLoading(int index, bool start)
{
    if (!start) {
        if (CAnimatedIcon * icon = (CAnimatedIcon*)tabIconLabel(index))
            icon->stop();
    }
}

void CTabBar::setTabIconTheme(int index, TabTheme theme)
{
    if (CAnimatedIcon * icon = (CAnimatedIcon*)tabIconLabel(index))
        icon->setSvgElement(theme == TabTheme::LightTab ? "dark" : "light");
}

void CTabBar::tabStartLoading(int index, const QString& theme)
{
    CAnimatedIcon * icon = (CAnimatedIcon*)tabIconLabel(index);
    if (icon && !icon->isStarted() )
        icon->startSvg(":/tabbar/icons/loader.svg", theme);
}

void CTabBar::setIgnoreActiveTabColor(bool ignore)
{
    d->ignore_tabcolor = ignore;
}

bool CTabBar::ignoreActiveTabColor()
{
    return d->ignore_tabcolor;
}

void CTabBar::polish()
{
    for (int i = 0; i < d->tabList.size(); i++)
        d->tabList[i]->polish();
    d->tabArea->style()->polish(d->tabArea);
    d->scrollFrame->style()->polish(d->scrollFrame);
    d->leftButton->style()->polish(d->leftButton);
    d->rightButton->style()->polish(d->rightButton);
}

int CTabBar::tabIndexAt(const QPoint &pos) const
{
    QPoint rel_pos = d->tabArea->mapFromParent(pos);
    if (d->tabArea->rect().contains(rel_pos)) {
        for (int i = 0; i < d->tabList.size(); i++) {
            if (d->_tabRect(i).contains(rel_pos))
                return i;
        }
    }
    return -1;
}

QWidget *CTabBar::tabIconLabel(int index) const
{
    return d->indexIsValid(index) ? d->tabList[index]->icon_label : nullptr;
}

QWidget *CTabBar::tabButton(int index) const
{
    return d->indexIsValid(index) ? d->tabList[index]->close_btn : nullptr;
}

//QVariant CTabBar::tabData(int index) const
//{
//    return d->indexIsValid(index) ? d->tabList[index]->property("TabData") : QVariant();
//}

QIcon CTabBar::tabIcon(int index) const
{
    if (d->indexIsValid(index) && d->tabList[index]->tab_icon)
        return *d->tabList[index]->tab_icon;
    return QIcon();
}

QRect CTabBar::tabRect(int index) const
{
    return d->indexIsValid(index) ? d->_tabRect(index) : QRect();
}

QString CTabBar::tabText(int index) const
{
    return (d->indexIsValid(index)) ? d->tabList[index]->text : QString();
}

QVariant CTabBar::tabProperty(int index, const char *name)
{
    return d->indexIsValid(index) ? d->tabList[index]->property(name) : QVariant();
}

void CTabBar::tabInserted(int index)
{
    Q_UNUSED(index)
}

void CTabBar::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    for (int i = 0; i < d->tabList.size(); i++)
        d->tabList[i]->setFixedHeight(d->tabArea->height());

    int tabsWidth = d->tabList.isEmpty() ? 0 : count() * d->cellWidth();
    bool visible = (tabsWidth > d->tabArea->width());
    d->leftButton->setVisible(visible);
    d->rightButton->setVisible(visible);

    const int lastIndex = d->tabList.size() - 1;
    const int offsetX = (lastIndex == -1) ? 0 : d->tabArea->rect().right() - d->_tabRect(lastIndex).right();
    if (offsetX > TABSPACING) {
        for (int i = 0; i < d->tabList.size(); i++)
            d->tabList[i]->move(d->_tabRect(i).x() + offsetX - TABSPACING, 0);
    }
    d->changeScrollerState();
}

void CTabBar::wheelEvent(QWheelEvent *event)
{
    QFrame::wheelEvent(event);
    if (!d->animationInProgress && d->tabArea->underMouse()) {
#ifdef DONT_USE_SIMPLE_WHEEL_SCROLL
        if (d->currentIndex > 0 && event->angleDelta().y() > 0) {
            emit onCurrentChangedByWhell(d->currentIndex - 1);
            d->onCurrentChanged(d->currentIndex - 1);
        } else
        if (d->currentIndex < count() - 1 && event->angleDelta().y() < 0) {
            emit onCurrentChangedByWhell(d->currentIndex + 1);
            d->onCurrentChanged(d->currentIndex + 1);
        }
#else
        if (event->angleDelta().y() != 0)
            d->scrollToDirection(event->angleDelta().y() > 0 ? d->Direction::Right : d->Direction::Left);
#endif
    }
}

bool CTabBar::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->tabArea) {
        switch (event->type()) {
        case QEvent::MouseMove: {
            QMouseEvent* me = dynamic_cast<QMouseEvent*>(event);
            if (me->buttons().testFlag(Qt::LeftButton)) {
                if (d->movedTab && !d->lock) {
                    d->movedTab->move(me->x() - d->movedTabPressPosX, 0);
                    int offsetX;
                    const int interIndex = d->getLayoutsIntersectedIndex(d->movedTab, offsetX);
                    if (/*interIndex != -1 &&*/ d->movedTab->index != interIndex && offsetX != 0) {
                        const int sign = signum(offsetX > 0);
                        if (sign * offsetX > d->movedTab->width()/2) {
                            if (d->indexIsValid(interIndex + sign)) {
                                int delta = d->tabLayouts[interIndex + sign].x() - d->_tabRect(interIndex).x();
                                d->slide(interIndex, interIndex, delta, ANIMATION_MOVE_TAB_MS);
                            }
                            d->tabIndex(interIndex) += sign;
                            d->movedTab->index = d->tabIndex(interIndex) - sign;
                            d->currentIndex = d->movedTab->index;
                            emit tabMoved(d->currentIndex, d->tabIndex(interIndex));
                            emit currentChanged(d->currentIndex);
                            d->reorderIndexes();
                        }
                    }
                    if (!d->tabArea->rect().contains(me->pos()) && d->tabArea->rect().right() >= me->x()) {
                        if (d->currentIndex != d->movedTabIndex)
                            d->reorderIndexes();
                        bool accepted = false;
                        emit tabUndock(d->currentIndex, accepted);
                        if (accepted) {
                            d->movedTab->hide();
                            d->movedTab = nullptr;
                            d->movedTabIndex = -1;
                            QTimer::singleShot(0, this, [=]() {
                                removeTab(d->currentIndex);
                                while (d->animationInProgress)
                                    qApp->processEvents();
                                d->changeScrollerState();
                            });
                        }
                    }
                }
            }
            break;
        }
        case QEvent::MouseButtonPress: {
            QMouseEvent* me = dynamic_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton) {
                if (!d->animationInProgress) {
                    for (int i = 0; i < d->tabList.size(); i++) {
                        if (d->_tabRect(i).contains(me->pos())) {
                            d->lock = true;
                            d->movedTab = d->tabList[i];
                            d->movedTabIndex = i;
                            const int offset = d->getIntersectedOffset(i);
                            d->movedTabPosX = d->movedTab->x() - offset;
                            d->movedTabPressPosX = me->x() - d->movedTabPosX;
                            QPoint oldCurPos = QCursor::pos();
                            d->tabLayouts.clear();
                            for (int j = 0; j < d->tabList.size(); j++) {
                                d->tabLayouts.append(d->_tabRect(j).translated(-1 * offset, 0).adjusted(0,0,TABSPACING,0));
                                if (j != i)
                                    d->tabList[j]->stackUnder(d->movedTab);
                            }
                            emit tabBarClicked(i);
                            if (i != d->currentIndex)
                                d->onCurrentChanged(i);
                            else
                                d->scrollTo(i);
                            while (d->animationInProgress)
                                qApp->processEvents();
                            QCursor::setPos(oldCurPos);
                            d->lock = false;
                            return true;
                        }
                    }
                }
            }
            break;
        }
        case QEvent::MouseButtonRelease: {
            QMouseEvent* mouse_event = dynamic_cast<QMouseEvent*>(event);
            if (mouse_event->button() == Qt::LeftButton) {
                while (d->animationInProgress)
                    qApp->processEvents();
                if (d->movedTab) {
                    if (d->currentIndex != d->movedTabIndex) {
                        d->reorderIndexes();
                        int posX = d->tabLayouts[d->currentIndex].x();
                        d->movedTab->move(posX, 0);
                    } else {
                        d->movedTab->move(d->movedTabPosX, 0);
                    }
                    d->movedTab = nullptr;
                    d->movedTabIndex = -1;
                    d->changeScrollerState();
                    return true;
                }
            } else
            if (mouse_event->button() == Qt::MiddleButton) {
                for (int i = 0; i < d->tabList.size(); i++) {
                    if (d->tabList[i]->close_btn->underMouse()) {
                        emit tabCloseRequested(i);
                        return true;
                    }
                }
            }
            break;
        }
//        case QEvent::MouseButtonDblClick: {
//            QMouseEvent* mouse_event = dynamic_cast<QMouseEvent*>(event);
//            if (mouse_event->button() == Qt::LeftButton) {
//                for (int i = 0; i < d->tabList.size(); i++) {
//                    if (d->tabList[i]->underMouse()) {
//                        emit tabBarDoubleClicked(i);
//                        return true;
//                    }
//                }
//            }
//            break;
//        }
        default:
            break;
        }
    } else
    if (watched == this) {
        switch (event->type()) {
        case QEvent::Enter:
            setCursor(QCursor(Qt::ArrowCursor));
            break;
        case QEvent::Leave:
            QApplication::postEvent(d->tabArea, new QMouseEvent(QEvent::MouseButtonRelease, QCursor::pos(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier));
            break;
        default:
            break;
        }
    }
    return QFrame::eventFilter(watched, event);
}

#include "ctabbar.moc"
