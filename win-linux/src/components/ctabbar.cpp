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
//#include "ctabbar_p.h"
#include "private/qtabbar_p.h"
#include <QStylePainter>
#include <QHoverEvent>
#include <QDesktopWidget>
#include "components/canimatedicon.h"
#include "utils.h"

#define TAB_BTNCLOSE(index) tabButton(index, QTabBar::RightSide)
#define TAB_ICON(index) tabButton(index, QTabBar::LeftSide)

inline static bool verticalTabs(QTabBar::Shape shape)
{
    return shape == QTabBar::RoundedWest || shape == QTabBar::RoundedEast
           || shape == QTabBar::TriangularWest || shape == QTabBar::TriangularEast;
}

QMovableTabWidget::QMovableTabWidget(QWidget *parent)
    : QWidget(parent)
{
}

void QMovableTabWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QPainter p(this);
    p.drawPixmap(0, 0, m_pixmap);
}

/*
 *  TabBarPrivate class description
 *
*/
void QTabBarPrivate::layoutTab(int index)
{
    Q_Q(QTabBar);
    Q_ASSERT(index >= 0);

    Tab &tab = tabList[index];
    bool vertical = verticalTabs(shape);
    if (!(tab.leftWidget || tab.rightWidget))
        return;

    QStyleOptionTabV3 opt;
    q->initStyleOption(&opt, index);
    if (tab.leftWidget) {
        QRect rect = q->style()->subElementRect(QStyle::SE_TabBarTabLeftButton, &opt, q);
        QPoint p = rect.topLeft();
        if ((index == pressedIndex) || paintWithOffsets) {
            if (vertical)
                p.setY(p.y() + tabList[index].dragOffset); else
                p.setX(p.x() + tabList[index].dragOffset);
        }
        tab.leftWidget->move(p);
    }
    if (tab.rightWidget) {
        QRect rect = q->style()->subElementRect(QStyle::SE_TabBarTabRightButton, &opt, q);
        QPoint p = rect.topLeft();
        if ((index == pressedIndex) || paintWithOffsets) {
            if (vertical)
                p.setY(p.y() + tab.dragOffset);
            else
                p.setX(p.x() + tab.dragOffset);
        }

        tab.rightWidget->move(p);
    }
}

void QTabBarPrivate::layoutWidgets(int start)
{
    Q_Q(QTabBar);

    for (int i = start; i < q->count(); ++i) {
        layoutTab(i);
    }
}

void QTabBarPrivate::slide(int from, int to)
{
    Q_Q(QTabBar);

    if (from == to || !validIndex(from) || !validIndex(to))
        return;

    bool vertical = verticalTabs(shape);
    int preLocation = vertical ? q->tabRect(from).y() : q->tabRect(from).x();
    q->setUpdatesEnabled(false);
    q->moveTab(from, to);
    q->setUpdatesEnabled(true);
    int postLocation = vertical ? q->tabRect(to).y() : q->tabRect(to).x();
    int length = postLocation - preLocation;
    tabList[to].dragOffset -= length;
    tabList[to].startAnimation(this, ANIMATION_DURATION);
}

void QTabBarPrivate::moveTabFinished(int index)
{
    Q_Q(QTabBar);

    bool cleanup = (pressedIndex == index) || (pressedIndex == -1) || !validIndex(index);
    bool allAnimationsFinished = true;
#ifndef QT_NO_ANIMATION
    for(int i = 0; allAnimationsFinished && i < tabList.count(); ++i) {
        const Tab &t = tabList.at(i);
        if (t.animation && t.animation->state() == QAbstractAnimation::Running)
            allAnimationsFinished = false;
    }
#endif //QT_NO_ANIMATION
    if (allAnimationsFinished && cleanup) {
        if(movingTab)
            movingTab->setVisible(false); // We might not get a mouse release
        for (int i = 0; i < tabList.count(); ++i) {
            tabList[i].dragOffset = 0;
        }
        if (pressedIndex != -1 && movable) {
            pressedIndex = -1;
            dragInProgress = false;
            dragStartPosition = QPoint();
        }
        layoutWidgets();
    } else {
        if (!validIndex(index))
            return;
        tabList[index].dragOffset = 0;
    }

    q->update();
}

void QTabBarPrivate::setupMovableTab()
{
    Q_Q(QTabBar);

    if (!movingTab)
        movingTab = new QMovableTabWidget((QWidget *)q);

    int taboverlap = q->style()->pixelMetric(QStyle::PM_TabBarTabOverlap, 0 ,q);
    QRect grabRect = q->tabRect(pressedIndex);
    grabRect.adjust(-taboverlap, 0, taboverlap, 0);

    QPixmap grabImage(grabRect.size());
    grabImage.fill(Qt::transparent);
    QStylePainter p(&grabImage, q);
    p.initFrom(q);

    QStyleOptionTabV3 tab;
    q->initStyleOption(&tab, pressedIndex);
    tab.rect.moveTopLeft(QPoint(taboverlap, 0));

    QString text = tab.text;
    tab.text = "";
    p.drawControl(QStyle::CE_TabBarTab, tab);
    ((CTabBar*)q)->fillTabColor(&p, tab, pressedIndex, ((CTabBar*)q)->m_activeColor);
    ((CTabBar*)q)->drawTabCaption(&p, text, tab);
    p.end();

    QPalette pal;
    pal.setBrush(QPalette::All, QPalette::Window, grabImage);
    movingTab->setPalette(pal);
    movingTab->setGeometry(grabRect);
    movingTab->setAutoFillBackground(true);
    movingTab->raise();

    // Re-arrange widget order to avoid overlaps
    if (tabList[pressedIndex].leftWidget)
        tabList[pressedIndex].leftWidget->raise();
    if (tabList[pressedIndex].rightWidget)
        tabList[pressedIndex].rightWidget->raise();
    if (leftB) leftB->raise();
    if (rightB) rightB->raise();

    movingTab->setVisible(true);
}

void QTabBarPrivate::moveTab(int index, int offset)
{
    if (!validIndex(index))
        return;

    tabList[index].dragOffset = offset;
    layoutTab(index); // Make buttons follow tab
    q_func()->update();
}


void QTabBarPrivate::Tab::TabBarAnimation::updateCurrentValue(const QVariant &current)
{
    priv->moveTab(priv->tabList.indexOf(*tab), current.toInt());
}

void QTabBarPrivate::Tab::TabBarAnimation::updateState(QAbstractAnimation::State, QAbstractAnimation::State newState)
{
    if (newState == Stopped) priv->moveTabFinished(priv->tabList.indexOf(*tab));
}

/*
 *    CTabBar descrition
*/
CTabBar::CTabBar(QWidget * parent)
    : QTabBar(parent)
    , CScalingWrapper(parent)
    , m_scrollPos(0)
{
    hide();
    setDrawBase(false);

    if (Utils::getScreenDpiRatio(
                QApplication::desktop()->screen(QApplication::desktop()->primaryScreen())->geometry().topLeft()) > 1)
    {
        setProperty("scroll", "var2");
    }

    connect(this, &QTabBar::currentChanged, this, &CTabBar::onCurrentChanged);
}

CTabBar::~CTabBar()
{

}

void CTabBar::initCustomScroll(QFrame      *_pScrollerFrame,
                               QToolButton *_pLeftButton,
                               QToolButton *_pRightButton)
{
    // Bypassing the bug with tab scroller
    m_pScrollerFrame = _pScrollerFrame;
    m_pLeftButton = _pLeftButton;
    m_pRightButton = _pRightButton;
    m_pScrollerFrame->show();
    m_pScrollerFrame->raise();
    m_pLeftButton->show();
    m_pLeftButton->raise();
    m_pRightButton->show();
    m_pRightButton->raise();
    m_pScrollerFrame->setVisible(false);
    connect(m_pLeftButton, &QToolButton::clicked, this, [=](){
        Q_D(QTabBar);
        d->leftB->click();
        m_scrollPos = d->scrollOffset;
        changeCustomScrollerState();
    });
    connect(m_pRightButton, &QToolButton::clicked, this, [=](){
        Q_D(QTabBar);
        d->rightB->click();
        m_scrollPos = d->scrollOffset;
        changeCustomScrollerState();
    }); // End bypassing the bug
}

void CTabBar::mouseMoveEvent(QMouseEvent * event)
{
    Q_D(QTabBar);
    this->setCursor(QCursor(Qt::ArrowCursor));
    if (verticalTabs(d->shape)) {
        QTabBar::mouseMoveEvent(event);
        return;
    }

    if (d->movable) {
        // Be safe!
        if (d->pressedIndex != -1 && event->buttons() == Qt::NoButton)
            d->moveTabFinished(d->pressedIndex);

        // Start drag
        if (!d->dragInProgress && d->pressedIndex != -1) {
            if ((event->pos() - d->dragStartPosition).manhattanLength() > QApplication::startDragDistance()) {
                d->dragInProgress = true;
                d->setupMovableTab();
            }
        }

        int offset = (event->pos() - d->dragStartPosition).manhattanLength();
        if (event->buttons() == Qt::LeftButton
                && offset > QApplication::startDragDistance() && d->validIndex(d->pressedIndex)) {
            if ( abs(event->pos().y() - d->dragStartPosition.y()) > 30 ) {
                bool isaccepted = false;
                emit tabUndock(d->pressedIndex, &isaccepted);

                if (isaccepted)
                    interruptTabMoving(d->pressedIndex);
                return;
            }

            int dragDistance = (event->pos().x() - d->dragStartPosition.x());

            if ((d->pressedIndex == 0 && dragDistance < 0) ||
                    (d->pressedIndex + 1 == count() && dragDistance > 0))
            {
                dragDistance = 0;
            }

            d->tabList[d->pressedIndex].dragOffset = dragDistance;

            QRect startingRect = tabRect(d->pressedIndex);
            startingRect.moveLeft(startingRect.x() + dragDistance);

            int overIndex;
            if (dragDistance < 0)
                overIndex = tabAt(startingRect.topLeft()); else
                overIndex = tabAt(startingRect.topRight());

            if (overIndex != d->pressedIndex && overIndex != -1) {
                int offset = 1;
                if (isRightToLeft()) offset *= -1;
                if (dragDistance < 0) {
                    dragDistance *= -1;
                    offset *= -1;
                }
                for (int i = d->pressedIndex;
                     offset > 0 ? i < overIndex : i > overIndex;
                     i += offset) {
                    QRect overIndexRect = tabRect(overIndex);
                    int needsToBeOver = overIndexRect.width() / 2;
                    if (dragDistance > needsToBeOver)
                        d->slide(i + offset, d->pressedIndex);
                }
            }

            // Buttons needs to follow the dragged tab
            d->layoutTab(d->pressedIndex);

            update();
        }
    }

    if (event->buttons() != Qt::LeftButton) {
        event->ignore();
        return;
    }

    QStyleOptionTabBarBaseV2 optTabBase;
    optTabBase.init(this);
    optTabBase.documentMode = d->documentMode;
}

void CTabBar::mousePressEvent(QMouseEvent * e)
{
    if ( e->button() == Qt::LeftButton ) {
        QTabBar::mousePressEvent(e);
//        if ( count() == 1 ) e->ignore();
    } else e->ignore();
}

void CTabBar::mouseReleaseEvent(QMouseEvent * e)
{
    QTabBar::mouseReleaseEvent(e);
    releaseMouse();
}

void CTabBar::wheelEvent(QWheelEvent *event)
{
    QTabBar::wheelEvent(event);
    emit onCurrentChangedByWhell(currentIndex());
}

void CTabBar::drawTabCaption(QPainter * p, const QString& s, const QStyleOptionTab& t)
{
    if ( m_usePalette ) {
        if ( m_palette.currentColorGroup() != QPalette::Disabled &&
                t.state & QStyle::State_Selected /*|| t.state & QStyle::State_MouseOver*/ )
            p->setPen( QPen(m_palette.color(QPalette::Active, QPalette::ButtonText)));
        else p->setPen( QPen(m_palette.color(QPalette::Inactive, QPalette::ButtonText)));
    } else {
        p->setPen(QPen(t.palette.foreground().color()));
    }

    QPoint _lt = QPoint(15, 0) * scaling();
    QPoint _rb = QPoint(-22, -2) * scaling();

    QRect trect(t.rect.topLeft() + QPoint(t.iconSize.width(), 0) + _lt,
                    t.rect.bottomRight() + _rb);

//    QFont f = font();
//    f.setPointSize(8);

//    p->setFont(f);

    QString es = fontMetrics().elidedText(s, Qt::ElideRight, trect.width(), Qt::TextShowMnemonic);
    p->drawText(trect, Qt::AlignVCenter, es);
}

void CTabBar::paintEvent(QPaintEvent * event)
{
    Q_D(QTabBar);

#ifdef _QTVER_DOWNGRADE
    if (verticalTabs(d->shape)) {
        QTabBar::paintEvent(event);
        return;
    }

    QStyleOptionTabBarBaseV2 optTabBase;
    QTabBarPrivate::initStyleBaseOption(&optTabBase, this, size());

    QStylePainter p(this);
    int selected = -1;
    int cut = -1;
    bool rtl = optTabBase.direction == Qt::RightToLeft;
    QStyleOptionTab cutTab;
    selected = d->currentIndex;
    if (d->dragInProgress)
        selected = d->pressedIndex;

    for (int i = 0; i < d->tabList.count(); ++i)
         optTabBase.tabBarRect |= tabRect(i);

    optTabBase.selectedTabRect = tabRect(selected);

    if (d->drawBase)
        p.drawPrimitive(QStyle::PE_FrameTabBarBase, optTabBase);

    for (int i = 0; i < d->tabList.count(); ++i) {
        QStyleOptionTabV3 tab;
        initStyleOption(&tab, i);

        if (d->paintWithOffsets && d->tabList[i].dragOffset != 0) {
            tab.rect.moveLeft(tab.rect.x() + d->tabList[i].dragOffset);
        }
        if (!(tab.state & QStyle::State_Enabled)) {
            tab.palette.setCurrentColorGroup(QPalette::Disabled);
        }
        // If this tab is partially obscured, make a note of it so that we can pass the information
        // along when we draw the tear.
        if ((!rtl && tab.rect.left() < 0) || (rtl && tab.rect.right() > width())) {
            cut = i;
            cutTab = tab;
        }
        // Don't bother drawing a tab if the entire tab is outside of the visible tab bar.
        if (tab.rect.right() < 0 || tab.rect.left() > width())
            continue;

        optTabBase.tabBarRect |= tab.rect;
        if (i == selected)
            continue;

        QString text(tab.text);
        tab.text.clear();
        p.drawControl(QStyle::CE_TabBarTab, tab);
        drawTabCaption(&p, text, tab);
    }

    // Draw the selected tab last to get it "on top"
    if (selected >= 0) {
        QStyleOptionTabV3 tab;
        initStyleOption(&tab, selected);

        if (d->paintWithOffsets && d->tabList[selected].dragOffset != 0) {
            tab.rect.moveLeft(tab.rect.x() + d->tabList[selected].dragOffset);
        }
        if (!d->dragInProgress) {
            QString text(tab.text);
            tab.text.clear();
            p.drawControl(QStyle::CE_TabBarTab, tab);

            if ( m_activeColor != "none" )
                fillTabColor(&p, tab, selected, m_activeColor);
            drawTabCaption(&p, text, tab);
        } else {
            int taboverlap = style()->pixelMetric(QStyle::PM_TabBarTabOverlap, 0, this);
            d->movingTab->setGeometry(tab.rect.adjusted(-taboverlap, 0, taboverlap, 0));
        }
    }

    // Only draw the tear indicator if necessary. Most of the time we don't need too.
    if (d->leftB->isVisible() && cut >= 0) {
        cutTab.rect = rect();
        cutTab.rect = style()->subElementRect(QStyle::SE_TabBarTearIndicator, &cutTab, this);
        p.drawPrimitive(QStyle::PE_IndicatorTabTear, cutTab);
    }
#else
    QStyleOptionTabBarBase optTabBase;
    QTabBarPrivate::initStyleBaseOption(&optTabBase, this, size());

    QStylePainter p(this);
    int selected = -1;
    int cutLeft = -1;
    int cutRight = -1;
    bool vertical = verticalTabs(d->shape);
    QStyleOptionTab cutTabLeft;
    QStyleOptionTab cutTabRight;
    selected = d->currentIndex;
    if (d->dragInProgress)
        selected = d->pressedIndex;
    QRect scrollRect = d->normalizedScrollRect();

    for (int i = 0; i < d->tabList.count(); ++i)
         optTabBase.tabBarRect |= tabRect(i);

    optTabBase.selectedTabRect = tabRect(selected);

    if (d->drawBase)
        p.drawPrimitive(QStyle::PE_FrameTabBarBase, optTabBase);

    for (int i = 0; i < d->tabList.count(); ++i) {
        QStyleOptionTab tab;
        initStyleOption(&tab, i);
        if (d->paintWithOffsets && d->tabList[i].dragOffset != 0) {
            if (vertical) {
                tab.rect.moveTop(tab.rect.y() + d->tabList[i].dragOffset);
            } else {
                tab.rect.moveLeft(tab.rect.x() + d->tabList[i].dragOffset);
            }
        }
        if (!(tab.state & QStyle::State_Enabled)) {
            tab.palette.setCurrentColorGroup(QPalette::Disabled);
        }

        // If this tab is partially obscured, make a note of it so that we can pass the information
        // along when we draw the tear.
        QRect tabRect = d->tabList[i].rect;
        int tabStart = vertical ? tabRect.top() : tabRect.left();
        int tabEnd = vertical ? tabRect.bottom() : tabRect.right();
        if (tabStart < scrollRect.left() + d->scrollOffset) {
            cutLeft = i;
            cutTabLeft = tab;
        } else if (tabEnd > scrollRect.right() + d->scrollOffset) {
            cutRight = i;
            cutTabRight = tab;
        }

        // Don't bother drawing a tab if the entire tab is outside of the visible tab bar.
        if ((!vertical && (tab.rect.right() < 0 || tab.rect.left() > width()))
            || (vertical && (tab.rect.bottom() < 0 || tab.rect.top() > height())))
            continue;

        optTabBase.tabBarRect |= tab.rect;
        if (i == selected)
            continue;

        QString text(tab.text);
        tab.text.clear();
        p.drawControl(QStyle::CE_TabBarTab, tab);
        drawTabCaption(&p, text, tab);
    }

    // Draw the selected tab last to get it "on top"
    if (selected >= 0) {
        QStyleOptionTab tab;
        initStyleOption(&tab, selected);
        if (d->paintWithOffsets && d->tabList[selected].dragOffset != 0) {
            if (vertical)
                tab.rect.moveTop(tab.rect.y() + d->tabList[selected].dragOffset);
            else
                tab.rect.moveLeft(tab.rect.x() + d->tabList[selected].dragOffset);
        }
        if (!d->dragInProgress) {
            QString text(tab.text);
            tab.text.clear();
            p.drawControl(QStyle::CE_TabBarTab, tab);

            if ( m_activeColor != "none" )
                fillTabColor(&p, tab, selected, m_activeColor);
            drawTabCaption(&p, text, tab);
        } else {
            int taboverlap = style()->pixelMetric(QStyle::PM_TabBarTabOverlap, 0, this);
            if (verticalTabs(d->shape))
                d->movingTab->setGeometry(tab.rect.adjusted(0, -taboverlap, 0, taboverlap));
            else
                d->movingTab->setGeometry(tab.rect.adjusted(-taboverlap, 0, taboverlap, 0));
        }
    }

    // Only draw the tear indicator if necessary. Most of the time we don't need too.
    if (d->leftB->isVisible() && cutLeft >= 0) {
        cutTabLeft.rect = rect();
        cutTabLeft.rect = style()->subElementRect(QStyle::SE_TabBarTearIndicatorLeft, &cutTabLeft, this);
        p.drawPrimitive(QStyle::PE_IndicatorTabTearLeft, cutTabLeft);
    }

    if (d->rightB->isVisible() && cutRight >= 0) {
        cutTabRight.rect = rect();
        cutTabRight.rect = style()->subElementRect(QStyle::SE_TabBarTearIndicatorRight, &cutTabRight, this);
        p.drawPrimitive(QStyle::PE_IndicatorTabTearRight, cutTabRight);
    }
#endif

}

void CTabBar::resizeEvent(QResizeEvent *event)
{
    QTabBar::resizeEvent(event);
    Q_D(QTabBar);
    m_scrollPos = d->scrollOffset;
    changeCustomScrollerState();
}

void CTabBar::fillTabColor(QPainter * p, const QStyleOptionTab& tab, uint index, const QColor& color)
{
    int border_width = scaling() > 1 ? 2 : 1;

    QRect tabRect(tab.rect);
    tabRect.adjust(-border_width, 0, 0, 0);
    p->fillRect( tabRect, QBrush(QColor(color)) );

    if ( !tabData(index).isNull() && tabData(index).toInt() == TabTheme::LightTab ) {
        if ( !m_isUIThemeDark ) {
            QPen _pen = p->pen();
            _pen.setColor(QColor("#a5a5a5"));
            _pen.setWidth(border_width);

            p->setPen(_pen);
            p->drawLine(tabRect.bottomLeft(), tabRect.topLeft());
            p->drawLine(tabRect.bottomRight(), tabRect.topRight());
        }
    }
}

void CTabBar::setTabTextColor(QPalette::ColorGroup group, const QColor& color)
{
    m_usePalette = true;
    m_palette.setColor(group, QPalette::ButtonText, color);
}

QPalette& CTabBar::customColors()
{
    return m_palette;
}

void CTabBar::setUseTabCustomPalette(bool use)
{
    m_usePalette = use;
}

void CTabBar::tabInserted(int index)
{
    QToolButton * close = new QToolButton(this);
    close->setProperty("class", "tab-close");
    close->setFocusPolicy(Qt::NoFocus);
    close->setFixedSize( QSize(16, 16) * scaling() );
    if ( index == currentIndex() )
        close->setProperty("state", "active"); else
        close->hide();

    connect(close, &QToolButton::clicked, this, &CTabBar::onCloseButton);
    setTabButton(index, QTabBar::RightSide, close);

    CAnimatedIcon * icon = new CAnimatedIcon(this);
    icon->setFixedSize(iconSize());
    setTabButton(index, QTabBar::LeftSide, icon);

    QTabBar::tabInserted(index);
}

void CTabBar::onCurrentChanged(int index)
{
    Q_D(QTabBar);
    m_scrollPos = d->scrollOffset;
    if (this->count() == 0) {
        this->hide();
    } else if (this->isHidden()) {
        this->show();
    }

    QWidget * b = TAB_BTNCLOSE(m_current);
//    if ( tabData(m_current).isNull() )
    {
        if ( b ) {
            b->hide();
//            b->setProperty("state", "normal");
//            b->style()->polish(b);
        }
    }

//    if ( tabData(index).isNull() )
    {
        b = TAB_BTNCLOSE(index);
        if ( b ) {
            b->show();
//            b->setProperty("state", "active");
//            b->style()->polish(b);
        }
    }

    m_current = index;
    changeCustomScrollerState();
}

void CTabBar::changeCustomScrollerState()
{
    // Bypassing the bug with tab scroller
    Q_D(QTabBar);
    if (m_pScrollerFrame && m_pLeftButton && m_pRightButton) {
        if (d->leftB->isVisible()) {
            m_pScrollerFrame->setVisible(true);
        } else {
            m_pScrollerFrame->setVisible(false);
        }
        if (d->leftB->isEnabled()) {
            m_pLeftButton->setEnabled(true);
        } else {
            m_pLeftButton->setEnabled(false);
        }
        if (d->rightB->isEnabled()) {
            m_pRightButton->setEnabled(true);
        } else {
            m_pRightButton->setEnabled(false);
        }
    }
    // End bypassing the bug
}

void CTabBar::tabRemoved(int index)
{
    QWidget * i = TAB_ICON(index);
    if ( i ) {
//        qDebug() << "tab removed: " << index;
    } else {
//        qDebug() << "tab already removed: " << index;
    }

    QTabBar::tabRemoved(index);
}

void CTabBar::setTabIcon(int index, const QIcon &icon)
{
    QWidget * i = TAB_ICON(index);
    if ( i ) {
        QSize _iconSize = iconSize();
        QRect _tabRect = tabRect(index);
        int _top = (_tabRect.height() - _iconSize.height()) / 2;
        double dpi_ratio = scaling();

        ((CAnimatedIcon *)i)->setPixmap(icon.pixmap(_iconSize));
        int top_offset{1};
        if (dpi_ratio < 1.25) top_offset = 1; else
        if (dpi_ratio < 1.5) top_offset = 2; else
        if (dpi_ratio < 1.75) top_offset = 2; else
        if (dpi_ratio < 2) top_offset = 3; else
            top_offset = 4;

        i->setGeometry(QRect(QPoint(_tabRect.left() + 4, _top - top_offset),_iconSize));
        i->setFixedSize(_iconSize.width() + int(8 * dpi_ratio), iconSize().height() + int(4 * dpi_ratio));

        update(_tabRect);
    }
}

void CTabBar::setTabLoading(int index, bool start)
{
    if ( start ) {
//        tabStartLoading(index, index == currentIndex() && m_active ?  "light" : "dark");
    } else {
        CAnimatedIcon * icon = (CAnimatedIcon *)TAB_ICON(index);
        if ( icon ) icon->stop();
    }
}

void CTabBar::tabStartLoading(int index, const QString& theme)
{
    CAnimatedIcon * icon = (CAnimatedIcon *)TAB_ICON(index);
    if ( icon ) {
        if ( !icon->isStarted() )
            icon->startSvg(":/tabbar/icons/loader.svg", theme);
    }
}

void CTabBar::onCloseButton()
{
    QWidget * b = (QWidget *)sender();
    int tabToClose = -1;
    for (int i(0); i < count(); ++i) {
        if ( TAB_BTNCLOSE(i) == b ) {
            tabToClose = i;
            break;
        }
    }

    if ( !(tabToClose < -1) )
        emit tabCloseRequested(tabToClose);
}

void CTabBar::setTabTheme(int index, TabTheme theme)
{
    setTabData(index, theme);

    CAnimatedIcon * i = (CAnimatedIcon *)TAB_ICON(index);
    QWidget * b = TAB_BTNCLOSE(index);
    if ( theme == TabTheme::LightTab ) {
        if ( i ) {
            i->setSvgElement("dark");
        }

        if ( b && !(b->property("state") == "normal") ) {
            b->setProperty("state", "normal");
            b->style()->polish(b);
        }
    } else {
        if ( i ) {
            i->setSvgElement("light");
        }

        if ( b && !(b->property("state") == "active") ) {
            b->setProperty("state", "active");
            b->style()->polish(b);
        }
    }
}

void CTabBar::setUIThemeType(bool islight)
{
    Q_D(QTabBar);

    m_isUIThemeDark = !islight;
    if (m_pLeftButton && m_pRightButton) {
        m_pLeftButton->style()->polish(m_pLeftButton);
        m_pRightButton->style()->polish(m_pRightButton);
    }
    QTimer::singleShot(20, this, [=]() {
        Q_D(QTabBar);
        if (d->scrollOffset != m_scrollPos) {
            const int tabWidth = this->tabSizeHint(0).width();
            if (m_scrollPos % tabWidth == 0) {
                for (int i = 0; i < count(); i++) {
                    if (!d->rightB->isEnabled()) break;
                    d->rightB->click();
                }
                for (int i = 0; i < count(); i++) {
                    if (d->scrollOffset == m_scrollPos) break;
                    d->leftB->click();
                }

            } else {
                for (int i = 0; i < count(); i++) {
                    if (!d->leftB->isEnabled()) break;
                    d->leftB->click();
                }
                for (int i = 0; i < count(); i++) {
                    if (d->scrollOffset == m_scrollPos) break;
                    d->rightB->click();
                }
            }
        }
    });
}

void CTabBar::setActiveTabColor(const QString& color)
{
    m_activeColor = color;
}

void CTabBar::changeTabTheme(int index, TabTheme theme)
{
    if ( tabData(index).isNull() ) {
        CAnimatedIcon * i = (CAnimatedIcon *)TAB_ICON(index);
        QWidget * b = TAB_BTNCLOSE(index);
        if ( theme == TabTheme::LightTab ) {
            if ( i && i->isStarted() ) {
                i->setSvgElement("dark");
            }

            if ( b && !(b->property("state") == "normal") ) {
                b->setProperty("state", "normal");
                b->style()->polish(b);
            }
        } else {
            if ( i && i->isStarted() ) {
                i->setSvgElement("light");
            }

            if ( b && !(b->property("state") == "active") ) {
                b->setProperty("state", "active");
                b->style()->polish(b);
            }
        }
    }
}

void CTabBar::activate(bool a)
{
    if ( m_active != a ) {
        m_active = a;

//        if ( tabData(m_current).isNull() )
        {
            QWidget * b = TAB_BTNCLOSE(m_current);
            if ( b ) {
                if ( a ){
                    b->show();
//                    b->setProperty("state", "active");
                } else {
                    b->hide();
//                    b->setProperty("state", "normal");
                    m_overIndex = -1;
                }

                b->style()->polish(b);
            }
        }
    }
}

void CTabBar::updateScalingFactor(double f)
{
    CScalingWrapper::updateScalingFactor(f);

    for (int i(count()); !(--i < 0); ) {
        QWidget * b = TAB_BTNCLOSE(i);

        if ( b ) {
            b->setFixedSize( QSize(16, 16) * f );
        }
    }

    repaint();
}

bool CTabBar::event(QEvent * e)
{
    if ( e->type() == QEvent::HoverMove ) {
        Q_D(QTabBar);

        if ( !d->dragInProgress ) {
            QHoverEvent * _hover = static_cast<QHoverEvent *>(e);

            int _index = tabAt(_hover->pos());
            if ( m_overIndex != _index ) {
                int _hide(m_overIndex);
                m_overIndex = _index;

                QWidget * b;
                if ( !(_hide < 0) && (!m_active || _hide != currentIndex()) ) {
                    b = TAB_BTNCLOSE(_hide);
                    if ( b ) b->hide();
                }

                if ( !(_index < 0) ) {
                    b = TAB_BTNCLOSE(_index);
                    if ( b ) {
                        b->show();
                    }
                }
            }
        }
    } else
    if ( e->type() == QEvent::Leave ) {
        if ( !(m_overIndex < 0) && (m_overIndex != currentIndex() || !m_active) ) {
            QWidget * b = TAB_BTNCLOSE(m_overIndex);
            if ( b ) {
                b->hide();
            }

            m_overIndex = -1;
        }
    }

//    qDebug() << "event: " << e;
    return QTabBar::event(e);
}

int CTabBar::draggedTabIndex()
{
    Q_D(QTabBar);

    return d->pressedIndex;
}

QSize CTabBar::tabSizeHint(int index) const
{
    return QTabBar::tabSizeHint(index);
}

void CTabBar::interruptTabMoving(int index)
{
    Q_D(QTabBar);

#ifndef QT_NO_ANIMATION
    for (const auto& t : d->tabList) {
        if (t.animation &&
                t.animation->state() == QAbstractAnimation::Running )
            t.animation->stop();
    }
#endif //QT_NO_ANIMATION
    bool cleanup = (d->pressedIndex == index) || (d->pressedIndex == -1) || !d->validIndex(index);
    if (cleanup) {
        if(d->movingTab)
            d->movingTab->setVisible(false); // We might not get a mouse release

        for (auto& t : d->tabList)
            t.dragOffset = 0;

        if (d->pressedIndex != -1 && d->movable) {
            d->pressedIndex = -1;
            d->dragInProgress = false;
            d->dragStartPosition = QPoint();
        }
        d->layoutWidgets();
    } else {
        if (!d->validIndex(index))
            return;
        d->tabList[index].dragOffset = 0;
    }

    update();
}
