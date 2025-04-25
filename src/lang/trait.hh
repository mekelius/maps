#ifndef __TRAIT_H
#define __TRAIT_H

#include <string>

#include "type.hh"
#include "ast_node.hh"

namespace Maps {

class Trait {
public:
    std::string name;

private:
    // for now it's just a list of concrete types
    // later we want to do something more advanced with this, basically pattern match


};

} // namespace Maps

#endif