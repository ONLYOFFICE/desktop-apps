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


class CTabPanel : public QCefView
{
public:
    explicit CTabPanel(QWidget * parent)
        : QCefView(parent)
        , _data(nullptr)
    {}

    ~CTabPanel() {
        RELEASEOBJECT( _data );
    }

    void setData(CAscTabData * data)
    {
        _data = data;
    }

    CAscTabData * const data()
    {
        return _data;
    }
private:
    CAscTabData * _data;
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
    setTabsClosable(true);

    setIconSize(m_tabIconSize);
    setProperty("active", false);
    setProperty("empty", true);

    QObject::connect(this, &QTabWidget::currentChanged, [=](){updateIcons(); setFocusedView();});
#if defined(APP_MULTI_WINDOW)
    QObject::connect(tabs, &CTabBar::tabUndock, [=](int index){
        QTimer::singleShot(0, this, [=]{ emit tabUndockRequest(index);});
    });
#endif
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
    pView->SetBackgroundCefColor(244, 244, 244);
    pView->setGeometry(0,0, size().width(), size().height() - tabBar()->height());
    pView->Create(&AscAppManager::getInstance(), cvwtEditor);

    int tab_index = -1;
    bool res_open = true;
    CCefView * cview = pView->GetCefView();    
    if (opts.type == etLocalFile) {
        ((CCefViewEditor*)cview)->OpenLocalFile(opts.wurl, file_format);
//        opts.type = etUndefined;
    } else
    if (opts.type == etRecoveryFile) {
        res_open = ((CCefViewEditor*)cview)->OpenRecoverFile(opts.id);
//        opts.type = etUndefined;
    } else
    if (opts.type == etRecentFile) {
        res_open = ((CCefViewEditor*)cview)->OpenRecentFile(opts.id);
//        opts.type = etUndefined;
    } else
    if (opts.type == etNewFile) {
        ((CCefViewEditor*)cview)->CreateLocalFile(opts.format, opts.name.toStdWString());
//        opts.type = AscEditorType(opts.format);
    } else {
        cview->load(opts.wurl);
    }

    if (res_open) {
        int id_view = cview->GetId();

        CAscTabData * data = new CAscTabData(opts.name);
        data->setViewId(id_view);
        data->setUrl(opts.wurl);
        data->setLocal( opts.type == etLocalFile ||
                       (opts.type == etRecentFile && !CExistanceController::isFileRemote(opts.url)) );

        pView->setData(data);
        tab_index = addTab(pView, opts.name);
        tabBar()->setTabToolTip(tab_index, opts.name);

        //TODO: test for safe remove
//        applyDocumentChanging(id_view, opts.type);
        resizeEvent(NULL);
    } else {
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

//            if (r) emit tabClosed(i, tabBar()->count());
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

int CAscTabWidget::addPortal(QString url, QString name)
{
    if ( url.isEmpty() ) return -1;

    setProperty("empty", false);

    QString args;
    if ( !url.contains(QRegularExpression("desktop=true")) )
        args.append("/products/files/?desktop=true");

    CTabPanel * pView = new CTabPanel(this);
    pView->SetBackgroundCefColor(244, 244, 244);
    pView->setGeometry(0,0, size().width(), size().height() - tabBar()->height());
    pView->Create(&AscAppManager::getInstance(), cvwtSimple);
    pView->GetCefView()->load((url + args).toStdWString());
    int id_view = pView->GetCefView()->GetId();

    QString portal = name.isEmpty() ? Utils::getPortalName(url) : name;

    CAscTabData * data = new CAscTabData(portal, etPortal);
    data->setViewId(id_view);
    data->setUrl(url);
    pView->setData(data);

    int tab_index = -1;

    tab_index = insertTab(tab_index, pView, portal);
    tabBar()->setTabToolTip(tab_index, url);

#ifdef __APP_NEW_APPEARANCE
    QToolButton * b = (QToolButton *)tabBar()->tabButton(tab_index, QTabBar::RightSide);
    if ( b ) b->setProperty("static-state", "normal");

    CAnimatedIcon * i = (CAnimatedIcon *)tabBar()->tabButton(tab_index, QTabBar::LeftSide);
    if ( i ) i->setSvgStaticElement("dark");
    ((CTabBar *)tabBar())->setTabLoading(tab_index, true);
#endif

//    updateTabIcon(tabIndexByView(id));

    resizeEvent(NULL);
    return tab_index;
}

int  CAscTabWidget::addOAuthPortal(const QString& portal, const QString& type, const QString& service)
{
    if ( service.isEmpty() ) return -1;

    setProperty("empty", false);

    QString _prefix;
    if ( type == "sso" )
        _prefix = "sso:";

    CTabPanel * pView = new CTabPanel(this);
    pView->SetBackgroundCefColor(244, 244, 244);
    pView->setGeometry(0,0, size().width(), size().height() - tabBar()->height());
    pView->Create(&AscAppManager::getInstance(), cvwtSimple);
    pView->GetCefView()->load((_prefix + service).toStdWString());
    int id_view = pView->GetCefView()->GetId();

    QString _portal = portal.isEmpty() ? Utils::getPortalName(service) : Utils::getPortalName(portal);

    CAscTabData * data = new CAscTabData(_portal, etPortal);
    data->setViewId(id_view);
    data->setUrl(portal);
    pView->setData(data);

    int tab_index = -1;

    tab_index = insertTab(tab_index, pView, _portal);
    tabBar()->setTabToolTip(tab_index, portal);

#ifdef __APP_NEW_APPEARANCE
    QToolButton * b = (QToolButton *)tabBar()->tabButton(tab_index, QTabBar::RightSide);
    if ( b ) b->setProperty("static-state", "normal");

    CAnimatedIcon * i = (CAnimatedIcon *)tabBar()->tabButton(tab_index, QTabBar::LeftSide);
    if ( i ) i->setSvgStaticElement("dark");
    ((CTabBar *)tabBar())->setTabLoading(tab_index, true);
#endif

    resizeEvent(NULL);
    return tab_index;
}

int CAscTabWidget::pickupTab(QWidget * panel)
{
    CAscTabData * tabdata = ((CTabPanel *)panel)->data();

    int tabindex = insertTab(count(), panel, tabdata->title());
    tabBar()->setTabToolTip(tabindex, QString::fromStdWString(tabdata->url()));

    resizeEvent(nullptr);

    return tabindex;
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
        nTabBarWidth = m_isCustomStyle ?
                nControlWidth
                - m_widthParams.main_button_width - m_widthParams.main_button_span
                - m_widthParams.title_width - m_widthParams.tools_width - m_widthParams.custom_offset :
                nControlWidth - m_widthParams.main_button_width;

        int nTabWidth = (nTabBarWidth - /*(2+2)*/10 * nCountTabs) / nCountTabs;      // magic (2+2)
        if (nTabWidth > m_widthParams.tab.max) nTabWidth = m_widthParams.tab.max;
        if (nTabWidth < m_widthParams.tab.min) nTabWidth = m_widthParams.tab.min;

        int nMinTabBarWidth = (nTabWidth + /*(2+2)*/(10 * scaling()/*?*/)) * nCountTabs;
        if (nTabBarWidth > nMinTabBarWidth) nTabBarWidth = nMinTabBarWidth;
    }

    QString cssStyle = styleSheet();
    cssStyle
        .replace(QRegExp("QTabWidget::tab-bar\\s?\\{\\s?width\\:\\s?(\\-?\\d+px|auto)", Qt::CaseInsensitive),
                    QString("QTabWidget::tab-bar { width: %1px").arg(nTabBarWidth))
        .replace(QRegExp("QTabBar::tab\\s?\\{\\s?width\\:\\s?\\d+px", Qt::CaseInsensitive),
                    QString("QTabBar::tab { width: %1px").arg(nTabWidth));

    QTabWidget::setStyleSheet(cssStyle);
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
        CCefViewEditor * pEditor = (CCefViewEditor *)((QCefView*)(widget(index)))->GetCefView();

        if (pEditor) {
            bool is_active = isActive() && index == currentIndex();
            int tab_type = etUndefined;
#ifdef __USE_COLORED_TAB
            QString _color = "none";
#endif

            if (pEditor->GetType() == cvwtSimple) {
                tab_type = etPortal;
#ifdef __USE_COLORED_TAB
                _color = "#fff";
#endif
            } else {
                tab_type = pEditor->GetEditorType();
                switch ( tab_type ) {
#ifdef __USE_COLORED_TAB
                case etPresentation: _color = TAB_COLOR_PRESENTATION; break;
                case etSpreadsheet: _color = TAB_COLOR_SPREADSHEET; break;
                case etDocument: _color = TAB_COLOR_DOCUMENT; break;
                default: tab_type = etUndefined; _color = "#fff"; break;
#else
                case etPresentation:
                case etSpreadsheet:
                case etDocument: break;
                default: tab_type = etUndefined; break;
#endif
                }
            }

            QString icon_name = is_active ? m_mapTabIcons.at(tab_type).second : m_mapTabIcons.at(tab_type).first;
            ((CTabBar *)tabBar())->setTabIcon(index, QIcon(icon_name));

#ifdef __USE_COLORED_TAB
            if ( !isActive() )
                _color = "none";

            if ( index == currentIndex() ) {
                ((CTabBar *)tabBar())->setActiveTabColor(_color);
                ((CTabBar *)tabBar())->setUseTabCustomPalette( !(tab_type == etPortal || tab_type == etUndefined) );
            }
#endif
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
    ((CTabPanel *)widget(index))->data()->close();
}

int CAscTabWidget::tabIndexByView(int viewId)
{
    CAscTabData * doc;
    for (int i(count()); i-- > 0; ) {
        doc = ((CTabPanel *)widget(i))->data();

        if (doc && doc->viewId() == viewId)
            return i;
    }

    return -1;
}

int CAscTabWidget::tabIndexByTitle(QString t, CefType vt)
{
    CAscTabData * doc;
    for (int i(count()); i-- > 0; ) {
        doc = ((CTabPanel *)widget(i))->data();

        if (doc && doc->viewType() == vt && doc->title() == t)
            return i;
    }

    return -1;
}

int CAscTabWidget::tabIndexByTitle(QString t, AscEditorType et)
{
    CAscTabData * doc;
    for (int i(count()); i-- > 0; ) {
        doc = ((CTabPanel *)widget(i))->data();

        if (doc && doc->contentType() == et && doc->title() == t)
            return i;
    }

    return -1;
}

int CAscTabWidget::tabIndexByEditorType(AscEditorType et)
{
    CAscTabData * doc;
    for (int i(count()); i-- > 0; ) {
        doc = ((CTabPanel *)widget(i))->data();

        if (doc && doc->contentType() == et)
            return i;
    }

    return -1;
}

int CAscTabWidget::tabIndexByUrl(QString url)
{
    CAscTabData * doc;
    wstring _ws_url(url.toStdWString());
    for (int i(count()); !(--i < 0);) {
        doc = ((CTabPanel *)widget(i))->data();

        if (doc && doc->url() == _ws_url)
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
    int tabIndex = tabIndexByUrl(opts.url);

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
    int out_val = 1;

    QString portal_name = Utils::getPortalName(url);

    int tabIndex = tabIndexByTitle(portal_name, etPortal);
    if (tabIndex < 0) {
        tabIndex = addPortal(url, ""), out_val = 2;
    }

    setCurrentIndex(tabIndex);
    return out_val;
}

int CAscTabWidget::newPortal(const QString& url, const QString& name)
{
    int out_val = 1;

    int tabIndex = tabIndexByEditorType(etNewPortal);
    if ( tabIndex < 0 ) {
        if ( !((tabIndex = addPortal(url, name)) < 0) ) {
            ((CTabPanel *)widget(tabIndex))->data()->setContentType(etNewPortal);
            out_val = 2;
        }
    }

    setCurrentIndex(tabIndex);
    return out_val;
}

void CAscTabWidget::closePortal(const QString& url, bool editors)
{
    closeEditorByIndex(tabIndexByUrl(url));

    if (editors) {
        wstring wname = url.toStdWString();
        CAscTabData * doc;
        for (int i = tabBar()->count(); i-- > 0; ) {
            doc = ((CTabPanel *)widget(i))->data();

            if (doc->viewType() == cvwtEditor &&
                    doc->url().find(wname) != wstring::npos)
            {
                closeEditor(i, false, false);
            }
        }
    }
}

void CAscTabWidget::applyDocumentChanging(int viewId, const QString& name, const QString& info)
{
    int tabIndex = tabIndexByView(viewId);

    if (!(tabIndex < 0)) {
        CAscTabData * doc = ((CTabPanel *)widget(tabIndex))->data();
        doc->setTitle(name);
        if ( doc->local() ) {
            QString _path(info);
            doc->setUrl( Utils::replaceBackslash(_path) );
        }

        tabBar()->setTabText(tabIndex, doc->title());
        tabBar()->setTabToolTip(tabIndex, info);
    }
}

void CAscTabWidget::applyDocumentChanging(int viewId, bool state)
{
    int tabIndex = tabIndexByView(viewId);
    if (!(tabIndex < 0)) {
        CAscTabData * doc = ((CTabPanel *)widget(tabIndex))->data();

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
        CAscTabData * doc = ((CTabPanel *)widget(tabIndex))->data();
        if (doc->closed()) {
            cancel ? doc->reuse() : closeEditor(tabIndex, false, true);
        }
    }
}

void CAscTabWidget::applyDocumentChanging(int id, int type)
{
    int tabIndex = tabIndexByView(id);

#ifdef __APP_NEW_APPEARANCE
    if ( !(tabIndex < 0) )
        if ( type == -255) {
            ((CTabBar *)tabBar())->setTabLoading(tabIndex, true);
            return;
        } else
        if ( type == -254) {
            ((CTabBar *)tabBar())->setTabLoading(tabIndex, false);
            return;
        }
#endif

    CCefView * pView = AscAppManager::getInstance().GetViewById(id);
    if (NULL != pView && pView->GetType() == cvwtEditor) {
        ((CCefViewEditor *)pView)->SetEditorType(AscEditorType(type));
    }

    if ( !(tabIndex < 0) ) {
        ((CTabPanel *)widget(tabIndex))->data()
                ->setContentType(AscEditorType(type));
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
        doc = ((CTabPanel *)widget(i))->data();
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
        CAscTabData * doc = ((CTabPanel *)widget(index))->data();
        if (doc)
            return doc->title(mod);
    }

    return "";
}

QString CAscTabWidget::urlByView(int id)
{
    CAscTabData * doc;
    for (int i(count()); i-- > 0; ) {
        doc = ((CTabPanel *)widget(i))->data();

        if (doc && doc->viewId() == id)
            return QString::fromStdWString(doc->url());
    }

    return "";
}

bool CAscTabWidget::modifiedByIndex(int index)
{
    if (!(index < 0) && index < count()) {
        CAscTabData * doc = ((CTabPanel *)widget(index))->data();
        return doc->changed() && !doc->closed();
    }

    return false;
}

bool CAscTabWidget::closedByIndex(int index) {
    if (!(index < 0) && index < count()) {
        CAscTabData * doc = ((CTabPanel *)widget(index))->data();
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
        doc = ((CTabPanel *)widget(i))->data();

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
        doc = ((CTabPanel *)widget(i))->data();

        if ( doc->isViewType(cvwtEditor) &&
                doc->changed() && !doc->closed() &&
                (portal.empty() || doc->url().find(portal) != wstring::npos))
        {
            return i;
        }
    }

    return -1;
}

void CAscTabWidget::setFullScreen(bool apply, int id)
{
    QWidget * fsWidget;
    static QMetaObject::Connection cefConnection;
    if (!apply) {
        if (m_dataFullScreen) {
            fsWidget = m_dataFullScreen->widget();
            fsWidget->showNormal();

            disconnect(cefConnection);

            int index = m_dataFullScreen->tabindex();
            CAscTabData * doc = ((CTabPanel *)fsWidget)->data();

            insertTab(index, fsWidget, doc->title());
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
            fsWidget->showFullScreen();
            ((QCefView *)fsWidget)->GetCefView()->focus();

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
