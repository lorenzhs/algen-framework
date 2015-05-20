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
				REQUIRE(m[0] == 0);
				REQUIRE(m[1] == 1);
				REQUIRE(m[2] == 4);
				REQUIRE(m[10] == 100);
				REQUIRE(m[99] == 9801);
			}
		}

		WHEN("We ask for elements that don't exist") {
			THEN("Find returns nothing") {
				REQUIRE(m.find(n) == nothing<unsigned int>());
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
				REQUIRE(m.size() == n+1);
			}
		}

		WHEN("We ask its size") {
			THEN("It returns the correct value") {
				REQUIRE(m.size() == n);
			}
		}

		WHEN("We delete half the elements") {
			for (size_t i = 0; i < n/2; ++i) {
				m.erase(i);
			}
			THEN("The size decreases") {
				REQUIRE(m.size() == n-n/2);
			}
			AND_THEN("We won't be able to find them any more") {
				REQUIRE(m.find(0) == nothing<unsigned int>());
				REQUIRE(m.find(n/2-1) == nothing<unsigned int>());
				REQUIRE(m.find(n/2) == just<unsigned int>((n/2)*(n/2)));
			}
			AND_THEN("operator[] will reinsert them") {
				REQUIRE(m[0] == 0);
				REQUIRE(m.find(0) == just<unsigned int>(0));
			}
		}

		WHEN("We clear it") {
			m.clear();
			THEN("Its size changes to 0") {
				REQUIRE(m.size() == 0);
			}
			AND_THEN("We won't be able to find the elements any more") {
				REQUIRE(m.find(0) == nothing<unsigned int>());
			}
		}
	}
}
