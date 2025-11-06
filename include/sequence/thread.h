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
        new_id = global_sequence->thread_prepare();

        t = std::thread(
            [](int id, bool* finished, F&& f, Args&&... args) {
                global_sequence->thread_child(id);

                std::move(f)(args...);

                *finished = true;
                global_sequence->thread_end();
            },
            new_id, &finished, std::forward<F>(f), std::forward<Args>(args)...
        );

        global_sequence->thread_parent();
    }

    void join(std::source_location location = std::source_location::current()) {
        global_sequence->log("thread::join", location);
        while (!finished) {
            global_sequence->yield_to(new_id);
        }
        t.join();
    }

    std::thread t;
    std::size_t new_id;
    bool finished = false;
};
}
