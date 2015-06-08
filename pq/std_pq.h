#pragma once

#include <queue>
#include <utility>

#include "priority_queue.h"

namespace pq {

template<typename T,
         typename Container = std::vector<T>,
         typename Compare = std::less<typename Container::value_type>>
class std_pq : public priority_queue<T> {
public:
    std_pq() : queue() {}

    static void register_contenders(common::contender_list<priority_queue<T>> &list) {
        using Factory = common::contender_factory<priority_queue<T>>;
        list.register_contender(Factory("std::priority_queue", "std::priority-queue",
            [](){ return new std_pq<T>();}
        ));
        list.register_contender(Factory("std::priority_queue with deque", "std::priority-queue-deque",
            [](){ return new std_pq<T, std::deque<T>>();}
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
    std::priority_queue<T, Container, Compare> queue;
};

}