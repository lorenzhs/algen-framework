#pragma once

namespace common {

class timer_instrumentation {

public:
	typedef timer::return_type return_type;
	timer_instrumentation() {}

	void setup() {
		t.reset();
	}

	void finish() {
		value = t.get();
	}

	return_type result() const {
		return value;
	}

private:
	timer t;
	return_type value;
};
}
