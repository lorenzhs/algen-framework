#pragma once

#include <cassert>
#include <unordered_map>

#include "../common/contenders.h"
#include "hashtable.h"

namespace hashtable {

template <typename Key,
          typename T,
          typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>,
          typename Allocator = std::allocator<std::pair<const Key, T>>>
class unordered_map : public hashtable<Key, T> {
public:
    unordered_map(const size_t bucket_count = 0) : map(bucket_count) {}
    virtual ~unordered_map() = default;

    // Register all contenders in the list
    static void register_contenders(common::contender_list<hashtable<Key, T>> &list) {
        using Factory = common::contender_factory<hashtable<Key, T>>;
        list.register_contender(Factory("std::unordered_map", "std::unordered-map",
            [](){ return new unordered_map<Key, T>();}
        ));
    }

    T& operator[](const Key &key) override {
        return map[key];
    }

    T& operator[](Key&& key) override {
        return map[std::move(key)];
    }

    maybe<T> find(const Key &key) const override {
        auto it = map.find(key);
        if (it == map.end()) {
            return nothing<T>();
        } else {
            assert(it->first == key);
            return just<T>(it->second);
        }
    }

    size_t erase(const Key &key) override {
        return map.erase(key);
    }

    size_t size() const override { return map.size(); }

    void clear() override { map.clear(); }

protected:
    std::unordered_map<Key, T, Hash, KeyEqual, Allocator> map;
};

}
