/*
 * (c) Copyright Ascensio System SIA 2010-2017
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
#include <QTimer>

#include "ctabbar.h"
#include "ctabstyle.h"
#include "casctabdata.h"
#include "common/Types.h"
#include "defines.h"
#include "utils.h"
#include "cfilechecker.h"
#include "canimatedicon.h"

#include "cascapplicationmanagerwrapper.h"
#include "ctabundockevent.h"

#include "private/qtabbar_p.h"

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
    COpenOptions(QString(), _type_, QString::fromStdWString(_url_), _id_)
{}

COpenOptions::COpenOptions(QString _name_, AscEditorType _type_, QString _url_) :
    COpenOptions(_name_, _type_, _url_, -1)
{}

COpenOptions::COpenOptions(QString _name_, AscEditorType _type_, QString _url_, int _id_) :
    name(_name_)
  , type(_type_)
  , url(Utils::replaceBackslash(_url_))
  , id(_id_)
  , wurl(url.toStdWString())
{}

COpenOptions::COpenOptions(QString _name_, AscEditorType _type_, std::wstring _url_, int _id_) :
    COpenOptions(_name_, _type_, QString::fromStdWString(_url_), _id_)
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
    , CScalingWrapper(parent)
    , m_pMainButton(NULL)
    , m_dataFullScreen(0)
    , m_widthParams({{100, 135, 9}, 68, 3, 0, WINDOW_TITLE_MIN_WIDTH, 140, 0})
    , m_defWidthParams(m_widthParams)
    , m_isCustomStyle(true)
    , m_tabIconSize(11, 11)
{
    CTabBar * tabs = new CTabBar(this);
    tabs->setObjectName("asc_editors_tabbar");
    tabs->setTabTextColor(QPalette::Active, QColor(51, 51, 51));
    tabs->setTabTextColor(QPalette::Inactive, QColor(51, 51, 51));
    setTabBar(tabs);

//    tabBar()->setStyle(new CTabStyle);
//    tabBar()->setFixedWidth(450);

    tabBar()->setMovable(true);
    tabBar()->setExpanding(false);
    setTabsClosable(true);

    setIconSize(m_tabIconSize);
    setProperty("active", false);
    setProperty("empty", true);

    QObject::connect(this, &QTabWidget::currentChanged, [=](){
        updateIcons();
        setFocusedView();

        m_dragIndex = -1;
    });
#if defined(__APP_MULTI_WINDOW)
    QObject::connect(tabs, &CTabBar::tabUndock, [=](int index){
        if ( m_dragIndex != index ) {
            CTabUndockEvent event(widget(index));
            QObject * obj = qobject_cast<QObject *>(
                                static_cast<CAscApplicationManagerWrapper *>(&AscAppManager::getInstance()));
            if ( QApplication::sendEvent(obj, &event) && event.isAccepted() ) {
                    m_dragIndex = index;
            }
        }
    });
#endif
}

CTabPanel * CAscTabWidget::panel(int index)
{
    return static_cast<CTabPanel *>(widget(index));
}

int CAscTabWidget::addEditor(COpenOptions& opts)
{
    if ( opts.url.isEmpty() && opts.type != etNewFile )
        return -1;

    setProperty("empty", false);

    int file_format = 0;
    if (opts.type == etLocalFile) {
        file_format = CCefViewEditor::GetFileFormat(opts.wurl);
        if (file_format == 0)
            /* TODO: show error for file format */
            return -255;
    }

    CTabPanel * pView = new CTabPanel(this);
    pView->view()->SetBackgroundCefColor(244, 244, 244);
    pView->setGeometry(0,0, size().width(), size().height() - tabBar()->height());
    pView->initAsEditor();

    int tab_index = -1;
    bool res_open = true;
    if (opts.type == etLocalFile) {
        pView->openLocalFile(opts.wurl, file_format);
//        opts.type = etUndefined;
    } else
    if (opts.type == etRecoveryFile) {
        res_open = pView->openRecoverFile(opts.id);
//        opts.type = etUndefined;
    } else
    if (opts.type == etRecentFile) {
        res_open = pView->openRecentFile(opts.id);
//        opts.type = etUndefined;
    } else
    if (opts.type == etNewFile) {
        pView->createLocalFile(opts.format, opts.name.toStdWString());
//        opts.type = AscEditorType(opts.format);
    } else {
        pView->cef()->load(opts.wurl);
    }

    if (res_open) {
        CAscTabData * data = new CAscTabData(opts.name);
        data->setViewId(pView->cef()->GetId());
        data->setUrl(opts.wurl);
        data->setLocal( opts.type == etLocalFile || opts.type == etNewFile ||
                       (opts.type == etRecentFile && !CExistanceController::isFileRemote(opts.url)) );

        pView->setData(data);
        tab_index = addTab(pView, opts.name);
        tabBar()->setTabToolTip(tab_index, opts.name);
        ((CTabBar *)tabBar())->tabStartLoading(tab_index);

        //TODO: test for safe remove
//        applyDocumentChanging(id_view, opts.type);
    } else {
        tab_index = -255;
        RELEASEOBJECT(pView)
    }

    return tab_index;
}

void CAscTabWidget::closeEditor(int i, bool m, bool r)
{
    if (!(i < 0) && i < count()) {
        CTabPanel * view = (CTabPanel *)widget(i);
        CAscTabData * doc = view->data();

        if (doc && (!m || !doc->changed())) {
            doc->close();
            AscAppManager::getInstance().DestroyCefView(doc->viewId());

//            RELEASEOBJECT(view)

//            adjustTabsSize();
        }

//        if (r) {
//            setProperty("empty", tabBar()->count()==0);
//            style()->polish(this);
//        }
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
}

int CAscTabWidget::count(int type) const
{
    if ( type < 0 )
        return QTabWidget::count();
    else {
        int _out(0);
        for (int i(count()); i-- > 0; ) {
            if ( ((CTabPanel *)widget(i))->data()->viewType() == type )
                ++_out;
        }
        return _out;
    }
}

int CAscTabWidget::addPortal(QString url, QString name)
{
    if ( url.isEmpty() ) return -1;

    setProperty("empty", false);

    QString args;
    if ( !url.contains(QRegularExpression("desktop=true")) )
        args.append("/products/files/?desktop=true");
    else {
        QRegularExpression _re("^((?:https?:\\/{2})?[^\\s\\/]+)([^\\s]+)", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch _re_match = _re.match(url);

        if ( _re_match.hasMatch() ) {
            url = _re_match.captured(1);
            args = _re_match.captured(2);
        }
    }

    CTabPanel * pView = new CTabPanel(this);
    pView->view()->SetBackgroundCefColor(244, 244, 244);
    pView->setGeometry(0,0, size().width(), size().height() - tabBar()->height());
    pView->initAsSimple();
    pView->cef()->load((url + args).toStdWString());

    QString portal = name.isEmpty() ? Utils::getPortalName(url) : name;

    CAscTabData * data = new CAscTabData(portal, etPortal);
    data->setViewId(pView->cef()->GetId());
    data->setUrl(url);
    pView->setData(data);

    int tab_index = -1;

    tab_index = insertTab(tab_index, pView, portal);
    tabBar()->setTabToolTip(tab_index, url);
    ((CTabBar *)tabBar())->setTabTheme(tab_index, CTabBar::Light);
    ((CTabBar *)tabBar())->tabStartLoading(tab_index);

//    updateTabIcon(tabIndexByView(id));

    return tab_index;
}

int  CAscTabWidget::addOAuthPortal(const QString& portal, const QString& type, const QString& service)
{
    if ( service.isEmpty() || !type.contains(QRegularExpression("sso|outer")) ) return -1;

    setProperty("empty", false);

    CTabPanel * pView = new CTabPanel(this);
    pView->view()->SetBackgroundCefColor(244, 244, 244);
    pView->setGeometry(0,0, size().width(), size().height() - tabBar()->height());
    pView->initAsSimple();

    if ( type == "sso" ) {
        pView->cef()->load(("sso:" + service).toStdWString());
    } else
    if ( type == "outer" ) {
        pView->cef()->SetExternalCloud(service.toStdWString());

        QString _postfix;
        if (service == "asc") _postfix = "/products/files/?desktop=true";
        pView->cef()->load((portal + _postfix).toStdWString());
    }

    QString _portal = portal.isEmpty() ? Utils::getPortalName(service) : Utils::getPortalName(portal);

    CAscTabData * data = new CAscTabData(_portal, etPortal);
    data->setViewId(pView->cef()->GetId());
    data->setUrl(portal);
    pView->setData(data);

    int tab_index = -1;

    tab_index = insertTab(tab_index, pView, _portal);
    tabBar()->setTabToolTip(tab_index, portal);
    ((CTabBar *)tabBar())->setTabTheme(tab_index, CTabBar::Light);
    ((CTabBar *)tabBar())->tabStartLoading(tab_index);

    return tab_index;
}

int CAscTabWidget::insertPanel(QWidget * panel, int index)
{
    int tabindex = -1;

    CTabPanel * _panel = dynamic_cast<CTabPanel *>(panel);
    if ( _panel ) {
        CAscTabData * tabdata = _panel->data();

        tabindex = insertTab(index, panel, tabdata->title());
        tabBar()->setTabToolTip(tabindex, QString::fromStdWString(tabdata->url()));
    }

    return tabindex;
}

QWidget * CAscTabWidget::releaseEditor(int index)
{
    if ( index < 0 || index >= count() )
        index = currentIndex();

    if ( index < 0 )
        return nullptr;
    else {
        QMouseEvent _event(QEvent::MouseButtonRelease,
            tabBar()->tabRect(index).topLeft() + (QPoint(5, 5)),
            Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent( tabBar(), &_event);

        return widget(index);
    }
}

void CAscTabWidget::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e);

    adjustTabsSize();

    if (e) {
        int w = e->size().width(),
            h = e->size().height() - tabBar()->height();

        CTabPanel * view = nullptr;
        for (int i(count()); i > 0;) {
            if (--i != currentIndex()) {
                view = panel(i);
                if (view) {
//                    view->cef()->resizeEvent(w, h);
                    view->resize(w,h);
                }
            }
        }
    }
}

void CAscTabWidget::tabInserted(int index)
{
    adjustTabsSize();
    emit editorInserted(index, count());
}

void CAscTabWidget::tabRemoved(int index)
{
    adjustTabsSize();
    emit editorRemoved(index, count());
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
        nTabBarWidth = m_isCustomStyle ?
                nControlWidth
                - m_widthParams.main_button_width - m_widthParams.main_button_span
                - m_widthParams.title_width - m_widthParams.tools_width - m_widthParams.custom_offset :
                nControlWidth - m_widthParams.main_button_width;

//        int nTabWidth = (nTabBarWidth - /*(2+2)*/10 * nCountTabs) / nCountTabs;      // magic (2+2)
//        if (nTabWidth > m_widthParams.tab.max) nTabWidth = m_widthParams.tab.max;
//        if (nTabWidth < m_widthParams.tab.min) nTabWidth = m_widthParams.tab.min;

        int nMinTabBarWidth = (nTabWidth + /*(2+2)*/(10 * scaling()/*?*/)) * nCountTabs;
        if (nTabBarWidth > nMinTabBarWidth) nTabBarWidth = nMinTabBarWidth;
    }

#if 1
    QString cssStyle = styleSheet();
    cssStyle
        .replace(QRegExp("QTabWidget::tab-bar\\s?\\{\\s?width\\:\\s?(\\-?\\d+px|auto)", Qt::CaseInsensitive),
                    QString("QTabWidget::tab-bar { width: %1px").arg(nTabBarWidth));
//        .replace(QRegExp("QTabBar::tab\\s?\\{\\s?width\\:\\s?\\d+px", Qt::CaseInsensitive),
//                    QString("QTabBar::tab { width: %1px").arg(nTabWidth));

    QTabWidget::setStyleSheet(cssStyle);
#else
    tabBar()->setFixedWidth(nTabBarWidth);
#endif
}

void CAscTabWidget::applyCustomTheme(bool iscustom)
{
    m_isCustomStyle = iscustom;
    m_widthParams.tools_width = (iscustom ? 50 : 0) * scaling();
    m_widthParams.title_width = (iscustom ? WINDOW_TITLE_MIN_WIDTH : 0) * scaling();
}

void CAscTabWidget::updateIcons()
{
    for (int i(count()); i-- > 0;) {
        updateTabIcon(i);
    }
}

void CAscTabWidget::updateTabIcon(int index)
{
    if ( !(index < 0) ) {
        CCefViewEditor * pEditor = (CCefViewEditor *)panel(index)->cef();

        if (pEditor) {
            bool is_active = isActive() && index == currentIndex();
            int tab_type = etUndefined;
            QString tab_color = "none";
            CTabBar::TabTheme tab_theme = is_active ? CTabBar::Dark : CTabBar::Light;

            if (pEditor->GetType() == cvwtSimple) {
                tab_type = etPortal;
                tab_color = "#fff";
                tab_theme = CTabBar::Light;
            } else {
                tab_type = pEditor->GetEditorType();
                switch ( tab_type ) {
                case etPresentation: tab_color = TAB_COLOR_PRESENTATION; break;
                case etSpreadsheet: tab_color = TAB_COLOR_SPREADSHEET; break;
                case etDocument: tab_color = TAB_COLOR_DOCUMENT; break;
                default:
                    tab_type = etUndefined;
                    tab_theme = CTabBar::Light;
                    tab_color = "#fff";
                    break;
                }
            }

            QString icon_name = is_active ? m_mapTabIcons.at(tab_type).second : m_mapTabIcons.at(tab_type).first;
            ((CTabBar *)tabBar())->setTabIcon(index, QIcon(icon_name));
//            ((CTabBar *)tabBar())->changeTabTheme(index, _theme);
            ((CTabBar *)tabBar())->setTabTheme(index, tab_theme);


            if ( !isActive() )
                tab_color = "none";

            if ( index == currentIndex() ) {
                ((CTabBar *)tabBar())->setActiveTabColor(tab_color);
                ((CTabBar *)tabBar())->setUseTabCustomPalette( !(tab_type == etPortal || tab_type == etUndefined) );
            }
        }
    }
}

void CAscTabWidget::setTabIcons(CTabIconSet& icons)
{
    m_mapTabIcons = icons;
}

/*
 *      Slots
*/


void CAscTabWidget::editorCloseRequest(int index)
{
    panel(index)->data()->close();
}

int CAscTabWidget::tabIndexByView(int viewId)
{
    CAscTabData * doc;
    for (int i(count()); i-- > 0; ) {
        doc = panel(i)->data();

        if (doc && doc->viewId() == viewId)
            return i;
    }

    return -1;
}

int CAscTabWidget::tabIndexByTitle(QString t, CefType vt)
{
    CAscTabData * doc;
    for (int i(count()); i-- > 0; ) {
        doc = panel(i)->data();

        if (doc && doc->viewType() == vt && doc->title() == t)
            return i;
    }

    return -1;
}

int CAscTabWidget::tabIndexByTitle(QString t, AscEditorType et)
{
    CAscTabData * doc;
    for (int i(count()); i-- > 0; ) {
        doc = panel(i)->data();

        if (doc && doc->contentType() == et && doc->title() == t)
            return i;
    }

    return -1;
}

int CAscTabWidget::tabIndexByEditorType(AscEditorType et)
{
    CAscTabData * doc;
    for (int i(count()); i-- > 0; ) {
        doc = panel(i)->data();

        if (doc && doc->contentType() == et)
            return i;
    }

    return -1;
}

int CAscTabWidget::tabIndexByUrl(QString url)
{
    CCefView * view = AscAppManager::getInstance().GetViewByUrl( url.toStdWString() );
    if ( view ) {
        return tabIndexByView(view->GetId());
    } else {
        CAscTabData * doc;
        wstring _ws_url(url.toStdWString());
        for (int i(count()); !(--i < 0);) {
            doc = panel(i)->data();

            if (doc && doc->url() == _ws_url)
                return i;
        }
    }

    return -1;
}

void CAscTabWidget::openCloudDocument(COpenOptions& opts, bool select, bool forcenew)
{
    int tabIndex;
    if (opts.id > 0 && !forcenew) {
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

int CAscTabWidget::openLocalDocument(COpenOptions& opts, bool select, bool forcenew)
{
    int tabIndex = -1;
    if ( !forcenew && opts.type != etRecoveryFile ) {
        CCefView * view = AscAppManager::getInstance().GetViewByRecentId( opts.id );
        if ( view ) {
            tabIndex = tabIndexByView(view->GetId());
        } else {
            tabIndex = tabIndexByUrl(opts.url);
        }
    }

    if (tabIndex < 0){
        opts.name = QFileInfo(opts.url).fileName();
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
    QString portal_name = Utils::getPortalName(url);

    int tabIndex = tabIndexByTitle(portal_name, etPortal);
    if (tabIndex < 0) {
        tabIndex = addPortal(url, "");
    }

    return tabIndex;
}

bool CAscTabWidget::updatePortal(int index,const QString& url)
{
    if ( !(index < 0) ) {
        CTabPanel * _panel = panel(index);

        if ( _panel->data()->contentType() == etPortal ) {
            _panel->cef()->load(url.toStdWString());

            return true;
        }
    }

    return false;
}

int CAscTabWidget::newPortal(const QString& url, const QString& name)
{
    int tabIndex = tabIndexByEditorType(etNewPortal);
    if ( tabIndex < 0 ) {
        if ( !((tabIndex = addPortal(url, name)) < 0) ) {
            panel(tabIndex)->data()->setContentType(etNewPortal);
        }
    }

    return tabIndex;
}

void CAscTabWidget::closePortal(const QString& url, bool editors)
{
    closeEditorByIndex(tabIndexByUrl(url));

    if (editors) {
        wstring wname = url.toStdWString();
        CAscTabData * doc;
        for (int i = tabBar()->count(); i-- > 0; ) {
            doc = panel(i)->data();

            if (doc->viewType() == cvwtEditor &&
                    doc->url().find(wname) != wstring::npos)
            {
                closeEditor(i, false, false);
            }
        }
    }
}

void CAscTabWidget::applyDocumentChanging(int viewId, const QString& name, const QString& path)
{
    int tabIndex = tabIndexByView(viewId);

    if (!(tabIndex < 0)) {
        CAscTabData * doc = panel(tabIndex)->data();
        doc->setTitle(name);
        if ( doc->local() && !path.isEmpty() ) {
            QString _path(path);
            doc->setUrl( Utils::replaceBackslash(_path) );
        }

        tabBar()->setTabText(tabIndex, doc->title());
        tabBar()->setTabToolTip(tabIndex, path.isEmpty() ? name : path);
    }
}

void CAscTabWidget::applyDocumentChanging(int viewId, bool state)
{
    int tabIndex = tabIndexByView(viewId);
    if (!(tabIndex < 0)) {
        CAscTabData * doc = panel(tabIndex)->data();

        /* TODO: if exists the saving error, sdk rise the changing event
         * again. maybe not good action.
        */
        if (state && doc->closed()) doc->reuse();
        /**/

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
        CAscTabData * doc = panel(tabIndex)->data();
        if (doc->closed()) {
            cancel ? doc->reuse() : closeEditor(tabIndex, false, true);
        }
    }
}

void CAscTabWidget::applyDocumentChanging(int id, int type)
{
    int tabIndex = tabIndexByView(id);
    if ( !(tabIndex < 0) ) {
        if ( type == DOCUMENT_CHANGED_LOADING_START ) {
//            ((CTabBar *)tabBar())->setTabLoading(tabIndex, true);
            return;
        } else
        if ( type == DOCUMENT_CHANGED_LOADING_FINISH ) {
            ((CTabBar *)tabBar())->setTabLoading(tabIndex, false);
            panel(tabIndex)->applyLoader("hide");

            return;
        } else
        if ( type == DOCUMENT_CHANGED_PAGE_LOAD_FINISH ) {
            CAscTabData * doc = panel(tabIndex)->data();
            if ( !doc->eventLoadSupported() ) {
                ((CTabBar *)tabBar())->setTabLoading(tabIndex, false);
                panel(tabIndex)->applyLoader("hide");
            }

            return;
        }
    }

    CCefView * pView = AscAppManager::getInstance().GetViewById(id);
    if (NULL != pView && pView->GetType() == cvwtEditor) {
        ((CCefViewEditor *)pView)->SetEditorType(AscEditorType(type));
    }

    if ( !(tabIndex < 0) ) {
        panel(tabIndex)->data()->setContentType(AscEditorType(type));

        switch (type) {
        case etDocument: panel(tabIndex)->applyLoader("loader:style", "word"); break;
        case etSpreadsheet: panel(tabIndex)->applyLoader("loader:style", "cell"); break;
        case etPresentation: panel(tabIndex)->applyLoader("loader:style", "slide"); break;
        default: break;
        }
    }

    updateTabIcon(tabIndexByView(id));
}

void CAscTabWidget::setDocumentWebOption(int id, const QString& option)
{
    int tabIndex = tabIndexByView(id);
    if ( !(tabIndex < 0) )
        if ( option == "loading" ) {
            CAscTabData * doc = panel(tabIndex)->data();
            doc->setEventLoadSupported(true);
        }
}

/*
 *      Slots description end
*/

void CAscTabWidget::setFocusedView(int index)
{
    int nIndex = !(index < 0) ? index : currentIndex();
    if (!(nIndex < 0 ))
        panel(nIndex)->cef()->focus();
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

    ((CTabBar*)tabBar())->activate(a);
    ((CTabBar*)tabBar())->customColors().setCurrentColorGroup(
                            a ? QPalette::Normal : QPalette::Disabled );
    tabBar()->repaint();
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
        doc = panel(i)->data();
        doc->changed() && mod_count++;
    }

    return mod_count;
}

int CAscTabWidget::viewByIndex(int index)
{
    if (!(index < 0) && index < count()) {
        CCefView * view = panel(index)->cef();
        return view ? view->GetId() : -1;
    }

    return -1;
}

QString CAscTabWidget::titleByIndex(int index, bool mod)
{
    if (!(index < 0) && index < count()) {
        CAscTabData * doc = panel(index)->data();
        if (doc)
            return doc->title(mod);
    }

    return "";
}

QString CAscTabWidget::urlByView(int id)
{
    CAscTabData * doc;
    for (int i(count()); i-- > 0; ) {
        doc = panel(i)->data();

        if (doc && doc->viewId() == id)
            return QString::fromStdWString(doc->url());
    }

    return "";
}

bool CAscTabWidget::modifiedByIndex(int index)
{
    if (!(index < 0) && index < count()) {
        CAscTabData * doc = panel(index)->data();
        return doc->changed() && !doc->closed();
    }

    return false;
}

bool CAscTabWidget::closedByIndex(int index) {
    if (!(index < 0) && index < count()) {
        CAscTabData * doc = panel(index)->data();
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
        doc = panel(i)->data();

        if (doc->isViewType(cvwtEditor) &&
                doc->changed() && !doc->closed() &&
                (portal.empty() || doc->url().find(portal) != wstring::npos))
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
        doc = panel(i)->data();

        if ( !doc->closed() && doc->isViewType(cvwtEditor) &&
                (portal.empty() || doc->url().find(portal) != wstring::npos) )
        {
            if ( doc->changed() ) {
                return i;
            }
        }
    }

    return -1;
}

int CAscTabWidget::findFragmented(const QString& portalname)
{
    wstring portal = portalname.toStdWString();
    CAscTabData * doc;
    CTabPanel * panel;
    for (int i(tabBar()->count()); i-- > 0; ) {
        panel = (CTabPanel *)widget(i);
        doc = panel->data();
         if ( !doc->closed() && doc->isViewType(cvwtEditor) &&
                (portal.empty() || doc->url().find(portal) != wstring::npos) )
        {
            if ( ((CCefViewEditor*)panel->cef())->CheckCloudCryptoNeedBuild() ) {
                return i;
            }
        }
    }
     return -1;
}

bool CAscTabWidget::isFragmented(int index)
{
    if (!(index < 0) && index < count()) {
        CTabPanel * panel = (CTabPanel *)widget(index);
        CAscTabData * doc = panel->data();
         return !doc->closed() && doc->isViewType(cvwtEditor) && ((CCefViewEditor *)panel->cef())->CheckCloudCryptoNeedBuild();
    }
     return false;
}

void CAscTabWidget::setFullScreen(bool apply, int id)
{
    QWidget * fsWidget;
    static QMetaObject::Connection cefConnection;
    if (!apply) {
        if (m_dataFullScreen) {
            fsWidget = m_dataFullScreen->widget();
            ((CTabPanel *)fsWidget)->showNormal();

            disconnect(cefConnection);

            int index = m_dataFullScreen->tabindex();
            CAscTabData * doc = ((CTabPanel *)fsWidget)->data();

            insertTab(index, fsWidget, doc->title());
            adjustTabsSize();
            tabBar()->setTabToolTip(index, doc->title());
            tabBar()->setCurrentIndex(index);

            RELEASEOBJECT(m_dataFullScreen)

//            updateGeometry();
        }
    } else {
        int tabIndex = tabIndexByView(id);
        if ( !(tabIndex < 0) ) {
            fsWidget = widget(tabIndex);
        } else {
            tabIndex = currentIndex();
            fsWidget = currentWidget();
        }

        if ( fsWidget ) {
            m_dataFullScreen = new CFullScreenData(tabIndex, fsWidget);

            removeTab(tabIndex);
#ifdef _WIN32
            fsWidget->setWindowIcon(Utils::appIcon());
            fsWidget->setParent(nullptr);
#else
            QWidget * grandpa = qobject_cast<QWidget *>(parent()->parent());
            if (grandpa) fsWidget->setParent(grandpa);
#endif
            ((CTabPanel *)fsWidget)->showFullScreen();
            ((CTabPanel *)fsWidget)->cef()->focus();

            cefConnection = connect(((CTabPanel *)fsWidget)->view(), &QCefView::closeWidget, [=](QCloseEvent * e){
                e->ignore();

                NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                pCommand->put_Command(L"editor:stopDemonstration");

                NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EDITOR_EXECUTE_COMMAND);
                pEvent->m_pData = pCommand;
                ((CTabPanel *)fsWidget)->cef()->Apply(pEvent);

                emit closeAppRequest();
            });

            QPoint pt = mapToGlobal(pos());
#ifdef _WIN32
            fsWidget->setGeometry(QApplication::desktop()->screenGeometry(pt));
#else

            QRect _scr_rect = QApplication::desktop()->screenGeometry(pt);
            fsWidget->setGeometry(QRect(QPoint(0,0), _scr_rect.size()));
#endif
        }
    }
}

QWidget * CAscTabWidget::fullScreenWidget()
{
    return m_dataFullScreen ? m_dataFullScreen->widget() : nullptr;
}

void CAscTabWidget::updateScaling(int f)
{
    CScalingWrapper::updateScaling(f);

    int dpi_ratio = scaling();

    setIconSize(m_tabIconSize * dpi_ratio);
    updateIcons();

    (m_widthParams = size_params(m_defWidthParams)).apply_scale(dpi_ratio);
    if ( m_isCustomStyle )
        m_widthParams.tools_width = 50 * dpi_ratio,
        m_widthParams.title_width = WINDOW_TITLE_MIN_WIDTH * dpi_ratio;
    else
        m_widthParams.tools_width = m_widthParams.title_width = 0;

    adjustTabsSize();
}

void CAscTabWidget::setStyleSheet(const QString& stylesheet)
{
    QTabWidget::setStyleSheet(stylesheet);

    auto _string_to_color = [](const QString& str) -> QColor {
        int r = -1, g = -1, b = -1;
        QRegExp re("^#([a-f0-9]{2})([a-f0-9]{2})([a-f0-9]{2})$", Qt::CaseInsensitive);
        if ( re.indexIn(str) < 0 ) {
            re.setPattern("^#([a-f0-9])([a-f0-9])([a-f0-9])$");

            if ( !(re.indexIn(str) < 0) ) {
                r = (re.cap(1)+re.cap(1)).toInt(nullptr, 16),
                g = (re.cap(2)+re.cap(2)).toInt(nullptr, 16),
                b = (re.cap(3)+re.cap(3)).toInt(nullptr, 16);
            }
        } else {
            r = re.cap(1).toInt(nullptr, 16),
            g = re.cap(2).toInt(nullptr, 16),
            b = re.cap(3).toInt(nullptr, 16);
        }

        if ( r < 0 || g < 0 || b < 0 )
            return QColor();
        else return {r,g,b};
    };

    QRegExp r("QTabBar::tab-label\\s?\\{\\s?active:\\s?([^;]{4,7});normal:\\s?([^;]{4,7})");
    if (!(r.indexIn(stylesheet) < 0)) {
        ((CTabBar *)tabBar())->setTabTextColor(QPalette::Active, _string_to_color(r.cap(1)) );
        ((CTabBar *)tabBar())->setTabTextColor(QPalette::Inactive, _string_to_color(r.cap(2)) );
    }

    r.setPattern("QTabBar::tab-icon\\s*\\{([^\\}]+)");
    if ( !(r.indexIn(stylesheet) < 0) ) {
        QRegExp ri("width:\\s*(\\d+);\\s*height:(\\d+)");

        if ( !(ri.indexIn(r.cap(1)) < 0) ) {
            m_tabIconSize.setWidth(ri.cap(1).toInt());
            m_tabIconSize.setHeight(ri.cap(2).toInt());
        }
    }
}
