#include "doctest.h"

#include <sstream>

#include "mapsc/parser/layer1.hh"
#include "mapsc/compilation_state.hh"
#include "mapsc/logging_options.hh"

using namespace Maps;
using namespace std;

inline std::tuple<CompilationState, Scope, stringstream> setup(const std::string& source) {
    auto [state, _1, _2] = CompilationState::create_test_state();

    return {
        std::move(state), 
        Scope{}, 
        stringstream{source}
    };
}

// TEST_CASE("Should create a scope for parameters") {
//     auto lock = LogOptions::set_global(LogLevel::debug_extra);

//     auto [state, outer_scope, source] = setup("let f = x y z -> return y");

//     auto result = run_layer1_eval(state, outer_scope, source);

//     CHECK(result.success);

//     auto f = outer_scope.get_identifier("f");
//     CHECK(f);

//     CHECK((*f)->is_function());
//     CHECK((*f)->arity() == 3);
//     CHECK((*f)->inner_scope());

//     CHECK((*f)->inner_scope()->size() == 3);
// }