#pragma once
#include <cstdint>

namespace eds
{
	void* AdvancePtr(void* ptr, int bytes)
	{
		return reinterpret_cast<uint8_t*>(ptr) + bytes;
	}
	const void* AdvancePtr(const void* ptr, int bytes)
	{
		return reinterpret_cast<const uint8_t*>(ptr) + bytes;
	}

	ptrdiff_t PtrDistance(const void* p1, const void* p2)
	{
		return reinterpret_cast<const uint8_t*>(p2) - reinterpret_cast<const uint8_t*>(p1);
	}
}