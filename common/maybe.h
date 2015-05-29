#pragma once

#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace common {
namespace monad {

// Kind of a maybe monad
template <typename T>
struct maybe {
    const T data;
    const bool valid;
    static const bool is_maybe = true;

    maybe() : data(), valid(false) {}
    maybe(const maybe &other) = default;
    maybe(maybe &&other) = default;
    maybe(const T &data) : data(data), valid(true) {}

    // Dereference to obtain value. Throws if nothing!
    T operator*() const {
        if (!valid)
            throw std::logic_error("Cannot dereference Nothing");
        return data;
    }
    const T& operator*() {
        if (!valid)
            throw std::logic_error("Cannot dereference Nothing");
        return data;
    }

    // define cast to T as unpacking. This throws, so be careful!
    // actually you shouldn't use this
    operator T() {
        if (!valid)
            throw std::logic_error("Cannot cast Nothing");
        return data;
    }

    // Need assignment operator because move constructor is defined
    void operator=(const maybe<T> &other) {
        data = other.data;
        valid = other.valid;
    }

    template<typename T2>
    bool operator==(const maybe<T2> &other) const {
        static_assert(std::is_convertible<T, T2>::value, "Cannot convert maybe<> types");
        return (valid == other.valid) && (!valid || data == other.data);
    }

    template<typename T2>
    bool operator!=(const maybe<T2> &other) const {
        return (valid != other.valid) || (valid && data != other.data);
    }

    friend std::ostream &operator<<(std::ostream &os, const maybe &m) {
        if (m.valid) os << "just(" << m.data << ")";
        else os << "nothing";
        return os;
    }

    template<typename U>
    friend constexpr maybe<U> nothing();
    template<typename U>
    friend constexpr maybe<U> just(const U &u);
    template<typename U>
    friend constexpr maybe<U> just(U &&u);
};

// dummy
template<>
struct maybe<void> {
    static const bool valid = false;
    static const bool is_maybe = true;

    maybe() {}

    template <typename T>
    maybe(const T &data) { static_assert(sizeof(T) == 0, "can't construct maybe<void> with value"); (void)data; }

    void operator*() const { throw std::logic_error("Cannot dereference maybe<void>"); }

    template<typename T>
    operator T() { throw std::logic_error("Cannot cast maybe<void> to value type"); }
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
auto operator>>=(common::monad::maybe<T> &t, Func &&f) -> decltype(f(t.data)) {
    static_assert(decltype(f(t.data))::is_maybe, "Function does not return a maybe<T>");

    if (!t.valid) {
        return decltype(f(t.data))(); // nothing
    } else {
        return std::forward<Func>(f)(t.data);
    }
}

// const version
template<typename T, typename Func>
auto operator>>=(const common::monad::maybe<T> &t, Func &&f) -> decltype(f(t.data)) {
    static_assert(decltype(f(t.data))::is_maybe, "Function does not return a maybe<T>");

    if (!t.valid) {
        return decltype(f(t.data))(); // nothing
    } else {
        return std::forward<Func>(f)(t.data);
    }
}

// Pipe syntax, which isn't as nice but has left-to-right associativity
// so you can write x | fun | otherfun | thirdfun and it will work
template<typename T, typename Func>
auto operator|(common::monad::maybe<T> &t, Func &&f) -> decltype(f(t.data)) {
    return t >>= f;
}

template<typename T, typename Func>
auto operator|(const common::monad::maybe<T> &t, Func &&f) -> decltype(f(t.data)) {
    return t >>= f;
}
