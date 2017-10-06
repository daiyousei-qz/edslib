#include "../unittest.h"
#include "../../src/text/text-utils.hpp"

TEST_CASE("::text")
{
	using namespace eds::text;

	SECTION("Consume")
	{
		zstring x = "hello world";

		REQUIRE(Consume(x) == 'h');
		REQUIRE(Consume(x) == 'e');
		REQUIRE(ConsumeIf(x, 'l'));
		REQUIRE(ConsumeIfSeq(x, "lo"));
		REQUIRE_FALSE(ConsumeIfRange(x, 'a', 'z'));
		REQUIRE(ConsumeIfAny(x, "\t\r\n "));
	}
}