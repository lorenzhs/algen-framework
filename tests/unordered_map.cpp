#include "catch.hpp"

#include <hashtable/unordered_map.h>

SCENARIO("unordered_map's basic functions work", "[hashtable]") {
	GIVEN("An unordered_map") {
		hashtable::unordered_map<unsigned int, unsigned int> m;
		const size_t n = 100;
		for (size_t i = 0; i < n; ++i) {
			m[i] = i*i;
		}

		WHEN("We ask for the elements") {
			THEN("Their values are correct") {
				CHECK(m[0] == 0);
				CHECK(m[1] == 1);
				CHECK(m[2] == 4);
				CHECK(m[10] == 100);
				CHECK(m[99] == 9801);
			}
		}

		WHEN("We ask for elements that don't exist") {
			THEN("Find returns nothing") {
				CHECK(m.find(n) == nothing<unsigned int>());
			}
			AND_THEN("operator[] returns 0, and afterwards, find will find them") {
				REQUIRE(m[n] == 0);
				REQUIRE(m.find(n) == just<unsigned int>(0));
			}
		}

		WHEN("We insert more elements") {
			m[n] = n;
			THEN("We can retrieve them again using find or operator[]") {
				REQUIRE(m.find(n) == just<unsigned int>(n));
				REQUIRE(m[n] == n);
			}
			AND_THEN("The size increases") {
				CHECK(m.size() == n+1);
			}
		}

		WHEN("We ask its size") {
			THEN("It returns the correct value") {
				CHECK(m.size() == n);
			}
		}

		WHEN("We delete half the elements") {
			for (size_t i = 0; i < n/2; ++i) {
				m.erase(i);
			}
			THEN("The size decreases") {
				CHECK(m.size() == n-n/2);
			}
			AND_THEN("We won't be able to find them any more") {
				CHECK(m.find(0) == nothing<unsigned int>());
				CHECK(m.find(n/2-1) == nothing<unsigned int>());
				CHECK(m.find(n/2) == just<unsigned int>((n/2)*(n/2)));
			}
			AND_THEN("operator[] will reinsert them") {
				REQUIRE(m[0] == 0);
				REQUIRE(m.find(0) == just<unsigned int>(0));
			}
		}

		WHEN("We clear it") {
			m.clear();
			THEN("Its size changes to 0") {
				CHECK(m.size() == 0);
			}
			AND_THEN("We won't be able to find the elements any more") {
				CHECK(m.find(0) == nothing<unsigned int>());
			}
		}
	}
}
