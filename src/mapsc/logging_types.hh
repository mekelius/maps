#ifndef __LOGGING_TYPES_HH
#define __LOGGING_TYPES_HH

#include <array>
#include <concepts>
#include <string_view>
#include <ranges>

#include "mapsc/source.hh"

namespace Maps {

template<class T>
concept HasLocation = 
    requires (T t) {{t.location()}      -> std::convertible_to<SourceLocation>; } ||
    requires (T t) {{t.get_location()}  -> std::convertible_to<SourceLocation>; } ||
    requires (T t) {{t.location}        -> std::convertible_to<SourceLocation>; }
;

template<class T>
concept HasLogRepresentation = requires(T t) {
    {t.log_message_string()} -> std::convertible_to<std::string_view>;
};

template<class T>
concept LogMessageSubject = HasLogRepresentation<T> && HasLocation<T>;

// template<class T>
// log()

} // namespace Maps

#endif