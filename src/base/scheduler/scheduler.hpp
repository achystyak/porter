#pragma once

#include "../common/http.hpp"
#include "../libs/scheduler/scheduler.h"

class Scheduler
{
public:
    Scheduler(const std::string &cron, const ScheduleCallback &callback)
        : m_cron(cron),
          m_callback(callback)
    {
        m_scheduler = new Bosma::Scheduler();
        m_scheduler->cron(m_cron, m_callback);
    }

    ~Scheduler()
    {
        delete m_scheduler;
        m_scheduler = nullptr;
    }

    const std::string &cron() const
    {
        return m_cron;
    }

    const ScheduleCallback &callback() const
    {
        return m_callback;
    }

private:
    std::string m_cron;
    Bosma::Scheduler *m_scheduler;
    ScheduleCallback m_callback;
};