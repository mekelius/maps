# Compilation pipeline

| Stage                 | Consumes          | Produces               |
| -----                 | ----------        | --------               |
| lexer                 | char              | token                  |
| parser layer1         | token             | definition <br> statement <br> expression <br> pragma |
| type name resolution  | type identifier <br> type operator | type reference <br> type constructor reference |
| parser layer1.5       | termed expression | type declaration |
| name resolution       | identifier        | reference |
| parser layer2         | termed expressions | call |
| transforms            |
| (repl wrapper)        | -                 | - |
| ir gen                | definition        | llvm::Module       | 




| Transform              | description | operates on |   
| ---                    | ---         | ---         |
| casting                | try to cast compile time constants into either declared or inferred types <br> insert run time cast function calls for compile time unknown values | value |
| simplification | perform various meaning-preserving transforms | * |
| substitution | replace references to known values with values | reference |
| inlining               | replace function calls with their bodies <br> replace pure function calls with known values with their results | call |
| template specialization | create concrete definitions for generics | concrete definition |
| type check | ensure every function call has the correct argument types | call, arg |
| argument type coercion | try to cast arguments into parameter types | reference | value |
| concretization         | cast abstract known values into concrete ones <br> ensure unknown values have concrete types | value | 