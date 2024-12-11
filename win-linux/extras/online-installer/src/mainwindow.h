#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "window.h"
#include <future>


class Label;
class ProgressBar;
class Widget;
class BoxLayout;
class CheckBox;
class RadioButton;
class Button;
class CDownloader;
class MainWindow : public Window
{
public:
    MainWindow(Widget *parent, const Rect &rc);
    ~MainWindow();

    void initInstallationMode(const std::wstring &url);
    void initControlMode(const std::wstring &arch);

protected:
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;

private:
    void startInstall(const std::wstring &url);
    void finishInstall(const std::wstring &app_path);
    void startUpdate();
    void startRepair();
    void startUninstall();
    void createSelectionPage();
    void createProgressPage(const std::wstring &text);
    void createCloseButton();
    void createCloseAndBackButtons();
    std::wstring fillInstalledVerInfo();
    CDownloader* startDownload(const std::wstring &url, const std::wstring &path, const std::function<void()> &onComplete);
    template<typename Fn, typename... Args>
    void invokeMethod(Fn&& fn, Args&&... args);

    enum class Mode : BYTE {Install, Control};
    enum Selectors : BYTE {UpdateRadio = 1, RepairRadio = 2, UninstRadio = 4, ClrDataCheck = 8, ClrStnCheck = 16, ClrAllCheck = 32, LaunchCheck = 64};
    std::future<void> m_future;
    std::wstring m_uninst_cmd,
                 m_ver,
                 m_arch,
                 m_package;
    Label       *m_comntLbl,
                *m_versionLbl,
                *m_comntInfoLbl;
    ProgressBar *m_bar;
    Widget      *m_cenPanel;
    BoxLayout   *m_cenPanelVlut;
    CheckBox    *m_launchCheck;
    RadioButton *m_updRadio,
                *m_repRadio,
                *m_uninsRadio;
    Button      *m_cancelBtn;
    Mode m_mode;
    int  m_resize_conn;
    BYTE m_checkState;
    bool m_is_checked,
         m_is_completed;
};

#endif // MAINWINDOW_H
