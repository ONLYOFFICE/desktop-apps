/*
 * (c) Copyright Ascensio System SIA 2010-2016
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

#include "cdownloadwidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QMenu>
#include <QEvent>
#include <QActionEvent>
#include <QApplication>
#include "qcefview.h"

#include <QDebug>

#define DOWNLOAD_WIDGET_MAX_WIDTH 350

using namespace NSEditorApi;

class CDownloadWidget::CDownloadItem {
public:
    CDownloadItem(QCefView * p, QWidget * w)
        : _p_view(p), _p_progress(w), _is_temp(true)
    {}

    QCefView * view() const { return _p_view; }
    QWidget * progress() const { return _p_progress; }
//    void * info() const { return _p_info; }

    bool is_temporary() const { return true; }
    void set_is_temporary(bool v) { _is_temp = v; }
private:
    QCefView * _p_view;
    QWidget * _p_progress;
//    void * _p_info;
    bool _is_temp;
};

CDownloadWidget::CDownloadWidget(QWidget *parent)
    : QWidget(parent), m_pManager(NULL), m_parentButton(NULL)
{
    setLayout(new QVBoxLayout);
    connect(this, SIGNAL(cancelDownload(int)), this, SLOT(onDownloadCanceled(int)), Qt::QueuedConnection);

    setMaximumWidth(DOWNLOAD_WIDGET_MAX_WIDTH);
}

CDownloadWidget::~CDownloadWidget()
{

}

void CDownloadWidget::setManagedElements(CAscApplicationManager * m, QPushButton * b)
{
    m_pManager = m;
    m_parentButton = b;

    m_parentButton->hide();
}

QWidget * CDownloadWidget::addFile(const QString& fn, int id)
{
    QWidget * widget = new QWidget(this);
    QGridLayout * grid = new QGridLayout;

    QLabel * name = new QLabel(fn);
    name->setObjectName("labelName");
    QProgressBar * progress = new QProgressBar;
    QPushButton * cancel = new QPushButton(tr("Cancel"));
    cancel->setObjectName("buttonCancel");
    connect(cancel, &QPushButton::clicked, [=](){
        if (m_parentButton)
            m_parentButton->menu()->close();

        emit cancelDownload(id);
    });

    progress->setTextVisible(false);

    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 0);

    grid->addWidget(name, 0, 0, 1, -1);
    grid->addWidget(progress, 1, 0, 1, 1);
    grid->addWidget(cancel, 1, 1, 1, 1);

    widget->setLayout(grid);
    layout()->addWidget(widget);

    updateLayoutGeomentry();
    return widget;
}

void CDownloadWidget::onDocumentDownload(void * info, bool manual)
{
    if (NULL == m_pManager || info == NULL )
        return;

    CAscDownloadFileInfo * pData = reinterpret_cast<CAscDownloadFileInfo *>(info);
    int id = pData->get_Id();

    std::map<int, CDownloadItem *>::iterator iter = m_mapDownloads.find(id);
    if (pData->get_IsComplete()) {
        removeFile(iter);
    } else
    if (pData->get_IsCanceled()) {
        onDownloadCanceled(id);
    } else {
        if (iter == m_mapDownloads.end()) {
//            ADDREFINTERFACE(pData);

            QString path = QString::fromStdWString(pData->get_FilePath()),
                    file_name = "Unconfirmed";

            if (path.length()) {
                file_name = getFileName(path);
            }

            QCefView * pView = NULL;
            if (manual) {
                pView = new QCefView(NULL);
                pView->Create(m_pManager, cvwtEditor);
                pView->GetCefView()->SetParentCef(id);
                pView->GetCefView()->load(pData->get_Url());

                id = pView->GetCefView()->GetId();
            }

            QWidget * download_field = addFile(file_name, id);
            CDownloadItem * item = new CDownloadItem(pView, download_field);

            iter = m_mapDownloads.insert( std::pair<int, CDownloadItem *>(id, item) ).first;

            if (!path.isEmpty()) {
                item->set_is_temporary(false);

                if ( !m_parentButton->isVisible()) {
                    m_parentButton->setVisible(true);
                }
            }
        }

        updateProgress(iter, pData);
    }

    RELEASEINTERFACE(pData);
}

void CDownloadWidget::updateLayoutGeomentry()
{
    adjustSize();

    if (m_parentButton && m_parentButton->menu()) {
        QActionEvent e(QEvent::ActionChanged, m_parentButton->menu()->actions().at(0));
        QApplication::sendEvent(m_parentButton->menu(), &e);
    }
}

void CDownloadWidget::onDownloadCanceled(int id)
{
    removeFile(id);
}

void CDownloadWidget::removeFile(int id)
{
    removeFile(m_mapDownloads.find(id));
}

void CDownloadWidget::removeFile(MapItem iter)
{
    if (iter != m_mapDownloads.end()) {
        CDownloadItem * di = reinterpret_cast<CDownloadItem *>((*iter).second);

//        CAscDownloadFileInfo * pData = reinterpret_cast<CAscDownloadFileInfo *>(di->info());
//        RELEASEINTERFACE(pData)

        QCefView * pView = di->view();
        QWidget * pItemWidget = di->progress();

        layout()->removeWidget(pItemWidget);

        RELEASEOBJECT(pView)
        RELEASEOBJECT(pItemWidget)
        RELEASEOBJECT(di)

        updateLayoutGeomentry();
        m_mapDownloads.erase(iter);

        if (!m_mapDownloads.size() && m_parentButton->isVisible())
            m_parentButton->menu()->hide();
            m_parentButton->hide();
    }
}

void CDownloadWidget::updateProgress(MapItem iter, void * data)
{
    QLabel * label_name;
    QProgressBar * progress;
    CDownloadItem * d_item;
    CAscDownloadFileInfo * pData;

    d_item = reinterpret_cast<CDownloadItem *>((*iter).second);
    if (d_item) {
//        pData = reinterpret_cast<CAscDownloadFileInfo *>(d_item->info());
        pData = reinterpret_cast<CAscDownloadFileInfo *>(data);
        progress = qobject_cast<QProgressBar *>(d_item->progress()->layout()->itemAt(1)->widget());

        if (progress && pData) {
            progress->setValue(pData->get_Percent());
        }

        if (d_item->is_temporary()) {
            QString path = QString().fromStdWString(pData->get_FilePath());

            if (!path.isEmpty()) {
                label_name = qobject_cast<QLabel *>(d_item->progress()->layout()->itemAt(0)->widget());

                QFontMetrics metrics(label_name->font());
                label_name->setText(metrics.elidedText(getFileName(path), Qt::ElideRight, DOWNLOAD_WIDGET_MAX_WIDTH - 36));
                d_item->set_is_temporary(false);

                d_item->progress()->adjustSize();

                if ( !m_parentButton->isVisible()) {
                    m_parentButton->setVisible(true);
                }
            }
        }
    }
}

QString CDownloadWidget::getFileName(const QString& path) const
{
    if (path.length()) {
        QRegExp rx("([^\\\\\"]+)\"?$");
        if (!(rx.indexIn(path) < 0))
            return rx.cap(1);
    }

    return "";
}

void CDownloadWidget::resizeEvent(QResizeEvent * e)
{
    qDebug() << "resize: " << e->size();
}

//void CDownloadWidget::updateProgress()
//{
//    for (auto e : m_mapDownloads) {
//        updateProgress(e);
//    }
//}

//void CDownloadWidget::cancelAll()
//{
//    for (auto e : m_mapDownloads) {
//        removeFile(e.first);
//    }
//}

