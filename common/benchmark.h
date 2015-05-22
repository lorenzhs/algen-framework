#pragma once

#include <functional>
#include <string>
#include <type_traits>

#include "contenders.h"

namespace common {

struct benchmark_result {
	virtual std::ostream& print(std::ostream &os) const = 0;
	virtual std::ostream& result(std::ostream &os) const = 0;
	friend std::ostream& operator<<(std::ostream &os, const benchmark_result &res) {
		return res.print(os);
	}
protected:
	// remove spaces for sqlplottools
	std::string& format_result_column(std::string&& str) const {
		size_t pos = 0;

		while ((pos = str.find(" ", pos)) != std::string::npos) {
			str.replace(pos, 1, "-");
			pos++;
		}
		return str;
	}
};

template <typename DataStructure, typename Configuration>
class benchmark {
public:
	using F = std::function<void(DataStructure&&, Configuration&)>;
	F function, setup, teardown;
	const std::vector<Configuration> &configurations;

	// Constructor from benchmark function and configuration vector
	benchmark(F &&function, const std::vector<Configuration> &configurations)
		: function(std::move(function))
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
		Configuration &configuration)
		-> decltype(instrumentation()->result())
	{
		// Create data structure and set up benchmark
		DataStructure *instance = factory();
		if (setup) setup(std::forward<DataStructure>(*instance), configuration);

		// Set up instrumentation
		Instrumentation *instr = instrumentation();
		instr->setup();

		// Run benchmark
		function(std::forward<DataStructure>(*instance), configuration);

		// stop and destroy instrumentation
		instr->finish();
		auto result = instr->result();
		instrumentation.destroy(instr);

		// Tear down benchmark and destroy data structure
		if (teardown) teardown(std::forward<DataStructure>(*instance), configuration);
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
		std::forward<std::string>(description),
		std::forward<std::string>(key),
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
		std::forward<std::string>(description),
		std::forward<std::string>(key),
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
		std::forward<std::string>(description),
		std::forward<std::string>(key),
		[        setup = std::forward<Setup   >(setup),
		      function = std::forward<Function>(function),
	          teardown = std::forward<Teardown>(teardown),
	    configurations = std::forward<Configs >(configurations)]() -> Benchmark* {
			return new Benchmark(setup, function, teardown, configurations);
		}
	);
}

}
