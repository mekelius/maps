import Data.Char

data AST a = Error | Empty | Leaf a | Branch Operator (AST a) (AST a)
    deriving (Show)

data Operator = Add | Multiply | Subtract
instance Show Operator where {
    show op = case op of
        Add         -> "+"
        Multiply    -> "*"
        Subtract    -> "-"
}

opFromChar :: Char -> Operator
opFromChar c
    | c == '*'
        = Multiply
    | c == '+'
        = Add
    | c == '-'
        = Subtract

operators :: [Char]
operators = ['+', '*', '-']

operator :: Char -> Bool
operator = (`elem` operators)

precedence :: Operator -> Int
precedence op = case op of
    Subtract    -> 1
    Add         -> 2
    Multiply    -> 3

runOp :: Operator -> (Int -> Int -> Int)
runOp op = case op of
    Add         -> (+)
    Multiply    -> (*)
    Subtract    -> (-)

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
        = Branch lo lhs (reduceAst' (Branch (opFromChar ro) (Leaf r) Empty) rs)
    | otherwise
        = reduceAst' (Branch lo lhs (Leaf r)) (ro:rs)

reduceAst' lhs@(Branch{}) [rhs] = Error

reduceAst' lhs@(Branch{}) (r:rs)
    | not $ operator r
        = Error
    | otherwise
        = reduceAst' (Branch (opFromChar r) lhs Empty) rs

evalAst :: AST Char -> AST Int
evalAst Empty = Empty
evalAst (Branch op Empty _) = Empty
evalAst (Branch op _ Empty) = Empty
evalAst Error = Error
evalAst (Branch op Error _) = Error
evalAst (Branch op _ Error) = Error
evalAst (Leaf a)
    | isDigit a
        = Leaf (read [a])
    | otherwise
        = Error
evalAst (Branch op lhs@(Leaf lv) rhs@(Leaf rv))
    | all isDigit [lv, rv]
        = Leaf $ runOp op (read [lv]) (read [rv])
    | otherwise
        = Error
evalAst (Branch op lhs rhs)
    = Leaf $ runOp op (fromLeaf $ evalAst lhs) (fromLeaf $ evalAst rhs)
        where fromLeaf leaf = case leaf of
                Leaf x -> x


