#ifndef CSNAP_H
#define CSNAP_H

#include <QWidget>
#include <QPushButton>
#include <QObject>
#include <QTimer>
#include <QMap>
#include <windows.h>


class CWin11Snap : public QObject
{
    Q_OBJECT
public:
    CWin11Snap(QPushButton *btn);
    ~CWin11Snap();

private:
    void show();
    virtual bool eventFilter(QObject*, QEvent*) final;
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static void CALLBACK snapPopupEventProc(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG, LONG, DWORD, DWORD);
    static QMap<HWINEVENTHOOK, HWND> m_hookMap;
    HWINEVENTHOOK m_snapPopupEventHook;
    QPushButton *m_pBtn;
    QWidget   *m_pTopLevelWidget;
    QTimer    *m_pTimer;
    HWND      m_hWnd;
    bool      m_allowedChangeSize;
};

#endif // CSNAP_H
