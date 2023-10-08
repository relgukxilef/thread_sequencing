#pragma once
#include "sequence/sequence.h"

namespace sequence {
template<class T>
struct atomic {
    atomic() = default;
    atomic(T value) : value(value) {}

    void store(T desired) {
        value = desired;
        global_sequence->synchronize();
    }
    T load() {
        return value;
        global_sequence->synchronize();
    }

    T value;
};
}
