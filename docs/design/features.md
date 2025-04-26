# Features (the current vision)

v0.1

## Language paradigm
- explicitly mark imperative and pure contexts with different semantics, or leave under-specified
- all the normal functional stuff
    - higher order functions
    - recursion
    - lambdas
    - closures
    - currying and partial application
    - iterators
- contexts (how exactly?)

## "How it should've always been done"-features
- optionals as first-class citizens and with elegant syntax
    - monadic optional unwrapping
    - optionals baked into primitives and casting
    - make any value or type optional by adding ?
    - first class constructions for handing optionals nonobtrusively with line-end-statements
        - also makes error handling and debug outputs more elegant
    - unsafe access shouldn't be necessary, but marked with !
- variants are also here
- contextual isomorphisms for extremely clean code
- parentheses are inserted around operators applied without spaces

## Type-system
- type trait system
- static inferred types
- subset types for communicating caller responsibilities (and enforcing them)

## "underspecified types and operators"
    - Mapping is a very general abstracted type that includes functions and containers
    - underspecified access operators allow for cose at use sites to stay valid even if the underlying structure changes completely
    - compiler will pick datastructures based on use, and/or hints, or the user can make the choice
    - elegant performance hints with underspecified access operators

## Experimental ideas
- line-end real-estate used for context manipulation and debugging info
    - probably not very ergonomic until somebody makes a editor plugin for this, but it looks neat!
- imperative functions strict by default, pure functions non-strict by default (kinda scary)
- datastructures eager by default, opt-in lazy
- pattern matching (how can we make it as smart as possible?)
    - allow values in patterns? at least constants

## Interactive compilation
- compiler suggests simple semantics preserving transforms, such as changing the access operator to a more appropriate one
- compiler suggests fixes for trivial issues
- closed world compilation, i.e. zero cost modules (we will try this until proven too slow)