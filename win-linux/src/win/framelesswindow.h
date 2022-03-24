#ifndef CFRAMELESSWINDOW_H
#define CFRAMELESSWINDOW_H

#include <qsystemdetection.h>
#include <QObject>
#include <QMainWindow>
#include <QWidget>
#include <QList>
#include <QMargins>
#include <QRect>


class CFramelessWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit CFramelessWindow(QWidget *parent = nullptr);
    ~CFramelessWindow();
    void setResizeable(bool resizeable = true);
    bool isResizeable(){return m_bResizeable;}
    void setResizeableAreaWidth(int width = 5);
    void setContentsMargins(const QMargins &margins);
    void setContentsMargins(int left, int top, int right, int bottom);
    QMargins contentsMargins() const;
    QRect contentsRect() const;
    void getContentsMargins(int *left, int *top, int *right, int *bottom) const;

protected:
    void setTitleBar(QWidget* titlebar);

    //generally, we can add widget say "label1" on titlebar, and it will cover the titlebar under it
    //as a result, we can not drag and move the MainWindow with this "label1" again
    //we can fix this by add "label1" to a ignorelist, just call addIgnoreWidget(label1)
    void addIgnoreWidget(QWidget* widget);
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);

private:
    QWidget* m_titlebar;
    QList<QWidget*> m_whiteList;
    int m_borderWidth;
    QMargins m_margins;
    QMargins m_frames;
    bool m_bJustMaximized;
    bool m_bResizeable;

public slots:
    void showFullScreen();

private slots:
    void onTitleBarDestroyed();
};

#endif // CFRAMELESSWINDOW_H
