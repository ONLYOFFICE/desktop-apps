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

#include "components/cpushbutton.h"
#include <QPainter>
#include <math.h>

#define ANIMATION_MS 2500


CPushButton::CPushButton(QWidget *parent)
    : QPushButton(parent)
{}

CPushButton::~CPushButton()
{
    if (m_animation) {
        m_animation->stop();
        m_animation->disconnect();
        delete m_animation, m_animation = nullptr;
    }
}

void CPushButton::setAnimatedIcon(const QString &path)
{
    if (m_animation) {
        m_animation->stop();
        m_animation->disconnect();
        delete m_animation, m_animation = nullptr;
    }

    if (m_renderer)
        delete m_renderer, m_renderer = nullptr;
    m_renderer = new QSvgRenderer(path, this);

    m_animation = new QVariantAnimation(this);
    m_animation->setStartValue(0.0);
    m_animation->setKeyValueAt(0.5, 1.0);
    m_animation->setEndValue(0.0);
    m_animation->setDuration(ANIMATION_MS);
    m_animation->setLoopCount(-1);
    m_animation->setEasingCurve(QEasingCurve::Linear);
    connect(m_animation, &QVariantAnimation::valueChanged, this, [=](const QVariant &val) {
        double opacity = round(val.toReal() * 100) / 100;
        if (qRound(opacity * 100) % 2 == 0) // frequency limitation
            applyAnimatedIcon(opacity);
    });
    m_animation->start(QAbstractAnimation::KeepWhenStopped);
}

void CPushButton::applyAnimatedIcon(double opacity)
{
    if (m_renderer && m_renderer->isValid()) {
        QSize icon_size = iconSize();
        QImage img(icon_size, QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        QPixmap pixmap = QPixmap::fromImage(img, Qt::NoFormatConversion);

        QPainter painter(&pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setOpacity(opacity);
        m_renderer->render(&painter, QRect(QPoint(0,0), icon_size));
        painter.end();
        setIcon(QIcon(pixmap));
    }
}
