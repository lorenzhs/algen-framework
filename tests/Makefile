CXX ?= g++

CFLAGS = -std=c++11 -g -Wall -Wextra -Werror -I..

# This is where the test files go
SRC = maybe.cpp \
      unordered_map.cpp

BUILDDIR ?= build

OBJ = $(patsubst %.cpp,${BUILDDIR}/%.o,$(SRC))

.PHONY: directories

all: compile

compile: directories tests

directories: ${BUILDDIR}

build:
	mkdir -p ${BUILDDIR}

build/%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

tests: $(OBJ)
	$(CXX) $(CFLAGS) -o tests tests.cpp $^

clean:
	rm -rf ${BUILDDIR} tests

run: compile
	./tests

gdb: compile
	gdb --args ./tests --break
