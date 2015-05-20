CXX ?= g++

CFLAGS = -std=c++11 -Wall -Wextra -Werror

maybe_test: maybe_test.cpp *.h */*.h
	$(CXX) $(CFLAGS) -o maybe_test maybe_test.cpp

unordered_map_test: unordered_map_test.cpp *.h */*.h
	$(CXX) $(CFLAGS) -o unordered_map_test unordered_map_test.cpp
