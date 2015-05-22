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
    unordered_map(const size_t bucket_count = 0) : hashtable<Key, T>(), map(bucket_count) {}
    virtual ~unordered_map() = default;

    // Register all contenders in the list
    static void register_contenders(common::contender_list<hashtable<Key, T>> &list) {
        using Factory = common::contender_factory<hashtable<Key, T>>;
        list.register_contender(Factory("std::unordered_map", "std__unordered_map",
            [](){ return new unordered_map<Key, T>();},
            [](hashtable<Key, T>* ht) { delete (unordered_map<Key, T>*) ht; }
        ));
    }

    T& operator[](const Key &key) {
        return map[key];
    }

    T& operator[](Key&& key) {
        return map[std::forward<Key>(key)];
    }

    maybe<T> find(const Key &key) {
        auto it = map.find(key);
        if (it == map.end()) {
            return nothing<T>();
        } else {
            assert(it->first == key);
            return just<T>(it->second);
        }
    }

    size_t erase(const Key &key) {
        return map.erase(key);
    }

    size_t size() const { return map.size(); }

    void clear() { map.clear(); }

protected:
    std::unordered_map<Key, T, Hash, KeyEqual, Allocator> map;
};

};
