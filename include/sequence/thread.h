#pragma once
#include <thread>
#include <source_location>

#include "sequence/sequence.h"

namespace sequence {
struct thread {
    thread() = default;

    template <class F, class... Args>
    explicit thread(
        F&& f, Args&&... args,
        std::source_location location = std::source_location::current()
    ) {
        global_sequence->log("thread::thread", location);
        new_id = global_sequence->before_thread_start();

        t = std::thread(
            [](int id, F&& f, Args&&... args) {
                global_sequence->thread_begin(id);

                f(args...);

                global_sequence->thread_end();
            },
            new_id, std::forward<F>(f), std::forward<Args>(args)...
        );

        global_sequence->thread_start();
    }

    void join(std::source_location location = std::source_location::current()) {
        global_sequence->log("thread::join", location);
        global_sequence->join(new_id);
        t.join();
    }

    std::thread t;
    std::size_t new_id;
};
}
