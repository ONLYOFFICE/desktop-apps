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


#include "cuserprofilewidget.h"

#include <QGridLayout>
#include <QLabel>

#include <QPushButton>
#include <QMenu>
#include <QEvent>

#include <QDebug>
#include <QResizeEvent>

#define PROFILE_WIDGET_MAX_WIDTH 600

    CProfileMenuFilter::CProfileMenuFilter(QObject *parent)
        : QObject(parent), _parentButton(NULL)
    {}

    bool CProfileMenuFilter::eventFilter(QObject * obj, QEvent *event) {
        if (!_parentButton)
            return false;

        QMenu * menu = dynamic_cast<QMenu*>(obj);
        if (!menu)
            return false;

        if (event->type() == QEvent::Show && obj == _parentButton->menu()) {
            QPoint pos = ((QWidget*)_parentButton->parent())->mapToGlobal(_parentButton->pos());
            pos += QPoint(_parentButton->width() - menu->width(), _parentButton->height() + 6);
            _parentButton->menu()->move(pos);

            return true;
        }

        return false;
    }

    void CProfileMenuFilter::setMenuButton(QPushButton * button)
    {
        _parentButton = button;
    }


CUserProfileWidget::CUserProfileWidget(QWidget * parent)
    : QWidget(parent), _label_name(new QLabel), _label_portal(new QLabel),
      _label_email(new QLabel), _profile(new CAscUser),
      _caption_portal(), _caption_email()
{
    QGridLayout * grid = new QGridLayout;

    _caption_portal.setText(tr("portal:"));
    _caption_email.setText(tr("email:"));

    grid->addWidget(_label_name, 0, 0, 1, -1);
    grid->addWidget(&_caption_portal, 1, 0, 1, 1);
    grid->addWidget(_label_portal, 1, 1, 1, 1);
    grid->addWidget(&_caption_email, 2, 0, 1, 1);
    grid->addWidget(_label_email, 2, 1, 1, 1);
    grid->setColumnStretch(1,1);

    setLayout(grid);

    _label_name->setObjectName("labelName");
    _label_portal->setObjectName("labelPortal");
    _label_email->setObjectName("labelEmail");

    setMaximumWidth(PROFILE_WIDGET_MAX_WIDTH);
}

CUserProfileWidget::~CUserProfileWidget()
{
    if (_profile) {
        delete _profile;
        _profile = NULL;
    }
}

void CUserProfileWidget::fillProfile(CAscUser * profile)
{
    _label_name->setText(profile->displayName());
    _label_portal->setText(profile->portal());
    _label_email->setText(profile->email());

    updateGeometry();
}

void CUserProfileWidget::parseProfile(const QString& json)
{
    _profile->clear();

    if (json.length() &&
            _profile->parse(json)) {
        fillProfile(_profile);
    }
}

const CAscUser * CUserProfileWidget::info()
{
    return _profile;
}

void CUserProfileWidget::resizeEvent(QResizeEvent *)
{
    QFontMetrics metrics(_label_name->font());
    _label_name->setText(metrics.elidedText(_label_name->text(), Qt::ElideRight, PROFILE_WIDGET_MAX_WIDTH - 38));

    metrics = QFontMetrics(_label_portal->font());
    int label_width = PROFILE_WIDGET_MAX_WIDTH - _caption_email.width() - 38;
    _label_portal->setText(metrics.elidedText(_label_portal->text(), Qt::ElideRight, label_width));
    _label_email->setText(metrics.elidedText(_label_email->text(), Qt::ElideRight, label_width));
}
