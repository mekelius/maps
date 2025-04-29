#include <cassert>

#include "../logging.hh"
#include "pragma.hh"

namespace Maps {

Pragmas::Pragmas() {
    for (const PragmaFlag& flag: flags) {
        // gotta insert the default at the start, because the begin is never read
        declarations_.insert({flag.name, {{SourceLocation{0,0}, flag.default_value}}});
    }
}

bool Pragmas::set_flag(const std::string& flag_name, bool value, const SourceLocation& location) {
    auto flag_declarations_it = declarations_.find(flag_name);
    
    if (flag_declarations_it == declarations_.end()) {
        Logging::log_error("tried to set unkown pragma: " + std::string{flag_name}, location);
        return false;
    }

    flag_declarations_it->second.insert({location, value});
    Logging::log_info(
        "set pragma: " + std::string(value ? "enable " : "disable ") + std::string{flag_name}, 
        Logging::MessageType::pragma_debug, location);
    return true;
}

// NOTE: if these were ever somehow to become a bottleneck they could be cached
bool Pragmas::check_flag_value(const std::string& flag_name, const SourceLocation& location) const {
    auto flag_declarations_it = declarations_.find(flag_name);
    assert(flag_declarations_it != declarations_.end() && "tried to read an unknown pragma");
    
    auto flag_declarations = flag_declarations_it->second;

    // get the last value that is above the location in the source file  
    // never hitting the begin() is ok, since that just contains the default value (set in the constructor)
    auto it = flag_declarations.lower_bound(location);
    assert(it != flag_declarations.end() && "somehow check_flag_value failed to produce a value");
    return it->second;
}

} // namespace Pragmas