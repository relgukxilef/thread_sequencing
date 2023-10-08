#pragma once
#include "sequence/sequence.h"

namespace sequence {
template<class T>
struct atomic {
    atomic() = default;
    atomic(T value) : value(value) {}

    void store(
        T desired,
        std::source_location location = std::source_location::current()
    ) {
        global_sequence->log("atomic::store", location);
        value = desired;
        global_sequence->synchronize();
    }
    T load(std::source_location location = std::source_location::current()) {
        global_sequence->log("atomic::load", location);
        auto return_value = value;
        global_sequence->synchronize();
        return return_value;
    }

    T value;
};
}
