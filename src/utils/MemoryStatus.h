#pragma once

namespace rde
{
struct MemoryStatus
{
	MemoryStatus();

	static MemoryStatus GetCurrent();

	size_t	totalFree;
	size_t	largestFree;
	size_t	totalReserved;
	size_t	totalCommited;
};
}
