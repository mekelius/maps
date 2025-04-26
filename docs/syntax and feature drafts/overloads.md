# Overloads
***This is a blue-skies draft, the features don't exist yet***

Functions can be given multiple definitions (overloads), but they have to be given locally

```
let print =
    String string ~> IO: 
        puts string

    Int int ~> IO: 
        print: String int       // overlads are allowed to delegate as long as there are no cycles
```

Pros:
- This avoids the unneeded hassle of creating and naming a type trait for every single
    polymorphic function. It can be thought of as anonymous implicit traits. Sometimes
    only connecting attribute of a set of types is that this particular function can
    accept them, which is not a reason to abstact.
     
Cons:
- This isn't very extendable.
    - it can be argued, that once you need to extend this you need to make it a trait. Library
          writers should favor traits over this. However, with isomorphisms and subset types
           a user can perhaps do some extensions wihtout modifying the original code
    - ok if we implement these as anonymous traits, maybe we can allow a mechanism like @Traitof print?



So the above would be equivalent to:
```
trait printable:
    let print = 
        String ~> IO: puts
        Int ~> IO: print . String
```
