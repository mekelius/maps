#ifndef __CHUNK_HH
#define __CHUNK_HH

#include <variant>

namespace Maps {
    
class Pragma;
class DefinitionHeader;
struct Expression;
struct Statement;

using Chunk = std::variant<std::monostate, DefinitionHeader*, Expression*, Statement*, Pragma*>;

} // namespace Maps

#endif