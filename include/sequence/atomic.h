#pragma once
#include "sequence/sequence.h"

namespace sequence {
template<class T>
struct atomic {
    atomic() = default;
    atomic(T value) : value(value) {}

    void store(T desired) {
        global_sequence->log += "atomic::store\n";
        value = desired;
        global_sequence->synchronize();
    }
    T load() {
        global_sequence->log += "atomic::load\n";
        auto return_value = value;
        global_sequence->synchronize();
        return return_value;
    }

    T value;
};
}
