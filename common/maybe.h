#pragma once

#include <functional>
#include <ostream>
#include <type_traits>
#include <utility>

namespace common {
namespace maybe {

// Kind of a maybe monad
template <typename T>
struct maybe {
	T data;
	bool valid;

	maybe() : data(), valid(false) {}
	maybe(const T& data) : data(data), valid(true) {}

	const T operator*() const { return data; }
	T&& operator*() { return std::forward<T>(data); }

	operator T() { return data; }

	friend std::ostream &operator<<(std::ostream &os, const maybe &m) {
		if (!m.valid) os << "nothing";
		else os << "just(" << m.data << ")";
		return os;
	}
};

// dummy
template<>
struct maybe<void> {
	static const bool valid = false;
	maybe() {}

	template <typename T>
	maybe(const T &data) {}

	const void operator*() const {}

	friend std::ostream &operator<<(std::ostream &os, const maybe &m) {
		return os << "nothing";
	}
};

template<typename T=void>
constexpr maybe<T> nothing() { return maybe<T>(); } 

template<typename T>
constexpr maybe<T> just(const T &t) { return maybe<T>(t); }

template<typename T>
constexpr maybe<T> just(T&& t) { return maybe<T>(std::forward<T>(t)); }

}
}

// Haskell >>= syntax, which unfortunately has right-to-left associativity
// so x >>= fun >>= otherfun won't work, you need (x >>= fun) >>= otherfun
template<typename T, typename Func>
auto operator>>=(common::maybe::maybe<T> &t, Func &&f) -> decltype(f(t.data)) {
	static_assert(std::is_same<decltype(f(t.data)), common::maybe::maybe<T>>::value, "Function does not return a maybe<T>");

	if (!t.valid) {
		return common::maybe::nothing<T>();
	} else {
		return std::forward<Func>(f)(t.data);
	}
}

// const version
template<typename T, typename Func>
auto operator>>=(const common::maybe::maybe<T> &t, Func &&f) -> decltype(f(t.data)) {
	static_assert(std::is_same<decltype(f(t.data)), common::maybe::maybe<T>>::value, "Function does not return a maybe<T>");

	if (!t.valid) {
		return common::maybe::nothing<T>();
	} else {
		return std::forward<Func>(f)(t.data);
	}
}

// Pipe syntax, which isn't as nice but has left-to-right associativity
// so you can write x | fun | otherfun | thirdfun and it will work
template<typename T, typename Func>
auto operator|(common::maybe::maybe<T> &t, Func &&f) -> decltype(f(t.data)) {
	return t >>= f;
}

template<typename T, typename Func>
auto operator|(const common::maybe::maybe<T> &t, Func &&f) -> decltype(f(t.data)) {
	return t >>= f;
}
