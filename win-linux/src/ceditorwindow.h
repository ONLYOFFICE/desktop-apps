#ifndef CEDITORWINDOW_H
#define CEDITORWINDOW_H

#ifdef __linux__
# include "linux/csinglewindowplatform.h"
#else
# include "win/csinglewindowplatform.h"
#endif
#include "ctabpanel.h"
#include <windows.h>
#include <memory>

class CEditorWindow : public CSingleWindowPlatform
{
public:
    CEditorWindow();
    CEditorWindow(const QRect& rect, CTabPanel* view);
    ~CEditorWindow();

    bool holdView(int id) const override;
    int closeWindow();
    CTabPanel * mainView() const;
    CTabPanel * releaseEditorView() const;
    const QString& documentName() const;

private:
    CEditorWindow(const QRect&, const QString&, QWidget *);
    QWidget * createMainPanel(QWidget * parent, CTabPanel* const panel);
    QWidget * createMainPanel(QWidget * parent, const QString& title, bool custom, QWidget * panel) override;
    void recalculatePlaces();
    const QObject * receiver() override;

protected:
    void onCloseEvent() override;
    void onMinimizeEvent() override;
    void onMaximizeEvent() override;
    void onSizeEvent(int) override;
    void onScreenScalingFactor(uint) override;

    void onLocalFileSaveAs(void *);

private:
    friend class CEditorWindowPrivate;
    std::unique_ptr<CEditorWindowPrivate> d_ptr;
};

#endif // CEDITORWINDOW_H
