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

#include "components/asctabwidget.h"
#include <QRegExp>
#include <QHBoxLayout>
#include <QApplication>
#include "casctabdata.h"
#include "common/Types.h"
#include "defines.h"
#include "utils.h"
#include "cfilechecker.h"
#include "ceditortools.h"
#include "ctabundockevent.h"


using namespace std;

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
    srctype(etUndefined), id(-1)
{}

COpenOptions::COpenOptions(wstring _url_) :
    COpenOptions(_url_, etUndefined, -1)
{}

COpenOptions::COpenOptions(wstring _url_, AscEditorType _srctype_) :
    COpenOptions(_url_, _srctype_, -1)
{}

COpenOptions::COpenOptions(wstring _url_, AscEditorType _srctype_, int _id_) :
    COpenOptions(QString(), _srctype_, QString::fromStdWString(_url_), _id_)
{}

COpenOptions::COpenOptions(QString _name_, AscEditorType _srctype_, QString _url_) :
    COpenOptions(_name_, _srctype_, _url_, -1)
{}

COpenOptions::COpenOptions(QString _name_, AscEditorType _srctype_, QString _url_, int _id_) :
    name(_name_)
  , srctype(_srctype_)
  , url(Utils::replaceBackslash(_url_))
  , id(_id_)
  , wurl(url.toStdWString())
{}

COpenOptions::COpenOptions(QString _name_, AscEditorType _srctype_, std::wstring _url_, int _id_) :
    COpenOptions(_name_, _srctype_, QString::fromStdWString(_url_), _id_)
{}

COpenOptions::COpenOptions(QString _name_, AscEditorType _srctype_) :
    COpenOptions(_name_, _srctype_, "")
{}

/*
 *  TabWidget component
 *
*/

auto createTabPanel(QWidget * parent, CTabPanel * panel = nullptr) -> QWidget * {
    QWidget * panelwidget = new QWidget(parent);

    panelwidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QGridLayout *layout = new QGridLayout(panelwidget);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    panelwidget->setLayout(layout);
    panelwidget->layout()->addWidget(panel ? panel : new CTabPanel);

    return panelwidget;
}

auto panelfromwidget(QWidget * panelwidget) -> CTabPanel * {
    return panelwidget->children().count() ? static_cast<CTabPanel *>(panelwidget->findChild<CTabPanel*>()) : nullptr;
}

CAscTabWidget::CAscTabWidget(QWidget *parent, CTabBar *_pBar)
    : QStackedWidget(parent)
    , CScalingWrapper(parent)
    , m_dataFullScreen(0)
    , m_widthParams({{100, 135, 9}, 68, 3, 0, WINDOW_TITLE_MIN_WIDTH, 140, 0})
    , m_defWidthParams(m_widthParams)
    , m_isCustomStyle(true)
    , m_tabIconSize(16, 16)
    , m_pBar(_pBar)
{
    m_pBar->setObjectName("asc_editors_tabbar");
    setProperty("active", false);
    setProperty("empty", true);
    m_pBar->setProperty("active", false);

    static int _dropedindex = -1;
    QObject::connect(this, &CAscTabWidget::currentChanged, this, [=](int index) {
        QTimer::singleShot(0, this, [=]() {
            updateIcons();
        });
        setFocusedView();
        _dropedindex = -1;
    });
    QObject::connect(this, &CAscTabWidget::widgetRemoved, this, [=](int index) {
        emit editorRemoved(index, count());
    });
    QObject::connect(m_pBar, &CTabBar::tabUndock, this, [=](int index, bool &accept) {
        if (index == _dropedindex) return;

        const CTabPanel * _panel = panel(index);

        if ( _panel->data()->viewType() == cvwtEditor ) {
            CTabUndockEvent event(index);
            QObject * obj = qobject_cast<QObject*>(&AscAppManager::getInstance());
            if ( QApplication::sendEvent(obj, &event) && event.isAccepted() ) {
                _dropedindex = index;
                accept = true;

                QTimer::singleShot(0, this, [=]() {
                    if (widget(index)) {
                        widget(index)->deleteLater();
                    }
                });
            }
        }
    });
    auto turnOffAltHints = [=](int old_index, int index) {
        this->setCurrentIndex(index);
        if (old_index > -1)
            AscAppManager::sendCommandTo(panel(old_index)->cef(), L"althints:show", L"false");
    };
    QObject::connect(m_pBar, &CTabBar::tabBarClicked, this, [=](int index) {
        turnOffAltHints(m_pBar->currentIndex(), index);
    });
    QObject::connect(m_pBar, &CTabBar::onCurrentChangedByWhell, this, [=](int index) {
        turnOffAltHints(m_pBar->currentIndex(), index);
    });
    QObject::connect(m_pBar, &CTabBar::tabMoved, this, [=](int from, int to) {
        if (from < 0 || from >= count() || to < 0 || to >= count() || from == to)
            return;
        auto wgt = widget(from);
        removeWidget(wgt);
        insertWidget(to, wgt);
    });
}

CTabPanel * CAscTabWidget::panel(int index) const
{
    QWidget * _w = widget(index);
    return _w->children().count() ? static_cast<CTabPanel *>(_w->findChild<CTabPanel*>()) : nullptr;
}

CTabBar *CAscTabWidget::tabBar() const
{
    return m_pBar;
}

int CAscTabWidget::addEditor(const COpenOptions& opts)
{
    if ( opts.url.isEmpty() && opts.srctype != etNewFile )
        return -1;

    setProperty("empty", false);

    int file_format = 0;
    if (opts.srctype == etLocalFile) {
        file_format = CCefViewEditor::GetFileFormat(opts.wurl);
        if (file_format == 0)
            /* TODO: show error for file format */
            return -255;
    }

    QWidget * panelwidget = createTabPanel(this);
    CTabPanel * pView = panelfromwidget(panelwidget);

    pView->initAsEditor();

    int tab_index = -1;
    bool res_open = true;
    if (opts.srctype == etLocalFile) {
        pView->openLocalFile(opts.wurl, file_format, L"");
    } else
    if (opts.srctype == etRecoveryFile) {
        res_open = pView->openRecoverFile(opts.id);
    } else
    if (opts.srctype == etRecentFile) {
        res_open = pView->openRecentFile(opts.id);
    } else
    if (opts.srctype == etNewFile) {
        pView->createLocalFile(CEditorTools::editorTypeFromFormat(opts.format), opts.name.toStdWString());
    } else {
        pView->cef()->load(opts.wurl);
    }

    if (res_open) {
        CAscTabData * data = new CAscTabData(opts.name);
        data->setUrl(opts.wurl);
        data->setIsLocal( opts.srctype == etLocalFile || opts.srctype == etNewFile ||
                       (opts.srctype == etRecentFile && !CExistanceController::isFileRemote(opts.url)) );

        data->setContentType(CEditorTools::editorTypeFromFormat(opts.format));
        data->setChanged(opts.srctype == etRecoveryFile);

        pView->setData(data);
        tab_index = addWidget(panelwidget);
        m_pBar->addTab(data->title());
        m_pBar->setTabToolTip(tab_index, data->title());
        m_pBar->tabStartLoading(tab_index);
        //m_pBar->setCurrentIndex(tab_index);

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
            m_pBar->removeTab(i);
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

int CAscTabWidget::count(int type) const
{
    if ( type < 0 )
        return QStackedWidget::count();
    else {
        int _out(0);
        for (int i(count()); i-- > 0; ) {
            if ( (panel(i))->data()->viewType() == type )
                ++_out;
        }
        return _out;
    }
}

int CAscTabWidget::count(const wstring& portal, bool exclude)
{
    if ( portal.empty() )
        return QStackedWidget::count();
    else {
        int _out(0);
        for (int i(count()); i-- > 0; ) {
            if ( panel(i)->data()->url().find(portal) != wstring::npos ) {
                if ( exclude && panel(i)->data()->isViewType(cvwtSimple) )
                    continue;

                ++_out;
            }
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

int CAscTabWidget::addPortal(const QString& url, const QString& name, const QString& provider, const QString& entrypage)
{
    if ( url.isEmpty() ) return -1;

    setProperty("empty", false);

    QString args, _url = url;
    if ( provider == "onlyoffice" && !_url.contains(QRegularExpression("desktop=true")) )
        args.append("/?desktop=true");
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

    pView->initAsSimple();
    pView->cef()->SetExternalCloud(provider.toStdWString());
    pView->cef()->load((_url + entrypage + args).toStdWString());

    QString portal = name.isEmpty() ? Utils::getPortalName(url) : name;

    CAscTabData * data = new CAscTabData(portal, etPortal);
    data->setUrl(_url);
    pView->setData(data);

    int tab_index = -1;

    tab_index = insertWidget(tab_index, panelwidget);
    m_pBar->insertTab(tab_index, portal);
    m_pBar->setTabToolTip(tab_index, _url);
    m_pBar->setTabIconTheme(tab_index, CTabBar::LightTab);
    m_pBar->tabStartLoading(tab_index);
//    updateTabIcon(tabIndexByView(id));

    return tab_index;
}

int CAscTabWidget::addOAuthPortal(const QString& portal, const QString& type, const QString& service, const QString& entrypage)
{
    if ( service.isEmpty() || !type.contains(QRegularExpression("sso|outer")) ) return -1;

    setProperty("empty", false);

    QWidget * panelwidget = createTabPanel(this);
    CTabPanel * pView = panelfromwidget(panelwidget);
    pView->initAsSimple();

    if ( type == "sso" ) {
        pView->cef()->load(("sso:" + service).toStdWString());
    } else
    if ( type == "outer" ) {
        pView->cef()->SetExternalCloud(service.toStdWString());

        QString _postfix;
        if (service == "onlyoffice") _postfix = "/?desktop=true";
        pView->cef()->load((portal + entrypage + _postfix).toStdWString());
    }

    QString _portal = portal.isEmpty() ? Utils::getPortalName(service) : Utils::getPortalName(portal);

    CAscTabData * data = new CAscTabData(_portal, etPortal);
    data->setUrl(portal);
    pView->setData(data);

    int tab_index = -1;

    tab_index = insertWidget(tab_index, panelwidget);
    m_pBar->insertTab(tab_index, _portal);
    m_pBar->setTabToolTip(tab_index, portal);
    m_pBar->setTabIconTheme(tab_index, CTabBar::LightTab);
    m_pBar->tabStartLoading(tab_index);

    return tab_index;
}

int CAscTabWidget::insertPanel(QWidget * panel, int index)
{
    int tabindex = -1;

    CTabPanel * _panel = dynamic_cast<CTabPanel *>(panel);
    Q_ASSERT(_panel != nullptr);
    if ( _panel ) {
        const CAscTabData * tabdata = _panel->data();

        QWidget * panelwidget = createTabPanel(this, _panel);

        tabindex = insertWidget(index, panelwidget);
        m_pBar->insertTab(tabindex, tabdata->title());
        m_pBar->setTabToolTip(tabindex, tabdata->title());
    }

    return tabindex;
}

int CAscTabWidget::insertWidget(int index, QWidget* widget)
{
    int actual_index = QStackedWidget::insertWidget(index, widget);
    emit editorInserted(actual_index, count());
    return actual_index;
}

void CAscTabWidget::setCustomWindowParams(bool iscustom)
{
    m_isCustomStyle = iscustom;
    m_widthParams.tools_width = int((iscustom ? 50 : 0) * scaling());
    m_widthParams.title_width = int((iscustom ? WINDOW_TITLE_MIN_WIDTH : 0) * scaling());
}

void CAscTabWidget::updateIcons()
{
    for (int i(count()); i-- > 0;) {
        updateTabIcon(i);
    }
}

void CAscTabWidget::updateTabIcon(int index)
{
    if ( !(index < 0) && panel(index)) {
        CCefViewEditor * pEditor = (CCefViewEditor *)panel(index)->cef();

        if (pEditor) {
            bool is_active = isActiveWidget() && index == currentIndex();
            int tab_type = etUndefined;
            QString active_tab_color = "none";
            CTabBar::TabTheme tab_theme = is_active ? CTabBar::DarkTab : CTabBar::LightTab;

            auto _is_editor_supports_theme = [&](int index) {
                CAscTabData& data = *(panel(index)->data());
                return data.isViewType(cvwtEditor) && (data.features().empty() || data.hasFeature(L"uithemes"));
            };
            const CTheme& ui_theme = AscAppManager::themes().current().isDark() && _is_editor_supports_theme(index) ?
                                            AscAppManager::themes().current() : AscAppManager::themes().defaultLight();

            tab_type = pEditor->GetEditorType();
            switch ( tab_type ) {
            case etPresentation: case etSpreadsheet: case etDocument: break;
            default: tab_type = panel(index)->data()->contentType(); break;
            }

            if ( !is_active ) {
                tab_theme = AscAppManager::themes().current().isDark() ? CTabBar::DarkTab : CTabBar::LightTab;
            } else {
                switch ( tab_type ) {
                case etPresentation: active_tab_color = QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabSlideActive)); break;
                case etSpreadsheet: active_tab_color =  QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabCellActive)); break;
                case etDocumentMasterForm:
                case etDocument: active_tab_color =  QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabWordActive)); break;
                case etNewPortal:
                case etPortal:
                    active_tab_color =  QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabSimpleActiveBackground));
                    tab_theme = CTabBar::LightTab;
                    break;
                default:
                    tab_type = etUndefined;
                    active_tab_color =  QString::fromStdWString(ui_theme.value(CTheme::ColorRole::ecrTabDefaultActiveBackground));
                    tab_theme = AscAppManager::themes().isColorDark(active_tab_color) ? CTabBar::DarkTab : CTabBar::LightTab;
                    break;
                }
            }
            QString icon_name = is_active ? m_mapTabIcons.at(tab_type).second : m_mapTabIcons.at(tab_type).first;
            m_pBar->setTabIcon(index, QIcon(icon_name));
//            m_pBar->setTabIconTheme(index, tab_theme);
            if ( index == currentIndex() ) {
                if (tab_type == etPortal || tab_type == etNewPortal || tab_type == etUndefined)
                    m_pBar->setUseTabCustomPalette(index, true);
                else {
                    m_pBar->setUseTabCustomPalette(index, false);
                    m_pBar->setActiveTabColor(index, active_tab_color);
                }
            }
        }
    }
}

void CAscTabWidget::setTabIcons(CTabIconSet& icons)
{
    m_mapTabIcons = icons;
}

void CAscTabWidget::reloadTabIcons()
{
    m_mapTabIcons.clear();
    m_mapTabIcons.insert({
        {etUndefined, std::make_pair(":/tabbar/icons/newdoc.svg", ":/tabbar/icons/newdoc.svg")},
        {etDocument, std::make_pair(":/tabbar/icons/de.svg", ":/tabbar/icons/de.svg")},
        {etPresentation, std::make_pair(":/tabbar/icons/pe.svg", ":/tabbar/icons/pe.svg")},
        {etDocumentMasterForm, std::make_pair(":/tabbar/icons/docxf.svg", ":/tabbar/icons/docxf.svg")},
        {etSpreadsheet, std::make_pair(":/tabbar/icons/se.svg", ":/tabbar/icons/se.svg")}
    });

    AscAppManager::themes().current().isDark() ?
        m_mapTabIcons.insert({{etPortal, std::make_pair(":/tabbar/icons/portal_light.svg", ":/tabbar/icons/portal.svg")},
                        {etNewPortal, std::make_pair(":/tabbar/icons/portal_light.svg", ":/tabbar/icons/portal.svg")}}) :
        m_mapTabIcons.insert({{etPortal, std::make_pair(":/tabbar/icons/portal.svg", ":/tabbar/icons/portal.svg")},
                         {etNewPortal, std::make_pair(":/tabbar/icons/portal.svg", ":/tabbar/icons/portal.svg")}});

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
            m_pBar->setCurrentIndex(tabIndex);
    } else {
        opts.name   = tr("Document");
//        opts.type   = etUndefined;
        tabIndex    = addEditor(opts);

        updateIcons();

        if (select && !(tabIndex < 0))
            m_pBar->setCurrentIndex(tabIndex);
    }

    return tabIndex;
}

int CAscTabWidget::openLocalDocument(const COpenOptions& options, bool select, bool forcenew)
{
    int tabIndex = -1;
    if ( !forcenew && options.srctype != etRecoveryFile ) {
        CCefView * view = AscAppManager::getInstance().GetViewByRecentId( options.id );
        if ( view ) {
            tabIndex = tabIndexByView(view->GetId());
        } else {
            tabIndex = tabIndexByUrl(options.wurl);
        }
    }

    if (tabIndex < 0){
        COpenOptions _opts{options};
        _opts.name = QFileInfo(options.url).fileName();
        tabIndex = addEditor(_opts);

        if (!(tabIndex < 0))
            updateIcons();
    }

    if (select && !(tabIndex < 0))
        setCurrentIndex(tabIndex);

    /* TODO: rise message if index < 0 */

    return tabIndex;
}

int CAscTabWidget::openPortal(const QString& url, const QString& provider, const QString& entrypage)
{
    QString portal_name = Utils::getPortalName(url);

    int tabIndex = tabIndexByTitle(portal_name, etPortal);
    if (tabIndex < 0) {
        tabIndex = addPortal(url, "", provider, entrypage);
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
            if ( url.isEmpty() )
                _panel->cef()->load(_panel->data()->url());
            else _panel->cef()->load(url.toStdWString());

            return true;
        }
    }

    return false;
}

int CAscTabWidget::newPortal(const QString& url, const QString& name)
{
    int tabIndex = tabIndexByEditorType(etNewPortal);
    if ( tabIndex < 0 ) {
        if ( !((tabIndex = addPortal(url, name, "onlyoffice")) < 0) ) {
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
        for (int i = m_pBar->count(); i-- > 0; ) {
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

        m_pBar->setTabText(tabIndex, doc->title());
        m_pBar->setTabToolTip(tabIndex, path.isEmpty() ? doc->title() : path);
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
            m_pBar->setTabText(tabIndex, doc->title());
            m_pBar->setTabToolTip(tabIndex, doc->title());
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
            m_pBar->setTabLoading(tabIndex, false);
            panel(tabIndex)->applyLoader("hide");
            panel(tabIndex)->setReady();
            return;
        } else
        if ( type == DOCUMENT_CHANGED_PAGE_LOAD_FINISH ) {
            if ( !panel(tabIndex)->data()->eventLoadSupported() ) {
                m_pBar->setTabLoading(tabIndex, false);
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
        CAscTabData * doc = panel(tabIndex)->data();

        doc->setFeatures(option);
        updateTabIcon(tabIndex);

        size_t _pos;
        if ((_pos = option.find(L"eventloading:")) != wstring::npos) {
            if (option.find(L"true", _pos + 1) != wstring::npos)
                doc->setEventLoadSupported(true);
        }

        if (option.find(L"readonly\":") != wstring::npos) {
            m_pBar->setTabText(tabIndex, doc->title());
            m_pBar->setTabToolTip(tabIndex, doc->title());
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
    if ( !isActiveWidget() )
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
    this->setHidden(!a);
    if (property("active").toBool() != a) {
        this->setProperty("active", a);
        m_pBar->setProperty("active", a);
    }
    updateIcons();
    m_pBar->polish();
}

bool CAscTabWidget::isActiveWidget()
{
    return property("active").toBool();
}

int CAscTabWidget::modifiedCount()
{
    int mod_count = 0;
    const CAscTabData * doc;

    for (int i = m_pBar->count(); i-- > 0; ) {
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
    for (int i(m_pBar->count()); i-- > 0; i++) {
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
    for (int i(m_pBar->count()); i-- > 0; ) {
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
    for (int i(m_pBar->count()); i-- > 0; ) {
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
    CTabPanel * fsWidget;
    static QMetaObject::Connection cefConnection;
    if (!apply) {
        if (m_dataFullScreen) {
            disconnect(cefConnection);

#ifdef _LINUX
            AscAppManager::mainWindow()->show(false);
#else
            AscAppManager::mainWindow()->show(false);
#endif

            int index = m_dataFullScreen->tabindex();
            fsWidget = qobject_cast<CTabPanel *>(m_dataFullScreen->widget());
            widget(index)->layout()->addWidget(fsWidget);

            RELEASEOBJECT(m_dataFullScreen->parent)
            RELEASEOBJECT(m_dataFullScreen)
//            updateGeometry();
        }
    } else {
        int tabIndex = tabIndexByView(id);
        if ( tabIndex < 0 )
            tabIndex = currentIndex();
        else
        if ( tabIndex != currentIndex() ) {
            setCurrentIndex(tabIndex);
        }

        fsWidget = panel(tabIndex);

        if ( fsWidget ) {
            m_dataFullScreen = new CFullScreenData(tabIndex, fsWidget);

            m_dataFullScreen->parent = WindowHelper::constructFullscreenWidget(fsWidget);
            fsWidget->view()->setFocusToCef();
            AscAppManager::mainWindow()->hide();

            auto _break_demonstration = [=]() {
                NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
                pCommand->put_Command(L"editor:stopDemonstration");
                NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EDITOR_EXECUTE_COMMAND);
                pEvent->m_pData = pCommand;
                fsWidget->cef()->Apply(pEvent);
            };

            cefConnection = connect(fsWidget, &CTabPanel::closePanel, [=](QCloseEvent * e){
                _break_demonstration();
                e->ignore();
                // TODO: associate panel with reporter window and close both simultaneously
                QTimer::singleShot(10, [=] {emit m_pBar->tabCloseRequested(m_dataFullScreen->tabindex());});
            });

            connect((CFullScrWidget*)m_dataFullScreen->parent, &CFullScrWidget::closeRequest, this, [=]() {
                disconnect((CFullScrWidget*)m_dataFullScreen->parent);
                _break_demonstration();
            });
        }
    }
}

QWidget * CAscTabWidget::fullScreenWidget()
{
    return m_dataFullScreen ? m_dataFullScreen->widget() : nullptr;
}

void CAscTabWidget::setStyleSheet(const QString& stylesheet)
{
    QStackedWidget::setStyleSheet(stylesheet);

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
//    if (!(r.indexIn(stylesheet) < 0)) {
//        ((CTabBar *)tabBar())->setTabTextColor(QPalette::Active, _string_to_color(r.cap(1)) );
//        ((CTabBar *)tabBar())->setTabTextColor(QPalette::Inactive, _string_to_color(r.cap(2)) );
//    }

    r.setPattern("QTabBar::tab-icon\\s*\\{([^\\}]+)");
    if ( !(r.indexIn(stylesheet) < 0) ) {
        QRegExp ri("width:\\s*(\\d+);\\s*height:\\s*(\\d+)");

        if ( !(ri.indexIn(r.cap(1)) < 0) ) {
            m_tabIconSize.setWidth(ri.cap(1).toInt());
            m_tabIconSize.setHeight(ri.cap(2).toInt());
        }
    }
}

void CAscTabWidget::applyUITheme(const std::wstring& theme)
{
    reloadTabIcons();
    updateIcons();
    m_pBar->setIgnoreActiveTabColor(AscAppManager::themes().current().isDark());
    m_pBar->polish();
    style()->polish(this);

    QColor back_color = AscAppManager::themes().current().color(CTheme::ColorRole::ecrWindowBackground);
    for (int i(count()); i-- > 0; ) {
        panel(i)->setBackground(back_color);
    }
}
