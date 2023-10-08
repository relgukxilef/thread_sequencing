#include <thread>

#include "sequence/sequence.h"
#include "sequence/thread.h"

using thread = sequence::thread;

void test() {
    thread t(
        [](){}
    );

    t.join();
}

int main() {
    sequence::global_sequence = new sequence::sequence;

    do {
        test();
    } while (sequence::global_sequence->next_sequence());
}
