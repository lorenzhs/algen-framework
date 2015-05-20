#include "catch.hpp"

#include <common/maybe.h>

using namespace common::monad;

SCENARIO("maybe monad", "[maybe]") {
	GIVEN("Some maybe monads") {
		maybe<int> a(just(2));
		maybe<int> b(just(3));
		maybe<int> c(nothing<int>());
		maybe<int> d(just(3));

		WHEN("We take their size") {
			THEN("They're 2*sizeof(T)") {
				CHECK(sizeof(a) == 2*sizeof(int));
				CHECK(sizeof(c) == 2*sizeof(int));
			}
		}

		WHEN("We compare them") {
			THEN("It works as it should") {
				CHECK(a != b);
				CHECK(b == d);
				CHECK(a != c);
			}
		}

		WHEN("We dereference them") {
			THEN("We get their values") {
				CHECK(*a == 2);
				CHECK(*b == 3);
			}
		}

		WHEN("We cast them to their data type") {
			THEN("We get their values") {
				CHECK((int)a == 2);
				CHECK((int)b == 3);
			}
		}
	}

	GIVEN("A void monad or nothing") {
		maybe<int> a(nothing<int>());
		maybe<void> n(nothing());

		WHEN("We try to dereference it") {
			THEN("It throws") {
				CHECK_THROWS(*a);
				CHECK_THROWS(*n);
			}
		}
		WHEN("We try to cast it") {
			THEN("It throws"){
				CHECK_THROWS((int)a);
				CHECK_THROWS(char b = (char)a);
				CHECK_THROWS((int)n);
				CHECK_THROWS((void*)n);
			}
		}
		WHEN("We take the void monad's size") {
			THEN("It's size is 1") {
				CHECK(sizeof(n) == 1);
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
				CHECK(f(a) == b);
			}
		}
		WHEN("We use the >>= syntax") {
		 	THEN("We can apply the function") {
		 		CHECK((a >>= f) == b);
		 		CHECK((n >>= f) == n);
		 	}
		 	AND_THEN("We can chain the >>= operators") {
		 		CHECK(((a >>= f) >>= f) == just<int>(3));
		 	}
		}

		WHEN("We use the | syntax") {
			THEN("We can apply and chain") {
				CHECK((a | f | f) == just<int>(3));
				CHECK((n | f) == n);
			}
		}

		WHEN("We use a function from maybe<int> to maybe<double>") {
			THEN("That works") {
				CHECK(*g(a) == Approx(2.0));
				CHECK(*(a >>= g) == Approx(2.0));
				CHECK(*(a | g) == Approx(2.0));
			}
		}
	}
}