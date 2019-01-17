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

#ifndef CDOWNLOADWIDGET_H
#define CDOWNLOADWIDGET_H

#include <QWidget>
#include "cpushbutton.h"
#include "cscalingwrapper.h"

//class CProfileMenuFilter;
class CProfileMenuFilter : public QObject {
public:
    CProfileMenuFilter(QObject *);

    bool eventFilter(QObject *, QEvent *);
    void setMenuButton(QPushButton *);
private:
    QPushButton * _parentButton;
};

class CDownloadWidget : public QWidget, public CScalingWrapper
{
    Q_OBJECT

    class CDownloadItem;
    typedef std::map<int, CDownloadItem *>::const_iterator MapItem;

public:
    explicit CDownloadWidget(QWidget *parent = 0);
    ~CDownloadWidget();

    void downloadProcess(void *);
    QPushButton * toolButton();
//    void updateProgress();
//    void cancelAll();

    void updateScaling(int);

protected:
    QWidget * addFile(const QString&, int);
    void removeFile(int);
    void removeFile(MapItem);
    void updateLayoutGeomentry();
    void updateProgress(MapItem, void *);
    QString getFileName(const QString&) const;

    void applyScaling(int);
    void resizeEvent(QResizeEvent *);

private:
    CPushButton * m_pToolButton;
    std::map<int, CDownloadItem *> m_mapDownloads;
    QMargins m_defMargins;
    int m_defSpacing;

signals:
    void downloadCanceled(int);

private slots:
    void slot_downloadCanceled(int);
};

#endif // CDOWNLOADWIDGET_H
