#include "doctest.h"

#include <sstream>
#include <tuple>

#include "mapsc/ast/ast_store.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/parser/layer2.hh"
#include "mapsc/logging_options.hh"

using namespace Maps;
using namespace std;

tuple<CompilationState, shared_ptr<AST_Store>, RT_Scope> setup() {
    auto [state, _0, _1] = CompilationState::create_test_state();

    return {
        std::move(state),
        state.ast_store_,
        RT_Scope{}
    };
}

TEST_CASE("Should report failure correctly") {
    auto [state, _0, types] = CompilationState::create_test_state();
    auto& ast_store = *state.ast_store_;

    auto value = Expression::numeric_literal(ast_store, "23", TSL);

    auto outer = Expression::termed_testing(ast_store, {value, value, value}, TSL);

    auto success = run_layer2(state, outer);

    CHECK(!success);
}

TEST_CASE("TermedExpressionParser should replace a single value term with that value") {
    auto [state, _0, _1] = CompilationState::create_test_state();

    Expression* expr = Expression::termed_testing(*state.ast_store_, {}, TSL);

    REQUIRE(expr->terms().size() == 0);

    SUBCASE("String value") {
        Expression* value = Expression::string_literal(*state.ast_store_, "TEST_STRING:oasrpkorsapok", TSL);
        expr->terms().push_back(value);
        run_layer2(state, expr);

        CHECK(expr->expression_type == ExpressionType::string_literal);
        CHECK(expr->string_value() == value->string_value());
    }

    SUBCASE("Number value") {
        Expression* value = Expression::numeric_literal(*state.ast_store_, "234.52", TSL);
        expr->terms().push_back(value);
        run_layer2(state, expr);

        CHECK(expr->expression_type == ExpressionType::numeric_literal);
        CHECK(expr->string_value() == value->string_value());
    }
}

TEST_CASE("TermedExpressionParser should handle haskell-style call expressions") {
    auto [state, _0, types] = CompilationState::create_test_state();
    RT_Scope globals{};
    AST_Store& ast = *state.ast_store_;

    Expression* expr = Expression::termed_testing(ast, {}, TSL);
    
    SUBCASE("1 arg") {    
        const Type* function_type = types->get_function_type(&Void, array{&String}, true);

        RT_Definition function{"test_f", External{}, function_type, true, TSL};
        globals.create_identifier(&function);

        Expression* id = Expression::reference(ast, &function, TSL);
        Expression* arg1 = Expression::string_literal(ast, "", TSL);

        expr->terms().push_back(id);
        expr->terms().push_back(arg1);

        run_layer2(state, expr);
        
        CHECK(expr->expression_type == ExpressionType::call);
        auto [callee, args] = expr->call_value();
        CHECK(callee->name() == "test_f");
        CHECK(args.size() == 1);
        CHECK(args.at(0) == arg1);
    }

    SUBCASE("4 args") {    
        const Type* function_type = types->get_function_type(&Void, 
            array{&String, &String, &String, &String}, true);
        
        RT_Definition function{"test_f", External{}, function_type, true, TSL};
        Expression* id{Expression::reference(ast, &function, TSL)};
        id->type = function_type;
    
        Expression* arg1 = Expression::string_literal(ast, "", TSL);
        Expression* arg2 = Expression::string_literal(ast, "", TSL);
        Expression* arg3 = Expression::string_literal(ast, "", TSL);
        Expression* arg4 = Expression::string_literal(ast, "", TSL);

        expr->terms().push_back(id);
        expr->terms().push_back(arg1);
        expr->terms().push_back(arg2);
        expr->terms().push_back(arg3);
        expr->terms().push_back(arg4);

        run_layer2(state, expr);
        
        CHECK(expr->expression_type == ExpressionType::call);
        auto [callee, args] = expr->call_value();
        CHECK(callee->name() == "test_f");
        CHECK(args.size() == 4);
        CHECK(args.at(0) == arg1);
        CHECK(args.at(1) == arg2);
        CHECK(args.at(2) == arg3);
        CHECK(args.at(3) == arg4);
    }

    SUBCASE("If the call is not partial, the call expression's type should be the return type") {
        const Type* function_type = types->get_function_type(&Number, std::array{&String}, true);
        
        RT_Definition function{"test_f", External{}, function_type, true, TSL};
        Expression* ref = Expression::reference(ast, &function, TSL);

        Expression* arg1 = Expression::string_literal(ast, "", TSL);

        expr->terms().push_back(ref);
        expr->terms().push_back(arg1);

        run_layer2(state, expr);
        
        CHECK(expr->type == &Number);
    }
}

TEST_CASE("Should perform known value substitution") {
    auto [state, ast_store, scope] = setup();

    auto known_val = Expression::known_value(state, 34, &Int, TSL);
    auto known_val_def = ast_store->allocate_definition(RT_Definition{"x", *known_val, true, TSL});

    auto known_val_ref = Expression::reference(*ast_store, known_val_def, TSL);

    REQUIRE(known_val_ref->expression_type == ExpressionType::known_value_reference);

    auto termed = Expression::termed_testing(*ast_store, {known_val_ref}, TSL);

    CHECK(run_layer2(state, termed));

    CHECK(termed->expression_type == ExpressionType::known_value);
    CHECK(termed->value == (*known_val)->value);
}