#include "doctest.h"

#include <iostream>

#include "../src/ir/ir_generator.hh"
#include "../src/lang/ast.hh"

// TEST_CASE("Simple constant ir gen should work") {
//     AST::AST ast{};
//     IR::IR_Generator generator{"test module", &std::cerr};

//     AST::Expression* str = ast.create_string_literal("testtest", {0,0});
    
//     generator.run(ast, *str);
// }