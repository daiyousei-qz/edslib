#pragma once
#include <cstdint>

namespace eds
{
	inline void* AdvancePtr(void* ptr, int bytes)
	{
		return reinterpret_cast<uint8_t*>(ptr) + bytes;
	}
	inline const void* AdvancePtr(const void* ptr, int bytes)
	{
		return reinterpret_cast<const uint8_t*>(ptr) + bytes;
	}
	inline void* AdvancePtr(void* ptr, size_t bytes)
	{
		return reinterpret_cast<uint8_t*>(ptr) + bytes;
	}
	inline const void* AdvancePtr(const void* ptr, size_t bytes)
	{
		return reinterpret_cast<const uint8_t*>(ptr) + bytes;
	}

	inline ptrdiff_t PtrDistance(const void* p1, const void* p2)
	{
		return reinterpret_cast<const uint8_t*>(p2) - reinterpret_cast<const uint8_t*>(p1);
	}

	inline constexpr size_t RoundToAlign(size_t sz, size_t alignment) noexcept
	{
		size_t kAlignMask = alignment - 1;

		return (sz + kAlignMask) & (~kAlignMask);
	}
}