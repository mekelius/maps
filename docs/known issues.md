# Known issues

- Lambdas would not be visited by AST::visit_nodes if they existed
- editline library is limited to lines of 80 characters (in the REPL that is)
- string literals missing closing \" are parsed as valid
- PragmaStore WILL break silently when we get to multiple source files
- Callable::attempt_simplify doesn't delete the nodes properly. Really Callables should hold a pointer to their store.
- ReverseParser looks like a stream but can't handle a lot of stuff and doesn't inherit from ostream
    - also stack overflow if you feed it a pointer??
        - sounds like a disaster
- Scope::create_unary_operator forces the type into Hole->Hole
- Scope::create_binary_operator should be combined with the logic used in layer2 CallState
    - also only handles binary functions
- :stop_at doesn't print error message on fail
- lexer constructor eats a single character from the input stream, which is not ideal
- reverse parse doesn't print string values quoted
- :t doesn't show function types