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

#include "components/cdownloadwidget.h"
#include "components/celipsislabel.h"
#include "cascapplicationmanagerwrapper.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QApplication>
#include <QScrollBar>
#include <QFileInfo>
#include "common/Types.h"

#define DOWNLOAD_WIDGET_MIN_SIZE QSize(450, 250)
#define MARGINS 6
#define SPACING 6

using namespace NSEditorApi;


class CDownloadWidget::CDownloadItem
{
public:
    CDownloadItem(QWidget * w)
        : _p_progress(w), _is_temp(true)
    {}

    QWidget * progress() const { return _p_progress; }
    bool is_temporary() const { return _is_temp; }
    void set_is_temporary(bool v) { _is_temp = v; }

private:
    QWidget * _p_progress;
    bool _is_temp;
};

static void polishItem(QWidget *item)
{
    item->style()->polish(item);
    if (QLayout *lut = item->layout()) {
        for (int i = 0; i < lut->count(); i++) {
            QLayoutItem *litem = lut->itemAt(i);
            if (litem && litem->widget())
                litem->widget()->style()->polish(litem->widget());
        }
    }
}


CDownloadWidget::CDownloadWidget(QWidget *parent)
    : QDialog(parent)
    , m_pToolButton(new CPushButton)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMinimumSize(DOWNLOAD_WIDGET_MIN_SIZE);

    QVBoxLayout *main_lut = new QVBoxLayout(this);
    main_lut->setContentsMargins(0, 0, 0, 0);
    main_lut->setSpacing(0);
    setLayout(main_lut);

    m_pArea = new QScrollArea(this);
    m_pArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_pArea->setWidgetResizable(true);

    m_pContentArea = new QWidget(m_pArea);
    m_pContentArea->setObjectName("contentArea");
    QVBoxLayout *lut = new QVBoxLayout(m_pContentArea);
    lut->setContentsMargins(MARGINS, MARGINS, MARGINS, MARGINS);
    lut->setSpacing(SPACING);
    m_pContentArea->setLayout(lut);
    m_pArea->setWidget(m_pContentArea);
    main_lut->addWidget(m_pArea);
    m_pContentArea->setGeometry(0, 0, width(), height());

    connect(this, &CDownloadWidget::downloadCanceled, this, [=](int id) {
        AscAppManager::getInstance().CancelDownload(id);
        slot_downloadCanceled(id);
    });

    m_pToolButton->setObjectName("toolButtonDownload");
    m_pToolButton->setProperty("act", "tool");
    m_pToolButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_pToolButton->setVisible(false);
    connect(m_pToolButton, &QPushButton::clicked, this, [=]() {
        show();
    });
    m_pToolButton->setAnimatedIcon(":/downloading.svg");
    QSpacerItem *spacer = new QSpacerItem(5, 5, QSizePolicy::Fixed, QSizePolicy::Expanding);
    lut->addSpacerItem(spacer);    
}

CDownloadWidget::~CDownloadWidget()
{

}

QPushButton * CDownloadWidget::toolButton()
{
    return m_pToolButton;
}

QWidget * CDownloadWidget::addFile(const QString& fn, int id)
{
    QWidget * widget = new QWidget(m_pContentArea);
    QGridLayout * grid = new QGridLayout;

    CElipsisLabel * name = new CElipsisLabel(fn);
    name->setObjectName("labelName");
    name->setEllipsisMode(Qt::ElideRight);
    name->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QProgressBar * progress = new QProgressBar;
    progress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QPushButton * cancel = new QPushButton(tr("Cancel"));
    cancel->setObjectName("buttonCancel");
    cancel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    connect(cancel, &QPushButton::clicked, this, [=](){
        emit downloadCanceled(id);
    });

    progress->setTextVisible(false);

    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 0);

    grid->addWidget(name, 0, 0, 1, 1);
    grid->addWidget(progress, 1, 0, 1, 1);
    grid->addWidget(cancel, 1, 1, 1, 1);

    widget->setLayout(grid);
    if (QVBoxLayout *lut = dynamic_cast<QVBoxLayout*>(m_pContentArea->layout()))
        lut->insertWidget(lut->count() - 1, widget);
    widget->show();
    return widget;
}

void CDownloadWidget::downloadProcess(void * info)
{
    if ( info == NULL )
        return;

    CAscDownloadFileInfo * pData = reinterpret_cast<CAscDownloadFileInfo *>(info);
    int id = pData->get_IdDownload();

    std::map<int, CDownloadItem *>::iterator iter = m_mapDownloads.find(id);
    if (pData->get_IsCanceled()) {
        slot_downloadCanceled(id);
    } else
    if (pData->get_IsComplete()) {
        removeFile(iter);
    } else {
        if (iter == m_mapDownloads.end()) {
            QString path = QString::fromStdWString(pData->get_FilePath()),
                    file_name = "Unconfirmed";

            if (path.length()) {
                file_name = getFileName(path);
            }
            CDownloadItem * item = new CDownloadItem(addFile(file_name, id));
            iter = m_mapDownloads.insert( std::pair<int, CDownloadItem *>(id, item) ).first;
            if (!path.isEmpty()) {
                item->set_is_temporary(false);

                if ( !m_pToolButton->isVisible() ) {
                    m_pToolButton->setVisible(true);
                }
            }
        }
        updateProgress(iter, pData);
    }
}

void CDownloadWidget::slot_downloadCanceled(int id)
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
        CDownloadItem * di = iter->second;
        layout()->removeWidget(di->widget);
        RELEASEOBJECT(di->widget)
        RELEASEOBJECT(di)
        m_mapDownloads.erase(iter);
        if (m_mapDownloads.empty()) {
            m_pToolButton->deleteLater();
            deleteLater();
        }
    }
}

void CDownloadWidget::updateProgress(MapItem iter, void * data)
{
    CElipsisLabel * label_name;
    QProgressBar * progress;
    CDownloadItem * d_item;
    CAscDownloadFileInfo * pData;

    d_item = static_cast<CDownloadItem *>((*iter).second);
    if (d_item) {
        pData = reinterpret_cast<CAscDownloadFileInfo *>(data);
        progress = qobject_cast<QProgressBar *>(d_item->progress()->layout()->itemAt(1)->widget());

        if (progress && pData) {
            progress->setValue(pData->get_Percent());
        }

        if (d_item->is_temporary()) {
            QString path = QString().fromStdWString(pData->get_FilePath());

            if (!path.isEmpty()) {
                label_name = static_cast<CElipsisLabel*>(d_item->progress()->layout()->itemAt(0)->widget());
                label_name->setText(getFileName(path));
                d_item->set_is_temporary(false);

                if ( !m_pToolButton->isVisible()) {
                    m_pToolButton->setVisible(true);
                }
            }
        }
    }
}

QString CDownloadWidget::getFileName(const QString& path) const
{
    QFileInfo info(path);
    return info.fileName();
}

void CDownloadWidget::closeEvent(QCloseEvent *ev)
{
    ev->ignore();
    hide();
}

void CDownloadWidget::polish()
{
    style()->polish(this);
    m_pArea->style()->polish(m_pArea);
    m_pArea->verticalScrollBar()->style()->polish(m_pArea->verticalScrollBar());
    m_pContentArea->style()->polish(m_pContentArea);
    for (int i(0); i < m_pContentArea->layout()->count(); ++i) {
        auto item = m_pContentArea->layout()->itemAt(i);
        if (item && item->widget())
            polishItem(item->widget());
    }
}

void CDownloadWidget::updateScalingFactor(double factor)
{
    setProperty("zoom", QString::number(factor) + "x");
    setMinimumSize(DOWNLOAD_WIDGET_MIN_SIZE * factor);
    m_pArea->verticalScrollBar()->setFixedWidth(qRound(10 * factor));
    int mrg = qRound(MARGINS * factor);
    m_pContentArea->layout()->setContentsMargins(mrg, mrg, mrg, mrg);
    m_pContentArea->layout()->setSpacing(qRound(SPACING * factor));
    m_pToolButton->setIconSize((QSizeF(20,20) * factor).toSize());
    polish();
}

void CDownloadWidget::applyTheme(const QString &theme)
{
    setProperty("uitheme", theme);
    polish();
}

