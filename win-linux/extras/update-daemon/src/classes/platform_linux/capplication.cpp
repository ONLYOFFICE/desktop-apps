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

#include "capplication.h"
#include "platform_linux/utils.h"
#include "classes/translator.h"
#include <iostream>
#include <SDL2/SDL.h>


CApplication::CApplication()
{
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
        NS_Logger::WriteLog(_TR(MESSAGE_TEXT_ERR16) + _T(" ") + SDL_GetError(), true);
}

CApplication::~CApplication()
{
    SDL_Quit();
}

int CApplication::exec()
{
    int exit_code = 1;
    SDL_Event event;
    while (m_run && SDL_WaitEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                exit_code = event.user.code;
                break;
            default:
                break;
        }
    }
    return exit_code;
}

void CApplication::exit(int code)
{
    m_run = false;
    SDL_Event event;
    event.type = SDL_QUIT;
    event.user.code = code;
    SDL_PushEvent(&event);
}
