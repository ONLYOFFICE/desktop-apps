#ifndef CANIMATEDICON_H
#define CANIMATEDICON_H

#include <QLabel>
#include <QSvgRenderer>

class CAnimatedIcon : public QLabel
{
public:
    explicit CAnimatedIcon(QWidget * parent = nullptr);

    void setPixmap(const QPixmap &, bool forcestop = false);
    void setIconSize(const QSize&, bool forcegeometry = true);

    void startSvg(const QString&, const QString& id = QString());
    void setSvgElement(const QString&);
    void stop();
    bool isStarted();

private:
    QSvgRenderer * m_svg = nullptr;
    QString m_svgElemId;
    QSize m_svgSize;
    QPixmap * m_image = nullptr;
    QPixmap * m_static = nullptr;

private slots:
    void onSvgRepaint();
};

#endif // CANIMATEDICON_H
