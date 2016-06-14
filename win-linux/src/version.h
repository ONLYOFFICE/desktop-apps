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

#ifndef VERSION_H
#define VERSION_H

#define VER_FILEVERSION             3,8,5,234
#define VER_FILEVERSION_STR         "3.8.5.234\0"

#define VER_PRODUCTVERSION          VER_FILEVERSION
#define VER_PRODUCTVERSION_STR      "3.8\0"

#ifdef _IVOLGA_PRO
  #define VER_COMPANYNAME_STR         "Novie kommunikacionnie tehnologii CJSC\0"
  #define VER_FILEDESCRIPTION_STR     "Ivolga PRO\0"
  #define VER_INTERNALNAME_STR        "Desktop Editors\0"
  #define VER_LEGALCOPYRIGHT_STR      "Novie kommunikacionnie tehnologii CJSC, 2016\0"
  #define VER_LEGALTRADEMARKS1_STR    "All rights reserved\0"
  #define VER_LEGALTRADEMARKS2_STR    VER_LEGALTRADEMARKS1_STR
  #define VER_ORIGINALFILENAME_STR    "ivolgapro.exe\0"
  #define VER_PRODUCTNAME_STR         "Ivolga PRO\0"
  #define VER_COMPANYDOMAIN_STR       "www.ivolgapro.ru\0"
  #define ABOUT_COPYRIGHT_STR         "1999-2016 ЗАО 'НКТ'\0"
#elif defined(_AVS)
  #define VER_COMPANYNAME_STR         "Online Media Technologies Ltd.\0"
  #define VER_FILEDESCRIPTION_STR     "AVS Document Editor\0"
  #define VER_INTERNALNAME_STR        "Document Editor\0"
  #define VER_LEGALCOPYRIGHT_STR      "Online Media Technologies Ltd., 2016\0"
  #define VER_LEGALTRADEMARKS1_STR    "All rights reserved\0"
  #define VER_LEGALTRADEMARKS2_STR    VER_LEGALTRADEMARKS1_STR
  #define VER_ORIGINALFILENAME_STR    "avsdocumenteditor.exe\0"
  #define VER_PRODUCTNAME_STR         "AVS Document Editor\0"
  #define VER_COMPANYDOMAIN_STR       "www.avs4you.com\0"
  #define ABOUT_COPYRIGHT_STR         VER_LEGALCOPYRIGHT_STR
#else
  #define VER_COMPANYNAME_STR         "Ascensio System SIA\0"
  #define VER_FILEDESCRIPTION_STR     "ONLYOFFICE Desktop Editors\0"
  #define VER_INTERNALNAME_STR        "Desktop Editors\0"
  #define VER_LEGALCOPYRIGHT_STR      "Ascensio System SIA 2016\0"
  #define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved\0"
  #define VER_LEGALTRADEMARKS2_STR    VER_LEGALTRADEMARKS1_STR
  #define VER_ORIGINALFILENAME_STR    "documenteditor.exe\0"
  #define VER_PRODUCTNAME_STR         "ONLYOFFICE Desktop Editors\0"
  #define VER_COMPANYDOMAIN_STR       "www.onlyoffice.com\0"
  #define ABOUT_COPYRIGHT_STR         VER_LEGALCOPYRIGHT_STR

#endif

#endif

