/*
 * (c) Copyright Ascensio System SIA 2010-2017
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

#ifndef CTABBAR_H
#define CTABBAR_H

#include <QTabBar>
#include "cscalingwrapper.h"

//class CTabBarPrivate;

class CTabBar : public QTabBar, public CScalingWrapper
{
    Q_OBJECT

public:
    explicit CTabBar(QWidget * parent = 0);
    virtual ~CTabBar();

    void setTabTextColor(QPalette::ColorGroup, const QColor&);
    void setUseTabCustomPalette(bool);
    QPalette& customColors();

    void setTabIcon(int index, const QIcon &icon);
    void setTabLoading(int, bool);
    void tabStartLoading(int, const QString& theme = QString());
    void activate(bool);

    void updateScaling(int);
    int draggedTabIndex();

    enum TabTheme {
        Light,
        Dark
    };
    void changeTabTheme(int, TabTheme);
    void setTabTheme(int, TabTheme);

protected:
    bool event(QEvent * e);
    void mousePressEvent (QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);
    void tabInserted(int);
    void tabRemoved(int index);
    void drawTabCaption(QPainter *, const QString&, const QStyleOptionTab&);
    void fillTabColor(QPainter *, const QStyleOptionTab&, uint, const QColor&);

private slots:
    void onCloseButton();
    void onCurrentChanged(int);

private:
    QPalette m_palette;
    bool m_usePalette = false;
    int  m_overIndex = -1;
    int  m_current = -1;
    bool m_active = false;

signals:
    void tabUndock(int);

#ifdef __USE_COLORED_TAB
public:
    void setActiveTabColor(const QString& color)
    {
        m_activeColor = color;
    }
private:
    QString m_activeColor = "none";
#endif

private:
    Q_DECLARE_PRIVATE(QTabBar)
};

#endif // CTABBAR_H
