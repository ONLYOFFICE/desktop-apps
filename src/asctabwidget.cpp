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

#include "asctabwidget.h"
#include <QRegExp>

#include <QDebug>

#include <QHBoxLayout>
#include <QLabel>
#include <QStylePainter>
#include <QDesktopWidget>
#include "csavefilemessage.h"
#include "defines.h"

#include "private/qtabbar_p.h"

extern byte g_dpi_ratio;

/*
 *
 *  Tab data
 *
*/

template <class T> class VPtr
{
public:
    static T * asPtr(QVariant v) {return  (T *) v.value<void *>();}
    static QVariant asQVariant(T * ptr){return qVariantFromValue((void *) ptr);}
};

struct CAscTabData
{
public:
    CAscTabData(const QString &);
    ~CAscTabData() {}

    void    setTitle(const QString&);
    void    setChanged(bool);
    void    setViewId(int);
    void    close();
    QString title(bool orig = false) const;
    bool    changed() const;
    int     viewId() const;
    bool    closed() const;
private:
    QString _title;
    bool _is_changed;
    bool _is_closed;
    int _panel_id;
};

CAscTabData::CAscTabData(const QString& t)
    : _is_changed(false), _is_closed(false), _title(t), _panel_id(-1)
{}

void CAscTabData::setTitle(const QString& t)
{
    _title = t;
}

QString CAscTabData::title(bool orig) const
{
    return !orig && _is_changed ? _title + "*": _title;
}

void CAscTabData::setChanged(bool s)
{
    _is_changed = s;
}

bool CAscTabData::changed() const
{
    return _is_changed;
}

void CAscTabData::setViewId(int id)
{
    _panel_id = id;
}

int CAscTabData::viewId() const
{
    return _panel_id;
}

void CAscTabData::close() {
    _is_closed = true;
}

bool CAscTabData::closed() const
{
    return _is_closed;
}

/*
*
*   TabBar component
*
*/

inline static bool verticalTabs(QTabBar::Shape shape)
{
    return shape == QTabBar::RoundedWest || shape == QTabBar::RoundedEast
           || shape == QTabBar::TriangularWest || shape == QTabBar::TriangularEast;
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
                p.setY(p.y() + tab.dragOffset); else
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
        movingTab = new QWidget(q);

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
    ((CAscTabBar*)q)->drawTabCaption(&p, text, tab);
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
 *     CAscTabBar
*/

CAscTabBar::CAscTabBar(QWidget *parent)
    : QTabBar(parent), m_capColor("nocolor")
{
    setDrawBase(false);
}

CAscTabBar::~CAscTabBar()
{
}

void CAscTabBar::mouseMoveEvent (QMouseEvent * event)
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
//#ifdef Q_WS_MAC
//    } else if (!d->documentMode && event->buttons() == Qt::LeftButton && d->previousPressedIndex != -1) {
//        int newPressedIndex = d->indexAtPos(event->pos());
//        if (d->pressedIndex == -1 && d->previousPressedIndex == newPressedIndex) {
//            d->pressedIndex = d->previousPressedIndex;
//            update(tabRect(d->pressedIndex));
//        } else if(d->pressedIndex != newPressedIndex) {
//            d->pressedIndex = -1;
//            update(tabRect(d->previousPressedIndex));
//        }
//#endif
    }

    if (event->buttons() != Qt::LeftButton) {
        event->ignore();
        return;
    }

    QStyleOptionTabBarBaseV2 optTabBase;
    optTabBase.init(this);
    optTabBase.documentMode = d->documentMode;
}

void CAscTabBar::drawTabCaption(QPainter * p, const QString& s, const QStyleOptionTab& t)
{
    if (m_capColor.name() != "nocolor")
        p->setPen(QPen(m_capColor));

    QRect trect = t.rect;
    trect.setLeft(t.rect.left() + t.iconSize.width() + 6);
    trect.setWidth(trect.width() - 26);
    p->setFont(font());

    QString es = fontMetrics().elidedText(s, Qt::ElideRight, trect.width(), Qt::TextShowMnemonic);

    p->drawText(trect, Qt::AlignVCenter, es);
}

void CAscTabBar::paintEvent(QPaintEvent * event)
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

void CAscTabBar::setTabTextColor(const QColor& c)
{
    m_capColor.setNamedColor("");
    m_capColor = c;
}


/*
 *  TabWidget component
 *
*/

CAscTabWidget::CAscTabWidget(QWidget *parent)
    : QTabWidget(parent), m_pMainButton(NULL),
      m_dataFullScreen(0)
{
    CAscTabBar * tabs = new CAscTabBar;
    tabs->setObjectName("asc_editors_tabbar");
    tabs->setTabTextColor(QColor(51, 51, 51));
    setTabBar(tabs);

    tabBar()->setMovable(true);
    setTabsClosable(true);

    QFile styleFile( ":/sep-styles/tabbar.qss" );
    if (g_dpi_ratio > 1) styleFile.setFileName(":/styles@2x/tabbar.qss");
    styleFile.open( QFile::ReadOnly );
    setStyleSheet(QString(styleFile.readAll()));

    setIconSize(QSize(18*g_dpi_ratio, 10*g_dpi_ratio));
    setProperty("active", false);

    QObject::connect(this, &QTabWidget::currentChanged, [=](){updateIcons();});
}

int CAscTabWidget::addEditor(QString strName, AscEditorType etType, std::wstring url)
{
    Q_UNUSED(etType);

    if (!m_pManager || !url.length())
        return -1;

    QCefView* pView = new QCefView(this);
    pView->SetBackgroundCefColor(244, 244, 244);
    pView->setGeometry(0,0, size().width(), size().height() - tabBar()->height());
    pView->Create(m_pManager, cvwtEditor);
    pView->GetCefView()->load(url);
    int id_view = pView->GetCefView()->GetId();

    CAscTabData * data = new CAscTabData(strName);
    data->setViewId(id_view);

    int tab_index = this->addTab(pView, strName);
    tabBar()->setTabData(tab_index, VPtr<CAscTabData>::asQVariant(data));
    tabBar()->setTabToolTip(tab_index, strName);

    onDocumentType(id_view, etType);

//    emit sendAddEditor();
    resizeEvent(NULL);

    return tab_index;
}

void CAscTabWidget::closeEditorByIndex(int index, bool checkmodified)
{
    if (!(index < 0) && index < count()) {
        CAscTabData * doc = VPtr<CAscTabData>::asPtr( tabBar()->tabData(index) );
        if (doc && (!checkmodified || !doc->changed())) {
            m_pManager->DestroyCefView(doc->viewId());

            QCefView * view = (QCefView *)widget(index);

            RELEASEOBJECT(doc)
            RELEASEOBJECT(view)

            adjustTabsSize();

            emit tabClosed(index, tabBar()->count());
        }
    }
}

void CAscTabWidget::closeAllEditors()
{
    for (int i = tabBar()->count(); i-- > 0; ) {
        closeEditorByIndex(i);
    }
}

void CAscTabWidget::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e);

    adjustTabsSize();

    if (e) {
        int w = e->size().width(),
            h = e->size().height() - tabBar()->height();

        CCefView * view;
        for (int i(count()); i > 0;) {
            if (--i != currentIndex()) {
                view = ((QCefView *)widget(i))->GetCefView();

                if (view) view->resizeEvent(w, h);
            }
        }
    }
}

void CAscTabWidget::adjustTabsSize()
{
    int nMin = 41 * g_dpi_ratio;
    int nMax = 135 * g_dpi_ratio;

    int nFirst = 44 * g_dpi_ratio;
    int nStartOffset = 5 * g_dpi_ratio;
    int nBetweenApp = 5 * g_dpi_ratio;
    int nButtonW = 16 * g_dpi_ratio;
    int nEndOffset = 140 * g_dpi_ratio;

    int nControlWidth = this->parentWidget()->width();
    int nTabBarWidth = nControlWidth - nFirst - nStartOffset - nEndOffset - 3 * nButtonW - 2 * nBetweenApp;

    int nCountTabs = this->tabBar()->count();
    if (nCountTabs == 0)
        nCountTabs = 1;

    int nTabWidth = (nTabBarWidth - (2 + 2) * nCountTabs) / nCountTabs;
    if (nTabWidth > nMax) nTabWidth = nMax;
    if (nTabWidth < nMin) nTabWidth = nMin;

    int nMinTabBarWidth = (nTabWidth + 2 + 2) * nCountTabs;
    if (nTabBarWidth > nMinTabBarWidth)
        nTabBarWidth = nMinTabBarWidth;

    QString cssStyle = styleSheet();
    cssStyle
        .replace(QRegExp("QTabWidget::tab-bar\\s?\\{\\s?width\\:\\s?(\\d+px|auto)", Qt::CaseInsensitive),
                    QString("QTabWidget::tab-bar { width: %1px").arg(nTabBarWidth))
        .replace(QRegExp("QTabBar::tab\\s?\\{\\s?width\\:\\s?\\d+px", Qt::CaseInsensitive),
                    QString("QTabBar::tab { width: %1px").arg(nTabWidth));

//    qDebug() << "styles: " << cssStyle;
    setStyleSheet(cssStyle);
}

void CAscTabWidget::updateIcons()
{
    QString icon_name;
    int current = isActive() ? tabBar()->currentIndex() : -1;
    for (int i(count()); i-- > 0;) {
        CCefViewEditor * pEditor = (CCefViewEditor *)((QCefView*)(widget(i)))->GetCefView();

        if (pEditor) {
            switch (pEditor->GetEditorType()) {
            case etPresentation: icon_name = i == current ? ":/pe_active.png" : ":/pe_normal.png"; break;
            case etSpreadsheet:  icon_name = i == current ? ":/se_active.png" : ":/se_normal.png"; break;
            case etDocument:     icon_name = i == current ? ":/de_active.png" : ":/de_normal.png"; break;
            default:             icon_name = ":/newdocument.png"; break;
            }

            if (g_dpi_ratio > 1)
                icon_name.replace(".png", "@2x.png");

            tabBar()->setTabIcon(i, QIcon(icon_name));
        }
    }
}

void CAscTabWidget::updateTabIcon(int index)
{
    if ( !(index < 0) ) {
        CCefViewEditor * pEditor = (CCefViewEditor *)((QCefView*)(widget(index)))->GetCefView();

        if (pEditor) {
            bool is_active = isActive() && index == currentIndex();
            QString icon_name;

            switch (pEditor->GetEditorType()) {
            case etPresentation: icon_name = is_active ? ":/pe_active.png" : ":/pe_normal.png"; break;
            case etSpreadsheet:  icon_name = is_active ? ":/se_active.png" : ":/se_normal.png"; break;
            case etDocument:     icon_name = is_active ? ":/de_active.png" : ":/de_normal.png"; break;
            default:             icon_name = ":/newdocument.png"; break;
            }

            if (g_dpi_ratio > 1)
                icon_name.replace(".png", "@2x.png");

            tabBar()->setTabIcon(index, QIcon(icon_name));
        }
    }
}

/*
 *      Slots
*/


void CAscTabWidget::editorCloseRequest(int index)
{
    VPtr<CAscTabData>::asPtr(tabBar()->tabData(index))
        ->close();
}

int CAscTabWidget::tabIndexByView(int viewId)
{
    CAscTabData * doc;
    for (int i(count()); i-- > 0; ) {
        doc = VPtr<CAscTabData>::asPtr(tabBar()->tabData(i));

        if (doc && doc->viewId() == viewId)
            return i;
    }

    return -1;
}

void CAscTabWidget::onDocumentOpen(std::wstring path, bool select, int id)
{
    if (id > 0) {
        int tabIndex = tabIndexByView(id);
        if (!(tabIndex < 0))
            setCurrentIndex(tabIndex);
    } else {
        int index = addEditor(tr("Document"), etUndefined, path);
        updateIcons();

        if (select && !(index < 0))
            tabBar()->setCurrentIndex(index);
    }
}

void CAscTabWidget::onDocumentNameChanged(int viewId, QString name)
{
    int tabIndex = tabIndexByView(viewId);

    if (!(tabIndex < 0)) {
        CAscTabData * doc = VPtr<CAscTabData>::asPtr( tabBar()->tabData(tabIndex) );
        doc->setTitle(name);
        tabBar()->setTabText(tabIndex, doc->title());
        tabBar()->setTabToolTip(tabIndex, name);
    }
}

void CAscTabWidget::onDocumentChanged(int viewId, bool state)
{
    int tabIndex = tabIndexByView(viewId);
    if (!(tabIndex < 0)) {
        CAscTabData * doc = VPtr<CAscTabData>::asPtr( tabBar()->tabData(tabIndex) );
        if (doc->changed() != state) {
            doc->setChanged(state);
            tabBar()->setTabText(tabIndex, doc->title());
            tabBar()->setTabToolTip(tabIndex, doc->title());
        }
    }
}

void CAscTabWidget::onDocumentSave(int viewId)
{
    int tabIndex = tabIndexByView(viewId);
    if (!(tabIndex < 0)) {
        CAscTabData * doc = VPtr<CAscTabData>::asPtr( tabBar()->tabData(tabIndex) );
        if (doc->closed()) {
            closeEditorByIndex(tabIndex, false);
        }
    }
}

void CAscTabWidget::onDocumentType(int id, int type)
{
    CCefView * pView = m_pManager->GetViewById(id);
    if (NULL != pView && pView->GetType() == cvwtEditor) {
        ((CCefViewEditor *)pView)->SetEditorType(AscEditorType(type));
    }

    updateTabIcon(tabIndexByView(id));
}

/*
 *      Slots description end
*/

void CAscTabWidget::setFocusedView(int index)
{
    int nIndex = !(index < 0) ? index : currentIndex();
    if (!(nIndex < 0 ))
        ((QCefView *)this->widget(nIndex))->GetCefView()->focus();
}

void CAscTabWidget::activate(bool a)
{
    if (property("active").toBool() != a) {
        setProperty("active", a);
        setStyleSheet(styleSheet());
    }

    QString strVal = a ? "normal" : "active";
    if (m_pMainButton != NULL && m_pMainButton->property("class") != strVal) {
        m_pMainButton->setProperty("class", strVal);
        m_pMainButton->style()->polish(m_pMainButton);
        m_pMainButton->update();
    }

    updateTabIcon(currentIndex());
}

bool CAscTabWidget::isActive()
{
    return property("active").toBool();
}

int CAscTabWidget::modifiedCount()
{
    int mod_count = 0;
    CAscTabData * doc;

    for (int i = tabBar()->count(); i-- > 0; ) {
        doc = VPtr<CAscTabData>::asPtr( tabBar()->tabData(i) );
        doc->changed() && mod_count++;
    }

    return mod_count;
}

int CAscTabWidget::viewByIndex(int index)
{
    if (!(index < 0) && index < count()) {
        CCefView * view = ((QCefView*)widget(index))->GetCefView();
        return view ? view->GetId() : -1;
    }

    return -1;
}

QString CAscTabWidget::titleByIndex(int index, bool mod)
{
    if (!(index < 0) && index < count()) {
        CAscTabData * doc = VPtr<CAscTabData>::asPtr( tabBar()->tabData(index) );
        if (doc)
            return doc->title(mod);
    }

    return "";
}

bool CAscTabWidget::modifiedByIndex(int index)
{
    if (!(index < 0) && index < count()) {
        CAscTabData * doc = VPtr<CAscTabData>::asPtr( tabBar()->tabData(index) );
        return doc->changed();
    }

    return false;
}

void CAscTabWidget::setFullScreen(bool apply)
{
    QWidget * fsWidget;
    if (!apply) {
        if (m_dataFullScreen) {
            fsWidget = m_dataFullScreen->widget();
            fsWidget->showNormal();

            int index = m_dataFullScreen->tabindex();
            CAscTabData * doc = (CAscTabData *)m_dataFullScreen->data();

            insertTab(index, fsWidget, doc->title());
            tabBar()->setTabData(index, VPtr<CAscTabData>::asQVariant(doc));
            tabBar()->setTabToolTip(index, doc->title());

            RELEASEOBJECT(m_dataFullScreen)

//            updateGeometry();
        }
    } else {
        fsWidget = currentWidget();

        if (fsWidget) {
            m_dataFullScreen = new CFullScreenData(currentIndex(),
                    fsWidget, VPtr<void>::asPtr(tabBar()->tabData(currentIndex())));

            removeTab(currentIndex());

            fsWidget->setParent(0);
            fsWidget->showFullScreen();

            QPoint pt = mapToGlobal(pos());
            fsWidget->setGeometry(QApplication::desktop()->screenGeometry(pt));
        }
    }
}
