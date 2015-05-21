#pragma once

#include <functional>
#include <type_traits>

#include "contenders.h"

namespace common {

struct benchmark_result {};

template <typename DataStructure>
class benchmark {
public:
	using F = std::function<void(DataStructure&&)>;
	F function, setup, teardown;

	benchmark(F &&function) : function(std::forward<F>(function)) {};
	benchmark(F &&setup, F &&function) : setup(std::forward<F>(setup)), function(std::forward<F>(function)) {};
	benchmark(F &&setup, F &&function, F &&teardown)
		: setup(std::forward<F>(setup))
		, function(std::forward<F>(function))
		, teardown(std::forward<F>(teardown)) {};
	benchmark(const F &other) = delete;
	benchmark() = delete;

	template <typename Instrumentation>
	auto run(contender_factory<DataStructure> &factory, Instrumentation &instr) -> decltype(instr.result()) {
		//std::cout << "two lvalue references" << std::endl;
		DataStructure* instance = factory();
		if (setup) setup(std::forward<DataStructure>(*instance));
		instr.setup();
		function(std::forward<DataStructure>(*instance));
		instr.finish();
		if (teardown) teardown(std::forward<DataStructure>(*instance));
		factory.destroy(instance);
		return instr.result();
	}

	template <typename Instrumentation>
	auto run(contender_factory<DataStructure> &factory, Instrumentation &&instr) -> decltype(instr.result()) {
		//std::cout << "lvalue factory, rvalue instrumentation" << std::endl;
		DataStructure* instance = factory();
		if (setup) setup(std::forward<DataStructure>(*instance));
		instr.setup();
		function(std::forward<DataStructure>(*instance));
		instr.finish();
		if (teardown) teardown(std::forward<DataStructure>(*instance));
		factory.destroy(instance);
		return instr.result();
	}
};

template <typename F, typename Benchmark>
void register_benchmark(std::string &&description, F &&f, common::contender_list<Benchmark> &benchmarks) {
	benchmarks.register_contender(std::forward<std::string>(description),
		[f = std::forward<F>(f)]() -> Benchmark* {
			return new Benchmark(f);
		}
	);
}

template <typename Setup, typename Function, typename Benchmark>
void register_benchmark(std::string &&description,
	Setup &&setup, Function &&function,
	common::contender_list<Benchmark> &benchmarks)
{
	benchmarks.register_contender(std::forward<std::string>(description),
		[  setup = std::forward<Setup   >(setup),
		function = std::forward<Function>(function)]() -> Benchmark* {
			return new Benchmark(setup, function);
		}
	);
}

template <typename Setup, typename Function, typename Teardown, typename Benchmark>
void register_benchmark(std::string &&description,
	Setup &&setup, Function &&function, Teardown &&teardown,
	common::contender_list<Benchmark> &benchmarks)
{
	benchmarks.register_contender(std::forward<std::string>(description),
		[  setup = std::forward<Setup   >(setup),
		function = std::forward<Function>(function),
	    teardown = std::forward<Teardown>(teardown)]() -> Benchmark* {
			return new Benchmark(setup, function, teardown);
		}
	);
}

}
