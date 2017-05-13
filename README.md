README
=======

Minilan Interpreter with GC support.

# Grammar

```
Number   -> (0..9)+
Name     -> (a..z)+
VarName  -> (a..z)+

Block    -> Begin (Function|Command)* End
Function -> Function Name Paras (Name)* Block

Command  -> Var (Name)+ End
          | Assign Name Expr
          | Call Name Argus (Expr)* End
          | Read Name
          | Print Expr
          | If BoolExpr Block Else Block
          | While BoolExpr Block
          | Return Expr

Expr     -> Number
          | VarName
          | Plus Expr Expr
          | Minus Expr Expr
          | Mult Expr Expr
          | Div Expr Expr
          | Mod Expr Expr
          | Apply Name Argus (Expr)* End

BoolExpr -> Lt Expr Expr
          | Gt Expr Expr
          | Eq Expr Expr
          | And BoolExpr BoolExpr
          | Or BoolExpr BoolExpr
          | Negb BoolExpr
```
