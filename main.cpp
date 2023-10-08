#include "sequence/sequence.h"

void test() {
    sequence::global_sequence->synchronize(); // 1

    int new_id = sequence::global_sequence->before_thread_start(); // 2

    std::thread t(
        [](int id){
            sequence::global_sequence->thread_begin(id); // 2

            sequence::global_sequence->thread_end(); // 3
        }, new_id
    );
    sequence::global_sequence->thread_start(); // 2

    sequence::global_sequence->join(); // 3

    t.join();
}

int main() {
    sequence::global_sequence = new sequence::sequence;

    do {
        test();
    } while (sequence::global_sequence->next_sequence());
}
