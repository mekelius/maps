# Known issues

- Lambdas would not be visited by AST::visit_nodes if they existed
- editline library is limited to lines of 80 characters (in the REPL that is)
- string literals missing closing \" are parsed as valid
- AST automatically creating root and lying about being empty smells
    - Pragmastore lies as well, but that's not an issue I think
- PragmaStore WILL break silently when we get to multiple source files
- Callable::attempt_simplify doesn't delete the nodes properly. Really Callables should hold a pointer to their store.
- anticipated issue: Callable::attempt_simplify messes with the nodes in a way that is likely to lead to some references to deleted nodes sooner or later
- ReverseParser looks like a stream but can't handle a lot of stuff and doesn't inherit from ostream
    - also stack overflow if you feed it a pointer??
        - sounds like a disaster
- Expression::has_native_representation atm. hard codes functions as the only complex type
- Scope::create_unary_operator forces the type into Hole->Hole
- Scope::create_binary_operator should be combined with the logic used in layer2 CallState
    - also only handles binary functions
- :stop_at doesn't print error message on fail
- llvm generated code uses i64 instead of i32
- lexer constructor eats a single character from the input stream, which is not ideal