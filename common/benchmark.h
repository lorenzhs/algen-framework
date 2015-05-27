#pragma once

#include <functional>
#include <string>
#include <type_traits>

#include "contenders.h"

namespace common {

struct benchmark_result {
	template <typename Configuration>
	void set_properties(const std::string &benchmark_name,
		const std::string &instance_desc,
		const Configuration &configuration_obj)
	{
		benchmark = benchmark_name;
		instance = instance_desc;

		std::stringstream s;
		s << configuration_obj;
		configuration = s.str();
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

	virtual std::ostream& print(std::ostream &os) const = 0;
	virtual std::ostream& result(std::ostream &os) const = 0;
	friend std::ostream& operator<<(std::ostream &os, const benchmark_result &res) {
		os << "Benchmark '" << res.benchmark
		   << "' on instance '" << res.instance
		   << "' with configuration '" << res.configuration << "': ";
		return res.print(os);
	}
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

	std::string benchmark, instance, configuration;
};

template <typename DataStructure, typename Configuration>
class benchmark {
public:
	using F = std::function<void(DataStructure&, Configuration&)>;
	F function, setup, teardown;
	const std::vector<Configuration> &configurations;

	// Constructor from benchmark function and configuration vector
	benchmark(F &&function, const std::vector<Configuration> &configurations)
		: function(std::forward<F>(function))
		, configurations(configurations) {}

	// Constructor from setup, benchmark, and configurations
	benchmark(F &&setup, F &&function, const std::vector<Configuration> &configurations)
		: function(std::forward<F>(function))
		, setup(std::forward<F>(setup))
		, configurations(configurations) {}

	// Constructor from setup, benchmark, teardown + configurations
	benchmark(F &&setup, F &&function, F &&teardown, const std::vector<Configuration> &configurations)
		: function(std::forward<F>(function))
		, setup(std::forward<F>(setup))
		, teardown(std::forward<F>(teardown))
		, configurations(configurations) {}

	benchmark(const F &other) = delete;
	benchmark(F &&other) = delete;
	benchmark() = delete;

	template <typename Instrumentation>
	auto run(contender_factory<DataStructure> &factory,
		contender_factory<Instrumentation> &instrumentation,
		Configuration &configuration,
		contender_factory<benchmark> &benchmark_factory)
		-> decltype(instrumentation()->result())
	{
		// Create data structure and set up benchmark
		DataStructure *instance = factory();
		if (setup) setup(*instance, configuration);

		// Set up instrumentation
		Instrumentation *instr = instrumentation();
		instr->setup();

		// Run benchmark
		function(*instance, configuration);

		// stop and destroy instrumentation
		instr->finish();
		auto result = instr->result();

		result->set_properties(benchmark_factory.description(), factory.description(), configuration);

		instrumentation.destroy(instr);

		// Tear down benchmark and destroy data structure
		if (teardown) teardown(*instance, configuration);
		factory.destroy(instance);

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
