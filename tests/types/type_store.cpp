#include "doctest.h"

#include <array>

#include "mapsc/types/type_defs.hh"
#include "mapsc/types/type_store.hh"

using namespace std;
using namespace Maps;

TEST_CASE("Should be able to create a non-existent function type based on signature") {
    TypeStore types{};

    SUBCASE("nullary function") {
        auto function_type = types.get_function_type(TestingType, {}, true);
        CHECK(function_type->arity() == 0);
        CHECK(*function_type->return_type() == TestingType);
    
        auto params = function_type->param_types();
        
        CHECK(params.size() == 0);
    }

    SUBCASE("unary function") {
        auto function_type = types.get_function_type(TestingType, {&TestingType}, true);
        CHECK(function_type->arity() == 1);
        CHECK(*function_type->return_type() == TestingType);
    
        auto params = function_type->param_types();
        
        CHECK(params.size() == 1);
        CHECK(**params.begin() == TestingType);
    }

    SUBCASE("binary function") {
        auto function_type = types.get_function_type(TestingType, {&TestingType, &TestingType}, true);
        CHECK(function_type->arity() == 2);
        CHECK(*function_type->return_type() == TestingType);
    
        auto params = function_type->param_types();
        
        CHECK(params.size() == 2);
        auto params_it = params.begin();
        
        CHECK(**params_it == TestingType);
        params_it++;
        CHECK(**params.begin() == TestingType);
    }
}

const auto test_ct_function_type = 
    CTFunctionType<1>{"asronasroi", &TestingType, {&TestingType}, false};

const std::array<const FunctionType*, 1> test_builtin_function_types = {
    &test_ct_function_type
};

TEST_CASE("Should get a compile-time functiontype without issue") {
    TypeStore types{BUILTIN_TYPES, test_builtin_function_types};

    auto function_type = types.get_function_type(TestingType, {&TestingType}, true);

    CHECK(*function_type == test_ct_function_type);
    CHECK(function_type == &test_ct_function_type);
}

