# Known issues

## Critical

- 8* reduces to 8
- REPL should check if a definition can produce valid IR before saving it, currently easy to get stuck
    - also allow deleting names
- Definition::attempt_simplify doesn't delete the nodes when simplifying single-statement blocks and sometimes segfaults (the problem line is commented out atm)
- is not dealt with properlu in stage 2
- something's still wrong with clearing the unresolved identifiers
    - should assert
    - try this (with --stop-after=layer2):
        - a - b
        - let a
        - lte b

## Compilation and validity

- string literals missing closing \" are parsed as valid

## Logging and feedback

- :t doesn't show function types
- :stop_at doesn't print anything on success
- reverse parse doesn't print string values quoted
- Reverse parse doesn't print reference names correctly because definitions store string_views
    - bandaid fix is to add scope reference to the definitions and have them check the name from the scope
        - honestly definitions need a bit of a rework, but that should be combined with making expressions and statements constexprable
            - how can this be done without virtualizing them as well?

## Memory

- I think repl drops the root definitions without deleting them (and leaks everything else as well)
- Callable::attempt_simplify doesn't delete the nodes properly. Really Callables should hold a pointer to their store.

## Will come to bite us at some point

- Lambdas would not be visited by AST::visit_nodes if they existed
- editline library is limited to lines of 80 characters (in the REPL that is)
- PragmaStore WILL break silently when we get to multiple source files
    - Pragmas are a mess atm
- ReverseParser looks like a stream but can't handle a lot of stuff and doesn't inherit from ostream
    - also stack overflow if you feed it a pointer??
        - sounds like a disaster
- type identifiers arent scoped, how should this be handled?

## Misc jank

- lexer constructor eats a single character from the input stream, which is not ideal
- at line 1000 the logging will shift right