# All kinds of ideas

## vertically split code
- use | as a separator

Enum mapping types should be iterable, how to do that?
    meaning you can loop through all the values in the enum

## ideas:
- ad hoc function overloading might cause some mistakes, maybe add some sort of decoration to enable it?
- allow (x == y == z == 34 != h) as a special case. It's very intuitive
- use << as append to list maybe (we have ; for monadic discard)
- maybe we can diff the code to last git version to know what changed and produce better error messages
- all strings are streams
- let user take control of gc
- define contextual isomorphisms to reduce a bunch of boilerplate
    - for example we might say that a user is isomorphic to a user_id. In this context everything that accepts a user should accept a user id and vice versa.
- subset and range types, for example: 
    $ type mono_pair a => strict total Mapping [1..2] Int -> a
        - how about non-mono pair
            $ let pair = provider type a b => total mapping [0..1] int -> a | b

- let could maybe be const or var depending on context
    - const globally, var locally
    - or just use const and let, whatever
        - benefits are pretty minimal, but there are maybe some
- could we do algebra on datatypes based on isomorphism during compilation?
    - the compiler would pick a single internal representation for all of them
    - is that what haskell does?
        - could be kinda confusing
- how about types acting as constrains on traits bottom-up
    type TypeA:
        definition TypeA::method1
- automatic invariant checking
    - some way to define invariants for a type as assertions, then some descriptive way to check them at
        the start of a function at correct times
- marking purity of individual args? does this make sense?
- pre-hashed static hashmaps and prebuilt static trees
    - for example cl args

- comment and logstring macros
    - current function name etc.

- Number -> Number -> Number -> Number
    - sort of syntactic sugar for: ```Number a, Number b, Number c => a -> b -> c -> ???```
        - but how to deduce the return type, and how to write it desugared
    - sort of syntactic sugar for: ```Number a, Number b, Number c, Number d => a -> b -> c -> Derived d```

## inpurity and purity, strictness, laziness
- {}-access denotes lazy access
- ()-access denotes impure function call
- whitespace access denotes pure function call (this one might be iffy)
- functions are mappings
- specifier pure is good, but does impure mean anything?
    - maybe better to have more specific specifiers, IO, async etc...
- {}-block denotes strictness
- ()-block denotes non-strictness
    - sometimes this doesn't matter, sometimes it does
- lazy and strict specifiers (lazy semantics? does a lazy function mean that it's return value is lazy?)
    - functions maybe lazy by default, datastructures strict by default
- subset types
- unsafe functions should end with !
    - should optional functions end with ? or is that too annoying?

## about strictness and purity by inference
- purity of functions can be left up to inference? can we, in all cases float an impure function up from a deep stack of impure ones
    - impurity leaves a trace as a specifier, so maybe we can!

- strictness however cannot be inferred, it has to be explicit since it affects semantics
    - sometimes laziness is an implementation detail, sometimes not
    - it could be way more confusing, if with data structures we teach users that {} vs () does not affect correctness but with blocks it does
    - how about during definitions: {},[] == strict, () == non-strict, indentation == strict if impure, non-strict if pure
        - this conflicts somewhat with access operators, but oh well
    - another model would be to use {}-access to explicitly denote non-strictness, or some other access
