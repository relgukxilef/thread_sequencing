#include <thread>
#include <atomic>

#include "sequence/sequence.h"
#include "sequence/thread.h"
#include "sequence/atomic.h"

using thread = sequence::thread;
template<class T>
using atomic = sequence::atomic<T>;

struct check_exception : std::exception {
    const char *what() const override {
        return "Assertion failed.";
    }
};

void check(bool expression) {
    if (!expression) {
        throw check_exception();
    }
}

void test() {
    atomic<int> value(20);

    thread t(
        [&](){
            value.store(value.load() + 5);
        }
    );

    value.store(value.load() - 10);

    t.join();

    check(value.load() == 20 + 5 - 10);
}

int main() {
    sequence::global_sequence = new sequence::sequence;

    int count = 0;

    do {
        try {
            test();
        } catch (check_exception) {
            fprintf(
                stderr, "Check failed in sequence:\n%s\n",
                sequence::global_sequence->log.c_str()
            );
        }

        count++;
    } while (sequence::global_sequence->next_sequence());

    printf("Tested %i sequences", count);
}
