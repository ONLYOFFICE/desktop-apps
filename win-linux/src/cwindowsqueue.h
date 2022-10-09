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

#ifndef CWINDOWSQUEUE_H
#define CWINDOWSQUEUE_H

#include <thread>
#include <mutex>
#include <utility>
#include <vector>
#include <atomic>

#include <QDebug>

#define THREAD_WAIT_INTERVAL 10

template<typename T>
class CWindowsQueue
{
    std::vector<std::thread> m_threads;
    std::mutex m_mutex;
    std::vector<T> m_wintoclose;
    std::atomic_bool m_queueCanceled{false};

    std::function<void(T)> m_callback;
public:
    CWindowsQueue()
    {}

    ~CWindowsQueue()
    {
        for (std::thread& t: m_threads)
        {
            if ( t.joinable() )
                t.join();
        }
    }

    void start_queue() {
        if ( !m_wintoclose.empty() ) {
            trigger_callback(*m_wintoclose.begin());
        }
    }

    void enter(const T& iter)
    {
        m_wintoclose.push_back(iter);
        if ( !(m_wintoclose.size() > 1) ) {
            m_queueCanceled.store(false);

            std::function<void()> start_func_(std::bind(&CWindowsQueue::start_queue, this));

            std::thread([start_func_]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_WAIT_INTERVAL));
                start_func_();
            }).detach();
        }
    }

    void leave(T iter)
    {
        m_threads.push_back(std::thread(&CWindowsQueue::leave_thread_func, this, iter));
    }

    void cancel()
    {
        m_threads.push_back(std::thread(&CWindowsQueue::cancel_thread_func, this));
    }

    void setcallback(std::function<void(T)>& fn)
    {
        m_callback = fn;
    }


private:
    void leave_thread_func(T iter)
    {
        std::lock_guard<std::mutex> lock{m_mutex};

        const auto& it = std::find_if(m_wintoclose.begin(), m_wintoclose.end(), [&](T i){ return i == iter; });

        if ( it != m_wintoclose.end() )
            m_wintoclose.erase(it);

        if ( !m_wintoclose.empty() && !m_queueCanceled.load() ) {
            trigger_callback(*m_wintoclose.begin());
        }
    }

    void cancel_thread_func()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_wintoclose.clear();
        m_queueCanceled.store(true);
    }

    void trigger_callback(const T& iter)
    {
        if ( m_callback )
            m_callback(iter);
    }
};

#endif // CWINDOWSQUEUE_H
