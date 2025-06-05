#include "operator.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/source_location.hh"
#include "mapsc/ast/definition.hh"
#include "mapsc/ast/let_definition.hh"

namespace Maps {

Operator Operator::create_binary(std::string name, DefinitionHeader* value, 
    Operator::Precedence precedence, Operator::Associativity associativity, 
    SourceLocation location) {

    return Operator { 
        std::move(name),
        value, 
        Properties {
            Fixity::binary, 
            precedence, 
            associativity 
        }, 
        std::move(location)
    };
}

Operator* create_binary_operator(AST_Store& ast_store, std::string name, DefinitionHeader* value, 
    Operator::Precedence precedence, Operator::Associativity associativity, 
    SourceLocation location) {

    return ast_store.allocate_operator(RT_Operator{ 
        std::move(name), 
        value, 
        Operator::Properties {
            Operator::Fixity::binary, 
            precedence, 
            associativity 
        },
        std::move(location) 
    });
}

Operator* create_binary_operator(AST_Store& ast_store, std::string name, DefinitionHeader* value, 
    Operator::Precedence precedence, SourceLocation location) {

    return create_binary_operator(
        ast_store, std::move(name), value, precedence, Operator::Associativity::left, std::move(location));
}

} // namespace Maps