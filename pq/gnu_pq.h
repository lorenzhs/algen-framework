#pragma once

// This class uses features from GNU libstdc++'s policy-base
// data structures library
#if defined(__GNUG__) && !(defined(__APPLE_CC__))

#include <ext/pb_ds/priority_queue.hpp>
#include <utility>

#include "priority_queue.h"

namespace pq {

template<typename T,
         typename Cmp_Fn = std::less<T>,
         typename Tag = __gnu_pbds::pairing_heap_tag,
         typename Allocator = std::allocator<char>>
class gnu_pq : public priority_queue<T> {
public:
    gnu_pq() : queue() {}

    virtual ~gnu_pq() {
        bool is_pairing_heap = std::is_same<Tag,  __gnu_pbds::pairing_heap_tag>::value;
        if (is_pairing_heap) {
            // Pairing heap has a recursive destructor
            queue.clear();
        }
    };

    static void register_contenders(common::contender_list<priority_queue<T>> &list) {
        using Factory = common::contender_factory<priority_queue<T>>;
        // The pairing heap has a recursive destructor, be careful
        list.register_contender(Factory("GNU Pairing Heap", "GNU-pairing-heap",
            [](){ return new gnu_pq<T, std::less<T>, __gnu_pbds::pairing_heap_tag>();}
        ));
        // This one is mind-bogglingly slow, no idea what they did there
        //list.register_contender(Factory("GNU Binary Heap", "GNU-binary-heap",
        //    [](){ return new gnu_pq<T, std::less<T>, __gnu_pbds::binary_heap_tag>();}
        //));
        list.register_contender(Factory("GNU Binomial Heap", "GNU-binomial-heap",
            [](){ return new gnu_pq<T, std::less<T>, __gnu_pbds::binomial_heap_tag>();}
        ));
        list.register_contender(Factory("GNU RC Binomial Heap", "GNU-rc-binomial-heap",
            [](){ return new gnu_pq<T, std::less<T>, __gnu_pbds::rc_binomial_heap_tag>();}
        ));
        list.register_contender(Factory("GNU Thin Heap", "GNU-thin-heap",
            [](){ return new gnu_pq<T, std::less<T>, __gnu_pbds::thin_heap_tag>();}
        ));
    }

    /// Add an element to the priority queue by const lvalue reference
    void push(const T& value) override {
        queue.push(value);
    }
    /// Add an element to the priority queue by rvalue reference (with move)
    void push(T&& value) override {
        queue.push(std::move(value));
    }

    /// Add an element in-place without copying or moving
    template <typename... Args>
    void emplace(Args&&... args) {
        queue.emplace(std::forward<Args>(args)...);
    }

    /// Deletes the top element
    void pop() override {
        queue.pop();
    }

    /// Retrieves the top element
    const T& top() override {
        return queue.top();
    }

    /// Get the number of elements in the priority queue
    size_t size() override {
        return queue.size();
    }

protected:
    __gnu_pbds::priority_queue<T, Cmp_Fn, Tag, Allocator> queue;
};

}

#endif
