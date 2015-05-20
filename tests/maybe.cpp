#include "catch.hpp"

#include <common/maybe.h>

using namespace common::monad;

SCENARIO("maybe monad", "[maybe]") {
	GIVEN("Some maybe monads") {
		maybe<int> a(just(2));
		maybe<int> b(just(3));
		maybe<int> c(nothing<int>());
		maybe<int> d(just(3));

		WHEN("We compare them") {
			THEN("It works as it should") {
				REQUIRE(a != b);
				REQUIRE(b == d);
				REQUIRE(a != c);
			}
		}

		WHEN("We dereference them") {
			THEN("We get their values") {
				REQUIRE(*a == 2);
				REQUIRE(*b == 3);
			}
		}

		WHEN("We cast them to their data type") {
			THEN("We get their values") {
				REQUIRE((int)a == 2);
				REQUIRE((int)b == 3);
			}
		}
	}

	GIVEN("A void monad or nothing") {
		maybe<int> a(nothing<int>());
		maybe<void> n(nothing());

		WHEN("We try to dereference it") {
			THEN("It throws") {
				REQUIRE_THROWS(*a);
				REQUIRE_THROWS(*n);
			}
		}
		WHEN("We try to cast it") {
			THEN("It throws"){
				REQUIRE_THROWS((int)a);
				char b(0); // stupid hack to fool compiler
				REQUIRE_THROWS(b = (char)a);
				REQUIRE(b == 0); // another stupid hack to fool compiler (otherwise b is set but not used...)
				REQUIRE_THROWS((int)n);
				REQUIRE_THROWS((void*)n);
			}
		}
	}

	GIVEN("Some maybe monads and a function") {
		maybe<int> a(just(1));
		maybe<int> b(just(2));
		maybe<int> n(nothing<int>());

		auto f = [](maybe<int> a) -> maybe<int> {
			return a + 1;
		};

		auto g = [](maybe<int> a) -> maybe<double> {
			return a * 2.0;
		};

		WHEN("We apply the function normally") {
			THEN("It works") {
				REQUIRE(f(a) == b);
			}
		}
		WHEN("We use the >>= syntax") {
		 	THEN("We can apply the function") {
		 		REQUIRE((a >>= f) == b);
		 		REQUIRE((n >>= f) == n);
		 	}
		 	AND_THEN("We can chain the >>= operators") {
		 		REQUIRE(((a >>= f) >>= f) == just<int>(3));
		 	}
		}

		WHEN("We use the | syntax") {
			THEN("We can apply and chain") {
				REQUIRE((a | f | f) == just<int>(3));
				REQUIRE((n | f) == n);
			}
		}

		WHEN("We use a function from maybe<int> to maybe<double>") {
			THEN("That works") {
				REQUIRE(*g(a) == Approx(2.0));
				REQUIRE(*(a >>= g) == Approx(2.0));
				REQUIRE(*(a | g) == Approx(2.0));
			}
		}
	}
}