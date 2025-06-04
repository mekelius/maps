#include "definition.hh"

#include <variant>
#include <cassert>
#include <string>

#include "common/std_visit_helper.hh"

#include "mapsc/logging.hh"

#include "mapsc/types/type.hh"
#include "mapsc/types/type_defs.hh"

#include "mapsc/ast/statement.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/expression_properties.hh"
#include "mapsc/ast/ast_store.hh"
#include "mapsc/source_location.hh"
#include "mapsc/compilation_state.hh"

using std::optional, std::nullopt;


namespace Maps {

using Log = LogNoContext;

std::optional<LetDefinitionValue> DefinitionHeader::get_body_value() const {
    if (!body_)
        return nullopt;

    return (*body_)->get_value();
}


RT_DefinitionHeader::RT_DefinitionHeader(DefinitionType definition_type, const std::string& name, const Type* type, 
    Scope* outer_scope, bool is_top_level, SourceLocation location)
:DefinitionHeader(definition_type, {}, type, outer_scope, is_top_level, location), name_string_(name) {
    name_ = name_string_;
}

RT_DefinitionHeader::RT_DefinitionHeader(DefinitionType definition_type, const std::string& name, Scope* outer_scope,
    bool is_top_level, SourceLocation location)
:RT_DefinitionHeader(definition_type, name, &Hole, outer_scope, is_top_level, location) {}

RT_DefinitionHeader::RT_DefinitionHeader(DefinitionType definition_type, const std::string& name, const Type* type, 
    SourceLocation location)
:DefinitionHeader(definition_type, {}, type, location), name_string_(name) {
    name_ = name_string_;
}

RT_DefinitionHeader::RT_DefinitionHeader(DefinitionType definition_type, const std::string& name, SourceLocation location)
:RT_DefinitionHeader(definition_type, name, &Hole, location){}



// ------------------------------------- FACTORY FUNTIONS -----------------------------------------



// DefinitionHeader DefinitionHeader::testing_definition(const Type* type, bool is_top_level) {
//     return DefinitionHeader{"DUMMY_DEFINITION", {}, type, is_top_level, TSL};
// }

// --------------------------------------- CONSTRUCTORS -------------------------------------------

// ----------------------------------- SETTERS AND GETTERS ---------------------------------------

// TODO: change to std::visitor
// const Type* DefinitionHeader::get_type() const {
//     return std::visit(overloaded {
//         [](Error) -> const Type* { return &Absurd;},
//         [this](Undefined) -> const Type* { return type_ ? *type_ : &Hole; },
//         [this](External) -> const Type* { return *type_; },

//         [](Expression* expression) -> const Type* { return expression->type; },

//         [this](Statement* statement) -> const Type* {
//             if (statement->statement_type == StatementType::expression_statement)
//                 return std::get<Expression*>(statement->value)->type;

//             return type_ ? *type_ : &Hole;
//         },

//         [](BTD_Binding binding) { return binding.type; }
//     }, body_);
// }

bool DefinitionHeader::is_operator() const {
    switch (definition_type_) {
        case DefinitionType::operator_def:
            return true;
        default:
            return false;
    }
}

// bool Definition::is_empty() const {
//     return std::visit(overloaded {
//         [](Error) { return true; },
//         [](External) { return false; },
//         [](Undefined) { return true; },
//         [](const Expression*) { return false; },
//         [](const Statement* statement) { return statement->is_empty(); },
//         [](BTD_Binding) { return false; }
//     }, const_body());
// }

bool DefinitionHeader::is_known_scalar_value() const {
    if (!body_)
        return false;

    return std::visit(overloaded {
        [](Expression* expression) { 
            return (
                !(expression->type->is_function()) && 
                    is_constant_value(*expression));
        },
        [](auto) { return false; }
    }, (*body_)->get_value());
}

} // namespace Maps