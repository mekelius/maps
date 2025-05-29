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
