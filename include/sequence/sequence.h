#pragma once
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <assert.h>

// only allow one thread to run at a time
// the currently running thread runs until it hits the next sync point
// then the next thread on the current path is woken up
// repeat
namespace sequence {
extern struct sequence {
    static thread_local unsigned char id;

    std::mutex lock;
    std::condition_variable condition;
    std::vector<unsigned char> path, count, waiting, running, free, position;
    std::string log;
    unsigned depth = 0;
    unsigned thread_count = 0;

    sequence() {
        id = 0;
        thread_count = 1;
        running = {0};
        position = {0};
    }

    void add_depth() {
        depth++;
        if (depth <= count.size()) {
            assert(running.size() == count[depth - 1]);
        } else {
            count.push_back(running.size());
            path.push_back(0);
        }
        condition.notify_all();
    }

    unsigned before_thread_start() {
        // run before starting a new thread
        // returns the id of the new thread?
        std::unique_lock l(lock);
        unsigned new_id;
        if (free.empty()) {
            new_id = thread_count++;
            position.push_back(running.size());
        } else {
            new_id = free.back();
            free.pop_back();
            position[new_id] = running.size();
        }
        running.push_back(new_id);

        add_depth();

        return new_id;
    }
    void thread_begin(unsigned new_id) {
        // run in a thread when it starts
        // may run simultaneously with thread_started
        id = new_id;
        // synchronize calls can only finish in the order
        // ids are added to path
        // only the current thread can advance path
        std::unique_lock l(lock);
        id = new_id;
        condition.wait(l, [&](){
            return position[id] == path[depth - 1];
        });
    }
    void thread_start() {
        // run after a thread started a new thread
        // may run simultaneously with thread_begin
        std::unique_lock l(lock);
        condition.wait(l, [&](){
            return position[id] == path[depth - 1];
        });
    }
    void thread_end() {
        std::unique_lock l(lock);
        running[position[id]] = running.back();
        position[running.back()] = position[id];
        running.pop_back();
        position[id] = ~0;
        free.push_back(id);

        add_depth();
    }
    void synchronize() {
        // run after a globally visible operation like access to shared variables
        std::unique_lock l(lock);
        add_depth();
        condition.wait(l, [&](){
            return position[id] == path[depth - 1];
        });
    }
    void join() {
        // schedule other threads until there is only this one left
        std::unique_lock l(lock);
        running[position[id]] = running.back();
        position[running.back()] = position[id];
        running.pop_back();
        position[id] = ~0;
        waiting.push_back(id);

        add_depth();
        condition.wait(l, [&](){
            return running.empty();
        });

        for (auto t : waiting) {
            position[t] = running.size();
            running.push_back(t);
        }
        waiting.clear();
    }
    void wait() {
        // wait until next notify call, used to implement mutex
        // thread mustn't block any other way as otherwise
        // unblocking would be unpredictable
        std::unique_lock l(lock);
        running[position[id]] = running.back();
        position[running.back()] = position[id];
        running.pop_back();
        position[id] = ~0;
        waiting.push_back(id);

        add_depth();
        condition.wait(l, [&](){
            return position[id] == path[depth - 1];
        });
    }
    void notify() {
        // run to unblock waiting threads
        std::unique_lock l(lock);

        for (auto t : waiting) {
            position[t] = running.size();
            running.push_back(t);
        }
        waiting.clear();

        add_depth();
        condition.wait(l, [&](){
            return position[id] == path[depth - 1];
        });
    }

    bool next_sequence() {
        log.clear();
        assert(id == 0);
        depth = 0;
        while (!path.empty() && ++path.back() >= count.back()) {
            path.pop_back();
            count.pop_back();
        }
        return !path.empty();
    }

} *global_sequence;
}
