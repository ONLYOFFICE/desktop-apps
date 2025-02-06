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

#ifndef CMENU_H
#define CMENU_H

#include <QWidget>
#include <QIcon>


class CMenuWidget;
class CMenu : public QObject
{
    Q_OBJECT
public:
    explicit CMenu(QWidget * parent = nullptr);
    ~CMenu();

    enum Action {
        ActionClose = 0,
        ActionCloseSaved,
        ActionCloseAll,
        ActionShowInFolder,
        ActionMoveToStart,
        ActionMoveToEnd,
        ActionUnpinTab,
        ActionPinToTab,
        ActionCreateNew,
        ACTION_COUNT
    };

    QAction* addSection(Action action, const QIcon &icon = QIcon());
    QAction* addSection(const QString &text, const QIcon &icon = QIcon());
    QAction* addSeparator();
    void setSectionIcon(Action action, const QIcon &icon);
    void setSectionEnabled(Action action, bool enabled);
    void exec(const QPoint &pos);

signals:
    void wasHidden();

private:
    static const char* m_actionText[ACTION_COUNT];
    QVector<QAction*> m_actions;
    CMenuWidget *m_menu_widget;
    QWidget *m_parent;
};

#endif // CMENU_H
