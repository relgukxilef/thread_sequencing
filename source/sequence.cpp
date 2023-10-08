#include "sequence/sequence.h"

namespace sequence {
sequence *global_sequence;
thread_local unsigned char sequence::id;
}
