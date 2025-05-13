#ifndef __SOURCE_HH
#define __SOURCE_HH

#include <optional>
#include <string>

using SourceFileID = int;

constexpr SourceFileID DEFAULT_SOURCE_FILE = 0;
constexpr SourceFileID NULL_SOURCE_FILE = -1;
constexpr SourceFileID BUILTIN_SOURCE_FILE = -2;
constexpr SourceFileID EXTERNAL_SOURCE_FILE = -3;

struct SourceLocation {
    static constexpr int OUT_OF_SOURCE = -1;

    int line;
    int column;

    SourceFileID source_id = DEFAULT_SOURCE_FILE;
    
    auto operator<=>(const SourceLocation&) const = default;

    std::string to_string() const {
        if (line < 0)
            return "-:-";
        return std::to_string(line) + ":" + std::to_string(column);
    };
};

constexpr SourceLocation NO_SOURCE_LOCATION{
    SourceLocation::OUT_OF_SOURCE, SourceLocation::OUT_OF_SOURCE, NULL_SOURCE_FILE};

constexpr SourceLocation TEST_SOURCE_LOCATION{0, 0, NULL_SOURCE_FILE};    
constexpr auto TSL = TEST_SOURCE_LOCATION;

constexpr SourceLocation BUILTIN_SOURCE_LOCATION{
    SourceLocation::OUT_OF_SOURCE, SourceLocation::OUT_OF_SOURCE, BUILTIN_SOURCE_FILE};

constexpr SourceLocation EXTERNAL_SOURCE_LOCATION{
    SourceLocation::OUT_OF_SOURCE, SourceLocation::OUT_OF_SOURCE, EXTERNAL_SOURCE_FILE};

#endif