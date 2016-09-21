/*
 * (c) Copyright Ascensio System SIA 2010-2016
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
 * You can contact Ascensio System SIA at Lubanas st. 125a-25, Riga, Latvia,
 * EU, LV-1021.
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

#include "ctabbar.h"
//#include "ctabbar_p.h"
#include "private/qtabbar_p.h"
#include <QStylePainter>

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
    , m_capColor("nocolor")
{
    setDrawBase(false);
}

CTabBar::~CTabBar()
{

}

void CTabBar::mouseMoveEvent(QMouseEvent * event)
{
    Q_D(QTabBar);

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

void CTabBar::drawTabCaption(QPainter * p, const QString& s, const QStyleOptionTab& t)
{
    if (m_capColor.name() != "nocolor")
        p->setPen(QPen(m_capColor));

    QRect trect(QPoint(t.rect.left() + t.iconSize.width() + 6, t.rect.top()),
                    QPoint(t.rect.right() - 22, t.rect.bottom() - 2));

    QFont f = font();
    f.setPointSize(8);

    p->setFont(f);

    QString es = fontMetrics().elidedText(s, Qt::ElideRight, trect.width(), Qt::TextShowMnemonic);
    p->drawText(trect, Qt::AlignVCenter, es);
}

void CTabBar::paintEvent(QPaintEvent * event)
{
    Q_D(QTabBar);

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

        QString text = tab.text;
        tab.text = "";
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
            QString text = tab.text;
            tab.text = "";
            p.drawControl(QStyle::CE_TabBarTab, tab);
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
}

void CTabBar::setTabTextColor(const QColor& c)
{
    m_capColor.setNamedColor("");
    m_capColor = c;
}
