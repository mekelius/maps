#include <array>

#include "compiler_options.hh"

namespace Maps {

constexpr std::array<CompilerOptions::Entry, 3> default_entries {
    CompilerOptions::Entry  {"dummyvalue_1", "1"}, // dummyvalue_1 = 0
                            {"dummyvalue_2", "2"}, // dummyvalue_2
                            {"dummyvalue_3", "3"}, // dummyvalue_3
};

static_assert(default_entries.size() == COMPILER_OPTION_COUNT);
    
} // namespace Maps