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

#include "ctooltip.h"
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QLayout>
#include <QTimer>
#include <QCursor>
#include <QLineF>

#define FADE_TIMEOUT_MS 5000
#define ANIMATION_DURATION_MS 150


CToolTip::CToolTip(QWidget * parent, const QString &text,
                   const QPoint &pos) :
    QWidget(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint |
            Qt::BypassWindowManagerHint),
    m_activated(false)
{
    setAttribute(Qt::WA_TranslucentBackground);
//    setAttribute(Qt::WA_ShowWithoutActivating);
    setWindowModality(Qt::NonModal);
    setFocusPolicy(Qt::NoFocus);
    setObjectName("CToolTip");
    QVBoxLayout *lut = new QVBoxLayout(this);
    setLayout(lut);
    layout()->setContentsMargins(10,10,10,10);
    m_label = new QLabel(this);
    layout()->addWidget(m_label);
    m_label->setText(text);
    QGraphicsOpacityEffect *grEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(grEffect);
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(m_label);
    shadow->setBlurRadius(16.0);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0.0);
    m_label->setGraphicsEffect(shadow);
    move(pos);
    show();
    QTimer *tmr = new QTimer(this);
    tmr->setSingleShot(false);
    tmr->setInterval(100);
    connect(tmr, &QTimer::timeout, this, [=]() {
        if (QLineF(pos, QCursor::pos()).length() > 10.0) {
            tmr->stop();
            showEffect(EffectType::Fade);
        }
    });
    tmr->start();
}

CToolTip::~CToolTip()
{

}

void CToolTip::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_activated) {
        m_activated = true;
        showEffect(EffectType::Arise);
        QTimer::singleShot(FADE_TIMEOUT_MS, this, [=]() {
            showEffect(EffectType::Fade);
        });
    }
}

void CToolTip::showEffect(const EffectType efType)
{
    QPropertyAnimation *anm = new QPropertyAnimation(graphicsEffect(), "opacity");
    anm->setDuration(ANIMATION_DURATION_MS);
    if (efType == EffectType::Arise) {
        anm->setStartValue(0);
        anm->setEndValue(1);
        anm->setEasingCurve(QEasingCurve::InCurve);
    } else
    if (efType == EffectType::Fade) {
        anm->setStartValue(1);
        anm->setEndValue(0);
        connect(anm, &QPropertyAnimation::finished, this, [=](){
            deleteLater();
        });
    }
    anm->start(QPropertyAnimation::DeleteWhenStopped);
}
