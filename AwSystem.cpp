#include "AwSystem.h"
#include <windows.h>
#include <powerbase.h>
#include <powersetting.h>


#pragma comment(lib, "PowrProf.lib")


typedef struct _PROCESSOR_POWER_INFORMATION {
    ULONG Number;
    ULONG MaxMhz;
    ULONG CurrentMhz;
    ULONG MhzLimit;
    ULONG MaxIdleState;
    ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, * PPROCESSOR_POWER_INFORMATION;


uint64_t AwSystem::getCpuFrequency() {
    uint64_t cpuFreq = 0;
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);

    uint32_t cpuPowerInfoArraySize = sizeof(PROCESSOR_POWER_INFORMATION) * systemInfo.dwNumberOfProcessors;
    PROCESSOR_POWER_INFORMATION* cpuPowerInfoArray = new PROCESSOR_POWER_INFORMATION[systemInfo.dwNumberOfProcessors];

    if (cpuPowerInfoArray == nullptr) {
        return cpuFreq;
    }

    auto status = CallNtPowerInformation(ProcessorInformation, NULL, 0, (void*)cpuPowerInfoArray, cpuPowerInfoArraySize);
    /*
    if (status == 0) {
        for (size_t i = 0; i < systemInfo.dwNumberOfProcessors; i++)
        {
            std::cout << "Power levels for Processor #" << cpuPowerInfoArray[i].Number << "\n"
                "MaxMhz: " << cpuPowerInfoArray[i].MaxMhz << "\n"
                "CurrentMhz: " << cpuPowerInfoArray[i].CurrentMhz << "\n"
                "MhzLimit: " << cpuPowerInfoArray[i].MhzLimit << "\n"
                "MaxIdleState: " << cpuPowerInfoArray[i].MaxIdleState << "\n"
                "CurrentIdleState: " << cpuPowerInfoArray[i].CurrentIdleState << "\n"
                << std::endl;
        }
    }
    */

    cpuFreq = cpuPowerInfoArray[0].CurrentMhz;
    delete[] cpuPowerInfoArray;
    return cpuFreq;
}

bool AwSystem::isUsingPowerAdapter() {
    SYSTEM_POWER_STATUS powerStatus;

    GetSystemPowerStatus(&powerStatus);

    return powerStatus.ACLineStatus == 1;
}

void AwSystem::setCpuPowerIndex(uint64_t percentage, uint32_t sleepInMs) {
    GUID guid, * scheme = &guid;

    PowerGetActiveScheme(NULL, &scheme);
    if (isUsingPowerAdapter()) {
        PowerWriteACValueIndex(NULL, scheme, &GUID_PROCESSOR_SETTINGS_SUBGROUP, &GUID_PROCESSOR_THROTTLE_MAXIMUM, percentage);
    }
    else {
        PowerWriteDCValueIndex(NULL, scheme, &GUID_PROCESSOR_SETTINGS_SUBGROUP, &GUID_PROCESSOR_THROTTLE_MAXIMUM, percentage);
    }
    PowerSetActiveScheme(NULL, scheme);

    if (sleepInMs > 0) {
        Sleep(sleepInMs);
    }
}

uint64_t AwSystem::getCpuPowerIndex() {
    GUID guid, * scheme = &guid;
    uint64_t value = 0;
    uint64_t valueSize = sizeof(value);
    PUCHAR a;

    PowerGetActiveScheme(NULL, &scheme);
    if (isUsingPowerAdapter()) {
        PowerReadACValue(NULL, scheme, &GUID_PROCESSOR_SETTINGS_SUBGROUP, &GUID_PROCESSOR_THROTTLE_MAXIMUM, NULL, (PUCHAR)&value, (LPDWORD)&valueSize);
    }
    else {
        PowerReadDCValue(NULL, scheme, &GUID_PROCESSOR_SETTINGS_SUBGROUP, &GUID_PROCESSOR_THROTTLE_MAXIMUM, NULL, (PUCHAR)&value, (LPDWORD)&valueSize);
    }

    return value;
}
