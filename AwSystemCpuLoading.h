#pragma once
#include <Windows.h>
#include <pdh.h>
#include <thread>


class AwSystemCpuLoading
{
public:
    AwSystemCpuLoading();
    ~AwSystemCpuLoading();
    double getValue();

private:
    double getLoading();
    void init();

private:
    PDH_HQUERY cpuQuery;
    PDH_HCOUNTER cpuTotal;
    std::atomic<double> loadingValue = 0.0f;
    std::atomic_bool pollingThreadStop = false;
    std::thread pollingThread;
};
