#include "doctest.h"

#include <tuple>
#include <sstream>

#include "mapsc/ast/statement.hh"
#include "mapsc/parser/layer1.hh"
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

#define IF_CASE(source_string)\
TEST_CASE(source_string) {\
    LogNoContext::debug_extra(TSL) << "TEST_CASE:\n" << source_string;\
    auto [state, scope, source] = setup(source_string);\
    \
    auto result = run_layer1_eval(state, scope, source);\
    \
    CHECK(result.success);\
    CHECK(result.top_level_definition);\
    CHECK(result.unresolved_identifiers.size() == 1);\
    \
    auto root = *result.top_level_definition;\
    CHECK(std::holds_alternative<Statement*>(root->get_value()));\
    auto root_body = std::get<Statement*>(root->get_value());\
    \
    CHECK(root_body->statement_type == StatementType::conditional);\
    auto [condition, body, else_branch] = root_body->get_value<ConditionalValue>();\
    \
    CHECK(condition->expression_type == ExpressionType::identifier);\
    CHECK(condition->string_value() == "condition");\
    \
    CHECK(body->statement_type == StatementType::return_);\
    auto first_expression = body->get_value<Expression*>();\
    \
    CHECK(first_expression->expression_type == ExpressionType::known_value);\
    CHECK(first_expression->value == ExpressionValue{"1"});\
    \
    CHECK(!else_branch);\
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

IF_CASE("\n\
if\n\
    condition\n\
then\n\
    return 1\n\
")

// IF_CASE("\n\
// if (\n\
//     condition)\n\
// then\n\
//     return 1\n\
// ")

#define IF_ELSE_CASE(source_string)\
TEST_CASE(source_string) {\
    LogInContext<LogContext::layer1>::debug_extra(TSL) << "TEST_CASE:\n" << source_string;\
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
    CHECK(std::holds_alternative<Statement*>(root->get_value()));\
    auto root_body = std::get<Statement*>(root->get_value());\
    \
    CHECK(root_body->statement_type == StatementType::conditional);\
    auto [condition, body, else_branch] = root_body->get_value<ConditionalValue>();\
    \
    CHECK(condition->expression_type == ExpressionType::identifier);\
    CHECK(condition->string_value() == "condition");\
    \
    CHECK(body->statement_type == StatementType::return_);\
    auto first_expression = body->get_value<Expression*>();\
    \
    CHECK(first_expression->expression_type == ExpressionType::known_value);\
    CHECK(first_expression->value == ExpressionValue{"1"});\
    \
    CHECK(else_branch);\
    CHECK((*else_branch)->statement_type == StatementType::return_);\
    auto else_expression = (*else_branch)->get_value<Expression*>();\
    \
    CHECK(else_expression->expression_type == ExpressionType::known_value);\
    CHECK(else_expression->value == ExpressionValue{"2"});\
}

IF_ELSE_CASE("if (condition) { return 1 } else {return 2}");

IF_ELSE_CASE("if condition then return 1; else return 2;");

IF_ELSE_CASE("\n\
    if condition\n\
        return 1\n\
    else\n\
        return 2\n\
")

IF_ELSE_CASE("\n\
    if (condition) {\n\
        return 1\n\
    } else {\n\
        return 2\n\
    }\n\
")

IF_ELSE_CASE("\n\
    if (condition) then {\n\
        return 1\n\
    } else {\n\
        return 2\n\
    }\n\
")

// IF_ELSE_CASE("\n\
//     if (condition) then {\n\
//             return 1\n\
//         } else {\n\
//             return 2\n\
//     }\n\
// ")

IF_ELSE_CASE("\n\
    if condition\n\
        then \n\
            return 1\n\
        else \n\
            return 2\n\
    ;\
")

IF_ELSE_CASE("\n\
    if condition\n\
        then \n\
            return 1\n\
        else \n\
            return 2\n\
")

#define IF_ELSE_CHAIN_CASE(source_string)\
TEST_CASE(source_string) {\
    LogInContext<LogContext::layer1>::debug_extra(TSL) << "TEST_CASE:\n" << source_string;\
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
    CHECK(std::holds_alternative<Statement*>(root->get_value()));\
    auto root_body = std::get<Statement*>(root->get_value());\
    \
    CHECK(root_body->statement_type == StatementType::conditional);\
    auto [condition1, branch1, else_branch1] = root_body->get_value<ConditionalValue>();\
    \
    CHECK(condition1->expression_type == ExpressionType::identifier);\
    CHECK(condition1->string_value() == "condition1");\
    CHECK(branch1->statement_type == StatementType::return_);\
    auto expression1 = branch1->get_value<Expression*>();\
    CHECK(expression1->expression_type == ExpressionType::known_value);\
    CHECK(expression1->value == ExpressionValue{"1"});\
    \
    CHECK((*else_branch1)->statement_type == StatementType::conditional);\
    auto [condition2, branch2, else_branch2] = (*else_branch1)->get_value<ConditionalValue>();\
    CHECK(condition2->expression_type == ExpressionType::identifier);\
    CHECK(condition2->string_value() == "condition2");\
    CHECK(branch2->statement_type == StatementType::return_);\
    auto expression2 = branch2->get_value<Expression*>();\
    CHECK(expression2->expression_type == ExpressionType::known_value);\
    CHECK(expression2->value == ExpressionValue{"2"});\
    \
    CHECK((*else_branch2)->statement_type == StatementType::conditional);\
    auto [condition3, branch3, else_branch3] = (*else_branch2)->get_value<ConditionalValue>();\
    CHECK(condition3->expression_type == ExpressionType::identifier);\
    CHECK(condition3->string_value() == "condition3");\
    CHECK(branch3->statement_type == StatementType::return_);\
    auto expression3 = branch3->get_value<Expression*>();\
    CHECK(expression3->expression_type == ExpressionType::known_value);\
    CHECK(expression3->value == ExpressionValue{"3"});\
    \
    CHECK(else_branch3);\
    CHECK((*else_branch3)->statement_type == StatementType::return_);\
    auto final_expression = (*else_branch3)->get_value<Expression*>();\
    \
    CHECK(final_expression->expression_type == ExpressionType::known_value);\
    CHECK(final_expression->value == ExpressionValue{"4"});\
}

IF_ELSE_CHAIN_CASE("if (condition1) then {return 1} else if (condition2) then {return 2} else if (condition3) then {return 3} else {return 4}")
IF_ELSE_CHAIN_CASE("if (condition1) then {return 1;}; else if (condition2) {return 2} else if (condition3) then return 3 else return 4")

IF_ELSE_CHAIN_CASE("\n\
    if (condition1) {\n\
        return 1\n\
    } else if (condition2) {\n\
        return 2\n\
    } else if (condition3) {\n\
        return 3\n\
    } else {\n\
        return 4\n\
    }\n\
")

IF_ELSE_CHAIN_CASE("\n\
    if condition1\n\
        return 1\n\
    else if condition2 \n\
        return 2\n\
    else if condition3 \n\
        return 3\n\
    else \n\
        return 4\n\
    \n\
")

IF_ELSE_CHAIN_CASE("\n\
    if (condition1) \n\
        then\n\
            return 1\n\
        else if (condition2) \n\
            return 2\n\
        else if (condition3) then return 3 else return 4")


IF_ELSE_CHAIN_CASE("\n\
    if (condition1) \n\
        then\n\
            return 1\n\
        else if (condition2) \n\
            return 2\n\
        else if (condition3)\n\
        then return 3\n\
        else return 4")

TEST_CASE("Simple guard") {
    auto [state, scope, source] = setup("while id1 do { while id2 then id3 else if id4 then {id5;id6} else target7 } else id8");

    auto result = run_layer1_eval(state, scope, source);

    CHECK(result.success);
    CHECK(result.top_level_definition);
    CHECK(result.unresolved_identifiers.size() == 8);    
}

TEST_CASE("Simple guard") {
    auto [state, scope, source] = setup("guard\n     guard_condition");

    auto result = run_layer1_eval(state, scope, source);

    CHECK(result.success);
    CHECK(result.top_level_definition);
    CHECK(result.unresolved_identifiers.size() == 1);
    
    auto root = *result.top_level_definition;
    CHECK(std::holds_alternative<Statement*>(root->get_value()));
    auto root_body = std::get<Statement*>(root->get_value());

    CHECK(root_body->statement_type == StatementType::guard);
    auto condition_expression = root_body->get_value<Expression*>();
    CHECK(condition_expression->expression_type == ExpressionType::identifier);
    CHECK(condition_expression->string_value() == "guard_condition");
}

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
//     CHECK(std::holds_alternative<Statement*>(root->get_value()));
//     auto root_body = std::get<Statement*>(root->get_value());
// }
