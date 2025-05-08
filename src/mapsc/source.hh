#ifndef __SOURCE_HH
#define __SOURCE_HH

#include <optional>
#include <string>

using SourceID = int;

constexpr SourceID DEFAULT_SOURCE_ID = 0;
constexpr SourceID NULL_SOURCE_ID = -1;
constexpr SourceID BUILTIN_SOURCE_ID = -2;

struct SourceLocation {
    static constexpr int OUT_OF_SOURCE = -1;

    int line;
    int column;

    SourceID source_id = DEFAULT_SOURCE_ID;
    
    auto operator<=>(const SourceLocation&) const = default;

    std::string to_string() const {
        if (line < 0)
            return "-:-";
        return std::to_string(line) + ":" + std::to_string(column);
    };
};

constexpr SourceLocation NO_SOURCE_LOCATION{
    SourceLocation::OUT_OF_SOURCE, SourceLocation::OUT_OF_SOURCE, NULL_SOURCE_ID};

constexpr SourceLocation TEST_SOURCE_LOCATION{0, 0, NULL_SOURCE_ID};    
constexpr auto TSL = TEST_SOURCE_LOCATION;

constexpr SourceLocation BUILTIN_SOURCE_LOCATION{
    SourceLocation::OUT_OF_SOURCE, SourceLocation::OUT_OF_SOURCE, BUILTIN_SOURCE_ID};

#endif