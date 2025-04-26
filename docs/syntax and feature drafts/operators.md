# Operators
***This is a blue-skies draft, the features don't exist yet***

Operators can be defined like so
```
operator *:
    binary, precedence 2000

    let Number a * Number b -> Number: 
        multiply a b
```

Operators can have multiple disjoint definitions without an issue
```
operator +:
    binary, precedence 1000

    let Number + Number = add

operator +:
    let String + String = concat
```

Every operator creates a trait that can be named, or accessed via +::trait
If the definitions overlap, and the traits are proper subsets, we can use the most specific one