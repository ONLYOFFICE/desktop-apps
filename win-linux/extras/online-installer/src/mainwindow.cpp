#include "mainwindow.h"
#include "application.h"
#include "utils.h"
#include "checkbox.h"
#include "button.h"
#include "label.h"
#include "boxlayout.h"
#include "radiobutton.h"
#include "progressbar.h"
#include "metrics.h"
#include "palette.h"
#include "caption.h"
#include "resource.h"
#include "translator.h"
#include "cdownloader.h"
#include "baseutils.h"
#include <Msi.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include "../../src/defines.h"
#include "../../src/prop/defines_p.h"

#ifndef URL_INSTALL_X64
# define URL_INSTALL_X64 ""
#endif
#ifndef URL_INSTALL_X86
# define URL_INSTALL_X86 ""
#endif
#ifndef URL_INSTALL_X64_MSI
# define URL_INSTALL_X64_MSI ""
#endif
#ifndef URL_INSTALL_X86_MSI
# define URL_INSTALL_X86_MSI ""
#endif
#define _TR(str) Translator::tr(str).c_str()


template <class T>
static void setSelectorStyle(T *sel) // style for CheckBox and RadioButton
{
    sel->metrics()->setMetrics(Metrics::TextMarginLeft, 6);
    sel->metrics()->setMetrics(Metrics::PrimitiveRadius, 1);
    sel->metrics()->setMetrics(Metrics::AlternatePrimitiveWidth, 2);
    sel->palette()->setColor(Palette::Text, Palette::Disabled, 0x888888);
    sel->palette()->setColor(Palette::Text, Palette::Normal, 0x333333);
    sel->palette()->setColor(Palette::Text, Palette::Hover, 0x333333);
    sel->palette()->setColor(Palette::Text, Palette::Pressed, 0x333333);
    sel->palette()->setColor(Palette::Background, Palette::Disabled, 0xfefefe);
    sel->palette()->setColor(Palette::Background, Palette::Normal, 0xfefefe);
    sel->palette()->setColor(Palette::Background, Palette::Hover, 0xfefefe);
    sel->palette()->setColor(Palette::Background, Palette::Pressed, 0xfefefe);
    sel->palette()->setColor(Palette::Primitive, Palette::Disabled, 0x888888);
    sel->palette()->setColor(Palette::Primitive, Palette::Normal, 0x888888);
    sel->palette()->setColor(Palette::Primitive, Palette::Hover, 0x0055ff);
    sel->palette()->setColor(Palette::Primitive, Palette::Pressed, 0x0055ff);
}

static void setButtonStyle(Button *btn)
{
    btn->palette()->setColor(Palette::Text, Palette::Disabled, 0x888888);
    btn->palette()->setColor(Palette::Text, Palette::Normal, 0x333333);
    btn->palette()->setColor(Palette::Text, Palette::Hover, 0x333333);
    btn->palette()->setColor(Palette::Text, Palette::Pressed, 0x333333);
    btn->palette()->setColor(Palette::Background, Palette::Disabled, 0xeeeeee);
    btn->palette()->setColor(Palette::Background, Palette::Normal, 0xeeeeee);
    btn->palette()->setColor(Palette::Background, Palette::Hover, 0xe0e0e0);
    btn->palette()->setColor(Palette::Background, Palette::Pressed, 0xd0d0d0);
    btn->palette()->setColor(Palette::Border, Palette::Disabled, 0xbebebe);
    btn->palette()->setColor(Palette::Border, Palette::Normal, 0xbebebe);
    btn->palette()->setColor(Palette::Border, Palette::Hover, 0xbebebe);
    btn->palette()->setColor(Palette::Border, Palette::Pressed, 0xbebebe);
    btn->metrics()->setMetrics(Metrics::BorderWidth, 1);
}

static void setProgressStyle(ProgressBar *bar)
{
    bar->palette()->setColor(Palette::Background, Palette::Normal, 0xfefefe);
    bar->palette()->setColor(Palette::Base, Palette::Normal, 0xcccccc);
    bar->palette()->setColor(Palette::AlternateBase, Palette::Normal, 0x1e7aaa);
}

static void setLabelStyle(Label *lb)
{
    lb->palette()->setColor(Palette::Background, Palette::Normal, 0xfefefe);
    lb->palette()->setColor(Palette::Text, Palette::Normal, 0x888888);
}

static void setControlLabelStyle(Label *lb)
{
    lb->resize(50, 36);
    lb->palette()->setColor(Palette::Background, Palette::Normal, 0xfefefe);
    lb->palette()->setColor(Palette::Text, Palette::Normal, 0x333333);
    lb->metrics()->setMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVTop);
}

MainWindow::MainWindow(Widget *parent, const Rect &rc) :
    Window(parent, rc),
    m_comntLbl(nullptr),
    m_versionLbl(nullptr),
    m_comntInfoLbl(nullptr),
    m_bar(nullptr),
    m_launchCheck(nullptr),
    m_updRadio(nullptr),
    m_repRadio(nullptr),
    m_uninsRadio(nullptr),
    m_cancelBtn(nullptr),
    m_mode(Mode::Install),
    m_resize_conn(0),
    m_checkState(UpdateRadio | LaunchCheck),
    m_is_checked(false),
    m_is_completed(false)
{
    setWindowTitle(_TR(CAPTION));
    setResizable(false);
    setIcon(IDI_MAINICON);
    palette()->setColor(Palette::Background, Palette::Normal, 0xfefefe);
    palette()->setColor(Palette::Border, Palette::Normal, 0x888888);
    if (Utils::getWinVersion() > Utils::WinXP && Utils::getWinVersion() < Utils::Win10)
        metrics()->setMetrics(Metrics::BorderWidth, 1);

    Widget *cw = new Widget(this);
    cw->palette()->setColor(Palette::Background, Palette::Normal, 0xfefefe);
    setCentralWidget(cw);
    setContentsMargins(0,0,0,0);

    BoxLayout *cenVlut = new BoxLayout(BoxLayout::Vertical);
    cenVlut->setContentMargins(0, 0, 0, 0);
    cenVlut->setSpacing(0);
    cw->setLayout(cenVlut);

    /* Caption section*/
    Widget *topPanel = new Widget(cw);
    topPanel->resize(50,28);
    topPanel->palette()->setColor(Palette::Background, Palette::Normal, 0xfefefe);
    topPanel->setProperty(Widget::HSizeBehavior, Widget::Expanding);
    topPanel->setProperty(Widget::VSizeBehavior, Widget::Fixed);
    cenVlut->addWidget(topPanel);

    BoxLayout *topHlut = new BoxLayout(BoxLayout::Horizontal);
    topHlut->setContentMargins(0, 0, 0, 0);
    topHlut->setSpacing(0);
    topPanel->setLayout(topHlut);

    Caption *cap = new Caption(topPanel);
    cap->setResizingAvailable(false);
    cap->palette()->setColor(Palette::Background, Palette::Normal, 0xfefefe);
    cap->metrics()->setMetrics(Metrics::TextMarginLeft, 12);
    cap->metrics()->setMetrics(Metrics::TextAlignment, Metrics::AlignHLeft| Metrics::AlignVCenter);
    cap->resize(50,28);
    cap->setProperty(Widget::HSizeBehavior, Widget::Expanding);
    cap->setProperty(Widget::VSizeBehavior, Widget::Fixed);

    Button *closeBtn = new Button(topPanel);
    closeBtn->resize(40,28);
    closeBtn->setProperty(Widget::HSizeBehavior, Widget::Fixed);
    closeBtn->setProperty(Widget::VSizeBehavior, Widget::Fixed);
    closeBtn->palette()->setColor(Palette::Background, Palette::Normal, 0xfefefe);
    closeBtn->palette()->setColor(Palette::Background, Palette::Hover, 0xe81123);
    closeBtn->palette()->setColor(Palette::Background, Palette::Pressed, 0x8b0a14);
    closeBtn->palette()->setColor(Palette::Background, Palette::Disabled, 0x2b2b2b);
    closeBtn->palette()->setColor(Palette::Primitive, Palette::Normal, 0x000000);
    closeBtn->metrics()->setMetrics(Metrics::PrimitiveWidth, 1);
    closeBtn->setStockIcon(Button::CloseIcon);
    closeBtn->setIconSize(10, 10);
    closeBtn->onClick([this]() {
        close();
    });
    topHlut->addWidget(cap);
    topHlut->addWidget(closeBtn);

    /* Central section */
    m_cenPanel = new Widget(cw);
    m_cenPanel->palette()->setColor(Palette::Background, Palette::Normal, 0xfefefe);
    m_cenPanel->setProperty(Widget::HSizeBehavior, Widget::Expanding);
    m_cenPanel->setProperty(Widget::VSizeBehavior, Widget::Expanding);
    cenVlut->addWidget(m_cenPanel);

    m_cenPanelVlut = new BoxLayout(BoxLayout::Vertical);
    m_cenPanelVlut->setContentMargins(6, 12, 6, 48);
    m_cenPanelVlut->setSpacing(6);
    m_cenPanel->setLayout(m_cenPanelVlut);
}

MainWindow::~MainWindow()
{
    if (m_future.valid())
        m_future.wait();
}

void MainWindow::initInstallationMode(const std::wstring &url)
{
    m_mode = Mode::Install;
    /* Image section*/
    Label *wlcLbl = new Label(m_cenPanel);
    wlcLbl->resize(282, 200);
    wlcLbl->setImage(IDI_WELCOME, 282, 200);
    wlcLbl->palette()->setColor(Palette::Background, Palette::Normal, 0xfefefe);
    wlcLbl->setProperty(Widget::HSizeBehavior, Widget::Expanding);
    wlcLbl->setProperty(Widget::VSizeBehavior, Widget::Fixed);
    m_cenPanelVlut->addWidget(wlcLbl);

    /* Check box section*/
    CheckBox *chkBox = new CheckBox(m_cenPanel, _TR(CHECK_SILENT));
    chkBox->setGeometry(m_cenPanel->size().width/2 - 43, 254, 180, 16);
    setSelectorStyle(chkBox);
    chkBox->onClick([chkBox, this]() {
        m_is_checked = chkBox->isChecked();
    });

    /* Comment section */
    Label *comntLbl = new Label(m_cenPanel);
    comntLbl->setText(_TR(LABEL_WARN_CLOSE), true);
    comntLbl->setGeometry(0, m_cenPanel->size().height - 130, m_cenPanel->size().width, 48);
    setLabelStyle(comntLbl);

    /* Install button section */
    Button *instlBtn = new Button(m_cenPanel);
    instlBtn->setText(_TR(BUTTON_INSTALL));
    instlBtn->setGeometry(m_cenPanel->size().width/2 - 50, m_cenPanel->size().height - 76, 100, 28);
    setButtonStyle(instlBtn);
    instlBtn->onClick([=]() {
        m_cenPanel->disconnect(m_resize_conn);
        chkBox->close();
        comntLbl->close();
        instlBtn->close();
        startInstall(url);
    });

    m_resize_conn = m_cenPanel->onResize([chkBox, comntLbl, instlBtn](int w, int h) {
        chkBox->setGeometry(w/2 - 43, 254, 180, 16);
        comntLbl->setGeometry(0, h - 130, w, 48);
        instlBtn->setGeometry(w/2 - 50, h - 76, 100, 28);
    });
}

void MainWindow::initControlMode(const std::wstring &_arch)
{
    m_mode = Mode::Control;
    /* Comment section */
    m_versionLbl = new Label(m_cenPanel);
    setControlLabelStyle(m_versionLbl);
    m_versionLbl->setText(fillInstalledVerInfo());
    m_versionLbl->setProperty(Widget::HSizeBehavior, Widget::Expanding);
    m_versionLbl->setProperty(Widget::VSizeBehavior, Widget::Fixed);
    m_versionLbl->metrics()->setMetrics(Metrics::TextMarginLeft, 12);
    m_cenPanelVlut->setContentMargins(18, 6, 6, 6);
    m_cenPanelVlut->addWidget(m_versionLbl);

    if (m_package == _TR(LABEL_UNKN_PACK) || m_ver == _TR(LABEL_UNKN_VER) || _arch.empty() || m_arch != _arch) {
        Label *errLbl = new Label(m_cenPanel);
        setControlLabelStyle(errLbl);
        errLbl->setText(_TR(LABEL_NO_OPTIONS));
        errLbl->setProperty(Widget::HSizeBehavior, Widget::Expanding);
        errLbl->setProperty(Widget::VSizeBehavior, Widget::Expanding);
        errLbl->metrics()->setMetrics(Metrics::TextMarginLeft, 24);
        m_cenPanelVlut->addWidget(errLbl);
        return;
    }
    createSelectionPage();
}

bool MainWindow::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_INVOKEMETHOD: {
        if (std::function<void()> *func = (std::function<void()>*)wParam) {
            if (*func)
                (*func)();
            delete func;
        }
        break;
    }
    }
    return Window::event(msg, wParam, lParam, result);
}

void MainWindow::startInstall(const std::wstring &url)
{
    /* Comment section */
    m_comntLbl = new Label(m_cenPanel);
    m_comntLbl->setText(_TR(LABEL_DOWNLOAD), true);
    m_comntLbl->setGeometry(0, m_cenPanel->size().height - 156, m_cenPanel->size().width, 24);
    m_comntLbl->metrics()->setMetrics(Metrics::FontHeight, 20);
    setLabelStyle(m_comntLbl);

    m_comntInfoLbl = new Label(m_cenPanel);
    m_comntInfoLbl->setText(_TR(LABEL_ALMOST_DONE), true);
    m_comntInfoLbl->setGeometry(0, m_cenPanel->size().height - 112, m_cenPanel->size().width, 40);
    setLabelStyle(m_comntInfoLbl);

    /* Progress section */
    m_bar = new ProgressBar(m_cenPanel);
    m_bar->setGeometry(0, m_cenPanel->size().height - 126, m_cenPanel->size().width, 5);
    setProgressStyle(m_bar);
    m_bar->metrics()->setMetrics(Metrics::IconMarginLeft, 108);
    m_bar->metrics()->setMetrics(Metrics::IconMarginRight, 108);

    m_resize_conn = m_cenPanel->onResize([this](int w, int h) {
        m_comntLbl->setGeometry(0, h - 156, w, 24);
        m_comntInfoLbl->setGeometry(0, h - 112, w, 40);
        m_bar->setGeometry(0, m_cenPanel->size().height - 126, m_cenPanel->size().width, 5);
    });
    m_comntLbl->show();
    m_comntInfoLbl->show();
    m_bar->show();

    wstring path = NS_File::generateTmpFileName(L".exe");
    startDownload(url, path, [=]() {
            wstring args;
            if (m_is_checked) {
                args = _T("/VERYSILENT");
                m_comntLbl->setText(_TR(LABEL_INSTALL), true);
                m_bar->pulse(true);
            } else {
                hide();
            }
            if (!NS_File::runProcess(path, args)) {
                if (!m_is_checked)
                    show();
                m_bar->pulse(false);
                m_bar->setProgress(0);
                m_comntInfoLbl->setText(_TR(LABEL_ERR_RUNNING), true);
            } else {
                if (m_is_checked) {
                    wstring app_path;
                    if (NS_Utils::IsAppInstalled(app_path)) {
                        m_cenPanel->disconnect(m_resize_conn);
                        m_comntLbl->close();
                        m_comntInfoLbl->close();
                        m_bar->close();
                        invokeMethod(&MainWindow::finishInstall, this, app_path);
                    } else {
                        m_bar->pulse(false);
                        m_bar->setProgress(0);
                        m_comntLbl->setText(_TR(LABEL_ERR_INSTALL), true);
                    }
                } else {
                    close();
                }
            }
        });
}

void MainWindow::finishInstall(const std::wstring &app_path)
{
    /* Check box section*/
    m_is_checked = false;
    CheckBox *chkBox = new CheckBox(m_cenPanel, _TR(CHECK_LAUNCH));
    chkBox->setGeometry(m_cenPanel->size().width/2 - 43, 254, 180, 16);
    setSelectorStyle(chkBox);
    chkBox->onClick([chkBox, this]() {
        m_is_checked = chkBox->isChecked();
    });

    /* Comment section */
    wstring compl_text = _TR(LABEL_INSTALL_COMPL);
    NS_Utils::Replace(compl_text, L"%1", _T(WINDOW_NAME));
    Label *comntLbl = new Label(m_cenPanel);
    comntLbl->setText(compl_text, true);
    comntLbl->setGeometry(0, m_cenPanel->size().height - 130, m_cenPanel->size().width, 48);
    setLabelStyle(comntLbl);

    /* Install button section */
    Button *closeBtn = new Button(m_cenPanel);
    closeBtn->setText(_TR(BUTTON_CLOSE));
    closeBtn->setGeometry(m_cenPanel->size().width/2 - 50, m_cenPanel->size().height - 76, 100, 28);
    setButtonStyle(closeBtn);
    closeBtn->onClick([=]() {
        if (m_is_checked)
            NS_File::runProcess(app_path + _T(APP_LAUNCH_NAME), L"", false);
        close();
    });

    m_resize_conn = m_cenPanel->onResize([chkBox, comntLbl, closeBtn](int w, int h) {
        chkBox->setGeometry(w/2 - 43, 254, 180, 16);
        comntLbl->setGeometry(0, h - 130, w, 48);
        closeBtn->setGeometry(w/2 - 50, h - 76, 100, 28);
    });
    chkBox->show();
    comntLbl->show();
    closeBtn->show();
}

void MainWindow::startUpdate()
{
    wstring tmp_path = NS_File::toNativeSeparators(NS_File::generateTmpFileName(L"." + m_package));
    wstring url;
    if (m_package == L"msi") {
        url = (m_arch == L"x64") ? _T(URL_INSTALL_X64_MSI) : _T(URL_INSTALL_X86_MSI);
    } else {
        url = (m_arch == L"x64") ? _T(URL_INSTALL_X64) : _T(URL_INSTALL_X86);
    }

    CDownloader *dnl = startDownload(url, tmp_path, [=]() {
            m_bar->pulse(true);
            wstring args = L"/c \"" + tmp_path;
            args += (m_package == L"msi") ? L" /qn\"" : L" /UPDATE /VERYSILENT /NOLAUNCH\"";
            if (!NS_File::runProcess(L"cmd", args, true)) {
                m_bar->pulse(false);
                m_bar->setProgress(0);
                m_comntInfoLbl->setText(_TR(LABEL_ERR_RUNNING), true);
            } else {
                m_bar->pulse(false);
                m_bar->setProgress(100);
                m_comntLbl->setText(_TR(LABEL_UPDATE_COMPL));
                m_versionLbl->setText(fillInstalledVerInfo());
                m_is_completed = true;
            }
        });

    m_cancelBtn->onClick([=]() {
        dnl->stop();
    });
}

void MainWindow::startRepair()
{
    wstring tmp_path;
    if (m_package == L"msi") {
        wstring prodCode = NS_Utils::MsiProductCode(_T(WINDOW_NAME));
        if (prodCode.empty()) {
            m_comntInfoLbl->setText(_TR(LABEL_ERR_PROD_CODE), true);
            return;
        }
        wstring packageName =  NS_Utils::MsiGetProperty(prodCode.c_str(), INSTALLPROPERTY_PACKAGENAME);
        if (packageName.empty()) {
            m_comntInfoLbl->setText(_TR(LABEL_ERR_PACK_NAME), true);
            return;
        }
        tmp_path = NS_File::toNativeSeparators(NS_File::tempPath() + _T("/") + packageName);
    } else {
        tmp_path = NS_File::toNativeSeparators(NS_File::generateTmpFileName(L"." + m_package));
    }

    wstring url = L"https://github.com/%1/%2/releases/download/%3/%4";
    {
        wstring url_filename = L"DesktopEditors_" + m_arch;
        url_filename.append(L"." + m_package);

        wstring url_ver = L"v" + m_ver;
        size_t pos = url_ver.find_last_of(L'.');
        if (pos != std::wstring::npos)
            url_ver = url_ver.substr(0, pos);

        NS_Utils::Replace(url, L"%1", _T(REG_GROUP_KEY));
        NS_Utils::Replace(url, L"%2", _T(APP_NAME));
        NS_Utils::Replace(url, L"%3", url_ver);
        NS_Utils::Replace(url, L"%4", url_filename);
    }

    CDownloader *dnl = startDownload(url, tmp_path, [=]() {
            m_bar->pulse(true);
            wstring cmd = (m_package == L"msi") ? L"msiexec" : L"cmd",
                args = (m_package == L"msi") ? L"/fvamus \"" : L"/c \"";
                args += tmp_path;
                args += (m_package == L"msi") ? L"\" /qn" : L" /VERYSILENT\"";
            if (!NS_File::runProcess(cmd, args, true)) {
                m_bar->pulse(false);
                m_bar->setProgress(0);
                m_comntInfoLbl->setText(_TR(LABEL_ERR_RUNNING), true);
            } else {
                if (m_checkState & ClrDataCheck) {
                    wstring dataPath = NS_File::appDataPath();
                    if (!dataPath.empty())
                        NS_File::removeDirRecursively(dataPath);
                }
                if (m_checkState & ClrStnCheck) {
                    wstring key(L"SOFTWARE\\");
                    key.append(_T(REG_GROUP_KEY));
                    SHDeleteKey(HKEY_CURRENT_USER, key.c_str());
                }
                m_bar->pulse(false);
                m_bar->setProgress(100);
                m_comntLbl->setText(_TR(LABEL_REPAIR_COMPL));
                m_is_completed = true;
            }
        });

    m_cancelBtn->onClick([=]() {
        dnl->stop();
    });
}

void MainWindow::startUninstall()
{
    m_cancelBtn->setDisabled(true);
    m_bar->pulse(true);
    wstring args = L"/c \"" + m_uninst_cmd;
    args += (m_package == L"msi") ? L" /qn\"" : L" /VERYSILENT\"";
    m_future = std::async(std::launch::async, [=]() {
        if (!NS_File::runProcess(L"cmd", args, true)) {
            m_bar->pulse(false);
            m_bar->setProgress(0);
            m_comntInfoLbl->setText(_TR(LABEL_ERR_UNINST));
            createCloseAndBackButtons();
        } else {
            if (m_checkState & ClrAllCheck) {
                wstring dataPath = NS_File::appDataPath();
                if (!dataPath.empty())
                    NS_File::removeDirRecursively(dataPath);

                wstring key(L"SOFTWARE\\");
                key.append(_T(REG_GROUP_KEY));
                SHDeleteKey(HKEY_CURRENT_USER, key.c_str());
            }
            m_bar->pulse(false);
            m_bar->setProgress(100);
            m_comntLbl->setText(_TR(LABEL_UNINST_COMPL));
            // m_is_completed = true;
            createCloseButton();
        }
    });
}

void MainWindow::createSelectionPage()
{
    m_is_completed = false;
    /* Check box section*/
    CheckBox *clrChkBox = new CheckBox(m_cenPanel, _TR(CHECK_CLR_DATA));
    clrChkBox->setDisabled(!(m_checkState & RepairRadio));
    clrChkBox->setChecked(m_checkState & ClrDataCheck);
    clrChkBox->setGeometry(79, 114, 450, 18);
    setSelectorStyle(clrChkBox);
    clrChkBox->onClick([=]() {
        m_checkState = (m_checkState & ~ClrDataCheck) | (clrChkBox->isChecked() * ClrDataCheck);
    });

    CheckBox *stnChkBox = new CheckBox(m_cenPanel, _TR(CHECK_CLR_STNGS));
    stnChkBox->setDisabled(!(m_checkState & RepairRadio));
    stnChkBox->setChecked(m_checkState & ClrStnCheck);
    stnChkBox->setGeometry(79, 146, 450, 18);
    setSelectorStyle(stnChkBox);
    stnChkBox->onClick([stnChkBox, this]() {
        m_checkState = (m_checkState & ~ClrStnCheck) | (stnChkBox->isChecked() * ClrStnCheck);
    });

    CheckBox *clrAllChkBox = new CheckBox(m_cenPanel, _TR(CHECK_CLR_ALL));
    clrAllChkBox->setDisabled(!(m_checkState & UninstRadio));
    clrAllChkBox->setChecked(m_checkState & ClrAllCheck);
    clrAllChkBox->setGeometry(79, 216, 450, 18);
    setSelectorStyle(clrAllChkBox);
    clrAllChkBox->onClick([clrAllChkBox, this]() {
        m_checkState = (m_checkState & ~ClrAllCheck) | (clrAllChkBox->isChecked() * ClrAllCheck);
    });

    /* Update radio button section*/
    m_updRadio = new RadioButton(m_cenPanel, _TR(RADIO_UPDATE));
    m_updRadio->setChecked(m_checkState & UpdateRadio);
    m_updRadio->setGeometry(50, 48, 128, 18);
    setSelectorStyle(m_updRadio);
    m_updRadio->onClick([=]() {
        clrChkBox->setDisabled(true);
        stnChkBox->setDisabled(true);
        clrAllChkBox->setDisabled(true);
        if (m_repRadio->isChecked())
            m_repRadio->setChecked(false);
        if (m_uninsRadio->isChecked())
            m_uninsRadio->setChecked(false);
        m_checkState = (m_checkState | UpdateRadio) & ~(RepairRadio | UninstRadio);
    });

    /* Repair radio button section*/
    m_repRadio = new RadioButton(m_cenPanel, _TR(RADIO_REPAIR));
    m_repRadio->setChecked(m_checkState & RepairRadio);
    m_repRadio->setGeometry(50, 82, 128, 18);
    setSelectorStyle(m_repRadio);
    m_repRadio->onClick([=]() {
        clrChkBox->setDisabled(false);
        stnChkBox->setDisabled(false);
        clrAllChkBox->setDisabled(true);
        if (m_updRadio->isChecked())
            m_updRadio->setChecked(false);
        if (m_uninsRadio->isChecked())
            m_uninsRadio->setChecked(false);
        m_checkState = (m_checkState | RepairRadio) & ~(UninstRadio | UpdateRadio);
    });

    /* Uninstall radio button section*/
    m_uninsRadio = new RadioButton(m_cenPanel, _TR(RADIO_UNINST));
    m_uninsRadio->setChecked(m_checkState & UninstRadio);
    m_uninsRadio->setGeometry(50, 184, 128, 18);
    setSelectorStyle(m_uninsRadio);
    m_uninsRadio->onClick([=]() {
        clrChkBox->setDisabled(true);
        stnChkBox->setDisabled(true);
        clrAllChkBox->setDisabled(false);
        if (m_repRadio->isChecked())
            m_repRadio->setChecked(false);
        if (m_updRadio->isChecked())
            m_updRadio->setChecked(false);
        m_checkState = (m_checkState | UninstRadio) & ~(UpdateRadio | RepairRadio);
    });

    /* Apply button section */
    Button *applyBtn = new Button(m_cenPanel);
    applyBtn->setText(_TR(BUTTON_APPLY));
    applyBtn->setGeometry(m_cenPanel->size().width - 100 - 12, m_cenPanel->size().height - 28 - 12, 100, 28);
    setButtonStyle(applyBtn);
    applyBtn->onClick([=]() {
        wstring msg = m_uninsRadio->isChecked() ? _TR(MSG_REMOVE) : m_repRadio->isChecked() ? _TR(MSG_REPAIR) : _TR(MSG_UPDATE);
        NS_Utils::Replace(msg, L"%1", _T(WINDOW_NAME));
        if (IDOK == MessageBox(nativeWindowHandle(), msg.c_str(), _TR(CAPTION), MB_ICONWARNING | MB_OKCANCEL | MB_DEFBUTTON2)) {
            m_cenPanel->disconnect(m_resize_conn);
            m_updRadio->close();
            m_repRadio->close();
            clrChkBox->close();
            stnChkBox->close();
            m_uninsRadio->close();
            clrAllChkBox->close();
            applyBtn->close();
            msg = m_uninsRadio->isChecked() ? _TR(LABEL_UNINSTLING) : m_repRadio->isChecked() ? _TR(LABEL_REPAIRING) : _TR(LABEL_UPDATING);
            createProgressPage(msg);
            if (m_updRadio->isChecked() || m_repRadio->isChecked()) {
                /* Check box section*/
                m_launchCheck = new CheckBox(m_cenPanel, _TR(CHECK_LAUNCH));
                m_launchCheck->setChecked(m_checkState & LaunchCheck);
                m_launchCheck->setGeometry(42, 100, 450, 18);
                setSelectorStyle(m_launchCheck);
                m_launchCheck->onClick([this]() {
                    m_checkState = (m_checkState & ~LaunchCheck) | (m_launchCheck->isChecked() * LaunchCheck);
                });
            }
            if (m_uninsRadio->isChecked())
                startUninstall();
            else
            if (m_repRadio->isChecked())
                startRepair();
            else
                startUpdate();
        }
    });
    m_resize_conn = m_cenPanel->onResize([applyBtn](int w, int h) {
        applyBtn->setGeometry(w - 100 - 12, h - 28 - 12, 100, 28);
    });
    m_updRadio->show();
    m_repRadio->show();
    clrChkBox->show();
    stnChkBox->show();
    m_uninsRadio->show();
    clrAllChkBox->show();
    applyBtn->show();
}

void MainWindow::createProgressPage(const std::wstring &text)
{
    m_comntLbl = new Label(m_cenPanel);
    setControlLabelStyle(m_comntLbl);
    m_comntLbl->setText(text);
    m_comntLbl->metrics()->setMetrics(Metrics::TextMarginLeft, 12);
    m_comntLbl->setGeometry(30, 50, size().width - 30, 24);

    m_comntInfoLbl = new Label(m_cenPanel);
    setControlLabelStyle(m_comntInfoLbl);
    m_comntInfoLbl->metrics()->setMetrics(Metrics::TextMarginLeft, 12);
    m_comntInfoLbl->setGeometry(30, 92, m_cenPanel->size().width - 30, 40);

    /* Progress section */
    m_bar = new ProgressBar(m_cenPanel);
    m_bar->setGeometry(42, 80, 250, 5);
    setProgressStyle(m_bar);

    m_cancelBtn = new Button(m_cenPanel);
    m_cancelBtn->setText(_TR(BUTTON_CANCEL));
    m_cancelBtn->setGeometry(m_cenPanel->size().width - 100 - 12, m_cenPanel->size().height - 28 - 12, 100, 28);
    setButtonStyle(m_cancelBtn);
    m_resize_conn = m_cenPanel->onResize([this](int w, int h) {
        m_cancelBtn->setGeometry(w - 100 - 12, h - 28 - 12, 100, 28);
    });

    m_comntLbl->show();
    m_comntInfoLbl->show();
    m_bar->show();
    m_cancelBtn->show();
}

void MainWindow::createCloseButton()
{
    invokeMethod([=]() {
        m_cenPanel->disconnect(m_resize_conn);
        m_cancelBtn->close();
        Button *closeBtn = new Button(m_cenPanel);
        closeBtn->setText(_TR(BUTTON_CLOSE));
        closeBtn->setGeometry(m_cenPanel->size().width - 100 - 12, m_cenPanel->size().height - 28 - 12, 100, 28);
        setButtonStyle(closeBtn);
        closeBtn->onClick([=]() {
            m_cenPanel->disconnect(m_resize_conn);
            close();
        });
        m_resize_conn = m_cenPanel->onResize([closeBtn](int w, int h) {
            closeBtn->setGeometry(w - 100 - 12, h - 28 - 12, 100, 28);
        });
        closeBtn->show();
    });
}

void MainWindow::createCloseAndBackButtons()
{
    invokeMethod([=]() {
        m_cenPanel->disconnect(m_resize_conn);
        m_cancelBtn->close();

        if (m_launchCheck) {
            if (m_is_completed) {
                m_comntInfoLbl->hide();
                m_launchCheck->show();
            } else {
                m_launchCheck->close();
                m_launchCheck = nullptr;
            }
        }

        Button *closeBtn = new Button(m_cenPanel);
        closeBtn->setText(_TR(BUTTON_CLOSE));
        closeBtn->setGeometry(m_cenPanel->size().width - 100 - 12, m_cenPanel->size().height - 28 - 12, 100, 28);
        setButtonStyle(closeBtn);
        closeBtn->onClick([=]() {
            m_cenPanel->disconnect(m_resize_conn);
            if (m_launchCheck && m_is_completed && (m_checkState & LaunchCheck)) {
                wstring app_path;
                if (NS_Utils::IsAppInstalled(app_path))
                    NS_File::runProcess(app_path + _T(APP_LAUNCH_NAME), L"", false);
            }
            close();
        });

        Button *backBtn = new Button(m_cenPanel);
        backBtn->setText(_TR(BUTTON_BACK));
        backBtn->setGeometry(m_cenPanel->size().width - 100 - 12 - 106, m_cenPanel->size().height - 28 - 12, 100, 28);
        setButtonStyle(backBtn);
        backBtn->onClick([=]() {
            m_cenPanel->disconnect(m_resize_conn);
            if (m_launchCheck) {
                m_launchCheck->close();
                m_launchCheck = nullptr;
            }
            m_comntLbl->close();
            m_comntInfoLbl->close();
            m_bar->close();
            closeBtn->close();
            backBtn->close();
            createSelectionPage();
        });

        m_resize_conn = m_cenPanel->onResize([closeBtn, backBtn](int w, int h) {
            closeBtn->setGeometry(w - 100 - 12, h - 28 - 12, 100, 28);
            backBtn->setGeometry(w - 100 - 12 - 106, h - 28 - 12, 100, 28);
        });

        closeBtn->show();
        backBtn->show();
    });
}

wstring MainWindow::fillInstalledVerInfo()
{
    wstring text = _TR(LABEL_VERSION);
    NS_Utils::InstalledVerInfo(L"DisplayVersion", m_ver, m_arch);
    if (m_ver.empty())
        m_ver = _TR(LABEL_UNKN_VER);

    NS_Utils::InstalledVerInfo(L"UninstallString", m_uninst_cmd, m_arch);
    m_package = (m_uninst_cmd.find(L"msiexec") != std::wstring::npos) ? L"msi" : (m_uninst_cmd.find(L".exe") != std::wstring::npos) ? L"exe" : _TR(LABEL_UNKN_PACK);

    NS_Utils::Replace(text, L"%1", _T(WINDOW_NAME));
    NS_Utils::Replace(text, L"%2", m_ver);
    NS_Utils::Replace(text, L"%3", m_arch);
    NS_Utils::Replace(text, L"%4", m_package);
    return text;
}

CDownloader* MainWindow::startDownload(const std::wstring &url, const std::wstring &path, const std::function<void()> &onComplete)
{
    CDownloader *dnl = new CDownloader();
    dnl->onProgress([=](int percent) {
        m_bar->setProgress(percent);
    });
    dnl->onComplete([=](ulong error) {
        if (m_mode == Mode::Control)
            m_cancelBtn->setDisabled(true);
        if (error == ERROR_SUCCESS) {
            if (NS_File::verifyEmbeddedSignature(path)) {
                onComplete();
            } else {
                m_bar->setProgress(0);
                m_comntInfoLbl->setText(_TR(LABEL_NO_VER_AVAIL), true);
            }
            if (NS_File::fileExists(path))
                NS_File::removeFile(path);
        } else
        if (error == ERROR_CANCELLED) {
            m_comntInfoLbl->setText(_TR(LABEL_ERR_CANCELLED), true);
        } else {
            m_comntInfoLbl->setText(NS_Utils::GetLastErrorAsString(error), true);
        }

        if (m_mode == Mode::Control)
            createCloseAndBackButtons();
    });
    dnl->downloadFile(url, path);
    onAboutToDestroy([=]() {
        delete dnl;
    });
    return dnl;
}

template<typename Fn, typename... Args>
void MainWindow::invokeMethod(Fn&& fn, Args&&... args)
{
    std::function<void()> *func = new std::function<void()>(std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));
    PostMessage(m_hWnd, WM_INVOKEMETHOD, (WPARAM)func, 0);
}   // NOLINT
