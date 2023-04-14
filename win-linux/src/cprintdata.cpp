/*
 * (c) Copyright Ascensio System SIA 2010-2022
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

#include "cprintdata.h"
#include "utils.h"
#include "defines.h"
#include <QJsonObject>
#include <QRegularExpression>
#include <QSettings>
#include <cmath>

class CPrintData::CPrintDataPrivate
{
public:
    QPrinterInfo printer_info;
    QPrintDialog::PrintRange print_range{QPrintDialog::PrintRange::AllPages};
    QPageLayout::Orientation page_orientation{QPageLayout::Portrait};
    QPrinter::DuplexMode duplex_mode{QPrinter::DuplexMode::DuplexNone};
    bool is_quick = false;
    int page_from = 0,
        page_to = 0;
    int pages_count = -1,
        current_page = 0;
    int paper_width = 0,
        paper_height = 0;
    QString size_preset;
    int sender_id = -1;
    int copies_count = 1;

    auto parseJsonOptions(const std::wstring& json) -> bool {
        QJsonObject jsonOptions = Utils::parseJson(json);
        if ( jsonOptions.contains("nativeOptions") ) {
            QJsonObject native = jsonOptions["nativeOptions"].toObject();
            if ( native.contains("quickPrint") && native["quickPrint"].toBool() ) {
                is_quick = true;
                print_range = QPrintDialog::AllPages;
                return true;
            }

            if ( native.contains("pages") ) {
                QString range = native["pages"].toString();

                if ( range == "all" ) print_range = QPrintDialog::AllPages; else
                if ( range == "current" ) print_range = QPrintDialog::CurrentPage;
                else {
                    QRegularExpression re_range("(\\d+)(?:-(\\d+))?");
                    QRegularExpressionMatch re_match = re_range.match(range);

                    if ( re_match.hasMatch() ) {
                        print_range = QPrintDialog::PageRange;
                        page_from = re_match.captured(1).toInt();
                        page_to = !re_match.captured(2).isEmpty() ? re_match.captured(2).toInt() : page_from;

                        if ( page_from > pages_count )
                            page_from = pages_count;
                        if ( page_to > 0 && page_to > pages_count )
                            page_to = pages_count;
                    } else print_range = QPrintDialog::AllPages;
                }
            }

            if ( native.contains("paperOrientation") ) {
                page_orientation = native["paperOrientation"].toString() == "portrait" ? QPageLayout::Portrait : QPageLayout::Landscape;
            }

            if ( native.contains("paperSize") ) {
                QJsonObject size = native["paperSize"].toObject();

                paper_width = std::ceil(size["w"].toDouble(0));
                paper_height = std::ceil(size["h"].toDouble(0));
                size_preset = size["preset"].toString();
            }

            if ( native.contains("copies") ) {
                copies_count = native["copies"].toInt(1);
            }

            if ( native.contains("sides") ) {
                QString side = native["sides"].toString();
                if ( side == "both-long" ) {
                    duplex_mode = QPrinter::DuplexMode::DuplexLongSide;
                } else
                if ( side == "both-short" ) {
                    duplex_mode = QPrinter::DuplexMode::DuplexShortSide;
                } else {
//                    "one";
                    duplex_mode = QPrinter::DuplexMode::DuplexNone;
                }
            }

            return true;
        }

        return false;
    }

    auto init(NSEditorApi::CAscPrintEnd * data) -> void
    {
        is_quick = false;

        QPageSize def_size{QPageSize::A4};
        QSizeF paper_size = def_size.size(QPageSize::Millimeter);
        paper_width = paper_size.width();
        paper_height = paper_size.height();
        size_preset = def_size.name();

        pages_count = data->get_PagesCount();
        current_page = data->get_CurrentPage();

        parseJsonOptions(data->get_Options());
    }

};

CPrintData::CPrintData()
    : m_priv(new CPrintData::CPrintDataPrivate)
{

}

auto CPrintData::init(NSEditorApi::CAscPrintEnd * data) -> void
{
    m_priv->init(data);
}

auto CPrintData::init(int senderid, NSEditorApi::CAscPrintEnd * data) -> void
{
    m_priv->init(data);
    m_priv->sender_id = senderid;
}

auto CPrintData::printerInfo() const -> QPrinterInfo
{
    if ( m_priv->printer_info.printerName().isEmpty() ) {
        GET_REGISTRY_USER(reg_user);

        QString last_printer_name = reg_user.value("lastPrinterName").toString();
        if ( !last_printer_name.isEmpty() ) {
            QPrinterInfo info{QPrinterInfo::printerInfo(last_printer_name)};
            if ( !info.isNull() )
                return info;
        } else return QPrinterInfo();

        return QPrinterInfo::defaultPrinter();
    }

    return m_priv->printer_info;
}

auto CPrintData::setPrinterInfo(const QPrinterInfo& info) -> void
{
    GET_REGISTRY_USER(reg_user);
    reg_user.setValue("lastPrinterName", info.printerName());

    m_priv->printer_info = info;
}

auto CPrintData::setPrinterInfo(const QPrinter& printer) -> void
{
    setPrinterInfo(QPrinterInfo::printerInfo(printer.printerName()));
}

auto CPrintData::pageSize() const -> QPageSize
{
    if ( !m_priv->size_preset.isEmpty() ) {
        if ( m_priv->size_preset == "A0" )
            return QPageSize(QPageSize::A0);
        if ( m_priv->size_preset == "A1" )
            return QPageSize(QPageSize::A1);
        if ( m_priv->size_preset == "A2" )
            return QPageSize(QPageSize::A2);
        if ( m_priv->size_preset == "A3" )
            return QPageSize(QPageSize::A3);
        if ( m_priv->size_preset == "A4" )
            return QPageSize(QPageSize::A4);
        if ( m_priv->size_preset == "A5" )
            return QPageSize(QPageSize::A5);
        if ( m_priv->size_preset == "A6" )
            return QPageSize(QPageSize::A6);
        if ( m_priv->size_preset == "B5" )
            return QPageSize(QPageSize::B5);
        if ( m_priv->size_preset == "Tabloid" )
            return QPageSize(QPageSize::Tabloid);
        if ( m_priv->size_preset == "Envelope DL" )
            return QPageSize(QPageSize::EnvelopeDL);
        if ( m_priv->size_preset == "Envelope #10" )
            return QPageSize(QPageSize::Comm10E);
        if ( m_priv->size_preset == "Super B/A3" )
            return QPageSize(QPageSize::SuperB);
        if ( m_priv->size_preset == "Tabloid Oversize" )
            return QPageSize(QPageSize::TabloidExtra);
        if ( m_priv->size_preset == "US Letter" )
            return QPageSize(QPageSize::Letter);
        if ( m_priv->size_preset == "US Legal" )
            return QPageSize(QPageSize::Legal);
        if ( m_priv->size_preset == "Envelope Choukei 3" )
            return QPageSize(QPageSize::EnvelopeChou3);
    } else
    if ( !m_priv->paper_width || !m_priv->paper_height )
        return QPageSize(QPageSize::A4);

    return QPageSize(QSize(m_priv->paper_width, m_priv->paper_height), QPageSize::Millimeter);
}

auto CPrintData::pageOrientation() const -> QPageLayout::Orientation
{
    return m_priv->page_orientation;
}

auto CPrintData::pageFrom() const -> int
{
    return m_priv->page_from;
}

auto CPrintData::pageTo() const -> int
{
    return m_priv->page_to;
}

auto CPrintData::printRange() const -> QPrintDialog::PrintRange
{
    return m_priv->print_range;
}

auto CPrintData::isQuickPrint() const -> bool
{
    return m_priv->is_quick;
}

auto CPrintData::pagesCount() const -> int
{
    return m_priv->pages_count;
}

auto CPrintData::pageCurrent() const -> int
{
    return m_priv->current_page;
}
auto CPrintData::viewId() const -> int
{
    return m_priv->sender_id;
}

auto CPrintData::copiesCount() const -> int
{
    return m_priv->copies_count;
}

auto CPrintData::duplexMode() const -> QPrinter::DuplexMode
{
    return m_priv->duplex_mode;
}
