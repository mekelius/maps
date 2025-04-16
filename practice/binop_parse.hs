import Data.Char

-- FAILS ON CASE LIKE: "1+2-3*4+5/6"

data AST a = Error | Empty | Leaf a | Branch Operator (AST a) (AST a)
    deriving (Show)

data Operator = Add | Multiply | Subtract | Divide
instance Show Operator where {
    show op = case op of
        Add         -> "+"
        Multiply    -> "*"
        Subtract    -> "-"
        Divide      -> "/"
}

opFromChar :: Char -> Operator
opFromChar c
    | c == '*'
        = Multiply
    | c == '+'
        = Add
    | c == '-'
        = Subtract
    | c == '/'
        = Divide

operators :: [Char]
operators = ['+', '*', '-', '/']

operator :: Char -> Bool
operator = (`elem` operators)

precedence :: Operator -> Int
precedence op = case op of
    Add         -> 1
    Subtract    -> 2
    Multiply    -> 3
    Divide      -> 4

runOp :: Operator -> (Rational -> Rational -> Rational)
runOp op = case op of
    Add         -> (+)
    Multiply    -> (*)
    Subtract    -> (-)
    Divide      -> (/)

reduceAst :: String -> AST Char
reduceAst = reduceAst' Empty

reduceAst' :: AST Char -> String -> AST Char
reduceAst' Error _ 
    = Error

reduceAst' lhs [] 
    = lhs

reduceAst' Empty (x:xs) 
    | operator x 
        = Error
    | otherwise 
        = reduceAst' (Leaf x) xs

reduceAst' (Leaf l) (r:rs)
    | not $ operator r  
        = Error
    | otherwise         
        = reduceAst' lhs rs
            where lhs = Branch (opFromChar r) (Leaf l) Empty

reduceAst' (Branch lo lhs Empty) [r]
    | operator r
        = Error
    | otherwise
        = Branch lo lhs (Leaf r)

reduceAst' (Branch lo lhs Empty) (r:ro:rs)
    | operator r 
        = Error
    | precedence lo < precedence (opFromChar ro)
        = Branch lo lhs (reduceAst' (Branch (opFromChar ro) (Leaf r) Empty) rs) -- THIS IS WRONG
    | otherwise
        = reduceAst' (Branch lo lhs (Leaf r)) (ro:rs)

reduceAst' lhs@(Branch{}) [rhs] = Error

reduceAst' lhs@(Branch{}) (r:rs)
    | not $ operator r
        = Error
    | otherwise
        = reduceAst' (Branch (opFromChar r) lhs Empty) rs

evalAst :: AST Char -> AST Rational
evalAst Empty = Empty
evalAst (Branch op Empty _) = Empty
evalAst (Branch op _ Empty) = Empty
evalAst Error = Error
evalAst (Branch op Error _) = Error
evalAst (Branch op _ Error) = Error
evalAst (Leaf a)
    | isDigit a
        = Leaf (fromIntegral $ read [a])
    | otherwise
        = Error
evalAst (Branch op lhs@(Leaf lv) rhs@(Leaf rv))
    | all isDigit [lv, rv]
        = Leaf $ runOp op (fromIntegral $ read [lv]) (fromIntegral $ read [rv])
    | otherwise
        = Error
evalAst (Branch op lhs rhs)
    = Leaf $ runOp op (fromLeaf $ evalAst lhs) (fromLeaf $ evalAst rhs)
        where fromLeaf leaf = case leaf of
                Leaf x -> x

run = evalAst . reduceAst
