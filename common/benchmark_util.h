#pragma once

#include <random>

namespace common {
namespace util {
    template <typename T, typename F>
    static T* fill_data(size_t size, F&& cb) {
        auto data = new T[size];
        for (size_t i = 0; i < size; ++i)
            data[i] = cb(i);
        return data;
    }

    template <typename T>
    static void* fill_data_permutation(size_t size, size_t seed) {
        auto data = fill_data<T>(size, [](size_t i) {return i;});
        std::shuffle(data, data + size, std::mt19937{seed});
        return data;
    }

    template <typename T>
    static void* fill_data_random(size_t size, size_t seed) {
        std::mt19937 gen{seed};
        return fill_data<T>(size, [&gen](size_t) {return gen();});
    }

    template <typename T>
    static void delete_data(void* data) {
        delete[] static_cast<T*>(data);
    }

    // awful hack!
    static std::string hex_to_ascii(size_t hex) {
        union {
            size_t hex;
            char letters[sizeof(size_t)];
        } u = {hex};

        std::stringstream s;
        for (int i = sizeof(size_t) - 1; i >= 0; --i) {
            if (u.letters[i] > 0)
                s << u.letters[i];
        }
        return s.str();
    }
}
}
