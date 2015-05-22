#pragma once

#include <ostream>
#include <papi.h>

#include "../malloc_count/malloc_count.h"

#include "timer.h"
#include "benchmark.h"

namespace common {

class instrumentation {
public:
	virtual void setup() = 0;
	virtual void finish() = 0;
	virtual benchmark_result* result() const = 0;
	virtual void destroy(std::vector<benchmark_result*>::iterator begin, std::vector<benchmark_result*>::iterator end) = 0;
};

struct timer_result : public benchmark_result {
	double duration;
	timer_result(double d) : duration(d) {}
	virtual ~timer_result() = default;
	std::ostream& print(std::ostream& os) const {
		return os << duration << "ms";
	}
	std::ostream& result(std::ostream& os) const {
		return os << " time=" << duration;
	}
};

class timer_instrumentation : public instrumentation {
public:
	void setup() { t.reset(); }
	void finish() { value = t.get(); }
	virtual timer_result* result() const { return new timer_result(value); }
	virtual ~timer_instrumentation() = default;

	void destroy(std::vector<benchmark_result*>::iterator begin, std::vector<benchmark_result*>::iterator end) {
		while (begin != end)
			delete (timer_result*)*(begin++);
	}
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
	virtual ~papi_result() = default;

	static std::string describe_event(int event) {
		PAPI_event_info_t info;
		PAPI_get_event_info(event, &info);
		return std::string{(char*)info.short_descr};
	}

	std::ostream& print(std::ostream& os) const {
		return os << describe_event(events[0]) << ": " << counters[0] << "; "
				  << describe_event(events[1]) << ": " << counters[1] << "; "
				  << describe_event(events[2]) << ": " << counters[2] << ".";
	}

	std::ostream& result(std::ostream& os) const {
		return os << " " << format_result_column(describe_event(events[0])) << "=" << counters[0]
				  << " " << format_result_column(describe_event(events[1])) << "=" << counters[1]
				  << " " << format_result_column(describe_event(events[2])) << "=" << counters[2];
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
	virtual ~papi_instrumentation() = default;

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

	papi_result* result() const {
		return new papi_result(events, counters);
	}

	void destroy(std::vector<benchmark_result*>::iterator begin, std::vector<benchmark_result*>::iterator end) {
		while (begin != end)
			delete (papi_result*)*(begin++);
	}
};

using papi_instrumentation_cache = papi_instrumentation<>;
using papi_instrumentation_instr = papi_instrumentation<PAPI_BR_MSP, PAPI_TOT_INS, PAPI_TOT_CYC>;


struct memory_result : public benchmark_result {
	size_t peak;
	memory_result(size_t peak) : peak(peak) {}
	virtual ~memory_result() = default;
	std::ostream& print(std::ostream& os) const {
		return os << "peak memory: " << peak << "B (" << (1.0 * peak) / (1<<20) << " MB)";
	}
	std::ostream& result(std::ostream& os) const {
		return os << " peakmem=" << peak;
	}
};

class memory_instrumentation : public instrumentation {
public:
	void setup() { malloc_count_reset_peak(); }
	void finish() { peak = malloc_count_peak(); }
	virtual memory_result* result() const { return new memory_result(peak); }
	virtual ~memory_instrumentation() = default;

	void destroy(std::vector<benchmark_result*>::iterator begin, std::vector<benchmark_result*>::iterator end) {
		while (begin != end)
			delete (memory_result*)*(begin++);
	}
private:
	size_t peak;
};

}
