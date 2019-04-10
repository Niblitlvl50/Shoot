
#pragma once

#include "System/System.h"

namespace game
{
    class ScopedTimer
    {
    public:

        ScopedTimer(const char* scope_context)
            : m_start_time(System::GetPerformanceCounter())
            , m_scope_context(scope_context)
        { }

        ~ScopedTimer()
        {
            const uint64_t delta = System::GetPerformanceCounter() - m_start_time;
            std::printf("%s: %llums\n", m_scope_context, delta);
        }

        const uint64_t m_start_time;
        const char* m_scope_context;
    };

    template <typename T>
    class ScopedTimerCallback
    {
    public:

        ScopedTimerCallback(T&& callback)
            : m_start_time(System::GetPerformanceCounter())
            , m_callback(callback)
        { }

        ~ScopedTimerCallback()
        {
            const uint64_t delta = System::GetPerformanceCounter() - m_start_time;
            m_callback(delta);
        }

        const uint64_t m_start_time;
        T m_callback;
    };
}
