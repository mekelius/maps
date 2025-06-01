#include "pragma.hh"

#include <cassert>
#include <utility>

#include "mapsc/logging.hh"



namespace Maps {

using Log = LogInContext<LogContext::compiler_init>;

PragmaStore::PragmaStore() {
    for (const PragmaFlag& flag: flags) {
        // gotta insert the default at the start, because the begin is never read
        declarations_.insert({flag.name, {{SourceLocation{0,0}, flag.default_value}}});
    }
}

bool PragmaStore::set_flag(const std::string& flag_name, bool value, const SourceLocation& location) {
    auto flag_declarations_it = declarations_.find(flag_name);
    
    if (flag_declarations_it == declarations_.end()) {
        Log::error(location) << "tried to set unkown pragma: " << flag_name;
        return false;
    }

    flag_declarations_it->second.insert({location, value});
    Log::debug(location) << "set pragma: " << (value ? "enable " : "disable ") << flag_name;
    return true;
}

// NOTE: if these were ever somehow to become a bottleneck they could be cached
bool PragmaStore::check_flag_value(const std::string& flag_name, const SourceLocation& location) const {
    auto flag_declarations_it = declarations_.find(flag_name);
    assert(flag_declarations_it != declarations_.end() && "tried to read an unknown pragma");
    
    auto flag_declarations = flag_declarations_it->second;

    // get the last value that is above the location in the source file  
    // never hitting the begin() is ok, since that just contains the default value (set in the constructor)
    auto it = flag_declarations.lower_bound(location);
    assert(it != flag_declarations.end() && "somehow check_flag_value failed to produce a value");
    return it->second;
}

bool PragmaStore::empty() const { 
    // default flags are always there
    return declarations_.size() == flags.size(); 
}

size_t PragmaStore::size() const { 
    // default flags are always there
    return declarations_.size() - flags.size(); 
}

} // namespace Pragmas