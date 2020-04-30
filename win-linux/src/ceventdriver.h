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

#ifndef CEVENTDRIVER_H
#define CEVENTDRIVER_H

#include <QObject>

#ifdef Q_OS_WIN
#include <Windows.h>
using NativeHandle = HWND;
#else
#include <QWidget>
using NativeHandle = WId;
#endif

class CInAppEventBase
{
public:
    enum class CEventType
    {
        etModal
    };

    CInAppEventBase(CEventType t)
        : m_type(t)
    {}

    virtual ~CInAppEventBase(){}
    virtual CEventType type() const { return m_type; }

private:
    CEventType m_type;
};

class CInAppRunnigEvent : public CInAppEventBase
{
public:
    CInAppRunnigEvent(CEventType t) : CInAppEventBase(t) {}
    virtual ~CInAppRunnigEvent(){}

    bool finished() const { return m_finished; }
    void setFinished(bool f) { m_finished = f; }
private:
    bool m_finished = false;
};

class CInAppEventModal : public CInAppRunnigEvent
{
    NativeHandle m_handle;

public:
    CInAppEventModal(NativeHandle h)
        : CInAppRunnigEvent(CEventType::etModal)
        , m_handle(h)
    {}

    NativeHandle handle() const { return m_handle; }
    void setHandle(NativeHandle h) { m_handle = h; }
};

class CEventDriver : public QObject
{
    Q_OBJECT

public:
    explicit CEventDriver(QObject *parent = nullptr);

//    void signal(edEventType);
    void signal(CInAppEventBase *);
signals:
    void onModalDialog(bool status, NativeHandle handle);

public slots:
};

class CRunningEventHelper
{
public:
    CRunningEventHelper(CInAppRunnigEvent *);
    ~CRunningEventHelper();

private:
    CInAppRunnigEvent * m_event;
};

#endif // CEVENTDRIVER_H
