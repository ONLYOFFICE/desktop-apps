#include "cmessage.h"
#include <QMessageBox>

CMessage::CMessage(HWND hParentWnd)
    : QWinWidget(hParentWnd)
{

}

void CMessage::error(const QString& title, const QString& text)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle( title );
    msgBox.setText( text );
    msgBox.setWindowModality(Qt::ApplicationModal);
    msgBox.setIcon(QMessageBox::Critical);

    msgBox.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint
                          | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
    msgBox.exec();

//    msgBox->setModal( false ); // if you want it non-modal
//    msgBox->open( this, SLOT(msgBoxClosed(QAbstractButton*)) );
//    QMessageBox::critical(new CMessage(wnd), title, text, QMessageBox::Ok, QMessageBox::Ok);
}
