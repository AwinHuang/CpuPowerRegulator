#pragma once
#include <stdint.h>


class AwSystem
{
public:
	static uint64_t getCpuFrequency();
	static bool isUsingPowerAdapter();
	static void setCpuPowerIndex(uint64_t percentage, uint32_t sleepInMs = 0);
	static uint64_t getCpuPowerIndex();
};
