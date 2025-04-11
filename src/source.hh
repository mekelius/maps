#ifndef __SOURCE_HH
#define __SOURCE_HH

#include <string>

struct SourceLocation {
    unsigned int line;
    unsigned int column;
    
    auto operator<=>(const SourceLocation&) const = default;

    std::string to_string() const {
        return std::to_string(line) + ":" + std::to_string(column);
    };
};

// NOTE: Unused, just put it down because I think this is how it should be done 
// when we get to it
struct SourceReference {
    std::string filename;
    SourceLocation location;
};
#endif