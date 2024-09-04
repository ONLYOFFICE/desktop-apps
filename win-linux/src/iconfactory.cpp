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

#include "iconfactory.h"
#include "cascapplicationmanagerwrapper.h"
#include <QSvgRenderer>
#include <QPainter>

#define IMAGES 6
#define DEFAULT_SIZE 20
#define ADVANCED_OPACITY 0.8


QIcon IconFactory::icon(IconIndex icon, int pixelSize)
{
    const char* iconPath[IMAGES] = {
        ":/menu/icons/iconssmall_1x.png",
        ":/menu/icons/iconssmall_1.25x.png",
        ":/menu/icons/iconssmall_1.5x.png",
        ":/menu/icons/iconssmall_1.75x.png",
        ":/menu/icons/iconssmall_2x.png",
        ":/menu/icons/iconssmall_2.5x.svg"
    };

    int index = pixelSize == DEFAULT_SIZE ? 0 :
                pixelSize == 1.25 * DEFAULT_SIZE ? 1 :
                pixelSize == 1.5 * DEFAULT_SIZE ? 2 :
                pixelSize == 1.75 * DEFAULT_SIZE ? 3 :
                pixelSize == 2 * DEFAULT_SIZE ? 4 : IMAGES - 1;

    QPixmap pix(pixelSize, pixelSize);
    pix.fill(Qt::transparent);
    if (index < IMAGES - 1) {
        QPixmap image(iconPath[index]);
        // int iconsPerRow = pixmap.width() / pixelSize;
        // int iconsPerColumn = pixmap.height() / pixelSize;
        int x = AscAppManager::themes().current().isDark() ? pixelSize : 0;
        int y = static_cast<int>(icon) * pixelSize;
        QPainter p(&pix);
        p.setOpacity(ADVANCED_OPACITY);
        p.drawPixmap(0, 0, image.copy(x, y, pixelSize, pixelSize));
        p.end();

    } else {
        const QString node = icon == CreateNew ? "btn-add-text" :
                             icon == Browse ?    "btn-browse" :
                             icon == Pin ?       "btn-pin" :
                             icon == Unpin ?     "btn-unpin" : "";

        QString path(iconPath[index]);
        QSvgRenderer svg(path);
        QPainter p(&pix);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        QRectF bounds = svg.boundsOnElement(node);
        double kx = pixelSize / svg.viewBoxF().width();
        double ky = pixelSize / svg.viewBoxF().height();
        bounds = QRectF(bounds.x() * kx, bounds.y() * ky, bounds.width() * kx, bounds.height() * ky);
        svg.render(&p, node, bounds);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(pix.rect(), AscAppManager::themes().current().isDark() ? QColor(255, 255, 255, 200) : QColor(0, 0, 0, 200));
        p.end();
    }
    return QIcon(pix);
}
