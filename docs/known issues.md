# Known issues

- Lambdas would not be visited by AST::visit_nodes if they existed
- editline library is limited to lines of 80 characters (in the REPL that is)
- string literals missing closing \" are parsed as valid
- AST automatically creating root and lying about being empty smells
    - Pragmastore lies as well, but that's not an issue I think
- PragmaStore WILL break silently when we get to multiple source files