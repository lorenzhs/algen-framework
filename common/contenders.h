#pragma once

#include <functional>
#include <type_traits>
#include <vector>

namespace common {

template <typename R>
class contender_factory {
public:
	using F = std::function<R*(void)>;

	contender_factory(F &&factory) : factory(std::forward<F>(factory)) {}
	contender_factory(contender_factory &&other) = default;
	contender_factory(const contender_factory &other) = default;
	contender_factory() = delete;

	R* operator()() {
		return factory();
	}
protected:
	F factory;
};


template <typename Base>
class contender_list {
public:
	std::vector<contender_factory<Base>> contenders;

	contender_list() : contenders() {}
	contender_list(contender_list &other) = delete;

	void register_contender(const contender_factory<Base> &c) {
		contenders.emplace_back(c); // push_back?
	}

	void register_contender(contender_factory<Base> &&c) {
		contenders.emplace_back(std::forward<contender_factory<Base>>(c));
	}
};

}