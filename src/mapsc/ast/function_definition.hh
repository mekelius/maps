#ifndef __FUNCTION_DEFINITION_HH
#define __FUNCTION_DEFINITION_HH

#include <optional>
#include <string_view>
#include <string>
#include <variant>

#include "common/maps_datatypes.h"

#include "mapsc/log_format.hh"
#include "mapsc/source_location.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/ast/definition_body.hh"

namespace Maps {

class Type;
class TypeStore;
struct Expression;
struct Statement;
class AST_Store;
class CompilationState;

// class Parameter: public DefinitionHeader {
// public:
//     Parameter(const std::string& name, const Type* type, const SourceLocation& location)
//     :DefinitionHeader(name, type, location) {}

//     Parameter(const Type* type, const SourceLocation& location)
//     :DefinitionHeader("_", type, location), is_discarded_(true) {}

//     virtual std::string node_type_string() const { return "parameter"; };
//     virtual std::string name_string() const { return name_; };
//     virtual const Type* get_type() const {return type_; };

//     bool is_discarded_ = false;
// };

using Parameter = DefinitionHeader;

using ParameterList = std::vector<Parameter*>;

std::pair<DefinitionHeader*, DefinitionBody*> function_definition(CompilationState& state, 
    const ParameterList& parameter_list, Scope* inner_scope, LetDefinitionValue value, 
    bool is_top_level, const SourceLocation& location);

std::pair<DefinitionHeader*, DefinitionBody*> function_definition(CompilationState& state, 
    const ParameterList& parameter_list, Scope* inner_scope, bool is_top_level, 
    const SourceLocation& location);

std::pair<DefinitionHeader*, DefinitionBody*> function_definition(CompilationState& state, const std::string& name, 
    Expression* value, const SourceLocation& location);

std::pair<DefinitionHeader*, DefinitionBody*> create_nullary_function_definition(AST_Store& ast_store, 
    TypeStore& types, Expression* value, bool is_pure, const SourceLocation& location);


Parameter* create_parameter(AST_Store& ast_store, const std::string& name, const Type* type, 
    const SourceLocation& location);
Parameter* create_parameter(AST_Store& ast_store, const std::string& name, 
    const SourceLocation& location);
Parameter* create_discarded_parameter(AST_Store& ast_store, const Type* type, const SourceLocation& location);
Parameter* create_discarded_parameter(AST_Store& ast_store, const SourceLocation& location);

} // namespace Maps

#endif