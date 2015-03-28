#ifndef DEBUG_MEMORY_STATUS_H
#define DEBUG_MEMORY_STATUS_H

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

#endif
