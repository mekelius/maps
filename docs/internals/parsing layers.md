# Parsing layers

Parsing is done in layers

## v0.1

| Layer | Responsibilities                         | Strategy | Consumes   | Produces           |
| ----- | ----------------                         | -------- | ---------- | --------           |
| 0     | lexing                                   | LL/RD    | char       | Token              |
| 1     | statements, terminals, overall structure | RD       | Token      | Expression, Pragma |
| 1.5   | name resolution                          | just_lookup | *_identifier | *_reference |
| 2     | termed expressions (binops, calls...)    | RA/RD    | termed_expression <br> *_reference | call |
| Codegen | generating the ir                      |          | call              | llvm::Module | 

## v0.2 draft

| Layer | Responsibilities                         | Strategy | Consumes   | Produces           |
| ----- | ----------------                         | -------- | ---------- | --------           |
| 0     | lexing                                   | LL/RD    | char       | Token              |
| 1     | statements, terminals, overall structure | RD       | Token      | Expression, Pragma |
| 2.0   | type name resolution                     | just lookup | type_identifier <br> type_operator_identifier | type_reference <br> type_operator_reference <br> type_contructor_reference |
| 2.1   | binding type declarations                | RA       | identifier <br> type_* | field_name <br> type_construct etc... |
| 2.2   | name resolution                          | just lookup | *_identifier      | *_reference |
| 3     | termed expressions (binops, calls...)    | RA/RD    | termed_expression <br> *_reference | call <br> value |
| 4     | type inference                           | ?        | ??? polymorphic_call? | call       |
| Codegen | generating the ir                      |          | call <br> value | llvm::Module | 
