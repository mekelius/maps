#ifndef __COMPILATION_STATE_HH
#define __COMPILATION_STATE_HH

#include <cassert>
#include <concepts>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "mapsc/pragma.hh"
#include "mapsc/builtins.hh"

#include "mapsc/types/type_store.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/ast/statement.hh" 

#include "mapsc/procedures/reverse_parse.hh"


namespace Maps {

// interface for visitors
// return false from a visit method to short circuit
template<class T>
concept AST_Visitor = requires(T t) {
    {t.visit_expression(std::declval<Expression*>())} -> std::convertible_to<bool>;
    {t.visit_statement(std::declval<Statement*>())} -> std::convertible_to<bool>;
    {t.visit_definition(std::declval<RT_Definition*>())} -> std::convertible_to<bool>;
};

class CompilationState {
public:
    struct Options {
        ReverseParser::Options reverse_parse{};
    };

    struct SpecialDefinitions {
        CT_Operator* unary_minus;
        CT_Operator* binary_minus;
    };

    template<AST_Visitor T>
    bool walk_tree(T& visitor);

    template<AST_Visitor T>
    bool walk_expression(T visitor, Expression* expression);
    template<AST_Visitor T>
    bool walk_statement(T visitor, Statement* statement);
    template<AST_Visitor T>
    bool walk_definition(T visitor, RT_Definition* definition);

    static std::tuple<CompilationState, std::unique_ptr<const CT_Scope>, std::unique_ptr<TypeStore>> 
        create_test_state();

    CompilationState(const CT_Scope* builtins, TypeStore* types, 
        SpecialDefinitions specials = {&unary_minus_Int, &binary_minus_Int});

    CompilationState(const CT_Scope* builtins, TypeStore* types, 
        Options compiler_options,
        SpecialDefinitions specials = {&unary_minus_Int, &binary_minus_Int});

    // copy constructor
    CompilationState(const CompilationState&) = default;
    // copy assignment operator
    CompilationState& operator=(const CompilationState&) = default;
    ~CompilationState() = default;

    bool empty() const;

    [[nodiscard]] bool set_entry_point(RT_Definition* entrypoint);
    [[nodiscard]] bool set_entry_point(std::string name);

    void declare_invalid() { is_valid = false; };
    void dump(std::ostream&) const;

    bool is_valid = true;
    
    Options compiler_options_{};
    RT_Scope globals_ = {};
    std::shared_ptr<AST_Store> ast_store_ = std::make_shared<AST_Store>();
    PragmaStore pragmas_ = {};
    
    // container for top-level statements
    std::optional<RT_Definition*> entry_point_ = std::nullopt;
    
    TypeStore* types_;
    const CT_Scope* builtins_;
    SpecialDefinitions special_definitions_ = SpecialDefinitions{&unary_minus_Int, &binary_minus_Int};

    // layer1 fills these with pointers to expressions that need work so that layer 2 doesn't
    // need to walk the tree to find them
    std::vector<Expression*> unresolved_identifiers_ = {};
    std::vector<Expression*> unresolved_type_identifiers_ = {};
    // these have to be dealt with before name resolution
    std::vector<Expression*> possible_binding_type_declarations_ = {};
    std::vector<Expression*> unparsed_termed_expressions_ = {};
};

template<AST_Visitor T>
bool CompilationState::walk_expression(T visitor, Expression* expression) {
    if (!visitor.visit_expression(expression))
        return false;

    switch (expression->expression_type) {
        case ExpressionType::call:
            // can't visit the call target cause would get into infinite loops
            // !!! TODO: visit lambdas somehow
            for (Expression* arg: std::get<1>(expression->call_value())) {
                if (!walk_expression(visitor, arg))
                    return false;
            }
            return true;

        case ExpressionType::termed_expression:
            for (Expression* sub_expression: expression->terms()) {
                if (!walk_expression(visitor, sub_expression))
                    return false;
            }
            return true;

        case ExpressionType::type_construct:
        case ExpressionType::type_argument:
            assert(false && "not implemented");
            return false;
        
        default:
            return true;
    }
}

template<AST_Visitor T>
bool CompilationState::walk_statement(T visitor, Statement* statement) {
    if (!visitor.visit_statement(statement))
        return false;

    switch (statement->statement_type) {
        case StatementType::assignment:
        case StatementType::expression_statement:
        case StatementType::return_:
            return walk_expression(visitor, get<Expression*>(statement->value));
        
        case StatementType::block:
            for (Statement* sub_statement: get<Block>(statement->value)) {
                if (!walk_statement(visitor, sub_statement))
                    return false;
            }
            return true;

        case StatementType::let:
            assert(false && "not implemented");
            return false;

        case StatementType::operator_definition:
        default:
            return true;
    }
}

template<AST_Visitor T>
bool CompilationState::walk_definition(T visitor, RT_Definition* definition) {
    if (!visitor.visit_definition(definition))
        return false;

    if (Expression* const* expression = get_if<Expression*>(&definition->body())) {
        return walk_expression(visitor, *expression);
    } else if (Statement* const* statement = get_if<Statement*>(&definition->body())) {
        return walk_statement(visitor, *statement);
    }

    return true;
}

template<AST_Visitor T>
bool CompilationState::walk_tree(T& visitor) {
    for (auto [_1, definition]: globals_.identifiers_in_order_) {
        if (!walk_definition(visitor, definition))
            return false;
    }

    if (entry_point_ && !walk_definition(visitor, *entry_point_))
        return false;

    return true;
}

} // namespace Maps

#endif