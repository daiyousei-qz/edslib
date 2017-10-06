#include "../unittest.h"
#include "../../src/text/format.hpp"
#include <string>

TEST_CASE("format::")
{
	using namespace eds::text;

	{
		std::string expected = "a(1,2.2,3,\"4\")";
		std::string yield = Format("a({},{},{},{})", 1, 2.2, '3', "\"4\"");

		REQUIRE(expected == yield);
	}

	{
		std::string expected = "{text}";
		std::string yield = Format("{{{0}}}", "text");

		REQUIRE(expected == yield);
	}

	{
		std::string expected = "test-332211";
		std::string yield = Format("test-{2}{1}{0}", 11, 22, 33);

		REQUIRE(expected == yield);
	}

	{
		std::string expected = "112211!!";
		std::string yield = Format("{}{}{0}!!", 11, 22);

		REQUIRE(expected == yield);
	}
}