# Parsing layers

## v0.1

Parsing is done in layers

| Layer | Responsibilities                         | Strategy | Consumes   | Produces           |
| ----- | ----------------                         | -------- | ---------- | --------           |
| 0     | lexing                                   | LL/RD    | char       | Token              |
| 1     | statements, terminals, overall structure | RD       | Token      | Expression, Pragma |
| 1.5   | name resolution                          | work in progress | type_construction and friends | type_reference |
|       | type specifiers                          | just lookup | *_identifier      | *_reference    |
| 2     | termed expressions (binops, calls...)    | RA/RD    | termed_expression | call |
|       |                                          |          | *_reference       |      |
| Codegen | generating the ir                      |          | call              | llvm::Module | 
