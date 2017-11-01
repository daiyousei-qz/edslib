#include "../unittest.h"
#include "../../src/text/text-utils.h"

TEST_CASE("::text")
{
	using namespace eds::text;

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