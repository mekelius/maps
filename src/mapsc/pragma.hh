#ifndef __PRAGMA_HH
#define __PRAGMA_HH

#include <cstddef>
#include <array>
#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>

#include "mapsc/source.hh"

namespace Maps {

// all pragmas are bool for now
struct PragmaFlag {
    std::string_view name;
    bool default_value;
};

namespace Flags {

constexpr PragmaFlag top_level_evaluation_context{"top-level evaluation", false};
constexpr PragmaFlag mutable_global_variables{"mutable global variables", false};

}

constexpr std::array flags{
    Flags::top_level_evaluation_context,
    Flags::mutable_global_variables
};

// the idea is that we have to be able to determine retroactively whether something 
// was affected by a pragma. We can do this by storing the declarations by SourceLocation
// when we look for pragmas affecting a certain AST::Node, we search the multimap and look for the
// previous relevant pragma
// If these were to become a bottleneck we should cache them 
class PragmaStore {
  public:
    PragmaStore();
    
    // returns false if the flag doesn't exist
    bool set_flag(const std::string& flag_name, bool value, const SourceLocation& location);
    bool check_flag_value(const std::string& flag_name, const SourceLocation& location) const;

    bool empty() const;
    size_t size() const;
  private:
    std::unordered_map<
        std::string_view,
        // The locations are stored in reverse order so that we can easily find the
        // current value of the flag using std::map::lower_bound
        std::map<SourceLocation, bool, std::greater<SourceLocation>>
    > declarations_;
};

} // namespace Pragma

#endif