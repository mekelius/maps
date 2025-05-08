# Type oddities

- In this syntax, it's a bit ambiguous whther something is a simple value or a const function
    - for example these are differentiated only by declared type, but all are valid: 
        - Int 4 
        - Int -> Int: 4
        - Int -> String -> Whatever -> Int: 4
        - ...
    - this sounds kinda strange, but maybe it's a bood thing?
        - or should unused args be somehow dealt with?
    - in a sense, before we do name binding, we kind of have to run with partial calls, so it's just
        a question of for example codegen being able to deal with partial calls
    - but then how about being able to cast values into const function? should that be allowed
        - it's trivial isn't it?
        