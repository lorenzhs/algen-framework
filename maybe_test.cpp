#include <cassert>
#include <iostream>

#include "common/maybe.h"

using namespace common::monad;

maybe<int> foo(maybe<int> a) {
	return *a + 1;
}

int main(int argc, char **argv) {
	assert(nothing<int>() == nothing<int>());
	assert(just<int>(2) == just<int>(2));
	auto m = just<int>(2);

	maybe<int> x = (m >>= foo) >>= foo;
	x = x | foo | foo;

	auto y = nothing<int>() >>= foo;

	std::cout << "result: " << x << ", " << y <<  std::endl;
	std::cout << "maybe<void>: " << sizeof(maybe<void>) << ", int: " << sizeof(maybe<int>) << ", " << sizeof(just<int>(1)) << ", " << sizeof(nothing<int>()) << std::endl;
}
