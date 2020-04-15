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

#include "asctabwidget.h"
#include <QRegExp>

#include <QDebug>

#include <QHBoxLayout>
#include <QLabel>
#include <QStylePainter>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QTimer>
#include <regex>

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

auto createTabPanel(QWidget * parent, CTabPanel * panel = nullptr) -> QWidget * {
    QWidget * panelwidget = new QWidget(parent);

    panelwidget->setLayout(new QGridLayout);
    panelwidget->layout()->setContentsMargins(0,0,0,0);
    panelwidget->layout()->addWidget(panel ? panel : new CTabPanel);

    return panelwidget;
}

auto panelfromwidget(QWidget * panelwidget) -> CTabPanel * {
    return panelwidget->children().count() ? static_cast<CTabPanel *>(panelwidget->findChild<CTabPanel*>()) : nullptr;
}


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

    static int _dropedindex = -1;
    QObject::connect(this, &QTabWidget::currentChanged, [=](){
        updateIcons();
        setFocusedView();

        _dropedindex = -1;
    });

    QObject::connect(tabs, &CTabBar::tabUndock, [=](int index, bool * accept){
        if (index == _dropedindex) return;

        const CTabPanel * _panel = panel(index);

        if ( _panel->data()->viewType() == cvwtEditor ) {
            CTabUndockEvent event(index);
            QObject * obj = qobject_cast<QObject *>(
                        static_cast<CAscApplicationManagerWrapper *>(&AscAppManager::getInstance()));
            if ( QApplication::sendEvent(obj, &event) && event.isAccepted() ) {
                _dropedindex = index;
                *accept = true;

                QTimer::singleShot(10,[=](){
                    if ( widget(index) )
                        widget(index)->deleteLater();
                });
            }
        }
    });
}

CTabPanel * CAscTabWidget::panel(int index) const
{
    QWidget * _w = widget(index);
    return _w->children().count() ? static_cast<CTabPanel *>(_w->findChild<CTabPanel*>()) : nullptr;
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

    QWidget * panelwidget = createTabPanel(this);
    CTabPanel * pView = panelfromwidget(panelwidget);

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
        data->setUrl(opts.wurl);
        data->setIsLocal( opts.type == etLocalFile || opts.type == etNewFile ||
                       (opts.type == etRecentFile && !CExistanceController::isFileRemote(opts.url)) );

        pView->setData(data);
        tab_index = addTab(panelwidget, opts.name);
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
        CTabPanel * view = panel(i);
        CAscTabData * doc = view->data();

        if (doc && (!m || !doc->hasChanges())) {
            doc->close();
            AscAppManager::getInstance().DestroyCefView(view->cef()->GetId());

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
            if ( (panel(i))->data()->viewType() == type )
                ++_out;
        }
        return _out;
    }
}

int CAscTabWidget::count(const wstring& portal)
{
    if ( portal.empty() )
        return QTabWidget::count();
    else {
        int _out(0);
        for (int i(count()); i-- > 0; ) {
            if ( panel(i)->data()->url().find(portal) != wstring::npos )
                ++_out;
        }
        return _out;
    }
}

bool CAscTabWidget::hasForPortal(const QString& portal)
{
    const wstring _wsp = portal.toStdWString();
    for (int i(count()); i-- > 0; ) {
        if ( panel(i)->data()->isViewType(cvwtEditor) &&
                panel(i)->data()->url().find(_wsp) != wstring::npos )
        {
            return true;
        }
    }

    return false;
}

int CAscTabWidget::addPortal(const QString& url, const QString& name, const QString& provider)
{
    if ( url.isEmpty() ) return -1;

    setProperty("empty", false);

    QString args, _url = url;
    if ( provider == "asc" && !_url.contains(QRegularExpression("desktop=true")) )
        args.append("/products/files/?desktop=true");
    else {
        QRegularExpression _re("^((?:https?:\\/{2})?[^\\s\\?]+)(\\?[^\\s]+)?", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch _re_match = _re.match(url);

        if ( _re_match.hasMatch() ) {
            _url = _re_match.captured(1);
            args = _re_match.captured(2);
        }
    }

    QWidget * panelwidget = createTabPanel(this);
    CTabPanel * pView = panelfromwidget(panelwidget);

    pView->setGeometry(0,0, size().width(), size().height() - tabBar()->height());
    pView->initAsSimple();
    pView->cef()->SetExternalCloud(provider.toStdWString());
    pView->cef()->load((_url + args).toStdWString());

    QString portal = name.isEmpty() ? Utils::getPortalName(url) : name;

    CAscTabData * data = new CAscTabData(portal, etPortal);
    data->setUrl(_url);
    pView->setData(data);

    int tab_index = -1;

    tab_index = insertTab(tab_index, panelwidget, portal);
    tabBar()->setTabToolTip(tab_index, _url);
    ((CTabBar *)tabBar())->setTabTheme(tab_index, CTabBar::Light);
    ((CTabBar *)tabBar())->tabStartLoading(tab_index);

//    updateTabIcon(tabIndexByView(id));

    return tab_index;
}

int  CAscTabWidget::addOAuthPortal(const QString& portal, const QString& type, const QString& service)
{
    if ( service.isEmpty() || !type.contains(QRegularExpression("sso|outer")) ) return -1;

    setProperty("empty", false);

    QWidget * panelwidget = createTabPanel(this);
    CTabPanel * pView = panelfromwidget(panelwidget);
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
    data->setUrl(portal);
    pView->setData(data);

    int tab_index = -1;

    tab_index = insertTab(tab_index, panelwidget, _portal);
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
        const CAscTabData * tabdata = _panel->data();

        QWidget * panelwidget = createTabPanel(this, _panel);

        tabindex = insertTab(index, panelwidget, tabdata->title());
        tabBar()->setTabToolTip(tabindex, !tabdata->url().empty() ?
                                QString::fromStdWString(tabdata->url()) : tabdata->title() );
    }

    return tabindex;
}

void CAscTabWidget::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e);

    adjustTabsSize();

//    if (e) {
//        int w = e->size().width(),
//            h = e->size().height() - tabBar()->height();

//        CTabPanel * view = nullptr;
//        for (int i(count()); i > 0;) {
//            if (--i != currentIndex()) {
//                view = panel(i);
//                if (view) {
////                    view->cef()->resizeEvent(w, h);
//                    view->resize(w,h);
//                }
//            }
//        }
//    }
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
    for (int i(count()); i-- > 0; ) {
        if (panel(i) && panel(i)->cef()->GetId() == viewId)
            return i;
    }

    return -1;
}

int CAscTabWidget::tabIndexByTitle(QString t, CefType vt)
{
    const CAscTabData * doc;
    for (int i(count()); i-- > 0; ) {
        doc = panel(i)->data();

        if (doc && doc->viewType() == vt && doc->title() == t)
            return i;
    }

    return -1;
}

int CAscTabWidget::tabIndexByTitle(QString t, AscEditorType et)
{
    const CAscTabData * doc;
    for (int i(count()); i-- > 0; ) {
        doc = panel(i)->data();

        if (doc && doc->contentType() == et)
            if (doc->title() == t ||
                    (et == etPortal && doc->title().contains(t)))
            {
                return i;
            }
    }

    return -1;
}

int CAscTabWidget::tabIndexByEditorType(AscEditorType et)
{
    const CAscTabData * doc;
    for (int i(count()); i-- > 0; ) {
        doc = panel(i)->data();

        if (doc && doc->contentType() == et)
            return i;
    }

    return -1;
}

int CAscTabWidget::tabIndexByUrl(const QString& url)
{
    return tabIndexByUrl(url.toStdWString());
}

int CAscTabWidget::tabIndexByUrl(const wstring& url)
{
    CCefView * view = AscAppManager::getInstance().GetViewByUrl(url);
    if ( view ) {
        return tabIndexByView(view->GetId());
    } else {
        const CAscTabData * doc;
        for (int i(count()); !(--i < 0);) {
            doc = panel(i)->data();

            if (doc && doc->url().compare(url) == 0)
                return i;
        }
    }

    return -1;
}

int CAscTabWidget::openCloudDocument(COpenOptions& opts, bool select, bool forcenew)
{
    int tabIndex{-1};
    if (opts.id > 0 && !forcenew) {
        tabIndex = tabIndexByView(opts.id);
        if (!(tabIndex < 0))
            setCurrentIndex(tabIndex);
    } else {
        opts.name   = tr("Document");
//        opts.type   = etUndefined;
        tabIndex    = addEditor(opts);

        updateIcons();

        if (select && !(tabIndex < 0))
            tabBar()->setCurrentIndex(tabIndex);
    }

    return tabIndex;
}

int CAscTabWidget::openLocalDocument(COpenOptions& opts, bool select, bool forcenew)
{
    int tabIndex = -1;
    if ( !forcenew && opts.type != etRecoveryFile ) {
        CCefView * view = AscAppManager::getInstance().GetViewByRecentId( opts.id );
        if ( view ) {
            tabIndex = tabIndexByView(view->GetId());
        } else {
            tabIndex = tabIndexByUrl(opts.wurl);
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

int CAscTabWidget::openPortal(const QString& url, const QString& provider)
{
    QString portal_name = Utils::getPortalName(url);

    int tabIndex = tabIndexByTitle(portal_name, etPortal);
    if (tabIndex < 0) {
        tabIndex = addPortal(url, "", provider);
    } else {
//        updatePortal(tabIndex, url);
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
        if ( !((tabIndex = addPortal(url, name, "asc")) < 0) ) {
            panel(tabIndex)->data()->setContentType(etNewPortal);
        }
    }

    return tabIndex;
}

void CAscTabWidget::closePortal(const wstring& url, bool editors)
{
    closeEditorByIndex(tabIndexByUrl(url));

    if (editors) {
        const CAscTabData * doc;
        for (int i = tabBar()->count(); i-- > 0; ) {
            doc = panel(i)->data();

            if (doc->viewType() == cvwtEditor &&
                    doc->url().find(url) != wstring::npos)
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
        if ( doc->isLocal() && !path.isEmpty() ) {
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

        if (doc->hasChanges() != state && (!doc->closed() || state)) {
            doc->setChanged(state);
            tabBar()->setTabText(tabIndex, doc->title());
            tabBar()->setTabToolTip(tabIndex, doc->title());
        }
    }
}

void CAscTabWidget::cancelDocumentSaving(int index)
{
    if ( !(index < 0) ) {
        CAscTabData * doc = panel(index)->data();

        if ( doc->closed() ) {
            doc->reuse();
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
            if ( !panel(tabIndex)->data()->eventLoadSupported() ) {
                ((CTabBar *)tabBar())->setTabLoading(tabIndex, false);
                panel(tabIndex)->applyLoader("hide");
            }

            return;
        }
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

void CAscTabWidget::setEditorOptions(int id, const wstring& option)
{
    int tabIndex = tabIndexByView(id);
    if ( !(tabIndex < 0) ) {
        panel(tabIndex)->data()->setFeatures(option);

        size_t _pos;
        if ((_pos = option.find(L"eventloading:")) != wstring::npos) {
            if (option.find(L"true", _pos + 1) != wstring::npos)
                panel(tabIndex)->data()->setEventLoadSupported(true);
        }

//        if (std::regex_search(option, std::wregex(L"titlebuttons\":\\s?true"))) {
//            panel(tabIndex)->setWindowed(true);
//        }
    }
}

/*
 *      Slots description end
*/

void CAscTabWidget::setFocusedView(int index)
{
    if (!m_pMainWidget->isHidden())
    {
        if (!QCefView::IsSupportLayers())
        {
            if (this->currentWidget() && !this->currentWidget()->isHidden())
                this->currentWidget()->hide();
        }
        return;
    }
    int nIndex = !(index < 0) ? index : currentIndex();
    if (!(nIndex < 0 ))
    {
        if (!QCefView::IsSupportLayers())
        {
            if (this->currentWidget()->isHidden())
                this->currentWidget()->show();
        }

        if ( panel(nIndex) )
            panel(nIndex)->view()->setFocusToCef();
    }
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
    const CAscTabData * doc;

    for (int i = tabBar()->count(); i-- > 0; ) {
        doc = panel(i)->data();
        doc->hasChanges() && mod_count++;
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
        const CAscTabData * doc = panel(index)->data();
        if (doc)
            return doc->title(mod);
    }

    return "";
}

QString CAscTabWidget::urlByView(int id)
{
    for (int i(count()); i-- > 0; ) {
        if (panel(i)->cef()->GetId() == id)
            return QString::fromStdWString(panel(i)->data()->url());
    }

    return "";
}

bool CAscTabWidget::modifiedByIndex(int index)
{
    if (!(index < 0) && index < count()) {
        const CAscTabData * doc = panel(index)->data();
        return doc->hasChanges() && !doc->closed();
    }

    return false;
}

bool CAscTabWidget::isLocalByIndex(int index)
{
    if (!(index < 0) && index < count()) {
        return panel(index)->data()->isLocal();
    }

    return true;
}

bool CAscTabWidget::closedByIndex(int index) {
    if (!(index < 0) && index < count()) {
        return panel(index)->data()->closed();
    }

    return true;
}

MapEditors CAscTabWidget::modified(const QString& portalname)
{
    QMap<int, QString> mapModified;
    wstring portal = portalname.toStdWString();
    const CAscTabData * doc;
    for (int i(tabBar()->count()); i-- > 0; i++) {
        doc = panel(i)->data();

        if (doc->isViewType(cvwtEditor) &&
                doc->hasChanges() && !doc->closed() &&
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
    const CAscTabData * doc;
    for (int i(tabBar()->count()); i-- > 0; ) {
        doc = panel(i)->data();

        if ( !doc->closed() && doc->isViewType(cvwtEditor) &&
                (portal.empty() || doc->url().find(portal) != wstring::npos) )
        {
            if ( doc->hasChanges() ) {
                return i;
            }
        }
    }

    return -1;
}

int CAscTabWidget::findFragmented(const QString& portalname)
{
    wstring portal = portalname.toStdWString();
    const CAscTabData * doc;
    const CTabPanel * cefpanel;
    for (int i(tabBar()->count()); i-- > 0; ) {
        cefpanel = panel(i);
        doc = cefpanel->data();
        if ( !doc->closed() && doc->isViewType(cvwtEditor) &&
                (portal.empty() || doc->url().find(portal) != wstring::npos) )
        {
            if ( ((CCefViewEditor*)cefpanel->cef())->CheckCloudCryptoNeedBuild() ) {
                return i;
            }
        }
    }
    return -1;
}

bool CAscTabWidget::isFragmented(int index)
{
    if (!(index < 0) && index < count()) {
        const CTabPanel * cefpanel = panel(index);
        const CAscTabData * doc = cefpanel->data();
        return /*!doc->closed() &&*/ doc->isViewType(cvwtEditor) && ((CCefViewEditor *)cefpanel->cef())->CheckCloudCryptoNeedBuild();
    }
    return false;
}

int CAscTabWidget::findProcessed() const
{
    const CAscTabData * doc;
    const CTabPanel * cefpanel;
    for (int i(count()); i-- > 0; ) {
        cefpanel = panel(i);
        doc = cefpanel->data();
        if ( !doc->closed() && doc->isViewType(cvwtEditor) &&
                ((CCefViewEditor *)cefpanel->cef())->IsBuilding() )
        {
            return i;
        }
    }

    return -1;
}

bool CAscTabWidget::isProcessed(int index) const
{
    if (!(index < 0) && index < count()) {
        const CTabPanel * cefpanel = panel(index);
        const CAscTabData * doc = cefpanel->data();

        return /*!doc->closed() &&*/ doc->isViewType(cvwtEditor) && ((CCefViewEditor *)cefpanel->cef())->IsBuilding();
    }

    return false;
}

void CAscTabWidget::setFullScreen(bool apply, int id)
{
    QWidget * fsWidget;
    static QMetaObject::Connection cefConnection;
    if (!apply) {
        if (m_dataFullScreen) {
            disconnect(cefConnection);

#ifdef _LINUX
            AscAppManager::topWindow()->show();
#endif

            int index = m_dataFullScreen->tabindex();
            fsWidget = m_dataFullScreen->widget();
            widget(index)->layout()->addWidget(fsWidget);

            RELEASEOBJECT(m_dataFullScreen)

//            updateGeometry();
        }
    } else {
        int tabIndex = tabIndexByView(id);
        if ( tabIndex < 0 )
            tabIndex = currentIndex();
        fsWidget = panel(tabIndex);

        if ( fsWidget ) {
            m_dataFullScreen = new CFullScreenData(tabIndex, fsWidget);

            fsWidget->setWindowIcon(Utils::appIcon());
            fsWidget->setParent(nullptr);
#ifdef _WIN32
#else
            fsWidget->setWindowFlags(Qt::FramelessWindowHint);
            AscAppManager::topWindow()->hide();
#endif
            ((CTabPanel *)fsWidget)->showFullScreen();
            ((CTabPanel *)fsWidget)->view()->setFocusToCef();

            cefConnection = connect((CTabPanel *)fsWidget, &CTabPanel::closePanel, [=](QCloseEvent * e){
                NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                pCommand->put_Command(L"editor:stopDemonstration");

                NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EDITOR_EXECUTE_COMMAND);
                pEvent->m_pData = pCommand;
                ((CTabPanel *)fsWidget)->cef()->Apply(pEvent);

                e->ignore();
                // TODO: associate panel with reporter window and close both simultaneously
                QTimer::singleShot(10, [=] {emit tabCloseRequested(m_dataFullScreen->tabindex());});
//                emit closeAppRequest();
            });

            fsWidget->setGeometry(QApplication::desktop()->screenGeometry(mapToGlobal(pos())));
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
