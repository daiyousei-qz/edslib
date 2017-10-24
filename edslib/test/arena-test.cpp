#include "unittest.h"
#include "../src/arena.hpp"

namespace 
{
	class Inc
	{
	public:
		Inc(int* p) : p_(p)
		{
			*p_ += 1;
		}

		~Inc()
		{
			*p_ -= 1;
		}

	private:
		int* p_;
	};

	void DoAllocTest(eds::Arena& arena, int times)
	{
		for (int i = 0; i < times; ++i)
		{
			arena.Allocate(rand() % 3000);
		}
	}
}

TEST_CASE("arena::")
{
	using eds::Arena;

	SECTION("POD Types")
	{
		Arena arena;
		int *p1 = arena.Construct<int>(42);
		float *p2 = arena.Construct<float>(3.14f);
		CHECK(*p1 == 42);
		CHECK(*p2 == 3.14f);
		CHECK(arena.GetByteUsed() == sizeof(int) + sizeof(float));
	}

	SECTION("User Types With Dtor")
	{
		int count = 0;

		{
			Arena arena;
			auto inc1 = arena.Construct<Inc>(&count);
			auto inc2 = arena.Construct<Inc>(&count);
			CHECK(count == 2);
		}
		CHECK(count == 0);
	}

	SECTION("Random Allocation")
	{
		Arena arena;
		CHECK_NOTHROW(DoAllocTest(arena, 1000));
	}

}