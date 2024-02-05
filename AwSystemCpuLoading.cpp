#include "AwSystemCpuLoading.h"


#pragma comment(lib, "Pdh.lib")


AwSystemCpuLoading::AwSystemCpuLoading() {
    init();

    pollingThread = std::thread([this]() {
        while (1) {
            if (pollingThreadStop) {
                break;
            }

            this->loadingValue = this->getLoading();
            Sleep(1000);
        }
        });
}

AwSystemCpuLoading::~AwSystemCpuLoading() {
    pollingThreadStop = true;
    pollingThread.join();
    PdhCloseQuery(cpuQuery);
}

double AwSystemCpuLoading::getValue() {
    return this->loadingValue;
}

double AwSystemCpuLoading::getLoading() {
    PDH_FMT_COUNTERVALUE counterVal;

    PdhCollectQueryData(cpuQuery);
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
    return counterVal.doubleValue;
}

void AwSystemCpuLoading::init() {
    PdhOpenQuery(NULL, NULL, &cpuQuery);
    // You can also use L"\\Processor(*)\\% Processor Time" and get individual CPU values with PdhGetFormattedCounterArray()
    PdhAddCounter(cpuQuery, L"\\Processor(*)\\% Processor Time", NULL, &cpuTotal);
    //PdhAddCounter(cpuQuery, L"\\Processor Information(_Total)\\% Processor Time", NULL, &cpuTotal);
    PdhCollectQueryData(cpuQuery);
}
