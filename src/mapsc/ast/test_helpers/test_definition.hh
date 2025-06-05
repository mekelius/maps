#ifndef __TESTING_DEFINITION_HH
#define __TESTING_DEFINITION_HH

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/operator.hh"

namespace Maps {

Operator* create_testing_binary_operator(AST_Store& ast_store, std::string name, 
    const Type* type, Operator::Precedence precedence, Operator::Associativity associativity, 
    SourceLocation location);

Operator* create_testing_binary_operator(AST_Store& ast_store, std::string name, 
    const Type* type, Operator::Precedence precedence, SourceLocation location);

} // namespace Maps

#endif