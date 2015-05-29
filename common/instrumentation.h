#pragma once

#include <algorithm>
#include <ostream>
#include <papi.h>

#include "../malloc_count/malloc_count.h"

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>

#include "timer.h"
#include "benchmark.h"

namespace common {

class instrumentation {
public:
    virtual void setup() = 0;
    virtual void finish() = 0;
    virtual benchmark_result* result() const = 0;
    virtual benchmark_result* new_result(bool set_to_max = false) const = 0;
    virtual ~instrumentation() {}
};

class timer_result : public benchmark_result {
    friend class boost::serialization::access;
    double duration;
public:
    timer_result(double d) : duration(d) {}
    timer_result() : duration(0) {}
    virtual ~timer_result() {}
    std::ostream& print(std::ostream& os) const {
        return os << duration << "ms";
    }
    std::ostream& result(std::ostream& os) const {
        return os << " time=" << duration;
    }

    virtual void add(const benchmark_result *const other) {
        duration += dynamic_cast<const timer_result*>(other)->duration;
    };
    virtual void min(const benchmark_result *const other) {
        duration = std::min(duration, dynamic_cast<const timer_result*>(other)->duration);
    };
    virtual void max(const benchmark_result *const other) {
        duration = std::max(duration, dynamic_cast<const timer_result*>(other)->duration);
    };
    virtual void div(const int divisor) { duration /= divisor; };

    virtual std::vector<double> compare_to(const benchmark_result *other) {
        const timer_result *o = dynamic_cast<const timer_result*>(other);
        return std::vector<double>{duration / o->duration};
    }

    virtual std::ostream& print_component(int component, std::ostream &os) {
        assert(component == 0); (void)component;
        return os << duration << "ms";
    }

    template <typename Archive>
    void serialize(Archive & ar, const unsigned int file_version) {
        (void)file_version;
        ar & boost::serialization::base_object<benchmark_result>(*this);
        ar & duration;
    }
};

class timer_instrumentation : public instrumentation {
public:
    void setup() { t.reset(); }
    void finish() { value = t.get(); }
    virtual timer_result* result() const { return new timer_result(value); }
    virtual benchmark_result* new_result(bool set_to_max = false) const {
        return new timer_result(set_to_max ? 1e100 : 0);
    };
    virtual ~timer_instrumentation() = default;
private:
    timer t;
    double value;
};

class papi_result : public benchmark_result {
    friend class boost::serialization::access;
    long long counters[3];
    int events[3];
public:
    papi_result() : counters{0,0,0}, events{0,0,0} {}
    papi_result(int const *e, bool set_to_max = false) {
        long long val = set_to_max ? ((long long)1)<<62 : 0;
        counters[0] = val; counters[1] = val; counters[2] = val;
        events[0] = e[0]; events[1] = e[1]; events[2] = e[2];
    }
    papi_result(int const *e, long long const *c) {
        counters[0] = c[0]; counters[1] = c[1]; counters[2] = c[2];
        events[0] = e[0]; events[1] = e[1]; events[2] = e[2];
    }
    virtual ~papi_result() {}

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

    virtual std::ostream& print_component(int component, std::ostream &os) {
        assert(component >= 0 && component <= 2);
        return os << describe_event(events[component]) << ": " << counters[component];
    }

    std::ostream& result(std::ostream& os) const {
        return os << " " << format_result_column(describe_event(events[0])) << "=" << counters[0]
                  << " " << format_result_column(describe_event(events[1])) << "=" << counters[1]
                  << " " << format_result_column(describe_event(events[2])) << "=" << counters[2];
    }

    virtual void add(const benchmark_result *const other) {
        const papi_result* o = dynamic_cast<const papi_result*>(other);
        counters[0] += o->counters[0];
        counters[1] += o->counters[1];
        counters[2] += o->counters[2];
    };
    virtual void min(const benchmark_result *const other) {
        const papi_result* o = dynamic_cast<const papi_result*>(other);
        counters[0] = std::min(counters[0], o->counters[0]);
        counters[1] = std::min(counters[1], o->counters[1]);
        counters[2] = std::min(counters[2], o->counters[2]);
    };
    virtual void max(const benchmark_result *const other) {
        const papi_result* o = dynamic_cast<const papi_result*>(other);
        counters[0] = std::max(counters[0], o->counters[0]);
        counters[1] = std::max(counters[1], o->counters[1]);
        counters[2] = std::max(counters[2], o->counters[2]);
    };
    virtual void div(const int divisor) {
        counters[0] /= divisor;
        counters[1] /= divisor;
        counters[2] /= divisor;
    };

    virtual std::vector<double> compare_to(const benchmark_result *other) {
        const papi_result *o = dynamic_cast<const papi_result*>(other);
        auto divide = [](long long a, long long b) -> double {
            if (a == 0 && b == 0) return 1.0;
            else return (a * 1.0) / b;
        };
        return std::vector<double>{
            divide(counters[0], o->counters[0]),
            divide(counters[1], o->counters[1]),
            divide(counters[2], o->counters[2])
        };
    }

    template <typename Archive>
    void serialize(Archive & ar, const unsigned int file_version) {
        (void)file_version;
        ar & boost::serialization::base_object<benchmark_result>(*this);
        ar & events & counters;
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

    virtual benchmark_result* new_result(bool set_to_max = false) const {
        return new papi_result(events, set_to_max);
    };
};

using papi_instrumentation_cache = papi_instrumentation<>;
using papi_instrumentation_instr = papi_instrumentation<PAPI_BR_MSP, PAPI_TOT_INS, PAPI_TOT_CYC>;


class memory_result : public benchmark_result {
    friend class boost::serialization::access;
    size_t total, peak, count;
public:
    memory_result() : total(0), peak(0), count(0) {}
    memory_result(size_t total, size_t peak, size_t count) : total(total), peak(peak), count(count) {}
    virtual ~memory_result() {}

    std::ostream& print(std::ostream& os) const {
        return os
            << "total allocations: " << total << "B (" << (1.0 * total) / (1<<20) << " MB)"
            <<     "; peak memory: " <<  peak << "B (" << (1.0 *  peak) / (1<<20) << " MB)"
            << "; num mallocs: " << count;
    }
    std::ostream& result(std::ostream& os) const {
        return os << " totalmem=" << total << " peakmem=" << peak << " mallocs=" << count;
    }

    virtual void add(const benchmark_result *const other) {
        const memory_result* o = dynamic_cast<const memory_result*>(other);
        total += o->total;
        peak  += o->peak;
        count += o->count;
    };
    virtual void min(const benchmark_result *const other) {
        const memory_result* o = dynamic_cast<const memory_result*>(other);
        total = std::min(total, o->total);
        peak  = std::min(peak,  o->peak );
        count = std::min(count, o->count);
    };
    virtual void max(const benchmark_result *const other) {
        const memory_result* o = dynamic_cast<const memory_result*>(other);
        total = std::max(total, o->total);
        peak  = std::max(peak,  o->peak );
        count = std::max(count, o->count);
    };
    virtual void div(const int divisor) {
        total /= divisor;
        peak  /= divisor;
        count /= divisor;
    };

    virtual std::vector<double> compare_to(const benchmark_result *other) {
        const memory_result *o = dynamic_cast<const memory_result*>(other);
        auto divide = [](size_t a,size_t b) -> double {
            if (a == 0 && b == 0) return 1.0;
            else return (a * 1.0) / b;
        };
        return std::vector<double>{
            divide(total, o->total),
            divide(peak , o->peak),
            divide(count, o->count)
        };
    }

    virtual std::ostream& print_component(int component, std::ostream &os) {
        switch (component) {
        case 0: return os << "total allocations: " << total << "B (" << (1.0 * total) / (1<<20) << " MB)";
        case 1: return os <<       "peak memory: " <<  peak << "B (" << (1.0 *  peak) / (1<<20) << " MB)";
        case 2: return os << "num mallocs: " << count;
        default: assert(false); return os;
        }
    }

    template <typename Archive>
    void serialize(Archive & ar, const unsigned int file_version) {
        (void)file_version;
        ar & boost::serialization::base_object<benchmark_result>(*this);
        ar & total & peak & count;
    }
};

class memory_instrumentation : public instrumentation {
public:
    virtual ~memory_instrumentation() = default;
    void setup() {
        // measure base usage of framework
        last = base = malloc_count_current();
        total = count = 0;
        malloc_count_reset_peak();
        malloc_count_reset_total();
        malloc_count_reset_num_allocs();
    }

    void finish() {
        peak = malloc_count_peak();
        total = malloc_count_total();
        count = malloc_count_num_allocs();
    }

    virtual memory_result* result() const {
        // subtract framework memory from peak usage
        return new memory_result(total, peak - base, count);
    }

    virtual memory_result* new_result(bool set_to_max = false) const {
        size_t value = set_to_max ? ((size_t)1) << 62 : 0;
        return new memory_result(value, value, value);
    }

private:
    size_t base;
    size_t peak;
    size_t last;
    size_t total;
    size_t count;
};

}


BOOST_CLASS_EXPORT_KEY(common::benchmark_result)

BOOST_CLASS_EXPORT_KEY(common::timer_result)
BOOST_CLASS_EXPORT_IMPLEMENT(common::timer_result)

BOOST_CLASS_EXPORT_KEY(common::papi_result)
BOOST_CLASS_EXPORT_IMPLEMENT(common::papi_result)

BOOST_CLASS_EXPORT_KEY(common::memory_result)
BOOST_CLASS_EXPORT_IMPLEMENT(common::memory_result)
