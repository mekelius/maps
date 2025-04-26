# Identifying types internally

***NOTE:*** a lot of this is outdated so you probably don't want to read it

In order to be able to reference types in a meaningful way, there has to be a way to find the
object associated with the type.
The chosen approach is to use two hashmaps, one keyed by a given typename if any, the other by a string that hopefully 
will be reliably constructible from the structure of the type.

For example, when a user creates a type to represent a pair of ints and names it Point, two objects are created:
one to hold the information related to the type identifier and another to the type structure. If a trait is given
to the structure, the identifier's type get's the trait as well, but the reverse is not be true.

The structurally identified type-object acts as the prototype for the identifier-identified type-object.

If this behavior is not desired, the user must wrap the created type on creation




Strcture-based signatures used in lookup are constructed in the following way
First every type name is replaced by a running id (types are never deleted during the runtime, is this ok?)

named types have their id as their signature:

function types are constructed from other types as follows, without spaces
id-id-id=id

operators shouldnt be separated from functions at type level, but atm they are
binops are marked with B, unops with U

with minus denoting pure argument and equal sign an impure one, the last one denoting the purity of the whole
function

type constructors are applied with id{id}

impure functions with no args 



NON IMPLEMENTED TYPES (subject to change):

sequences are constructed with [id,id,id+] with + denoting repeating

mappings in general are constructed with [id:id,id:id]


Nested types are marked with parenthesis
(id->id)->id

Algebraic data types are marked with haskell-like notation
(id,id)
(id|id)



// IDEA: polymorphic wrapper types? how do these compare to regular sum types?