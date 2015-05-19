#include <cassert>
#include <iostream>

#include "common/maybe.h"

using namespace common::maybe;

maybe<int> foo(maybe<int> a) {
	return *a + 1;
}

int main(int argc, char **argv) {
	auto m = just<int>(2);

	maybe<int> x = (m >>= foo) >>= foo;
	x = x | foo | foo;

	auto y = nothing<int>() >>= foo;

	std::cout << "result: " << x << ", " << y <<  std::endl;
	std::cout << "maybe<void>: " << sizeof(maybe<void>) << ", int: " << sizeof(maybe<int>) << ", " << sizeof(just<int>(1)) << ", " << sizeof(nothing<int>()) << std::endl;
}
