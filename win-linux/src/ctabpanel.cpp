
#include "ctabpanel.h"
#include "cascapplicationmanagerwrapper.h"

#include <QDebug>

CTabPanel::CTabPanel(QWidget *parent)
    : QWidget(parent)
    , m_pViewer(new QCefView(this))
{
//    QGridLayout * _layout = new QGridLayout(this);
//    setLayout(_layout);

//    _layout->setMargin(0);
//    _layout->addWidget(m_pViewer);
}

CTabPanel::~CTabPanel()
{
    if ( m_pViewer )
        delete m_pViewer, m_pViewer = nullptr;

    if ( m_pLoader )
        delete m_pLoader, m_pLoader = nullptr;

    if ( m_pData )
        delete m_pData, m_pData = nullptr;
}

QCefView * CTabPanel::view()
{
    return m_pViewer;
}

CCefView * CTabPanel::cef()
{
    return m_pViewer->GetCefView();
}

void CTabPanel::setView(QCefView * v)
{
    if ( m_pViewer )
        delete m_pViewer, m_pViewer = nullptr;

    m_pViewer = v;
}

CAscTabData * CTabPanel::data()
{
    return m_pData;
}

void CTabPanel::setData(CAscTabData * d)
{
    if ( m_pData )
        delete m_pData, m_pData = nullptr;

    m_pData = d;
}

void CTabPanel::initAsEditor()
{
    m_pViewer->Create(&AscAppManager::getInstance(), cvwtEditor);
}

void CTabPanel::initAsSimple()
{
    m_pViewer->Create(&AscAppManager::getInstance(), cvwtSimple);
}

void CTabPanel::openLocalFile(const std::wstring& path, int format)
{
    static_cast<CCefViewEditor *>(m_pViewer->GetCefView())->OpenLocalFile(path, format);
}

void CTabPanel::createLocalFile(int format, const std::wstring& name)
{
    static_cast<CCefViewEditor *>(m_pViewer->GetCefView())->CreateLocalFile(format, name);
}

bool CTabPanel::openRecoverFile(int id)
{
    return static_cast<CCefViewEditor *>(m_pViewer->GetCefView())->OpenRecoverFile(id);
}

bool CTabPanel::openRecentFile(int id)
{
    return static_cast<CCefViewEditor *>(m_pViewer->GetCefView())->OpenRecentFile(id);
}

void CTabPanel::resizeEvent(QResizeEvent *event)
{
    m_pViewer->resize(event->size());
    cef()->resizeEvent(event->size().width(), event->size().height());
}

void CTabPanel::showEvent(QShowEvent *)
{
    cef()->resizeEvent();
}

void CTabPanel::resize(int w, int h)
{
    cef()->resizeEvent(w, h);
}
