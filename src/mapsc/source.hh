#ifndef __SOURCE_HH
#define __SOURCE_HH

#include <optional>
#include <string>
#include <string_view>

#include "mapsc/logging.hh"

namespace Maps {

using SourceFileID = int;

constexpr SourceFileID DEFAULT_SOURCE_FILE = 0;
constexpr SourceFileID NULL_SOURCE_FILE = -1;
constexpr SourceFileID BUILTIN_SOURCE_FILE = -2;
constexpr SourceFileID EXTERNAL_SOURCE_FILE = -3;
constexpr SourceFileID COMPILER_INIT_SOURCE_FILE = -4;

constexpr std::string MAPS_INTERNALS_PREFIX = "MAPS_";

struct SourceLocation {
    static constexpr int OUT_OF_SOURCE = -1;

    int line;
    int column;

    SourceFileID source_id = DEFAULT_SOURCE_FILE;
    
    auto operator<=>(const SourceLocation&) const = default;

    LogStream::InnerStream& log_self_to(LogStream::InnerStream& ostream) const;
    LogStream::InnerStream& log_self_to_with_padding(LogStream::InnerStream& ostream, 
        uint line_padding, uint col_padding) const;
};

#define TSL SourceLocation{__LINE__, 0, NULL_SOURCE_FILE}

constexpr SourceLocation NO_SOURCE_LOCATION{
    SourceLocation::OUT_OF_SOURCE, SourceLocation::OUT_OF_SOURCE, NULL_SOURCE_FILE};

constexpr SourceLocation COMPILER_INIT_SOURCE_LOCATION{
    SourceLocation::OUT_OF_SOURCE, SourceLocation::OUT_OF_SOURCE, COMPILER_INIT_SOURCE_FILE};
    
constexpr SourceLocation BUILTIN_SOURCE_LOCATION{
    SourceLocation::OUT_OF_SOURCE, SourceLocation::OUT_OF_SOURCE, BUILTIN_SOURCE_FILE};
        
constexpr SourceLocation EXTERNAL_SOURCE_LOCATION{
    SourceLocation::OUT_OF_SOURCE, SourceLocation::OUT_OF_SOURCE, EXTERNAL_SOURCE_FILE};

} // namespace Maps

#endif