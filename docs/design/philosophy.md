## On abstractions
1. (Correct) code using an abstraction should only need to change if the assumptions it makes about that abstraction become invalid
    - therefore these assumptions need to be documented with redundant symbols
    - This is the underlying idea of uniting pure functions and dictionaries into a common type.

2. Abstractions should be able to start broad and be narrowed down with additional conditions as the user gains more understanding

3. There should be elegant and non-tedious language for expressing performance assumptions. Performance assumptions becoming untrue shouldn't
   make the program invalid, but they should be found.

## General philosophy
1. Simple ideas shouldn't be complex or tedious to express, this is a failure of language

2. Warnings and errors should always point to the locations where the code was changed to cause them
    - not always possible, 

3. Structure of an expression should be considered before the symbols used.
    - for example (1,2,3,4) {1,2,3,4} and [1,2,3,4] are all clearly sequences. Therefore they should be treated as such. Using the wrong symbol to express
    the correct idea should (when non-ambiguous) cause a warning, not an error.

4. Let us try to be as permissive with syntax as possible without causing ambiguities.

5. Differing notations are a space that can carry pragmatic meanings beyond program correctness, this is where we have space for elegant encoding of assumptions.
    - is this idea possible to apply to laziness?

## Compilation and structuring of code
1. Compiler can use the whole of the source code for inferences and optimizations. There's no such thing as an open world.

2. Modularizing the code should be zero-cost.