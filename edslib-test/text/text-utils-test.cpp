#include "catch.hpp"
#include "text/text-utils.h"

using namespace eds::text;

TEST_CASE("::text")
{
	SECTION("Consume")
	{
		zstring x = "hello world";

		CHECK(Consume(x) == 'h');
		CHECK(Consume(x) == 'e');
		CHECK(ConsumeIf(x, 'l'));
		CHECK(ConsumeIfSeq(x, "lo"));
		CHECK_FALSE(ConsumeIfRange(x, 'a', 'z'));
		CHECK(ConsumeIfAny(x, "\t\r\n "));
	}
}