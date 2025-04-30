#include <sstream>

// TEST_CASE_CLASS("simplify_single_statement_block") {
//     AST ast;
//     Pragmas pragmas;

//     ParserLayer1 layer1{&ast, &pragmas};

//     auto block = ast.create_statement({StatementType::block}, TEST_SOURCE_LOCATION);

//     auto statement = layer1.simplify_single_statement_block(block);

    // CHECK(ast.is_valid);
    // CHECK(callable);
    // CHECK(std::holds_alternative<Expression*>((*callable)->body));

    // auto expression = std::get<Expression*>((*callable)->body);

    // CHECK(expression->expression_type == ExpressionType::numeric_literal);
    // CHECK(expression->string_value() == "4");
// }

// TEST_CASE("layer1 eval should collapse single statement blocks") {
//     AST ast;
//     Pragmas pragmas;

//     ParserLayer1 layer1{&ast, &pragmas};

//     std::string source = "{{ {4} }}";
//     std::stringstream source_s{source};

//     auto callable = layer1.eval_parse(source_s);

//     CHECK(ast.is_valid);
//     CHECK(callable);
//     CHECK(std::holds_alternative<Expression*>((*callable)->body));

//     auto expression = std::get<Expression*>((*callable)->body);

//     CHECK(expression->expression_type == ExpressionType::numeric_literal);
//     CHECK(expression->string_value() == "4");
// }