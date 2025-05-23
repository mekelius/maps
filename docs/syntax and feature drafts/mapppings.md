# Mappings
***This is a blue-skies draft, the features don't exist yet***

this is a mapping
```
let mapping1 = Mapping Int -> String {
    1: "number one"         // commas are optional here
    2: "number two"
}
```

Mappings are polymorphic types that unify functions, enumerations and (indexable) datastructures into a common interface
```
let mapping2 = mapping Int -> Int: 
    x -> 2*x
```

Type inference can make this cleaner
```
let times2 = Int x -> 2*x
```

Mappings can be initialized empty
```
let empty_map = mapping String -> String
```

Mapping values can be accessed with a variety of syntaxes
```
times2[6]                   // these are all equivalent, but provide different compiler hints
```

. and :: work as well, but they are special in that they will consider strings as string literals. Therefore they don't accept names
```
times2.6 
times2::6

times2{6} // these 3 are a bit special. Not sure how to handle them. Originally the idea was to use {} for linear time access, but maybe it should be used for lazy access instead 
times2 6
times2(6) // this should be reserved for function calls, possibly impure
```

. and :: are special, in that they will only 

Different methods of access do carry a hint for the compiler about performance assumptions, specifically about random access
```
times2::6 // :: indicates that the value might be replaced by a constant
times2.6  // . indicates very fast constant time access, compiler might choose a struct
times2[6] // [] indicates constant-time access, compiler might choose an array or an associative array
times2{6} // {} indicates ?? lazy ?? linear-time ?? what
times2(6) // () and whitespace indicate that the access is allowed to be slower. Compiler will not automatically choose datastructure slower than linear.
```

mappings can take a number of specifiers
```
let dict1 = dynamic Mapping String -> String
```

dynamic means that the mapping can be assigned to
```
let dict2 = dynamic Mapping String -> String
dict2["key"] = "value"
```

const means that it can't be assigned to
```
let enum1 = const Mapping String -> Int {
    SOME_VALUE1
    SOME_VALUE2
}
enum1[3] = "nope"      // error
```

Partial means that there could be misses
```
let partial_dict1 = dynamic partial Mapping String -> String {
    "some key": "existing value"
}
let fetched_value1 = String: partial_dict1["nonexistent_key"]   // error, need to use an optional with a partial mapping
let fetched_value2? = String?: partial_dict1["nonexistent_key"] // like this
let fetched_value3 = String: partial_dict1["some key"]          // accessing without optional is allowed, if the compiler can deduce that the key is guaranteed to exist
```

Total means that a valid value is always produced
```
let total_dict1 = total Mapping String -> String        // error, since didn't provide a default or cover the whole space
let total_dict2 = total Mapping String -> String {
    "some key": "existing value"
} with default ""                                       // daniel suggested ?? as shorthand for "with default"

    "asd":123:true:"yaas",
    "asd":123:true:"yaas",
    "asd":123:true:"yaas",
    "asd":123:true:"yaas",
} == [
    ["asd", 123, true, "yaas"],
    ["asd", 123, true, "yaas"],    
    ["asd", 123, true, "yaas"],    
    ["asd", 123, true, "yaas"],    
] ~= [
    {
        name: "asd", 
        number: 123, 
        yeaitis: true, 
        epic: "yaas",
    },
    {
        name: "asd", 
        number: 123, 
        yeaitis: true, 
        epic: "yaas",
    },
    {
        name: "asd", 
        number: 123, 
        yeaitis: true, 
        epic: "yaas",
    },
    {
        name: "asd", 
        number: 123, 
        yeaitis: true, 
        epic: "yaas",
    },
]
```

iterable means that a mapping can be iterated over (TODO: syntax)
ordered means that the keys will be kept in order

const means that the mapping cannot be assigned to
immutable is the same as const

mutable means that the keys can be reassigned, implies dynamic
dynamic means that keys can be deleted, implies mutable

retaining means that keys cannot be deleted, implies mutable
static is the same as retaining (should this exist?)

some specifiers can be inverted with non-
    non-retaining

all specifiers can be left up to inference by use
