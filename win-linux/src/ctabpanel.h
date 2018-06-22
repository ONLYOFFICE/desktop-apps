#ifndef CTABPANEL_H
#define CTABPANEL_H

#include <QWidget>
#include <QTimer>

#include "qcefview.h"
#include "casctabdata.h"

class CTabPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CTabPanel(QWidget *parent = nullptr);
    ~CTabPanel();

    QCefView * view();
    CCefView * cef();
    void setView(QCefView *);

    CAscTabData * data();
    void setData(CAscTabData *);

    void initAsEditor();
    void initAsSimple();

    void openLocalFile(const std::wstring& sFilePath, int nFileFormat);
    void createLocalFile(int nFileFormat, const std::wstring& sName = L"");
    bool openRecoverFile(int id);
    bool openRecentFile(int id);

    void resize(int w, int h);
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);

private:
    QCefView *      m_pViewer;
    QCefView *      m_pLoader = nullptr;
    CAscTabData *   m_pData = nullptr;

signals:
public slots:
};

#endif // CTABPANEL_H
