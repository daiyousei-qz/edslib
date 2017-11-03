#include "../unittest.h"
#include "ext/flat-set.h"
#include <array>
#include <algorithm>

namespace
{
	bool TestEqual(const eds::FlatSet<int>& set, std::vector<int> v)
	{
		std::sort(v.begin(), v.end());
		return set.data() == v;
	}
}

TEST_CASE("::FlatSet")
{
	using namespace std;
	using namespace eds;

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
			CHECK(s.size() == arr.size());
			// Note set is ordered
			CHECK(std::equal(s.begin(), s.end(), arr.begin(), arr.end()));
		};

		test_set(s1);
		test_set(s2);
		test_set(s3);
		test_set(s4);
	}

	SECTION("Access")
	{
		FlatSet<int> s = { 1,2,3,4,5,6,7,8 };
		CHECK(s.count(1) == 1);
		CHECK(s.count(0) == 0);
		CHECK(s.find(9) == s.end());
		CHECK(s.at(3) == 4);
		CHECK(s[6] == 7);
	}

	SECTION("Swap")
	{
		FlatSet<int> v1 = { 1,2,41,16,7 };
		FlatSet<int> v2 = { 1,-1,6,2,77,21,2,5,7 };

		auto v3 = v1;
		auto v4 = v2;
		v3.swap(v4);
		CHECK(v3 == v2);
		CHECK(v4 == v1);
		v3.swap(v4);
		CHECK(v3 == v1);
		CHECK(v4 == v2);
	}

	SECTION("Insertion And Erasure")
	{
		FlatSet<int> v;
		v.insert({ 1,2,3 });

		CHECK(TestEqual(v, { 1,2,3 }));

		v.insert(4);
		CHECK(TestEqual(v, { 1,2,3,4 }));

		int arr[] = { 4,5,6,7 };
		v.insert(begin(arr), end(arr));
		CHECK(TestEqual(v, { 1,2,3,4,5,6,7 }));

		v.erase(1);
		v.erase(6);
		CHECK(TestEqual(v, { 2,3,4,5,7 }));
	}
}