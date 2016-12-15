#include "cfilechecker.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QStorageInfo>

#include <QDebug>
#include <QElapsedTimer>

CFileChecker::CFileChecker(const QString& json)
    : QThread()
    , m_inJson(json)
{
}

void CFileChecker::run()
{
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(m_inJson.toUtf8(), &jerror);

    if(jerror.error == QJsonParseError::NoError) {
        QJsonObject objRoot = jdoc.object();

        if ( objRoot.size() ) {
                QJsonObject _json_obj;
                QString _file_name;

                QElapsedTimer timer;
                timer.start();

                foreach (QString s, objRoot.keys()) {
                    if ( isInterruptionRequested() ) return;

                    _file_name = objRoot[s].toString();

                    // check file is local
                    QStorageInfo storage(QFileInfo(_file_name).dir());
#ifdef Q_OS_WIN
                    if (storage.device().startsWith("\\\\?\\")) {
#else
                    if (storage.device().startsWith("/dev/")) {
#endif
                        QFileInfo info(_file_name);
                        if (!info.exists()) _json_obj[s] = "false";
                    }
                }

                qDebug() << "check file passed: " << currentThreadId() << ", " << timer.elapsed() << "ms";

                if ( _json_obj.size() && !isInterruptionRequested() ) {
                    emit resultReady(QJsonDocument(_json_obj).toJson(QJsonDocument::Compact));
                }
        }
    }
}
