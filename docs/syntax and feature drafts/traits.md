# Traits
***This is a blue-skies draft, the features don't exist yet***

## Creating traits
Traits live in the uppercase namespace

```
trait Printable:
    let print = Printable ~> IO
```

The suffix -able has special meaning: naming a type with that suffix is an error the name of the trait can be used as a type variable, with the implicit meaning that the type is constrained to have the trait.
The trait name without the -able in lowercase is assumed to be a method of that type
```
trait Crankable:
    let crank Crankable = //implementation
```

(Should every function should get a trait by default?)

Alternate trait def syntaxes using a mapping, if trait has only a single (implicit) function, the definitions can simply be listed
```
trait Printable:
    String ~> IO: // definition
    Int ~> IO: // definition
```

## Extending traits

Traits can be extended with
```
TypeA is Callable:
    call typeA args = // implementation
```

## Checking traits

In constraints and static checks, there are a number of ways to assert having traits
```
@ check TypeA has trait Callable @      // this one is most readable, esp. when checking operator types, see below
@ check Callable TypeA @
@ check TypeA is Callable @             // this one can be ambiguous if Callable is also used as a type variable
```

Can also be done on variables
```
TypeA a
@ check a is Callable @
```

Naming normal variables with -able should be a warning, maybe even an error? Traits should have an optional suffix marking them as traits (beside -able) <br>
_T? yes seems reasonable<br>
So we could say something like:

```
@ check TypeA has trait >>=_T @
```