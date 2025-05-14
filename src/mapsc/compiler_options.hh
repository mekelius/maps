#ifndef __COMPILER_OPTIONS_HH
#define __COMPILER_OPTIONS_HH

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

namespace Maps {

// Hacky way to count the number of options

// clang-format off
constexpr auto COMPILER_OPTIONS_START_LINE = __LINE__;
enum class CompilerOption {
    print_all_types     = 0,
    DEBUG_no_inline     = 1,
    dummyvalue_1        = 2,
    dummyvalue_2        = 3,
    dummyvalue_3        = 4,
};
constexpr auto COMPILER_OPTION_COUNT = __LINE__ - COMPILER_OPTIONS_START_LINE - 3;
// clang-format on


// This class is ment to be used as a sort of semi-global object
// Taking an instance of this class prevents the same thread from taking more
// instances, and will change the configuration for that thread specifically
// When the instance destructor runs, the options return to their defaults
// We'll see if this kind of hack works in the long run
// The thread safety is mainly for multithreaded testing, if we want to do that at some point
class CompilerOptions {
public:
    using Entries = std::map<CompilerOption, std::string>;
    using Entry = std::pair<std::string_view, std::string_view>;

    // Obtain a CompilerOptions object that serves as a lock for options in this thread
    // Allows the owner to set options, but if it destructs the options will revert
    // Other threads will not be affected in any way
    [[nodiscard]] static std::optional<std::unique_ptr<CompilerOptions>> 
        lock(const Entries& entries = {}, bool for_all_threads = true);
    
    [[nodiscard]] static std::optional<std::unique_ptr<CompilerOptions>> 
        lock_for_this_thread(const Entries& entries = {});
    
    static std::string_view get(CompilerOption key);
    static Entry get_key_val_pair(CompilerOption key);

    void set(CompilerOption key, const std::string& value);
    
    ~CompilerOptions();

private:
    CompilerOptions(const Entries& entries, bool for_all_threads = false);
    std::unique_ptr<Entries> entries_;
    
    static std::string_view get_key_string(CompilerOption key);
    static std::string_view get_default(CompilerOption key);

    static std::optional<Entries*> global_entries_;
    static std::map<std::jthread::id, Entries*> entries_per_thread;
};

} // namespace Maps

#endif