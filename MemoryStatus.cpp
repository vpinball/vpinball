#include "stdafx.h"
#include "MemoryStatus.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace rde
{
MemoryStatus::MemoryStatus()
:	totalFree(0),
	largestFree(0),
	totalReserved(0),
	totalCommited(0)
{
}

MemoryStatus MemoryStatus::GetCurrent()
{
	MemoryStatus status;

	MEMORY_BASIC_INFORMATION info;
	unsigned char* address(0);
	SIZE_T bytesInfo = ::VirtualQuery(address, &info, sizeof(info));
	while (bytesInfo != 0)
	{
		if (info.State & MEM_FREE)
		{
			status.totalFree += info.RegionSize;
			if (info.RegionSize > status.largestFree)
				status.largestFree = info.RegionSize;
		}
		else
		{
			if (info.State & MEM_RESERVE)
				status.totalReserved += info.RegionSize;
			if (info.State & MEM_COMMIT)
				status.totalCommited += info.RegionSize;
		}
		address += info.RegionSize;
		memset(&info, 0, sizeof(info));
		bytesInfo = ::VirtualQuery(address, &info, sizeof(info));
	}

	return status;
}

}
