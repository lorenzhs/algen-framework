#pragma once

#include "timer.h"
#include "benchmark.h"

namespace common {

class instrumentation {
	virtual void setup() = 0;
	virtual void finish() = 0;
	benchmark_result result() const;
};

struct timer_result : public benchmark_result {
	double duration;
	timer_result(double d) : duration(d) {}
};

class timer_instrumentation : public instrumentation {

public:
	timer_instrumentation() {}

	void setup() {
		t.reset();
	}

	void finish() {
		value = t.get();
	}

	timer_result result() const {
		return timer_result(value);
	}

private:
	timer t;
	double value;
};
}
