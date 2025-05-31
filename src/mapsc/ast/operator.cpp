#include "operator.hh"

#include "mapsc/ast/ast_store.hh"
#include "mapsc/source.hh"
#include "mapsc/ast/definition.hh"

namespace Maps {

Operator Operator::create_binary(const std::string& name, DefinitionHeader* value, 
    Operator::Precedence precedence, Operator::Associativity associativity, 
    SourceLocation location) {

    return Operator { 
        name, 
        value, 
        Properties {
            Fixity::binary, 
            precedence, 
            associativity 
        }, 
        location 
    };
}

Operator* create_binary_operator(AST_Store& ast_store, const std::string& name, DefinitionHeader* value, 
    Operator::Precedence precedence, Operator::Associativity associativity, 
    SourceLocation location) {

    // auto function = ast_store.allocate_definition(name + "_f", value,)

    return ast_store.allocate_operator(Operator{ 
        name, 
        value, 
        Operator::Properties {
            Operator::Fixity::binary, 
            precedence, 
            associativity 
        }, 
        location 
    });
}

Operator* create_binary_operator(AST_Store& ast_store, const std::string& name, DefinitionHeader* value, 
    Operator::Precedence precedence, SourceLocation location) {

    return create_binary_operator(
        ast_store, name, value, precedence, Operator::Associativity::left, location);
}


Operator* create_testing_binary_operator(AST_Store& ast_store, const std::string& name, 
    const Type* type, Operator::Precedence precedence, Operator::Associativity associativity, 
    SourceLocation location) {

    auto function = ast_store.allocate_definition({name + "_f", type, location});

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