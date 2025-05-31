#ifndef __EXTERNAL_DEFINITION_HH
#define __EXTERNAL_DEFINITION_HH

#include <optional>
#include <string_view>
#include <variant>

#include "common/std_visit_helper.hh"
#include "common/maps_datatypes.h"

#include "mapsc/log_format.hh"
#include "mapsc/source.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/ast/definition.hh"

namespace Maps {

class Type;
struct Expression;
struct Statement;
class AST_Store;
class CompilationState;

class External: public DefinitionHeader {
public:
    External(const std::string& name, const Type* type, const SourceLocation& location)
    :DefinitionHeader(name, type, location) {}

    virtual std::string node_type_string() const { return "external"; };
};

External* create_external(AST_Store& ast_store, const std::string& name, const Type* type, 
    const SourceLocation& location);

} // namespace Maps

#endif