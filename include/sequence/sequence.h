#pragma once
#include <vector>
#include <span>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <assert.h>
#include <source_location>
#include <sstream>

// only allow one thread to run at a time
// the currently running thread runs until it hits the next sync point
// then the next thread on the current path is woken up
// repeat
namespace sequence {
extern struct sequence {
    static thread_local std::size_t id;

    std::mutex mutex;
    std::condition_variable condition;
    std::vector<std::size_t> subtree, count, running, free, positions;
    std::span<std::size_t> path;
    std::stringstream log_buffer;
    std::size_t current_thread = 0;

    sequence() {
        id = 0;
        running = {0};
        positions = {0};
    }

    void log(
        const char* message,
        std::source_location location = std::source_location::current()
    ) {
        log_buffer
            << message
            << " \t"
            << location.file_name()
            << ':'
            << location.line()
            << '\n';
    }

    void wait_turn(std::unique_lock<std::mutex>& lock) {
        while (id != current_thread)
            condition.wait(lock);
    }

    void select_next_thread() {
        std::size_t position = 0;
        if (!path.empty()) {
            position = path[0] % running.size();
            path = path.subspan(1);
        }
        current_thread = running[position];
        count.push_back(running.size());
        condition.notify_all();
    }

    std::size_t thread_prepare() {
        // run before starting a new thread
        // returns the id of the new thread
        std::unique_lock<std::mutex> lock(mutex);
        auto new_id = running.size();
        if (!free.empty()) {
            new_id = free.back();
            free.pop_back();
            positions[new_id] = running.size();
        } else {
            positions.push_back(running.size());
        }
        running.push_back(new_id);
        select_next_thread();
        return new_id;
    }
    void thread_child(std::size_t new_id) {
        // run in a thread when it starts
        // may run simultaneously with thread_started
        id = new_id;
        std::unique_lock<std::mutex> lock(mutex);
        wait_turn(lock);
    }
    void thread_parent() {
        // run after a thread started a new thread
        // may run simultaneously with thread_begin
        std::unique_lock<std::mutex> lock(mutex);
        wait_turn(lock);
    }
    void thread_end() {
        std::unique_lock<std::mutex> lock(mutex);
        auto position = positions[id];
        free.push_back(id);
        assert(running[position] == id);
        running[position] = running.back();
        positions[running.back()] = position;
        running.pop_back();
        select_next_thread();
    }
    void synchronize() {
        // run after a globally visible operation like access to shared variables
        std::unique_lock<std::mutex> lock(mutex);
        select_next_thread();
        wait_turn(lock);
    }
    void yield_to(std::size_t other_id) {
        // block this thread an continue the specified one
        // useful for blocked mutexes and joins
        std::unique_lock<std::mutex> lock(mutex);
        current_thread = other_id;
        condition.notify_all();
        wait_turn(lock);
    }

    bool next_sequence() {
        log_buffer.seekp(0);
        assert(id == 0);
        subtree.resize(count.size());
        while (!subtree.empty() && ++subtree.back() >= count.back()) {
            subtree.pop_back();
            count.pop_back();
        }
        count.clear();
        path = subtree;
        return !subtree.empty();
    }

} *global_sequence;
}
