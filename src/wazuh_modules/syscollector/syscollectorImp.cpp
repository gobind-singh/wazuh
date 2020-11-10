/*
 * Wazuh SysCollector
 * Copyright (C) 2015-2020, Wazuh Inc.
 * October 7, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */
#include "syscollectorImp.h"
#include <iostream>


bool Syscollector::sleepFor()
{
    bool ret{false};
    std::unique_lock<std::mutex> lock{m_mutex};
    if (m_intervalUnit == "s")
    {
        ret = !m_cv.wait_for(lock, std::chrono::seconds{m_intervalValue}, [&](){return m_running;});
    }
    else if (m_intervalUnit == "m")
    {
        ret = !m_cv.wait_for(lock, std::chrono::minutes{m_intervalValue}, [&](){return m_running;});
    }
    else if (m_intervalUnit == "h")
    {
        ret = !m_cv.wait_for(lock, std::chrono::hours{m_intervalValue}, [&](){return m_running;});
    }
    else if (m_intervalUnit == "d")
    {
        const auto daysToHours{m_intervalValue * 24ul};
        ret = !m_cv.wait_for(lock, std::chrono::hours{daysToHours}, [&](){return m_running;});
    }
    else
    {
        ret = !m_cv.wait_for(lock, std::chrono::hours{1}, [&](){return m_running;});
    }
    return ret;
}

Syscollector::Syscollector(const std::shared_ptr<ISysInfo>& spInfo,
                           const std::string& interval,
                           const bool scanOnStart,
                           const bool hardware,
                           const bool os,
                           const bool network,
                           const bool packages,
                           const bool ports,
                           const bool portsAll,
                           const bool processes,
                           const bool hotfixes)
: m_spInfo{spInfo}
, m_intervalUnit{interval.back()}
, m_intervalValue{std::stoull(interval)}
, m_scanOnStart{scanOnStart}
, m_hardware{hardware}
, m_os{os}
, m_network{network}
, m_packages{packages}
, m_ports{ports}
, m_portsAll{portsAll}
, m_processes{processes}
, m_hotfixes{hotfixes}
, m_running{false}
, m_thread{std::bind(&Syscollector::syncThread, this)}
{
}

Syscollector::~Syscollector()
{
    std::unique_lock<std::mutex> lock{m_mutex};
    m_running = true;
    m_cv.notify_all();
    lock.unlock();
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}
void Syscollector::scanHardware()
{
    if (m_hardware)
    {
        const auto& hw{m_spInfo->hardware()};
        std::cout << hw.dump() << std::endl;
    }
}
void Syscollector::scanOs()
{
    if(m_os)
    {
        const auto& os{m_spInfo->os()};
        std::cout << os.dump() << std::endl;
    }
}
void Syscollector::scanNetwork()
{
    if (m_network)
    {
        const auto& networks{m_spInfo->networks()};
        std::cout << networks.dump() << std::endl;
    }
}
void Syscollector::scanPackages()
{
    if (m_packages)
    {
        const auto& packages{m_spInfo->packages()};
        std::cout << packages.dump() << std::endl;
        if (m_hotfixes)
        {
        }
    }
}
void Syscollector::scanPorts()
{
    if (m_ports)
    {
        if (m_portsAll)
        {
        }
    }
}
void Syscollector::scanProcesses()
{
    if (m_processes)
    {
        const auto& processes{m_spInfo->processes()};
        std::cout << processes.dump() << std::endl;
    }
}

void Syscollector::scan()
{
    scanHardware();
    scanOs();
    scanNetwork();
    scanPackages();
    scanPorts();
    scanProcesses();
}

void Syscollector::start()
{
    if (m_scanOnStart)
    {
        scan();
    }
    while(sleepFor())
    {
        scan();
    }
}

void Syscollector::syncThread()
{
    while(sleepFor())
    {
        //sync Rsync
    }
}