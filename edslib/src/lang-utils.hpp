#pragma once

namespace eds
{
	class NonCopyable
	{
	protected:
		NonCopyable() = default;
		NonCopyable(const NonCopyable&) = delete;
		NonCopyable& operator=(const NonCopyable&) = delete;
	};

	class NonMovable
	{
	protected:
		NonMovable() = default;
		NonMovable(NonMovable&&) = delete;
		NonMovable& operator=(NonMovable&&) = delete;
	};
}