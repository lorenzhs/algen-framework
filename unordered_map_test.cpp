#include <cassert>
#include <iostream>

#include "hashtable/unordered_map.h"

int main() {
	hashtable::unordered_map<unsigned int, unsigned int> m;
	const size_t n = 1000;
	for (size_t i = 0; i < n; ++i) {
		m[i] = i*i;
	}
	std::cout << "Size: " << m.size() << " (expecting " << n << ")" << std::endl;
	assert(m.size() == n);

	for (size_t i = 0; i < n; ++i) {
		assert(m[i] == i*i);
	}

	std::cout << "Found all the elements we put in" << std::endl;

	for (size_t i = n; i < 2*n; ++i) {
		assert(m.find(i) == nothing<unsigned int>());
	}

	std::cout << "Didn't find the elements we didn't put in" << std::endl;

	for (size_t i = 0; i < n/2; ++i) {
		m.erase(i);
		assert(m.find(i) == nothing<unsigned int>());
		// Accessing with operator[] creates if key isn't present
		assert(m[i] == 0);
		assert(m.find(i) == just<unsigned int>(0));
		assert(m.find(i) != nothing<unsigned int>());
		m.erase(i);
	}

	std::cout << "Didn't find the elements we erased, and accessing non-existent elements created them as expected" << std::endl;

	std::cout << "Size after deletions: " << m.size() << " (expected " << n - n/2 << ")" << std::endl;
	assert(m.size() == n - n/2);

	m.clear();
	assert(m.size() == 0);
	for (size_t i = 0; i < n; ++i) {
		assert(m.find(i) == nothing<unsigned int>());
		assert(m[i] == 0);
	}
	assert(m.size() == n); // assert(m[i] == 0) inserts!

	std::cout << "Clearing worked." << std::endl;
}