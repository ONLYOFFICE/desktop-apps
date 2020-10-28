#include "clogger.h"
#include "utils.h"
#include <memory>

#include <QDebug>
#include <QMessageBox>

CLogger::CLogger(QObject *parent, QString fileName)
    : QObject(parent)
{
    if ( !fileName.isEmpty() ) {
        m_file = new QFile(fileName);
        m_file->open(QIODevice::Append | QIODevice::Text);
    }
}

CLogger::~CLogger()
{
    if ( m_file )
        m_file->close();
}

void CLogger::write(const QString &value)
{
    QString text(value);// + "";
    if ( m_showDate )
        text = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss ") + text;

    QTextStream out(m_file);
    out.setCodec("UTF-8");
    if ( m_file ) {
        out << text << endl;
    }
}

//void CLogger::setShowDateTime(bool value)
//{
//    m_showDate = value;
//}

void CLogger::log(const QString& str)
{
    static const bool _enabled = InputArgs::contains(L"--log");
    if ( _enabled ) {
        QString _file_name = Utils::getAppCommonPath() + "/app.log";
        std::unique_ptr<CLogger> _logger(new CLogger(0, _file_name));

        _logger->write(str);
    }
}
