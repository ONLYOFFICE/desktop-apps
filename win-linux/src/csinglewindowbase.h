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

#ifndef CSINGLEWINDOWBASE_H
#define CSINGLEWINDOWBASE_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <memory>

class CElipsisLabel : public QLabel
{
public:
    CElipsisLabel(const QString &text, QWidget *parent=Q_NULLPTR);
    CElipsisLabel(QWidget *parent=Q_NULLPTR, Qt::WindowFlags f=Qt::WindowFlags());

    auto setText(const QString&) -> void;
    auto setEllipsisMode(Qt::TextElideMode) -> void;
    auto updateText() -> void;
protected:
    void resizeEvent(QResizeEvent *event) override;

    using QLabel::setText;
private:
    QString orig_text;
    Qt::TextElideMode elide_mode = Qt::ElideRight;
};

class CSingleWindowBase
{
public:
    CSingleWindowBase();
    CSingleWindowBase(QRect& rect);

    virtual ~CSingleWindowBase();

    virtual void setScreenScalingFactor(double);
    virtual bool holdView(int uid) const = 0;
    virtual QWidget * createMainPanel(QWidget * parent, const QString& title);
    virtual const QObject * receiver() = 0;
//    virtual Qt::WindowState windowState() = 0;
//    virtual void setWindowState(Qt::WindowState) = 0;
    virtual void setWindowTitle(const QString&);
    virtual void adjustGeometry();
    virtual void bringToTop() = 0;
    virtual void focus() = 0;
    virtual bool isCustomWindowStyle();
    virtual void updateScaling();
    virtual double scaling() const;

protected:
    double m_dpiRatio = 1;

    QWidget * m_boxTitleBtns = nullptr;
    QWidget * m_pMainPanel = nullptr;
    QWidget * m_pMainView = nullptr;

    QPushButton * m_buttonMinimize = nullptr;
    QPushButton * m_buttonMaximize = nullptr;
    QPushButton * m_buttonClose = nullptr;
    CElipsisLabel * m_labelTitle = nullptr;

protected:
    virtual void onCloseEvent();
    virtual void onMinimizeEvent();
    virtual void onMaximizeEvent();
    virtual void onSizeEvent(int);
    virtual void onMoveEvent(const QRect&) = 0;
    virtual QPushButton * createToolButton(QWidget * parent = nullptr);
    virtual void onExitSizeMove();
    virtual void onDpiChanged(double newfactor, double prevfactor);
    virtual int calcTitleCaptionWidth();
    virtual void updateTitleCaption();

    inline int dpiCorrectValue(int v) const
    {
        return int(v * m_dpiRatio);
    }

private:
    class impl;
    std::unique_ptr<impl> pimpl;
};

#endif // CSINGLEWINDOWBASE_H
