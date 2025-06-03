#include "doctest.h"

#include "mapsc/logging.hh"
#include "mapsc/ast/builtin.hh"
#include "mapsc/builtins.hh"

using namespace Maps;

TEST_CASE("Should have builtin values true and false") {
    auto builtins = get_builtins();

    auto true_ = builtins->get_identifier("true");
    CHECK(true_);
    CHECK((*true_)->is_const());
    CHECK((*true_)->get_body_value());
    // CHECK(std::get<BuiltinValue>((*(*true_)->get_body_value())) == true);
}
