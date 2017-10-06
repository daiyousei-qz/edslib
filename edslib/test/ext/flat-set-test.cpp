#include "../unittest.h"
#include "../../src/ext/flat-set.hpp"
#include <array>
#include <algorithm>

namespace
{
	bool TestEqual(const eds::ext::FlatSet<int>& set, std::vector<int> v)
	{
		std::sort(v.begin(), v.end());
		return set.data() == v;
	}
}

TEST_CASE("::FlatSet")
{
	using namespace std;
	using namespace eds::ext;

	SECTION("Construction")
	{
		std::array<int, 4> arr = { 1,2,3,4 };
		// via initializaer_list
		FlatSet<int> s1 = { 1,2,3,4 };
		// via iterator
		FlatSet<int> s2{ arr.begin(), arr.end() };
		// via copy ctor
		FlatSet<int> s3{ s1 };
		// via move ctor
		FlatSet<int> s4_{ s1 };
		FlatSet<int> s4{ std::move(s4_) };

		auto test_set = [&](FlatSet<int>& s)
		{
			REQUIRE(s.size() == arr.size());
			// Note set is ordered
			REQUIRE(std::equal(s.begin(), s.end(), arr.begin(), arr.end()));
		};

		test_set(s1);
		test_set(s2);
		test_set(s3);
		test_set(s4);
	}

	SECTION("Access")
	{
		FlatSet<int> s = { 1,2,3,4,5,6,7,8 };
		REQUIRE(s.count(1) == 1);
		REQUIRE(s.count(0) == 0);
		REQUIRE(s.find(9) == s.end());
		REQUIRE(s.at(3) == 4);
		REQUIRE(s[6] == 7);
	}

	SECTION("Swap")
	{
		FlatSet<int> v1 = { 1,2,41,16,7 };
		FlatSet<int> v2 = { 1,-1,6,2,77,21,2,5,7 };

		auto v3 = v1;
		auto v4 = v2;
		v3.swap(v4);
		REQUIRE(v3 == v2);
		REQUIRE(v4 == v1);
		v3.swap(v4);
		REQUIRE(v3 == v1);
		REQUIRE(v4 == v2);
	}

	SECTION("Insertion And Erasure")
	{
		FlatSet<int> v;
		v.insert({ 1,2,3 });

		REQUIRE(TestEqual(v, { 1,2,3 }));

		v.insert(4);
		REQUIRE(TestEqual(v, { 1,2,3,4 }));

		int arr[] = { 4,5,6,7 };
		v.insert(begin(arr), end(arr));
		REQUIRE(TestEqual(v, { 1,2,3,4,5,6,7 }));

		v.erase(1);
		v.erase(6);
		REQUIRE(TestEqual(v, { 2,3,4,5,7 }));
	}
}