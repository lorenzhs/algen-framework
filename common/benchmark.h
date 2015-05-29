#pragma once

#include <functional>
#include <string>
#include <sstream>
#include <type_traits>

#include <boost/serialization/base_object.hpp>

#include "contenders.h"
#include "terminal.h"

namespace common {

class benchmark_result {
    friend class boost::serialization::access;
public:
    virtual ~benchmark_result() {}

    virtual void add(const benchmark_result *const other) = 0;
    virtual void min(const benchmark_result *const other) = 0;
    virtual void max(const benchmark_result *const other) = 0;
    virtual void div(const int divisor) = 0;

    virtual std::vector<double> compare_to(const benchmark_result *other) = 0;
    virtual std::ostream& print_component(int component, std::ostream &os) = 0;

    virtual std::ostream& print(std::ostream &os) const = 0;
    virtual std::ostream& result(std::ostream &os) const = 0;

    template <typename Archive>
    void serialize(Archive & ar, const unsigned int file_version) { (void) ar; (void) file_version; }
protected:
    // remove spaces for sqlplottools
    template <typename String> // URef
    String&& format_result_column(String &&str) const {
        size_t pos = 0;

        while ((pos = str.find(" ", pos)) != std::string::npos) {
            str.replace(pos, 1, "_");
            pos++;
        }
        return std::forward<String>(str);
    }
};

class benchmark_result_aggregate {
    friend class boost::serialization::access;
public:
    benchmark_result_aggregate() : min(nullptr), max(nullptr), avg(nullptr), num_results(0) {}
    benchmark_result_aggregate(benchmark_result *min, benchmark_result *max, benchmark_result *avg)
        : min(min), max(max), avg(avg), num_results(0) {}
    benchmark_result_aggregate(const benchmark_result_aggregate &other) = default;
    benchmark_result_aggregate(benchmark_result_aggregate&& other) = default;
    void destroy() { // can't put this in d'tor because copies are made
        if (min != nullptr) delete min; min = nullptr;
        if (max != nullptr) delete max; max = nullptr;
        if (avg != nullptr) delete avg; avg = nullptr;
    }
    void add_result(const benchmark_result *const result) {
        ++num_results;
        min->min(result);
        max->max(result);
        avg->add(result);
    }

    void finish() {
        avg->div(num_results);
    }

    const benchmark_result * minimum() const { return min; }
    const benchmark_result * maximum() const { return max; }
    const benchmark_result * average() const { return avg; }

    std::vector<double> compare_to(const benchmark_result_aggregate &other) {
        return avg->compare_to(other.avg);
    }

    std::ostream& print_component(int component, std::ostream &os) {
        return avg->print_component(component, os);
    }

    template <typename Configuration>
    void set_properties(const std::string &benchmark_name,
        const std::string &instance_desc,
        const Configuration &configuration_obj)
    {
        benchmark = benchmark_name;
        instance = instance_desc;

        // convert configuration object into a string
        // this might be a trivial type (e.g. size_t)
        // so we can't just call a description member
        if (std::is_arithmetic<Configuration>::value) {
            configuration = std::to_string(configuration_obj);
        } else {
            std::ostringstream s;
            s << configuration_obj;
            configuration = s.str();
        }
    }

    const std::string& benchmark_name() const {
        return benchmark;
    }
    const std::string& instance_desc() const {
        return instance;
    }
    const std::string& configuration_desc() const {
        return configuration;
    }

    std::ostream& describe(std::ostream &os) const {
        return os << "Benchmark '" << term::bold << benchmark << term::reset
                  << "' on instance '" << term::bold << instance << term::reset
                  << "' with configuration '" << term::bold << configuration << term::reset << "': ";
    }

    friend std::ostream& operator<<(std::ostream &os, const benchmark_result_aggregate &res) {
        res.describe(os);
        if (res.num_results > 1) {
            os << res.num_results << " runs.";
            os << std::endl << "\tmin: "; res.min->print(os);
            os << std::endl << "\tmax: "; res.max->print(os);
            os << std::endl << "\tavg: "; res.avg->print(os);
        } else { // only one run, they're all equal
            res.min->print(os);
        }
        return os;
    }

    template <typename Archive>
    void serialize(Archive & ar, const unsigned int file_version) {
        (void) file_version; // unused
        ar & benchmark & instance & configuration;
        ar & min & max & avg & num_results;
    }
protected:
    std::string benchmark, instance, configuration;
    benchmark_result *min, *max, *avg;
    int num_results;
};

template <typename DataStructure, typename Configuration>
class benchmark {
public:
    using F = std::function<void(DataStructure&, Configuration&, void*)>;
    using S = std::function<void*(DataStructure&, Configuration&, void*)>;
    S setup;
    F function, teardown;
    const std::vector<Configuration> &configurations;
    void* data;

    // Constructor from benchmark function and configuration vector
    benchmark(F &&function, const std::vector<Configuration> &configurations)
        : function(std::move(function))
        , configurations(configurations) {}

    // Constructor from setup, benchmark, and configurations
    benchmark(S &&setup, F &&function, const std::vector<Configuration> &configurations)
        : setup(std::move(setup))
        , function(std::move(function))
        , configurations(configurations) {}

    // Constructor from setup, benchmark, teardown + configurations
    
    benchmark(S &&setup, F &&function, F &&teardown, const std::vector<Configuration> &configurations)
        : setup(std::move(setup))
        , function(std::move(function))
        , teardown(std::move(teardown))
        , configurations(configurations) {}

    benchmark(const benchmark &other) = delete;
    benchmark(benchmark &&other) = delete;
    benchmark() = delete;

    template <typename Instrumentation>
    auto run(contender_factory<DataStructure> &factory,
        Instrumentation *instrumentation,
        Configuration &configuration)
        -> decltype(instrumentation->result())
    {
        // Create data structure and set up benchmark
        DataStructure *instance = factory();
        if (setup) data = setup(*instance, configuration, data);

        // Set up instrumentation
        instrumentation->setup();

        // Run benchmark
        function(*instance, configuration, data);

        // stop and destroy instrumentation
        instrumentation->finish();
        auto result = instrumentation->result();

        // Tear down benchmark and destroy data structure
        if (teardown) teardown(*instance, configuration, data);
        delete instance;

        return result;
    }

    // Provide iterators for easy access (both non-const and const)
    auto begin() -> decltype(configurations.begin()) {
        return configurations.begin();
    }

    auto begin() const -> decltype(configurations.begin()) {
        return configurations.begin();
    }

    auto end() -> decltype(configurations.end()) {
        return configurations.end();
    }

    auto end() const -> decltype(configurations.end()) {
        return configurations.end();
    }
};

template <typename F, typename Configs, typename Benchmark>
void register_benchmark(std::string &&description, std::string &&key,
    F &&f, Configs &&configurations, common::contender_list<Benchmark> &benchmarks)
{
    benchmarks.register_contender(
        std::move(description),
        std::move(key),
        [f = std::forward<F>(f),
        configurations = std::forward<Configs>(configurations)]() -> Benchmark* {
            return new Benchmark(f, configurations);
        }
    );
}

template <typename Setup, typename Function, typename Configs, typename Benchmark>
void register_benchmark(std::string &&description, std::string &&key,
    Setup &&setup, Function &&function, Configs &&configurations,
    common::contender_list<Benchmark> &benchmarks)
{
    benchmarks.register_contender(
        std::move(description),
        std::move(key),
        [        setup = std::forward<Setup   >(setup),
              function = std::forward<Function>(function),
        configurations = std::forward<Configs >(configurations)]() -> Benchmark* {
            return new Benchmark(setup, function, configurations);
        }
    );
}

template <typename Setup, typename Function, typename Teardown, typename Configs, typename Benchmark>
void register_benchmark(std::string &&description, std::string &&key,
    Setup &&setup, Function &&function, Teardown &&teardown,
    Configs &&configurations, common::contender_list<Benchmark> &benchmarks)
{
    benchmarks.register_contender(
        std::move(description),
        std::move(key),
        [        setup = std::forward<Setup   >(setup),
              function = std::forward<Function>(function),
              teardown = std::forward<Teardown>(teardown),
        configurations = std::forward<Configs >(configurations)]() -> Benchmark* {
            return new Benchmark(setup, function, teardown, configurations);
        }
    );
}

}
