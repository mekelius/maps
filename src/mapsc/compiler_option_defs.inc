#include <array>

#include "compiler_options.hh"

namespace Maps {

// Options with DEBUG prefix are ignored on release builds
// TODO: do the same for testing
constexpr std::array<CompilerOptions::Entry, 4> default_entries {CompilerOptions::Entry 
    {"DEBUG_no_inline"  , "false"   }, // DEBUG_no_inline   = 0
    {"dummyvalue_1"     , "1"       }, // dummyvalue_1      = 1
    {"dummyvalue_2"     , "2"       }, // dummyvalue_2      = 2
    {"dummyvalue_3"     , "3"       }, // dummyvalue_3      = 3
};

static_assert(default_entries.size() == COMPILER_OPTION_COUNT);
    
} // namespace Maps