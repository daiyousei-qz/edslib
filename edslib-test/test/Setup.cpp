#define CATCH_CONFIG_MAIN
#include "unittest.h"

/*
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <deque>
#include <cassert>
#include <algorithm>
#include <random>
#include "arena.h"
#include "format.h"
#include "compression.h"

using namespace std;

void TestBitOps()
{
	static constexpr auto kTestEpoch = 100;
	static constexpr auto kTestScale = 1000;

	using namespace std;

	random_device rd;
	default_random_engine gen{ rd() };
	uniform_int_distribution<> dis_len{ 1, 31 };

	for (int i = 0; i < kTestEpoch; ++i)
	{
		vector<int> v_len;
		vector<uint32_t> v_data;

		for (auto i = 0; i < kTestScale; ++i)
		{
			auto len = dis_len(gen);
			v_len.push_back(len);

			uniform_int_distribution<> dis_byte{ 0, 1 << len - 1 };
			auto x = dis_byte(gen);
			v_data.push_back(x);
		}

		eds::BitEmitter emit;
		for (auto i = 0; i < kTestScale; ++i)
		{
			emit.Write(v_data[i], v_len[i]);
		}
		auto bin = emit.Export();

		eds::BitReader<uint8_t*> reader(bin.data(), bin.data() + bin.size());
		vector<uint32_t> result;
		for (auto len : v_len)
		{
			result.push_back(reader.Read(len));
		}

		if (result != v_data)
		{
			throw 0;
		}
	}
}

int main()
{
	using namespace eds::text;
	PrintFormatted("hello {}", "world\n");

	system("pause");
}
*/