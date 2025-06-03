#include "source_location.hh"

#include <iomanip>
#include <optional>
#include <string>
#include <string_view>

#include "mapsc/logging.hh"

namespace Maps {

LogStream::InnerStream& SourceLocation::log_self_to(LogStream::InnerStream& ostream) const {
    if (line < 0)
        return ostream << "-:-";
    
    return ostream << line << ':' << column;
}

LogStream::InnerStream& SourceLocation::log_self_to_with_padding(
    LogStream::InnerStream& ostream, uint line_padding, uint col_padding) const {

    if (line < 0)
        return ostream << std::right << std::setw(line_padding) << '-' << ':' << std::left << 
            std::setw(col_padding) << '-';
    
    return ostream << std::right << std::setw(line_padding) << line << ':' << std::left << 
        std::setw(col_padding) << column;
}


} // namespace Maps
