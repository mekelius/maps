#include "doctest.h"

#include <tuple>
#include <sstream>

#include "mapsc/ast/statement.hh"
#include "mapsc/parser/layer1.hh"
#include "mapsc/logging_options.hh"
#include "mapsc/compilation_state.hh"

using namespace std;
using namespace Maps;

inline std::tuple<CompilationState, RT_Scope, stringstream> setup(const std::string& source) {
    auto [state, _1, _2] = CompilationState::create_test_state();

    return {
        std::move(state), 
        RT_Scope{}, 
        stringstream{source}
    };
}

// auto lock = LogOptions::set_global(LogLevel::debug_extra);

#define IF_CASE(source_string)\
TEST_CASE(source_string) {\
    LogNoContext::debug_extra("TEST_CASE:\n" + std::string{source_string}, TSL);\
    auto [state, scope, source] = setup(source_string);\
    \
    auto result = run_layer1_eval(state, scope, source);\
    \
    CHECK(result.success);\
    CHECK(result.top_level_definition);\
    CHECK(result.unresolved_identifiers.size() == 1);\
    \
    auto root = *result.top_level_definition;\
    CHECK(std::holds_alternative<const Statement*>(root->const_body()));\
    auto root_body = std::get<const Statement*>(root->const_body());\
    \
    CHECK(root_body->statement_type == StatementType::if_chain);\
    auto [chain, final_else] = root_body->get_value<IfChainValue>();\
    \
    CHECK(chain.size() == 1);\
    auto [condition, first_branch] = *chain.begin();\
    CHECK(condition->expression_type == ExpressionType::identifier);\
    CHECK(condition->string_value() == "condition");\
    \
    CHECK(first_branch->statement_type == StatementType::return_);\
    auto first_expression = first_branch->get_value<Expression*>();\
    \
    CHECK(first_expression->expression_type == ExpressionType::known_value);\
    CHECK(first_expression->value == ExpressionValue{"1"});\
    \
    CHECK(!final_else);\
}

IF_CASE("if (condition) { return 1 }")

IF_CASE("if condition return 1")

IF_CASE("if condition then return 1")

IF_CASE("\n\
if (condition)\n\
    return 1\n\
")

IF_CASE("\n\
if condition then\n\
    return 1\n\
")

IF_CASE("\n\
if condition\n\
then\n\
    return 1\n\
")

IF_CASE("\n\
if condition\n\
    then return 1\n\
")

IF_CASE("\n\
if condition\n\
    then\n\
        return 1\n\
")

IF_CASE("\n\
if (condition) {\n\
    return 1\n\
}\
")

#define IF_ELSE_CASE(source_string)\
TEST_CASE(source_string) {\
    auto lock = LogOptions::set_global(LogLevel::debug_extra);\
    LogNoContext::debug_extra("TEST_CASE:\n" + std::string{source_string}, TSL);\
    \
    auto [state, scope, source] = setup(source_string);\
    \
    auto result = run_layer1_eval(state, scope, source);\
    \
    CHECK(result.success);\
    CHECK(result.top_level_definition);\
    CHECK(result.unresolved_identifiers.size() == 1);\
    \
    auto root = *result.top_level_definition;\
    CHECK(std::holds_alternative<const Statement*>(root->const_body()));\
    auto root_body = std::get<const Statement*>(root->const_body());\
    \
    CHECK(root_body->statement_type == StatementType::if_chain);\
    auto [chain, final_else] = root_body->get_value<IfChainValue>();\
    \
    CHECK(chain.size() == 1);\
    auto [condition, first_branch] = *chain.begin();\
    CHECK(condition->expression_type == ExpressionType::identifier);\
    CHECK(condition->string_value() == "condition");\
    \
    CHECK(first_branch->statement_type == StatementType::return_);\
    auto first_expression = first_branch->get_value<Expression*>();\
    \
    CHECK(first_expression->expression_type == ExpressionType::known_value);\
    CHECK(first_expression->value == ExpressionValue{"1"});\
    \
    CHECK(final_else);\
    CHECK((*final_else)->statement_type == StatementType::return_);\
    auto second_expression = first_branch->get_value<Expression*>();\
    \
    CHECK(second_expression->expression_type == ExpressionType::known_value);\
    CHECK(second_expression->value == ExpressionValue{"2"});\
}

IF_ELSE_CASE("if (condition) { return 1 } else {return 2}");

IF_ELSE_CASE("if condition; return 1; else return 2;");

IF_ELSE_CASE("\
    if condition\n\
        return 1\n\
    else\n\
        return 2\n\
")

// TEST_CASE("if_else_chain") {
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

// TEST_CASE("guard") {
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

// TEST_CASE("switch") {
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

// TEST_CASE("while") {
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
