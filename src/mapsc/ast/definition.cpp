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
#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"

using std::optional, std::nullopt;


namespace Maps {

using Log = LogNoContext;

// ------------------------------------- FACTORY FUNTIONS -----------------------------------------

RT_Definition RT_Definition::testing_definition(const Type* type, bool is_top_level) {
    return RT_Definition{"DUMMY_DEFINITION", External{}, type, is_top_level, TSL};
}


RT_Definition* RT_Definition::external(AST_Store& store, std::string_view name, const Type* type, 
    const SourceLocation& location) {
    
    return store.allocate_definition(RT_Definition{name, External{}, type, true, location});
}

RT_Definition* RT_Definition::parameter(AST_Store& store, std::string_view name, const Type* type, 
    const SourceLocation& location) {

    return store.allocate_definition(RT_Definition{
        name, BTD_Binding{BTD_Binding::Type::parameter, type}, type, false, location});
}

RT_Definition* RT_Definition::discarded_parameter(AST_Store& store, const Type* type, 
    const SourceLocation& location) {

    return store.allocate_definition(RT_Definition{
        "_", BTD_Binding{BTD_Binding::Type::discarded_parameter, type}, type, false, location});
}

RT_Definition* RT_Definition::function_definition(CompilationState& state, 
    const ParameterList& parameter_list, RT_Scope* inner_scope, DefinitionBody body, 
    bool is_top_level, const SourceLocation& location) {

    std::vector<const Type*> param_types{};

    for (auto param: parameter_list)
        param_types.push_back(param->get_type());

    auto type = state.types_->get_function_type(&Hole, param_types, false);

    return state.ast_store_->allocate_definition(RT_Definition{
        MAPS_INTERNALS_PREFIX + "anonymous_function", Undefined{}, type, is_top_level, location});
}

RT_Definition* RT_Definition::function_definition(CompilationState& state, 
    const ParameterList& parameter_list, RT_Scope* inner_scope, bool is_top_level, 
    const SourceLocation& location) {
    
    return function_definition(state, parameter_list, inner_scope, Undefined{}, is_top_level, location);
}

// --------------------------------------- CONSTRUCTORS -------------------------------------------

RT_Definition::RT_Definition(std::string_view name, DefinitionBody body, const Type* type, 
    bool is_top_level, const SourceLocation& location)
:name_(name), body_(body), location_(location), type_(type), is_top_level_(is_top_level) {
    assert((!std::holds_alternative<Expression*>(body)  || 
            *type == Hole                                ||
            *type == *std::get<Expression*>(body)->type) &&
            "Tried to initialize expression-bodied definition with a type, \
type should be set on the expression");
}

RT_Definition::RT_Definition(std::string_view name, DefinitionBody body, bool is_top_level, 
    const SourceLocation& location)
:RT_Definition(name, body, &Hole, is_top_level, location) {}

RT_Definition::RT_Definition(DefinitionBody body, bool is_top_level, const SourceLocation& location)
:name_("anonymous definition"), body_(body), location_(location), is_top_level_(is_top_level) {}

RT_Definition::RT_Definition(DefinitionBody body, const Type* type, bool is_top_level, 
    const SourceLocation& location)
:name_("anonymous definition"), body_(body), location_(location), type_(type), is_top_level_(is_top_level) {}


// ----------------------------------- SETTERS AND GETTERS ---------------------------------------

const_DefinitionBody RT_Definition::const_body() const {
    return std::visit([](auto body) { return const_DefinitionBody{body}; }, body_);
}

// TODO: change to std::visitor
const Type* RT_Definition::get_type() const {
    return std::visit(overloaded {
        [](Error) -> const Type* { return &Absurd;},
        [this](Undefined) -> const Type* { return type_ ? *type_ : &Hole; },
        [this](External) -> const Type* { return *type_; },

        [](Expression* expression) -> const Type* { return expression->type; },

        [this](Statement* statement) -> const Type* {
            if (statement->statement_type == StatementType::expression_statement)
                return std::get<Expression*>(statement->value)->type;

            return type_ ? *type_ : &Hole;
        },

        [](BTD_Binding binding) { return binding.type; }
    }, body_);
}

void RT_Definition::set_type(const Type* type) {
    std::visit(overloaded {
        [](Error) {},
        [this](External) { 
            Log::compiler_error("Attempting to set the type of an external", this->location()); 
        },

        [this, &type](Undefined) { 
            type_ = std::make_optional<const Type*>(type);
            Log::compiler_error("Attempting to set the type of an undefined", this->location()); 
        },

        [&type](Expression* expression) { expression->type = type; },

        [this, &type](Statement* statement) {
            if (statement->statement_type == StatementType::expression_statement) {
                std::get<Expression*>(statement->value)->type = type;
                return;
            } 

            type_ = std::make_optional<const Type*>(type);
        },

        [&type](BTD_Binding binding)-> void { binding.type = type; },
    }, body_);
}

std::optional<const Type*> RT_Definition::get_declared_type() const {
    if (Expression* const* expression = std::get_if<Expression*>(&body_)) {
        return (*expression)->declared_type;

    } else if (Statement* const* statement = std::get_if<Statement*>(&body_)) {
        if ((*statement)->statement_type == StatementType::expression_statement)
            return std::get<Expression*>((*statement)->value)->declared_type;

        return declared_type_;

    } else if (std::holds_alternative<External>(body_)) {
        return type_;
    }

    assert(false && "unhandled definition type in Definition::get_declared_type");
    return std::nullopt;
}

bool RT_Definition::set_declared_type(const Type* type) {
    if (Expression* const* expression = std::get_if<Expression*>(&body_)) {
        (*expression)->declared_type = type;
        return true;

    } else if (Statement* const* statement = std::get_if<Statement*>(&body_)) {
        if ((*statement)->statement_type == StatementType::expression_statement) {
            auto expression = std::get<Expression*>((*statement)->value);
            expression->declared_type = type;
            return true;
        }

        declared_type_ = type;
        return true;

    } else if (std::holds_alternative<External>(body_)) {
        LogNoContext::compiler_error("tried to set the declared type on an external", location_);
        assert(false && "tried to set the declared type on a builtin");
        return false;
    }

    return false;
}

bool Definition::is_empty() const {
    return std::visit(overloaded {
        [](Error) { return true; },
        [](External) { return false; },
        [](Undefined) { return true; },
        [](const Expression*) { return false; },
        [](const Statement* statement) { return statement->is_empty(); },
        [](BTD_Binding) { return false; }
    }, const_body());
}

} // namespace Maps