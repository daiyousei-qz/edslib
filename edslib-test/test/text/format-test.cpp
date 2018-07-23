#include "catch.hpp"
#include "text/format.h"
#include <string>

using namespace eds::text;

TEST_CASE("format::")
{
	{
		std::string expected = "a(1,2.2,3,\"4\")";
		std::string yield = Format("a({},{},{},{})", 1, 2.2, '3', "\"4\"");

		CHECK(expected == yield);
	}

	{
		std::string expected = "{text}";
		std::string yield = Format("{{{0}}}", "text");

		CHECK(expected == yield);
	}

	{
		std::string expected = "test-332211";
		std::string yield = Format("test-{2}{1}{0}", 11, 22, 33);

		CHECK(expected == yield);
	}

	{
		std::string expected = "112211!!";
		std::string yield = Format("{}{}{0}!!", 11, 22);

		CHECK(expected == yield);
	}
}