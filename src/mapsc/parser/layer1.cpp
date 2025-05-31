#include "layer1.hh"
#include "layer1/implementation.hh"

namespace Maps {

Layer1Result run_layer1(CompilationState& state, Scope& scope, std::istream& source) {
    return ParserLayer1{&state, &scope}.run(source);
}

Layer1Result run_layer1_eval(CompilationState& state, Scope& scope, std::istream& source) {
    return ParserLayer1{&state, &scope}.run_eval(source);
}

} // namespace Maps