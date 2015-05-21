#pragma once

#include <ostream>
#include <papi.h>

#include "timer.h"
#include "benchmark.h"

namespace common {

class instrumentation {
public:
	virtual void setup() = 0;
	virtual void finish() = 0;
	benchmark_result result() const;
};

struct timer_result : public benchmark_result {
	double duration;
	timer_result(double d) : duration(d) {}
	friend std::ostream& operator<<(std::ostream &os, const timer_result &res) {
		return os << res.duration << "ms";
	}
};

class timer_instrumentation : public instrumentation {
public:
	void setup() { t.reset(); }
	void finish() { value = t.get(); }
	timer_result result() const { return timer_result(value); }
private:
	timer t;
	double value;
};

struct papi_result : public benchmark_result {
	long long counters[3];
	int events[3];
	papi_result(int const *e, long long const *c) {
		counters[0] = c[0];	counters[1] = c[1];	counters[2] = c[2];
		events[0] = e[0]; events[1] = e[1]; events[2] = e[2];
	}

	static std::string describe_event(int event) {
		PAPI_event_info_t info;
		PAPI_get_event_info(event, &info);
		return std::string{(char*)info.short_descr};
	}

	friend std::ostream& operator<<(std::ostream &os, const papi_result &res) {
		return os << describe_event(res.events[0]) << ": " << res.counters[0] << "; "
				  << describe_event(res.events[1]) << ": " << res.counters[1] << "; "
				  << describe_event(res.events[2]) << ": " << res.counters[2] << ".";
	}
};

template<int event1 = PAPI_L1_DCM, int event2 = PAPI_L2_DCM, int event3 = PAPI_L3_TCM>
class papi_instrumentation : public instrumentation {
	long long counters[3];
	int events[3];
	papi_instrumentation(const papi_instrumentation &) = delete;
public:
	papi_instrumentation() : counters{0,0,0}, events{event1, event2, event3} {
		if (PAPI_is_initialized() == PAPI_NOT_INITED &&
			PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT) {
			std::cerr << "PAPI_library_init failed" << std::endl;
		}
	};

	void setup() {
		int ret = PAPI_start_counters(events, 3);
		if (ret != PAPI_OK) {
			std::cerr << "Failed to start PAPI counters" << std::endl;
		}
	}

	void finish() {
		int ret = PAPI_stop_counters(counters, 3);
		if (ret != PAPI_OK) {
			std::cerr << "PAPI_stop_counters failed" << std::endl;
		}
	}

	papi_result result() const {
		return papi_result(events, counters);
	}
};

using papi_instrumentation_cache = papi_instrumentation<>;
using papi_instrumentation_instr = papi_instrumentation<PAPI_BR_MSP, PAPI_TOT_INS, PAPI_TOT_CYC>;

}
