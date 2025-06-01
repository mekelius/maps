#include "test_definition.hh"

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/operator.hh"
#include "mapsc/ast/ast_store.hh"

namespace Maps {

DefinitionHeader* create_testing_let_definition(AST_Store& ast_store, const std::string& name, 
    const Type* type, const SourceLocation& location) {

    return ast_store.allocate_definition(
        DefinitionHeader{DefinitionType::let_definition, name, type, location}, Undefined{});
}


Operator* create_testing_binary_operator(AST_Store& ast_store, const std::string& name, 
    const Type* type, Operator::Precedence precedence, Operator::Associativity associativity, 
    SourceLocation location) {

    assert(type->arity() >= 2 && "Trying to create testing binary operator with type of arity <2");

    auto function = create_testing_let_definition(ast_store, name + "_f", type, location);

    return ast_store.allocate_operator(Operator{ 
        name, 
        function, 
        Operator::Properties {
            Operator::Fixity::binary, 
            precedence, 
            associativity 
        }, 
        location 
    });
}

Operator* create_testing_binary_operator(AST_Store& ast_store, const std::string& name, 
    const Type* type, Operator::Precedence precedence, SourceLocation location) {

    return create_testing_binary_operator(ast_store, name, type, precedence, 
        Operator::Associativity::left, location);
}

} // namespace Maps
