#ifndef __BUILTINS_HH
#define __BUILTINS_HH

#include <cassert>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "common/maps_datatypes.h"

#include "mapsc/source_location.hh"

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/builtin.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/operator.hh"
#include "mapsc/ast/scope.hh"

namespace Maps {

const Scope* get_builtins();

std::optional<DefinitionHeader*> find_external_runtime_cast(const Scope& scope, const Type* source_type, 
    const Type* target_type);

extern Operator unary_minus_Int;
extern Operator plus_Int;
extern Operator binary_minus_Int;
extern Operator mult_Int;
extern BuiltinExternal print_String;
extern BuiltinExternal print_MutString;
extern BuiltinExternal concat;

} // namespace Maps

#endif