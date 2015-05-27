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

	contender_factory(std::string &&desc, std::string &&key, F &&factory) :
		factory(std::move(factory)),
		_description(std::move(desc)),
		_key(std::move(key)) {}
	contender_factory(std::string &&desc, std::string &&key, F &&factory, D &&destructor) :
		factory(std::move(factory)),
		destructor(std::move(destructor)),
		_description(std::move(desc)),
		_key(std::move(key)) {}
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

	const std::string& key() {
		return _key;
	}

protected:
	F factory;
	D destructor;
	std::string _description;
	std::string _key;
};


template <typename Base>
class contender_list {
public:
	std::vector<contender_factory<Base>> contenders;

	contender_list() : contenders() {}
	contender_list(contender_list &other) = delete;

	template <typename Contender> // URef
	void register_contender(Contender &&c) {
		contenders.emplace_back(std::forward<Contender>(c));
	}

	template <typename F>
	void register_contender(std::string &&desc, std::string &&key, F &&f) {
		contenders.emplace_back(contender_factory<Base>(
			std::move(desc),
			std::move(key),
			std::forward<F>(f)));
	}

	template <typename F, typename D>
	void register_contender(std::string &&desc, std::string &&key, F &&f, D &&d) {
		contenders.emplace_back(contender_factory<Base>(
			std::move(desc),
			std::move(key),
			std::forward<F>(f),
			std::forward<D>(d)));
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
