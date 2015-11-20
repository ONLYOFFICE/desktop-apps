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

#include "ctabbar.h"
#include "ctabstyle.h"
#include "../common/libs/common/Types.h"

#include "private/qtabbar_p.h"

extern BYTE g_dpi_ratio;

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
    : _title(t), _is_changed(false), _is_closed(false), _panel_id(-1)
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
 *  TabWidget component
 *
*/

CAscTabWidget::CAscTabWidget(QWidget *parent)
    : QTabWidget(parent)
    , m_pMainButton(NULL)
    , m_dataFullScreen(0)
    , m_widthParams({{41, 135, 9}, 108, 3, 0, 100, 140, 0})
{
    CTabBar * tabs = new CTabBar;
    tabs->setObjectName("asc_editors_tabbar");
    tabs->setTabTextColor(QColor(51, 51, 51));
    setTabBar(tabs);

//    tabBar()->setStyle(new CTabStyle);
//    tabBar()->setFixedWidth(450);

    tabBar()->setMovable(true);
    setTabsClosable(true);

    setIconSize(QSize(18*g_dpi_ratio, 10*g_dpi_ratio));
    setProperty("active", false);
    setProperty("empty", true);

    QObject::connect(this, &QTabWidget::currentChanged, [=](){updateIcons();});

    m_widthParams.apply_dpi(g_dpi_ratio);
}

int CAscTabWidget::addEditor(QString strName, AscEditorType etType, std::wstring url)
{
    Q_UNUSED(etType);

    if (!m_pManager || !url.length())
        return -1;

    setProperty("empty", false);

    QCefView* pView = new QCefView(this);
    pView->SetBackgroundCefColor(244, 244, 244);
    pView->setGeometry(0,0, size().width(), size().height() - tabBar()->height());
    pView->Create(m_pManager, cvwtEditor);
    pView->GetCefView()->load(url);
    int id_view = pView->GetCefView()->GetId();

    CAscTabData * data = new CAscTabData(strName);
    data->setViewId(id_view);

    int tab_index = addTab(pView, strName);
    tabBar()->setTabData(tab_index, VPtr<CAscTabData>::asQVariant(data));
    tabBar()->setTabToolTip(tab_index, strName);

    applyDocumentChanging(id_view, etType);

//    emit sendAddEditor();
    resizeEvent(NULL);

    return tab_index;
}

void CAscTabWidget::closeEditor(int i, bool m, bool r)
{
    if (!(i < 0) && i < count()) {
        CAscTabData * doc = VPtr<CAscTabData>::asPtr( tabBar()->tabData(i) );
        if (doc && (!m || !doc->changed())) {
            m_pManager->DestroyCefView(doc->viewId());

            QCefView * view = (QCefView *)widget(i);

            RELEASEOBJECT(doc)
            RELEASEOBJECT(view)

            adjustTabsSize();

            if (r) emit tabClosed(i, tabBar()->count());
        }

        if (r) {
            setProperty("empty", tabBar()->count()==0);
            style()->polish(this);
        }
    }
}

void CAscTabWidget::closeEditorByIndex(int index, bool checkmodified)
{
    closeEditor(index, checkmodified, true);
}

void CAscTabWidget::closeAllEditors()
{
    for (int i = tabBar()->count(); i-- > 0; ) {
        closeEditor(i, false, false);
    }

//    if (!tabBar()->count()) {
//        setProperty("empty", true);
//        style()->polish(this);
//    }
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
//    int nMin = 41 * g_dpi_ratio;    // min tab width
//    int nMax = 135 * g_dpi_ratio;   // max tab width

//    int nFirst = 44 * g_dpi_ratio;          // main button's width
//    int nStartOffset = 5 * g_dpi_ratio;     // offset from main button
//    int nBetweenApp = 5 * g_dpi_ratio;      //
//    int nButtonW = 16 * g_dpi_ratio;        // tool button width
//    int nEndOffset = 140 * g_dpi_ratio;     // space for a caption

    int nControlWidth = parentWidget()->width();
//    int nTabBarWidth = nControlWidth - nFirst - nStartOffset - nEndOffset - 3 * nButtonW - 2 * nBetweenApp;
    int nTabBarWidth = nControlWidth
            - m_widthParams.main_button_width - m_widthParams.main_button_span
            - m_widthParams.title_width - m_widthParams.tools_width - m_widthParams.custom_offset;

    int nCountTabs = tabBar()->count();
    if (nCountTabs == 0) nCountTabs = 1;

    int nTabWidth = (nTabBarWidth - /*(2+2)*/9 * nCountTabs) / nCountTabs;      // magic (2+2)
    if (nTabWidth > m_widthParams.tab.max) nTabWidth = m_widthParams.tab.max;
    if (nTabWidth < m_widthParams.tab.min) nTabWidth = m_widthParams.tab.min;

    int nMinTabBarWidth = (nTabWidth + /*(2+2)*/9) * nCountTabs;
    if (nTabBarWidth > nMinTabBarWidth) nTabBarWidth = nMinTabBarWidth;

    QString cssStyle = styleSheet();
    cssStyle
        .replace(QRegExp("QTabWidget::tab-bar\\s?\\{\\s?width\\:\\s?(\\d+px|auto)", Qt::CaseInsensitive),
                    QString("QTabWidget::tab-bar { width: %1px").arg(nTabBarWidth))
        .replace(QRegExp("QTabBar::tab\\s?\\{\\s?width\\:\\s?\\d+px", Qt::CaseInsensitive),
                    QString("QTabBar::tab { width: %1px").arg(nTabWidth));

//    qDebug() << "styles: " << cssStyle;
    setStyleSheet(cssStyle);
}

void CAscTabWidget::applyCustomTheme(bool iscustom)
{
    m_widthParams.tools_width = (iscustom ? 50 : 140) * g_dpi_ratio;
    m_widthParams.title_width = (iscustom ? 100 : 0) * g_dpi_ratio;
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

void CAscTabWidget::openDocument(std::wstring url, int id, bool select)
{
    if (id > 0) {
        int tabIndex = tabIndexByView(id);
        if (!(tabIndex < 0))
            setCurrentIndex(tabIndex);
    } else {
        int index = addEditor(tr("Document"), etUndefined, url);
        updateIcons();

        if (select && !(index < 0))
            tabBar()->setCurrentIndex(index);
    }
}

void CAscTabWidget::applyDocumentChanging(int viewId, QString name)
{
    int tabIndex = tabIndexByView(viewId);

    if (!(tabIndex < 0)) {
        CAscTabData * doc = VPtr<CAscTabData>::asPtr( tabBar()->tabData(tabIndex) );
        doc->setTitle(name);
        tabBar()->setTabText(tabIndex, doc->title());
        tabBar()->setTabToolTip(tabIndex, name);
    }
}

void CAscTabWidget::applyDocumentChanging(int viewId, bool state)
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
            closeEditor(tabIndex, false, true);
        }
    }
}

void CAscTabWidget::applyDocumentChanging(int id, int type)
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
        style()->polish(tabBar());
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
            tabBar()->setCurrentIndex(index);

            RELEASEOBJECT(m_dataFullScreen)

//            updateGeometry();
        }
    } else {
        fsWidget = currentWidget();

        if (fsWidget) {
            m_dataFullScreen = new CFullScreenData(currentIndex(),
                    fsWidget, VPtr<void>::asPtr(tabBar()->tabData(currentIndex())));

            removeTab(currentIndex());
#ifdef _WIN32

//            QIcon icon(":/res/icons/desktop_icons.ico");
//            HICON hIcon = qt_pixmapToWinHICON(QSysInfo::windowsVersion() == QSysInfo::WV_XP ?
//                                                icon.pixmap(icon.availableSizes().first()) : icon.pixmap(QSize(32,32)) );
//            SendMessage((HWND)fsWidget->winId(), WM_SETICON, ICON_BIG, (LPARAM)hIcon);

            fsWidget->setParent(nullptr);
#else
            QWidget * grandpa = qobject_cast<QWidget *>(parent()->parent());
            if (grandpa) fsWidget->setParent(grandpa);
#endif
            fsWidget->showFullScreen();

            QPoint pt = mapToGlobal(pos());
            fsWidget->setGeometry(QApplication::desktop()->screenGeometry(pt));
        }
    }
}
