#ifndef __STRING_HELPERS_HH
#define __STRING_HELPERS_HH

#include <string>
#include <cctype>

namespace Maps {
namespace Helpers {
    
inline std::string&& capitalize(std::string&& string) {
    string.at(0) = std::toupper(string.at(0));
    return std::move(string);
}
    
} // namespace Helpers
} // namespace Maps

#endif