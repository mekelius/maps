#ifndef __BUILTIN_HH
#define __BUILTIN_HH

#include <string>

namespace Maps {

class Type;

struct Builtin {
    std::string name;
    const Type* type;
};

} // namespace Maps

#endif