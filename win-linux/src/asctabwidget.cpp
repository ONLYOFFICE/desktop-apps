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
#include <QFileInfo>

#include "ctabbar.h"
#include "ctabstyle.h"
#include "casctabdata.h"
#include "../common/libs/common/Types.h"
#include "defines.h"

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


/*
 *
 * COpenOptions structure definition
 *
*/

COpenOptions::COpenOptions() :
    type(etUndefined), id(-1)
{}

COpenOptions::COpenOptions(wstring _url_) :
    COpenOptions(_url_, etUndefined, -1)
{}

COpenOptions::COpenOptions(wstring _url_, AscEditorType _type_) :
    COpenOptions(_url_, _type_, -1)
{}

COpenOptions::COpenOptions(wstring _url_, AscEditorType _type_, int _id_) :
    type(_type_), url(QString::fromStdWString(_url_)), id(_id_), wurl(_url_)
{}

COpenOptions::COpenOptions(QString _name_, AscEditorType _type_, QString _url_) :
    COpenOptions(_name_, _type_, _url_, -1)
{}

COpenOptions::COpenOptions(QString _name_, AscEditorType _type_, QString _url_, int _id_) :
    name(_name_), type(_type_), url(_url_), id(_id_)
{}

COpenOptions::COpenOptions(QString _name_, AscEditorType _type_, std::wstring _url_, int _id_) :
    name(_name_), type(_type_), url(QString::fromStdWString(_url_)), id(_id_), wurl(_url_)
{}

COpenOptions::COpenOptions(QString _name_, AscEditorType _type_) :
    COpenOptions(_name_, _type_, "")
{}

/*
 *  TabWidget component
 *
*/

CAscTabWidget::CAscTabWidget(QWidget *parent)
    : QTabWidget(parent)
    , m_pMainButton(NULL)
    , m_dataFullScreen(0)
    , m_widthParams({{100, 135, 9}, 108, 3, 0, WINDOW_TITLE_MIN_WIDTH, 140, 0})
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

    QObject::connect(this, &QTabWidget::currentChanged, [=](){updateIcons(); setFocusedView();});

    m_widthParams.apply_dpi(g_dpi_ratio);
}

int CAscTabWidget::addEditor(COpenOptions& opts)
{
    if (!m_pManager ||
            (!(opts.url.length() > 0) && opts.type != etNewFile))
        return -1;

    setProperty("empty", false);

    int file_format = 0;
    if (opts.type == etLocalFile) {
        file_format = CCefViewEditor::GetFileFormat(opts.wurl);
        if (file_format == 0)
            /* TODO: show error for file format */
            return -255;
    }

    QCefView* pView = new QCefView(this);
    pView->SetBackgroundCefColor(244, 244, 244);
    pView->setGeometry(0,0, size().width(), size().height() - tabBar()->height());
    pView->Create(m_pManager, cvwtEditor);

    int tab_index = -1;
    bool res_open = true;
    CCefView * cview = pView->GetCefView();    
    if (opts.type == etLocalFile) {
        ((CCefViewEditor*)cview)->OpenLocalFile(opts.wurl, file_format);
        opts.type = etUndefined;
    } else
    if (opts.type == etRecoveryFile) {
        res_open = ((CCefViewEditor*)cview)->OpenRecoverFile(opts.id);
        opts.type = etUndefined;
    } else
    if (opts.type == etRecentFile) {
        res_open = ((CCefViewEditor*)cview)->OpenRecentFile(opts.id);
        opts.type = etUndefined;
    } else
    if (opts.type == etNewFile) {
        ((CCefViewEditor*)cview)->CreateLocalFile(opts.format, opts.name.toStdWString());
        opts.type = AscEditorType(opts.format);
    } else {
        cview->load(opts.wurl);
    }

    if (res_open) {
        int id_view = cview->GetId();

        CAscTabData * data = new CAscTabData(opts.name);
        data->setViewId(id_view);
        data->setUrl(opts.wurl);
    //    data->setLocal(etType == etLocalFile);

        tab_index = addTab(pView, opts.name);
        tabBar()->setTabData(tab_index, VPtr<CAscTabData>::asQVariant(data));
        tabBar()->setTabToolTip(tab_index, opts.name);

        applyDocumentChanging(id_view, opts.type);
        resizeEvent(NULL);
    } else {
        RELEASEOBJECT(pView)
    }

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

int CAscTabWidget::addPortal(QString url)
{
    Q_UNUSED(url);

    if (!m_pManager || !url.length())
        return -1;

    setProperty("empty", false);

    QCefView* pView = new QCefView(this);
    pView->SetBackgroundCefColor(244, 244, 244);
    pView->setGeometry(0,0, size().width(), size().height() - tabBar()->height());
    pView->Create(m_pManager, cvwtSimple);
    pView->GetCefView()->load((url + "/products/files/?desktop=true").toStdWString());
    int id_view = pView->GetCefView()->GetId();

    QRegularExpression re(rePortalName);
    QRegularExpressionMatch match = re.match(url);
    QString portal = match.hasMatch() ? match.captured(1) : url;

    CAscTabData * data = new CAscTabData(portal, cvwtSimple);
    data->setViewId(id_view);

    /* find out the index of the last portal's tab */
//    CAscTabData * doc;
//    int tab_index = 0;
//    for (int i(count()); i-- > 0; ) {
//        doc = VPtr<CAscTabData>::asPtr(tabBar()->tabData(i));

//        if (doc && doc->viewType() == cvwtSimple)
//            tab_index = i;
//    }
    int tab_index = -1;

    tab_index = insertTab(tab_index, pView, portal);
    tabBar()->setTabData(tab_index, VPtr<CAscTabData>::asQVariant(data));
    tabBar()->setTabToolTip(tab_index, url);

//    updateTabIcon(tabIndexByView(id));

    resizeEvent(NULL);
    return tab_index;
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

    int nTabBarWidth    = 0,
        nTabWidth       = m_widthParams.tab.max,
        nCountTabs      = tabBar()->count();

    if (nCountTabs != 0) {
        int nControlWidth = parentWidget()->width();
        nTabBarWidth = nControlWidth
                - m_widthParams.main_button_width - m_widthParams.main_button_span
                - m_widthParams.title_width - m_widthParams.tools_width - m_widthParams.custom_offset;

        int nTabWidth = (nTabBarWidth - /*(2+2)*/9 * nCountTabs) / nCountTabs;      // magic (2+2)
        if (nTabWidth > m_widthParams.tab.max) nTabWidth = m_widthParams.tab.max;
        if (nTabWidth < m_widthParams.tab.min) nTabWidth = m_widthParams.tab.min;

        int nMinTabBarWidth = (nTabWidth + /*(2+2)*/(9 * g_dpi_ratio/*?*/)) * nCountTabs;
        if (nTabBarWidth > nMinTabBarWidth) nTabBarWidth = nMinTabBarWidth;
    }

    QString cssStyle = styleSheet();
    cssStyle
        .replace(QRegExp("QTabWidget::tab-bar\\s?\\{\\s?width\\:\\s?(\\-?\\d+px|auto)", Qt::CaseInsensitive),
                    QString("QTabWidget::tab-bar { width: %1px").arg(nTabBarWidth))
        .replace(QRegExp("QTabBar::tab\\s?\\{\\s?width\\:\\s?\\d+px", Qt::CaseInsensitive),
                    QString("QTabBar::tab { width: %1px").arg(nTabWidth));

//    qDebug() << "styles: " << cssStyle;
    setStyleSheet(cssStyle);
}

void CAscTabWidget::applyCustomTheme(bool iscustom)
{
    m_widthParams.tools_width = (iscustom ? 50 : 140) * g_dpi_ratio;
    m_widthParams.title_width = (iscustom ? WINDOW_TITLE_MIN_WIDTH : 0) * g_dpi_ratio;
}

void CAscTabWidget::updateIcons()
{
    QString icon_name;
    int current = isActive() ? tabBar()->currentIndex() : -1;
    for (int i(count()); i-- > 0;) {
        CCefViewEditor * pEditor = (CCefViewEditor *)((QCefView*)(widget(i)))->GetCefView();

        if (pEditor) {
            if (pEditor->GetType() == cvwtSimple) {
                icon_name = ":/portal.png";
            } else {
                switch (pEditor->GetEditorType()) {
                case etPresentation: icon_name = i == current ? ":/pe_active.png" : ":/pe_normal.png"; break;
                case etSpreadsheet:  icon_name = i == current ? ":/se_active.png" : ":/se_normal.png"; break;
                case etDocument:     icon_name = i == current ? ":/de_active.png" : ":/de_normal.png"; break;
                default:             icon_name = ":/newdocument.png"; break;
                }
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
            QString icon_name;
            if (pEditor->GetType() == cvwtSimple) {
                icon_name = ":/portal.png";
            } else {
                bool is_active = isActive() && index == currentIndex();

                switch (pEditor->GetEditorType()) {
                case etPresentation: icon_name = is_active ? ":/pe_active.png" : ":/pe_normal.png"; break;
                case etSpreadsheet:  icon_name = is_active ? ":/se_active.png" : ":/se_normal.png"; break;
                case etDocument:     icon_name = is_active ? ":/de_active.png" : ":/de_normal.png"; break;
                default:             icon_name = ":/newdocument.png"; break;
                }
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

int CAscTabWidget::tabIndexByTitle(QString t, CefType vt)
{
    CAscTabData * doc;
    for (int i(count()); i-- > 0; ) {
        doc = VPtr<CAscTabData>::asPtr(tabBar()->tabData(i));

        if (doc && doc->viewType() == vt && doc->title() == t)
            return i;
    }

    return -1;
}

void CAscTabWidget::openCloudDocument(COpenOptions& opts, bool select)
{
    int tabIndex;
    if (opts.id > 0) {
        tabIndex = tabIndexByView(opts.id);
        if (!(tabIndex < 0))
            setCurrentIndex(tabIndex);
    } else {
        opts.name   = tr("Document");
        opts.type   = etUndefined;
        tabIndex    = addEditor(opts);

        updateIcons();

        if (select && !(tabIndex < 0))
            tabBar()->setCurrentIndex(tabIndex);
    }
}

int CAscTabWidget::openLocalDocument(COpenOptions& opts, bool select)
{
    QString name = QFileInfo(opts.url).fileName();
    int tabIndex = tabIndexByTitle(name, cvwtEditor);

    if (tabIndex < 0){
        opts.name = name;
        tabIndex = addEditor(opts);

        if (!(tabIndex < 0))
            updateIcons();
    }

    if (select && !(tabIndex < 0))
        tabBar()->setCurrentIndex(tabIndex);

    /* TODO: rise message if index < 0 */

    return tabIndex;
}

int CAscTabWidget::openPortal(const QString& url)
{
    int out_val = 1;

    QRegularExpression re(rePortalName);
    QRegularExpressionMatch match = re.match(url);
    QString portal_name = match.hasMatch() ? match.captured(1) : url;

    int tabIndex = tabIndexByTitle(portal_name, cvwtSimple);
    if (tabIndex < 0) {
        tabIndex = addPortal(url), out_val = 2;
    }

    setCurrentIndex(tabIndex);
    return out_val;
}

void CAscTabWidget::closePortal(const QString& name, bool editors)
{
    closeEditorByIndex(tabIndexByTitle(name, cvwtSimple));

    if (editors) {
        wstring wname = name.toStdWString();
        CAscTabData * doc;
        for (int i = tabBar()->count(); i-- > 0; ) {
            doc = VPtr<CAscTabData>::asPtr( tabBar()->tabData(i) );

            if (doc->viewType() == cvwtEditor &&
                    doc->url().find(wname) != wstring::npos)
            {
                closeEditor(i, false, false);
            }
        }
    }
}

void CAscTabWidget::applyDocumentChanging(int viewId, const QString& name, const QString& descr)
{
    int tabIndex = tabIndexByView(viewId);

    if (!(tabIndex < 0)) {
        CAscTabData * doc = VPtr<CAscTabData>::asPtr( tabBar()->tabData(tabIndex) );
        doc->setTitle(name);
        tabBar()->setTabText(tabIndex, doc->title());
        tabBar()->setTabToolTip(tabIndex, descr);
    }
}

void CAscTabWidget::applyDocumentChanging(int viewId, bool state)
{
    int tabIndex = tabIndexByView(viewId);
    if (!(tabIndex < 0)) {
        CAscTabData * doc = VPtr<CAscTabData>::asPtr( tabBar()->tabData(tabIndex) );
        if (doc->changed() != state && (!doc->closed() || state)) {
            doc->setChanged(state);
            tabBar()->setTabText(tabIndex, doc->title());
            tabBar()->setTabToolTip(tabIndex, doc->title());
        }
    }
}

void CAscTabWidget::applyDocumentSave(int id, bool cancel)
{
    int tabIndex = tabIndexByView(id);
    if (!(tabIndex < 0)) {
        CAscTabData * doc = VPtr<CAscTabData>::asPtr( tabBar()->tabData(tabIndex) );
        if (doc->closed()) {
            cancel ? doc->reuse() : closeEditor(tabIndex, false, true);
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
        return doc->changed() && !doc->closed();
    }

    return false;
}

bool CAscTabWidget::closedByIndex(int index) {
    if (!(index < 0) && index < count()) {
        CAscTabData * doc = VPtr<CAscTabData>::asPtr( tabBar()->tabData(index) );
        return doc->closed();
    }

    return true;
}

MapEditors CAscTabWidget::modified(const QString& portalname)
{
    QMap<int, QString> mapModified;
    wstring portal = portalname.toStdWString();
    CAscTabData * doc;
    for (int i(tabBar()->count()); i-- > 0; i++) {
        doc = VPtr<CAscTabData>::asPtr( tabBar()->tabData(i) );

        if (doc->isViewType(cvwtEditor) &&
                doc->changed() && !doc->closed() &&
                (portal.length() == 0 || doc->url().find(portal) != wstring::npos))
        {
            mapModified.insert(viewByIndex(i), titleByIndex(i, true));
        }
    }

    return mapModified;
}

int CAscTabWidget::findModified(const QString& portalname)
{
    wstring portal = portalname.toStdWString();
    CAscTabData * doc;
    for (int i(tabBar()->count()); i-- > 0; ) {
        doc = VPtr<CAscTabData>::asPtr( tabBar()->tabData(i) );

        if ( doc->isViewType(cvwtEditor) &&
                doc->changed() && !doc->closed() &&
                (portal.length() == 0 || doc->url().find(portal) != wstring::npos))
        {
            return i;
        }
    }

    return -1;
}

void CAscTabWidget::setFullScreen(bool apply)
{
    QWidget * fsWidget;
    static QMetaObject::Connection cefConnection;
    if (!apply) {
        if (m_dataFullScreen) {
            fsWidget = m_dataFullScreen->widget();
            fsWidget->showNormal();

            disconnect(cefConnection);

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
  #ifdef _IVOLGA_PRO
            fsWidget->setWindowIcon(QIcon(":/ivolga/app.ico"));
  #else
            fsWidget->setWindowIcon(QIcon(":/res/icons/desktopeditors.ico"));
  #endif
            fsWidget->setParent(nullptr);
#else
            QWidget * grandpa = qobject_cast<QWidget *>(parent()->parent());
            if (grandpa) fsWidget->setParent(grandpa);
#endif
            fsWidget->showFullScreen();

            cefConnection = connect((QCefView *)fsWidget, &QCefView::closeWidget, [=](QCloseEvent * e){
                e->ignore();

                NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                pCommand->put_Command(L"editor:stopDemonstration");

                NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EDITOR_EXECUTE_COMMAND);
                pEvent->m_pData = pCommand;
                ((QCefView *)fsWidget)->GetCefView()->Apply(pEvent);

                emit closeAppRequest();
            });

            QPoint pt = mapToGlobal(pos());
            fsWidget->setGeometry(QApplication::desktop()->screenGeometry(pt));
        }
    }
}
