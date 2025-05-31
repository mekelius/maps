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
#include "mapsc/source.hh"
#include "mapsc/compilation_state.hh"

using std::optional, std::nullopt;


namespace Maps {

using Log = LogNoContext;

DefinitionHeader::DefinitionHeader(const std::string& name, const Type* type, Scope* outer_scope,
    bool is_top_level, SourceLocation location)
:name_(name), 
 type_(type), 
 is_top_level_(is_top_level),
 location_(location),
 outer_scope_(outer_scope) {}

DefinitionHeader::DefinitionHeader(const std::string& name, const Type* type, 
    SourceLocation location)
:name_(name), 
 type_(type), 
 is_top_level_(true),
 location_(location), 
 outer_scope_(nullopt) {} 

std::optional<LetDefinitionValue> DefinitionHeader::get_body_value() const {
    if (!body_)
        return nullopt;

    return (*body_)->value_;
}

// ------------------------------------- FACTORY FUNTIONS -----------------------------------------

// DefinitionHeader DefinitionHeader::testing_definition(const Type* type, bool is_top_level) {
//     return DefinitionHeader{"DUMMY_DEFINITION", {}, type, is_top_level, TSL};
// }

// DefinitionHeader* DefinitionHeader::function_definition(CompilationState& state, 
//     const ParameterList& parameter_list, Scope* inner_scope, DefinitionBody body, 
//     bool is_top_level, const SourceLocation& location) {

//     std::vector<const Type*> param_types{};

//     for (auto param: parameter_list)
//         param_types.push_back(param->get_type());

//     if (std::holds_alternative<Expression*>(body)) {
//         return dynamic_cast<DefinitionHeader*>(state.ast_store_->allocate_definition(DefinitionHeader{
//             MAPS_INTERNALS_PREFIX + "anonymous_function", body, is_top_level, location}));
//     }

//     auto type = state.types_->get_function_type(&Hole, param_types, false);

//     return dynamic_cast<DefinitionHeader*>(state.ast_store_->allocate_definition(DefinitionHeader{
//         MAPS_INTERNALS_PREFIX + "anonymous_function", body, type, is_top_level, location}));
// }

// DefinitionHeader* DefinitionHeader::function_definition(CompilationState& state, 
//     const ParameterList& parameter_list, Scope* inner_scope, bool is_top_level, 
//     const SourceLocation& location) {
    
//     return function_definition(state, parameter_list, inner_scope, {}, is_top_level, location);
// }

// --------------------------------------- CONSTRUCTORS -------------------------------------------

// DefinitionHeader::DefinitionHeader(std::string_view name, DefinitionBody* body, const Type* type, 
//     bool is_top_level, const SourceLocation& location)
// :name_(name), body_(body), location_(location), type_(type), is_top_level_(is_top_level) {
// }

// DefinitionHeader::DefinitionHeader(std::string_view name, DefinitionBody* body, bool is_top_level, 
//     const SourceLocation& location)
// :DefinitionHeader(name, body, &Hole, is_top_level, location) {}

// DefinitionHeader::DefinitionHeader(DefinitionBody body, bool is_top_level, const SourceLocation& location)
// :name_("anonymous definition"), body_(body), location_(location), is_top_level_(is_top_level) {}

// DefinitionHeader::DefinitionHeader(DefinitionBody body, const Type* type, bool is_top_level, 
//     const SourceLocation& location)
// :name_("anonymous definition"), body_(body), location_(location), type_(type), is_top_level_(is_top_level) {}


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

void DefinitionBody::set_type(const Type* type) {
    assert(false && "not updated");

//     if (!body)
    // std::visit(overloaded {
    //     [](Error) {},
    //     [this](External) { 
    //         Log::compiler_error("Attempting to set the type of an external", this->location()); 
    //     },

    //     [this, &type](Undefined) { 
    //         type_ = std::make_optional<const Type*>(type);
    //         Log::compiler_error("Attempting to set the type of an undefined", this->location()); 
    //     },

    //     [&type](Expression* expression) { expression->type = type; },

    //     [this, &type](Statement* statement) {
    //         if (statement->statement_type == StatementType::expression_statement) {
    //             std::get<Expression*>(statement->value)->type = type;
    //             return;
    //         } 

    //         type_ = std::make_optional<const Type*>(type);
    //     },

    //     [&type](BTD_Binding binding)-> void { binding.type = type; },
    // }, body_);
}

std::optional<const Type*> DefinitionBody::get_declared_type() const {
    if (Expression* const* expression = std::get_if<Expression*>(&value_)) {
        return (*expression)->declared_type;

    } else if (Statement* const* statement = std::get_if<Statement*>(&value_)) {
        if ((*statement)->statement_type == StatementType::expression_statement)
            return std::get<Expression*>((*statement)->value)->declared_type;

        return declared_type_;
    }

    assert(false && "unhandled definition type in Definition::get_declared_type");
    return std::nullopt;
}

bool DefinitionBody::set_declared_type(const Type* type) {
    if (Expression* const* expression = std::get_if<Expression*>(&value_)) {
        (*expression)->declared_type = type;
        return true;

    } else if (Statement* const* statement = std::get_if<Statement*>(&value_)) {
        if ((*statement)->statement_type == StatementType::expression_statement) {
            auto expression = std::get<Expression*>((*statement)->value);
            expression->declared_type = type;
            return true;
        }

        declared_type_ = type;
        return true;
    }

    return false;
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
    assert(false && "not updated");

    // return std::visit(overloaded {
    //     [](const Expression* expression) { 
    //         return (
    //             !(expression->type->is_function()) && 
    //                 is_constant_value(*expression));
    //     },
    //     [](auto) { return false; }
    // }, const_body());
}

} // namespace Maps