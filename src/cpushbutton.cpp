/*
 * (c) Copyright Ascensio System SIA 2010-2016
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

#include "cpushbutton.h"

#include <QMovie>
#include <QStyleOption>
#include <QStylePainter>

#include <QGraphicsEffect>

#include <QDebug>

CPushButton::CPushButton(QWidget *parent)
    : QPushButton(parent), _movie(NULL)
{
    setIconSize(QSize(12,12));

    int START_OPACITY = 0;

    QGraphicsOpacityEffect * effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(START_OPACITY);
    setGraphicsEffect(effect);

    _animation = new QPropertyAnimation(effect, "opacity");
    _animation->setDuration(500);
    _animation->setStartValue(START_OPACITY);
    _animation->setEndValue(1.0);

    connect(_animation, SIGNAL(finished()), this, SLOT(onAnimationFinished()));
}

CPushButton::~CPushButton()
{
    if (_movie) {
        disconnect(_movie, SIGNAL(finished()), _movie, SLOT(start()));
        _movie->stop();

        delete _movie, _movie = NULL;
    }

    if ( _animation ) {
        disconnect(_animation, SIGNAL(finished()));
        delete _animation, _animation = NULL;
    }
}

void CPushButton::setAnimatedIcon(const QString& f)
{
    if (NULL == _movie) {
        _movie = new QMovie(f);
        connect(_movie, SIGNAL(frameChanged(int)), this, SLOT(setButtonIcon(int)));
    } else {
        disconnect(_movie, SIGNAL(finished()), _movie, SLOT(start()));

        _movie->stop();
        _movie->setFileName(f);
    }

    if (_movie->loopCount() != -1) {
        connect(_movie, SIGNAL(finished()), _movie, SLOT(start()));
    }

    _movie->start();
}

void CPushButton::setButtonIcon(int frame)
{
    Q_UNUSED(frame)

    repaint();
}

void CPushButton::startIconAnimation(bool start)
{
    if (_movie) {
        if (start) {
            if (_movie->state() != QMovie::Running)
                _movie->start();
        } else {
            if (_movie->state() == QMovie::Running)
                _movie->jumpToFrame(0);
                _movie->stop();
        }
    }
}

void CPushButton::setEnabled(bool enable)
{
    startIconAnimation(enable);

    QPushButton::setEnabled(enable);
}

void CPushButton::setVisible(bool visible)
{
    if (visible != QPushButton::isVisible()) {
        if (visible) QPushButton::setVisible(visible);

        startIconAnimation(visible);

        if( _animation->state() == QAbstractAnimation::Running )
            _animation->pause();

        _animation->setDirection(visible ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
        _animation->start();

//        if (!visible) QPushButton::setVisible(visible);
    }
}

void CPushButton::paintEvent(QPaintEvent * e)
{
    Q_UNUSED(e)

//    QPushButton::paintEvent(e);
//    return;

    QStylePainter p(this);
    p.setOpacity(255);

    QStyleOptionButton option;
    initStyleOption(&option);

    option.icon = QIcon();
    p.drawControl(QStyle::CE_PushButton, option);

    QRect r = QRect(QPoint(0,0), option.iconSize);
    p.drawItemPixmap(r, Qt::AlignLeft | Qt::AlignVCenter, _movie->currentPixmap());
}

void CPushButton::onAnimationFinished()
{
    if (_animation->direction() == QAbstractAnimation::Backward) {
        QPushButton::setVisible(false);
    }
}
