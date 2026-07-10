#pragma once
#include <memory>

struct PasTableReadCounter
{
    static PasTableReadCounter& instance();

    PasTableReadCounter();

    size_t get(int table);
    void set(int table, size_t count);
    void increase(int table, size_t count = 1);
    void reset(int table);
    void reset();
    bool wait_for_connection_complete();

    struct Impl;
    std::shared_ptr<Impl> m_impl;
};
