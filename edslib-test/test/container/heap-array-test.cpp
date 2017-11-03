#include "../unittest.h"
#include "container/heap-array.h"

using namespace eds::container;

// TODO: add test cases
TEST_CASE("::HeapArray")
{
	HeapArray<int> x;
	x.Initialize(100);
	x.Initialize(100, 41);
}