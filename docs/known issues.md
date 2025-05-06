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