# Parsing layers

Parsing is done in layers

## v0.1

| Layer | Responsibilities                         | Strategy | Consumes   | Produces           |
| ----- | ----------------                         | -------- | ---------- | --------           |
| 0     | lexing                                   | LL/RD    | char       | Token              |
| 1     | statements, terminals, overall structure | RD       | Token      | Expression, Pragma |
| 1.5   | name resolution                          | just_lookup <br> work in progress | *_identifier <br> type_construction and friends | *_reference |
| 2     | termed expressions (binops, calls...)    | RA/RD    | termed_expression <br> *_reference | call |
| Codegen | generating the ir                      |          | call              | llvm::Module | 

## v0.2 draft

| Layer | Responsibilities                         | Strategy | Consumes   | Produces           |
| ----- | ----------------                         | -------- | ---------- | --------           |
| 0     | lexing                                   | LL/RD    | char       | Token              |
| 1     | statements, terminals, overall structure | RD       | Token      | Expression, Pragma |
| 2.1   | name resolution                          | work in progress | type_construction and friends | type_reference |
| 2.2   | type specifiers                          | just lookup | *_identifier      | *_reference    |
| 3     | termed expressions (binops, calls...)    | RA/RD    | termed_expression <br> *_reference | call <br> value |
| 4     | type inference                           | ?        | ? polymorphic_call? | call       |
| Codegen | generating the ir                      |          | call <br> value | llvm::Module | 
