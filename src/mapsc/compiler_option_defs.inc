#include <array>

#include "compiler_options.hh"

namespace Maps {

// Options with DEBUG prefix are ignored on release builds
// TODO: do the same for testing
constexpr std::array<CompilerOptions::Entry, 5> default_entries {CompilerOptions::Entry 
    {"print_all_types"  , "false"   }, // print_all_types   = 0
    {"DEBUG_no_inline"  , "false"   }, // DEBUG_no_inline   = 1
    {"dummyvalue_1"     , "1"       }, // dummyvalue_1      = 2
    {"dummyvalue_2"     , "2"       }, // dummyvalue_2      = 3
    {"dummyvalue_3"     , "3"       }, // dummyvalue_3      = 4
};

static_assert(default_entries.size() == COMPILER_OPTION_COUNT);
    
} // namespace Maps