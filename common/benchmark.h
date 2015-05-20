#pragma once

#include <functional>
#include <type_traits>

#include "contenders.h"

namespace common {

template <typename DataStructure>
class benchmark {
public:
	using F = std::function<void(DataStructure&&)>;
	F f;

	benchmark(F &&f) : f(std::forward<F>(f)) {};
	benchmark(const F &other) = delete;
	benchmark() = delete;

	template <typename Instrumentation>
	typename Instrumentation::result_type run(contender_factory<DataStructure> &&instance, Instrumentation instr) {
		instr.setup();
		f(std::forward<DataStructure>(instance));
		instr.finish();
		return instr.result();
	}
};

}