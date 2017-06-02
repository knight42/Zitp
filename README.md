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

# Build

NOTE: Only tested on ArchLinux.

```
$ mkdir build && cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
$ make
```

# Test

```
$ make test
```

# Garbage Collection

由于语言中的数据类型比较简单，因此基于引用计数的垃圾回收方案足以解决内存泄漏的问题。

程序中有两种对象：

* Value （具体分为 NullValue，IntValue，BoolValue，FuncValue）
* Scope （每次进入 Block 时创建，与上一级 Scope 形成 Scope chain，用于存储 Value 以及标识符解析）

前者由 `std::shared_ptr` 管理，当引用次数为 0 时会自动释放内存。其中 FuncValue 中会保存其所处的 Scope，造成循环引用，所以 FuncValue 中还有一个字段 `ref` 表示它在当前的 Scope 中的引用次数。

后者则是手动管理，只有当在函数内返回函数时，所返回的函数所处的 Scope （以及所有外层的 Scope）才会保留，否则当一个 Block 执行结束后，附属于它的 Scope 会被立即销毁（并沿着 Scope chain 一直往上销毁）。

由此可见，程序中可能产生的垃圾只会是未被销毁的 Scope。

函数在声明时会记录它所在的 Scope，保存在 FuncValue 中。当在函数中返回函数时，会重置所返回的 `FuncValue::ref` 为 0（赋值时会加 1）。当由于赋值导致 `ref` 减少到 0 或 Block 结束后，FuncValue 中保存的 Scope 将被销毁（并沿着 Scope chain 一直往上销毁）。
