#ifndef __PRAGMA_HH
#define __PRAGMA_HH

#include <string_view>
#include <map>
#include <unordered_map>
#include <array>

#include "../source.hh"

namespace Pragma {

namespace Flags {

// all pragmas are bool now
struct Flag {
    std::string_view name;
    bool default_value;
};

constexpr Flag top_level_evaluation_context{"top-level evaluation", false};
constexpr Flag mutable_global_variables{"mutable global variables", false};

}

constexpr std::array<Flags::Flag, 2> flags{
    Flags::top_level_evaluation_context,
    Flags::mutable_global_variables
};

// the idea is that we have to be able to determine retroactively whether something 
// was affected by a pragma. We can do this by storing the declarations by SourceLocation
// when we look for pragmas affecting a certain AST::Node, we search the multimap and look for the
// previous relevant pragma
// If these were to become a bottleneck we should cache them 
class Pragmas {
  public:
    Pragmas();
    
    // returns false if the flag doesn't exist
    bool set_flag(const std::string& flag_name, bool value, const SourceLocation& location);
    bool check_flag_value(const std::string& flag_name, const SourceLocation& location) const;

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