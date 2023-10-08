#include <thread>
#include <atomic>

#include "sequence/sequence.h"
#include "sequence/thread.h"
#include "sequence/atomic.h"

using thread = sequence::thread;
template<class T>
using atomic = sequence::atomic<T>;

void test() {
    atomic<int> value(20);

    thread t(
        [&](){
            value.store(value.load() + 5);
        }
    );

    value.store(value.load() - 10);

    t.join();
}

int main() {
    sequence::global_sequence = new sequence::sequence;

    int count = 0;

    do {
        test();
        count++;
    } while (sequence::global_sequence->next_sequence());

    printf("Tested %i sequences", count);
}
