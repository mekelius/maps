#include "implementation.hh"

#include <optional>

#include "mapsc/ast/misc_expression.hh"
#include "mapsc/ast/statement.hh"

using std::optional, std::nullopt, std::to_string;

namespace Maps {

using Log = LogInContext<LogContext::layer1>;

Statement* ParserLayer1::parse_while_loop() {
    get_token();

    assert(false && "not implemented");
    // return create_
}
Statement* ParserLayer1::parse_for_loop() {
    get_token();

    assert(false && "not implemented");
    // return create_
}
Statement* ParserLayer1::parse_yield_statement() {
    get_token();

    assert(false && "not implemented");
    // return create_
}

} // namespace Maps