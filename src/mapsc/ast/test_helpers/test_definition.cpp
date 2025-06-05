#include "test_definition.hh"

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/operator.hh"
#include "mapsc/ast/ast_store.hh"

namespace Maps {

DefinitionHeader* create_testing_let_definition(AST_Store& ast_store, std::string name, 
    const Type* type, SourceLocation location) {

    return ast_store.allocate_definition(
        RT_DefinitionHeader{
            DefinitionType::let_definition, std::move(name), type, std::move(location)}, 
            Undefined{}).first;
}

Operator* create_testing_binary_operator(AST_Store& ast_store, std::string name, 
    const Type* type, Operator::Precedence precedence, Operator::Associativity associativity, 
    SourceLocation location) {

    assert(type->arity() >= 2 && "Trying to create testing binary operator with type of arity <2");

    auto function = create_testing_let_definition(ast_store, name + "_f", type, location);

    return ast_store.allocate_operator(Operator{ 
        std::move(name), 
        function, 
        Operator::Properties {
            Operator::Fixity::binary, 
            precedence, 
            associativity 
        }, 
        std::move(location)
    });
}

Operator* create_testing_binary_operator(AST_Store& ast_store, std::string name, 
    const Type* type, Operator::Precedence precedence, SourceLocation location) {

    return create_testing_binary_operator(ast_store, std::move(name), type, precedence, 
        Operator::Associativity::left, std::move(location));
}

} // namespace Maps
