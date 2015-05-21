#pragma once

#include <functional>
#include <type_traits>
#include <string>
#include <vector>

namespace common {

// to be used for both data structure contenders and benchmarks
template <typename R>
class contender_factory {
public:
	using F = std::function<R*(void)>;
	using D = std::function<void(R*)>; // Destructor

	contender_factory(std::string &&desc, F &&factory) :
		_description(std::forward<std::string>(desc)),
		factory(std::forward<F>(factory)) {}
	contender_factory(std::string &&desc, F &&factory, D &&destructor) :
		_description(std::forward<std::string>(desc)),
		factory(std::forward<F>(factory)),
		destructor(std::forward<D>(destructor)) {}
	contender_factory(contender_factory &&other) = default;
	contender_factory(const contender_factory &other) = default;
	contender_factory() = delete;

	R* operator()() {
		return factory();
	}

	void destroy(R* instance) {
		destructor(instance);
	}

	const std::string& description() {
		return _description;
	}

protected:
	F factory;
	D destructor;
	std::string _description;
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

	template <typename F>
	void register_contender(std::string &&desc, F &&f) {
		contenders.emplace_back(contender_factory<Base>(std::forward<std::string>(desc),
			std::forward<F>(f)));
	}

	size_t size() const {
		return contenders.size();
	}

	contender_factory<Base>& operator[](const size_t index) {
		return contenders[index];
	}

	// Provide iterators for easy access (both non-const and const)
	auto begin() -> decltype(contenders.begin()) {
		return contenders.begin();
	}

	auto begin() const -> decltype(contenders.begin()) {
		return contenders.begin();
	}

	auto end() -> decltype(contenders.end()) {
		return contenders.end();
	}

	auto end() const -> decltype(contenders.end()) {
		return contenders.end();
	}
};

}
