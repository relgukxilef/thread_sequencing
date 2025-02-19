#include "sequence/sequence.h"

namespace sequence {
sequence *global_sequence;
thread_local std::size_t sequence::id;
}
