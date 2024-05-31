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

#include "components/cprintprogress.h"
#include <QDialog>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include "utils.h"

#ifdef __linux__
#endif


class CPrintProgress::CPrintProgressPrivate
{
    class CDialog : public QDialog
    {
    public:
        CDialog(QWidget *parent = nullptr) : QDialog(parent)
        {}
    private:
        virtual bool event(QEvent *ev) override
        {
            if (ev->type() == QEvent::KeyPress) {
                QKeyEvent *kev = static_cast<QKeyEvent*>(ev);
                if (kev->key() == Qt::Key_Escape)
                    return true;
            }
            return QDialog::event(ev);
        }
    };

public:
    CPrintProgressPrivate(QWidget *parent = nullptr) {
        const QString primaryText = QObject::tr("Printing...", "CPrintProgress");
        const QString secondaryText = QObject::tr("Document is preparing", "CPrintProgress");
        auto _dpi_ratio = Utils::getScreenDpiRatioByWidget(parent);
        qtDlg = new CDialog(parent);
        qtDlg->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::MSWindowsFixedSizeDialogHint);
        qtDlg->setMinimumWidth(400*_dpi_ratio);
        qtDlg->setWindowTitle(primaryText);

        QVBoxLayout * layout = new QVBoxLayout;
        layout->setSizeConstraint(QLayout::SetMaximumSize);

        qtProgressLabel = new QLabel;
        qtProgressLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        qtProgressLabel->setText(secondaryText);
        qtProgressLabel->setStyleSheet(QString("margin-bottom: %1px;").arg(8*_dpi_ratio));
        layout->addWidget(qtProgressLabel);

        QPushButton * btn_cancel = new QPushButton(QObject::tr("&Cancel", "CPrintProgress"));
        QWidget * box = new QWidget;
        box->setLayout(new QHBoxLayout);
        box->layout()->addWidget(btn_cancel);
        box->layout()->setContentsMargins(0,8*_dpi_ratio,0,0);
        layout->addWidget(box, 0, Qt::AlignCenter);

        qtDlg->setLayout(layout);
        qtDlg->setResult(QDialog::Accepted);
        QObject::connect(btn_cancel, &QPushButton::clicked, qtDlg, &QDialog::reject);
    }

    ~CPrintProgressPrivate() {
        if (qtDlg)
            qtDlg->deleteLater();
    }

    CDialog  *qtDlg = nullptr;
    QLabel   *qtProgressLabel = nullptr;
};

CPrintProgress::CPrintProgress(QWidget * parent)
    : QObject(parent),
    pimpl(new CPrintProgressPrivate(parent))
{}

CPrintProgress::~CPrintProgress()
{
    delete pimpl, pimpl = nullptr;
}

void CPrintProgress::setProgress(int current, int count)
{
    QString line = tr("Document is printing: page %1 of %2").arg(QString::number(current), QString::number(count));
    pimpl->qtProgressLabel->setText(line);
}

void CPrintProgress::startProgress()
{
    pimpl->qtDlg->show();
#ifdef __linux
    Utils::processMoreEvents(100);
#endif
}

bool CPrintProgress::isRejected()
{
    return pimpl->qtDlg->result() == QDialog::Rejected;
}
