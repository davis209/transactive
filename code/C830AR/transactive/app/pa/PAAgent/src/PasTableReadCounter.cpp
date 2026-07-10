#include "pch.h"
#include "PasTableReadCounter.h"
#include "CachedConfig.h"
#include "PASConnection.h"
#include "core/data_access_interface/src/LocationAccessFactoryEx.h"
#include "core/utilities/src/TAMap.h"
#include "core/utilities/src/RunParamsEx.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/utilities/src/stdutil/wait.h"
#include <boost/algorithm/cxx11/all_of.hpp>

#define RPARAM_WAIT_FOR_PA_CONNECTION_COMPLETE_TIMEOUT_SECONDS "WaitForPaConnectionCompleteTimeoutSeconds"

using namespace TA_Base_Core;
using namespace TA_IRS_App;

struct PasTableReadCounter::Impl
{
    size_t get(int table)
    {
        return m_counts[table];
    }

    void set(int table, size_t count)
    {
        m_counts[table] = count;
    }

    void increase(int table, size_t count)
    {
        m_counts[table] += count;
    }

    void reset(int table)
    {
        m_counts[table] = 0;
    }

    void reset()
    {
        m_counts.clear();
    }

    bool wait_for_connection_complete()
    {
        LOG_SCOPE("PasTableReadCounter::wait_for_connection_complete");

        static const size_t timeout_ms = RunParamsEx::get_or(RPARAM_WAIT_FOR_PA_CONNECTION_COMPLETE_TIMEOUT_SECONDS, CachedConfig::getInstance()->getSocketTimeoutInSecs()) * 1000;
        LOG_DEBUG("wait_for_connection_complete(): begin: %s", nvps(timeout_ms));

        static const auto tables = ThisLocation::is_occ_or_bocc()
            ? std::vector<int>{302, 100, 202, 550, 570, 590, 603, 604, 1200, 560, 1204, 1610}
        : std::vector<int>{302, 100, 202, 550, 570, 590, 603, 604, 200, 560, 204, 610}
        ;

        auto res = st::wait_for(timeout_ms, 100, [=]
        {
            if (!PASConnection::getInstance()->isConnected())
            {
                return false;
            }

            return boost::algorithm::all_of(tables, [=](int table) { return get(table); });
        });

        LOG_DEBUG("wait_for_connection_complete(): end: %s", nvps(timeout_ms, res));
        return res;
    }

    ta::map<int, size_t> m_counts;
};

PasTableReadCounter& PasTableReadCounter::instance()
{
    static auto s_instance = new PasTableReadCounter();
    return *s_instance;
}

PasTableReadCounter::PasTableReadCounter()
    : m_impl(std::make_shared<Impl>())
{
}

size_t PasTableReadCounter::get(int table)
{
    return m_impl->get(table);
}

void PasTableReadCounter::set(int table, size_t count)
{
    m_impl->set(table, count);
}

void PasTableReadCounter::increase(int table, size_t count)
{
    m_impl->increase(table, count);
}

void PasTableReadCounter::reset(int table)
{
    m_impl->reset(table);
}

void PasTableReadCounter::reset()
{
    m_impl->reset();
}

bool PasTableReadCounter::wait_for_connection_complete()
{
    return m_impl->wait_for_connection_complete();
}
