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
#include <QHBoxLayout>
#include <QPushButton>
#include <QKeyEvent>
#include <QCoreApplication>
#include "common/Types.h"
#include "utils.h"

#ifdef _WIN32
//#define WINVER 0x0500
# include <windows.h>
#else
# include "windows/platform_linux/cx11decoration.h"
#endif // Q_WS_WIN32

#include <QDebug>

class CDialogEventFilter : public QObject
{
    CPrintProgress * m_parent;

public:
    CDialogEventFilter(QObject * parent = 0) : QObject(parent)
      , m_parent(qobject_cast<CPrintProgress*>(parent))
    {}

    bool eventFilter(QObject * obj, QEvent * event) {
        QDialog * dlg = dynamic_cast<QDialog *>(obj);

        if (!dlg)
            return false;

        if (event->type() == QEvent::KeyPress) {
            QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);

            if (keyEvent->key() == Qt::Key_Escape) {
                return true;
            }
        } else
        if ( event->type() == QEvent::Paint ) {
            emit m_parent->signal(18);
        }

//        return QDialog::eventFilter(obj, event);
        return false;
    }
};


/*#if defined(_WIN32)
CPrintProgress::CPrintProgress(HWND hParentWnd)
    : QWinWidget(hParentWnd),
      m_Dlg(this),
#else*/
CPrintProgress::CPrintProgress(QWidget * parent)
    : QObject(parent),
      m_Dlg(parent),
//#endif
    m_fLayout(new QFormLayout),
    m_eventFilter(new CDialogEventFilter(this)), m_isRejected(false)
{
    m_Dlg.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint
                          | Qt::MSWindowsFixedSizeDialogHint);

    QVBoxLayout * layout = new QVBoxLayout;
    layout->setSizeConstraint(QLayout::SetMaximumSize);

//    QLabel * icon = new QLabel;
//    icon->setProperty("class","msg-icon");
//    icon->setProperty("type","msg-question");
//    icon->setFixedSize(35*g_dpi_ratio, 35*g_dpi_ratio);

    auto _dpi_ratio =
/*#if defined(_WIN32)
            Utils::getScreenDpiRatioByHWND(int(hParentWnd));
#else*/
            Utils::getScreenDpiRatioByWidget(parent);
//#endif

    m_progressText = tr("Document is printing: page %1 of %2");
    m_progressLabel.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    m_progressLabel.setText(tr("Document is preparing"));

    m_progressLabel.setStyleSheet(QString("margin-bottom: %1px;").arg(8*_dpi_ratio));
//    m_progressLabel.setStyleSheet("background: red;");
    layout->addWidget(&m_progressLabel);

    QPushButton * btn_cancel    = new QPushButton(tr("&Cancel"));
    QWidget * box = new QWidget;
    box->setLayout(new QHBoxLayout);
    box->layout()->addWidget(btn_cancel);
    box->layout()->setContentsMargins(0,8*_dpi_ratio,0,0);
//    h_layout1->addWidget(box, 0, Qt::AlignCenter);
    layout->addWidget(box, 0, Qt::AlignCenter);

    m_Dlg.setLayout(layout);
    m_Dlg.setMinimumWidth(400*_dpi_ratio);
    m_Dlg.setWindowTitle(tr("Printing..."));

    m_Dlg.installEventFilter(m_eventFilter);

    connect(btn_cancel, SIGNAL(clicked()), this, SLOT(onCancelClicked()));
//#ifdef __linux
    connect(this, &CPrintProgress::signal, [=](int){ if ( !m_showed ) m_showed = true;});
//#endif
}

CPrintProgress::~CPrintProgress()
{
/*#if defined(_WIN32)
    EnableWindow(parentWindow(), TRUE);
#endif*/

    RELEASEOBJECT(m_fLayout)
    RELEASEOBJECT(m_eventFilter)
}

void CPrintProgress::setProgress(int current, int count)
{
    m_progressLabel.setText(m_progressText.arg(current).arg(count));
}

void CPrintProgress::startProgress()
{
//    m_Dlg.adjustSize();

/*#ifdef _WIN32
    EnableWindow(parentWindow(), FALSE);

    RECT rc;
    ::GetWindowRect(parentWindow(), &rc);

    int x = rc.left + (rc.right - rc.left - m_Dlg.width())/2;
    int y = (rc.bottom - rc.top - m_Dlg.height())/2;

    m_Dlg.move(x, y);
#endif*/
    m_Dlg.show();

#ifdef __linux
    while ( !m_showed ) {
        qApp->processEvents();
    }
#endif
}

void CPrintProgress::onCancelClicked()
{
    m_isRejected = true;
    m_Dlg.reject();
}

bool CPrintProgress::isRejected()
{
    return m_isRejected;
}
