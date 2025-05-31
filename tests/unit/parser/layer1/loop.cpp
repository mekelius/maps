#include "doctest.h"

#include <tuple>
#include <sstream>

#include "mapsc/ast/statement.hh"
#include "mapsc/parser/layer1.hh"
#include "mapsc/logging_options.hh"
#include "mapsc/compilation_state.hh"

using namespace std;
using namespace Maps;

inline std::tuple<CompilationState, Scope, stringstream> setup(const std::string& source) {
    auto [state, _1, _2] = CompilationState::create_test_state();

    return {
        std::move(state), 
        Scope{}, 
        stringstream{source}
    };
}

// auto lock = LogOptions::set_global(LogLevel::debug_extra);

#define WHILE_CASE(source_string)\
TEST_CASE(source_string) {\
    LogNoContext::debug_extra("TEST_CASE:\n" + std::string{source_string}, TSL);\
    \
    auto [state, scope, source] = setup(source_string);\
    \
    auto result = run_layer1_eval(state, scope, source);\
    \
    CHECK(result.success);\
    CHECK(result.top_level_definition);\
    CHECK(result.unresolved_identifiers.size() == 2);\
    \
    auto root = *result.top_level_definition;\
    CHECK(std::holds_alternative<const Statement*>(root->const_body()));\
    auto root_body = std::get<const Statement*>(root->const_body());\
    \
    CHECK(root_body->statement_type == StatementType::loop);\
    auto loop_value = root_body->get_value<LoopStatementValue>();\
    \
    CHECK(!loop_value.initializer);\
    \
    CHECK(loop_value.condition->expression_type == ExpressionType::identifier);\
    CHECK(loop_value.condition->string_value() == "condition");\
    \
    auto loop_body = loop_value.body;\
    CHECK(loop_body->statement_type == StatementType::expression_statement);\
    auto loop_body_expression = loop_body->get_value<Expression*>();\
    CHECK(loop_body_expression->expression_type == ExpressionType::layer2_expression);\
    CHECK(loop_body_expression->terms().size() == 2);\
    CHECK(loop_body_expression->terms().at(0)->expression_type == ExpressionType::identifier);\
    CHECK(loop_body_expression->terms().at(0)->string_value() == "f");\
    CHECK(loop_body_expression->terms().at(1)->expression_type == ExpressionType::known_value);\
    CHECK(loop_body_expression->terms().at(1)->string_value() == "23");\
}

WHILE_CASE("\n\
        while (condition) {\n\
            f 23\n\
        }\n\
    ")

WHILE_CASE("\n\
        while condition do\n\
            f 23\n\
        \n\
")

#define WHILE_ELSE_CASE(source_string)\
TEST_CASE(source_string) {\
    LogNoContext::debug_extra("TEST_CASE:\n" + std::string{source_string}, TSL);\
    \
    auto [state, scope, source] = setup(source_string);\
    \
    auto result = run_layer1_eval(state, scope, source);\
    \
    CHECK(result.success);\
    CHECK(result.top_level_definition);\
    CHECK(result.unresolved_identifiers.size() == 3);\
    \
    auto root = *result.top_level_definition;\
    CHECK(std::holds_alternative<const Statement*>(root->const_body()));\
    auto root_body = std::get<const Statement*>(root->const_body());\
    \
    auto [condition, loop_branch, else_branch] = root_body->get_value<ConditionalValue>();\
    CHECK(loop_branch->statement_type == StatementType::loop);\
    auto loop_value = loop_branch->get_value<LoopStatementValue>();\
    \
    CHECK(!loop_value.initializer);\
    \
    CHECK(loop_value.condition == condition);\
    CHECK(loop_value.condition->expression_type == ExpressionType::identifier);\
    CHECK(loop_value.condition->string_value() == "condition");\
    \
    auto loop_body = loop_value.body;\
    CHECK(loop_body->statement_type == StatementType::expression_statement);\
    auto loop_body_expression = loop_body->get_value<Expression*>();\
    CHECK(loop_body_expression->expression_type == ExpressionType::identifier);\
    CHECK(loop_body_expression->string_value() == "f");\
    \
    CHECK(else_branch);\
    CHECK((*else_branch)->statement_type == StatementType::expression_statement);\
    auto else_expression = (*else_branch)->get_value<Expression*>();\
    CHECK(else_expression->expression_type == ExpressionType::identifier);\
    CHECK(else_expression->string_value() == "else_branch");\
}

WHILE_ELSE_CASE("while (condition) then {f} else {else_branch}")

WHILE_ELSE_CASE("\n\
    while (condition) \n\
        {\n\
            f\n\
        } \n\
    else { else_branch }")


WHILE_ELSE_CASE("\n\
    while (condition) {\n\
        f\n\
    } else {\n\
        else_branch\n\
    }")

// WHILE_ELSE_CASE("\n\
//     while (condition) {\n\
//             f\n\
//         } else\n\
//             else_branch")

// WHILE_ELSE_CASE("\n\
//     while (condition) {\n\
//             f\n\
//         } else {\n\
//             else_branch }")

// WHILE_ELSE_CASE("\n\
//     while (condition) \n\
//         {\n\
//             f\n\
//         } \n\
//     else\n\
//                 else_branch\n\
// ")

WHILE_ELSE_CASE("\n\
    while condition\n\
    then\n\
        f\n\
    else\n\
        else_branch\n\
")

WHILE_ELSE_CASE("\n\
    while\n\
        condition\n\
    then\n\
        f\n\
    else\n\
        else_branch\n\
")


// TEST_CASE("for") {
//     auto [state, scope, source] = setup("\
//         if condition\
//             return 34\
//         else\
//             return 22\
//     ");

//     auto result = run_layer1_eval(state, scope, source);

//     CHECK(result.success);
//     CHECK(result.top_level_definition);
//     CHECK(result.unresolved_identifiers.size() == 1);
    
//     auto root = *result.top_level_definition;
//     CHECK(std::holds_alternative<const Statement*>(root->const_body()));
//     auto root_body = std::get<const Statement*>(root->const_body());
// }
