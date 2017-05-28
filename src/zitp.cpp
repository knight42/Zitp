#include "zitp.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::shared_ptr;
using std::ifstream;
using std::ofstream;

typedef int32_t i32;
typedef uint32_t u32;

static shared_ptr<BoolValue> _bools[2] = {
    std::make_shared<BoolValue>(false),
    std::make_shared<BoolValue>(true),
};

#define to_int(v)  (std::static_pointer_cast<const IntValue>(v)->value())
#define to_bool(v) (std::static_pointer_cast<const BoolValue>(v)->value())
#define to_func(v) (std::static_pointer_cast<const FuncValue>(v)->value())

#define make_int(v)  (std::make_shared<IntValue>(v))
#define make_bool(v)  ((v) ? _bools[1] : _bools[0])

shared_ptr<Value> Zitp::eval_expr(Term *t, Scope *current) {

    shared_ptr<Value> l, r;
    auto first = t->sons.front();
    auto last = t->sons.back();
    shared_ptr<Value> var;
    if (t->kind == BoolExpr) {
        switch (t->subtype) {
            case Lt:
                l = eval_expr(first, current);
                r = eval_expr(last, current);
                return make_bool(to_int(l) < to_int(r));
            case Gt:
                l = eval_expr(first, current);
                r = eval_expr(last, current);
                return make_bool(to_int(l) > to_int(r));
            case Eq:
                l = eval_expr(first, current);
                r = eval_expr(last, current);
                return make_bool(to_int(l) == to_int(r));
            case And:
                l = eval_expr(first, current);
                if (!to_bool(l)) {
                    return make_bool(false);
                }
                r = eval_expr(last, current);
                return make_bool(to_bool(r));
            case Or:
                l = eval_expr(first, current);
                if (to_bool(l)) {
                    return make_bool(true);
                }
                r = eval_expr(last, current);
                return make_bool(to_bool(r));
            case Negb:
                l = eval_expr(first, current);
                return make_bool(!to_bool(l));
        }
    }
    else if (t->kind == Expr) {
        switch (t->subtype) {
            case Number:
                return make_int(t->number);
            case VarName:
                var = current->get_var(t->name);
                if (!var) return var;
                if (var->kind == Integer) {
                    //cout << "Var " << t->name << ": " << to_int(var.get()) << endl;
                    return std::static_pointer_cast<IntValue>(var);
                }
                else if (var->kind == Boolean) {
                    return std::static_pointer_cast<BoolValue>(var);
                }
                else if (var->kind == Func) {
                    return std::static_pointer_cast<FuncValue>(var);
                }
                cerr << "ERROR: Invalid kind of var: " << t->name <<endl;
                return var;
            case Plus:
                l = eval_expr(first, current);
                r = eval_expr(last, current);
                return make_int(to_int(l) + to_int(r));
            case Minus:
                l = eval_expr(first, current);
                r = eval_expr(last, current);
                return make_int(to_int(l) - to_int(r));
            case Mult:
                l = eval_expr(first, current);
                r = eval_expr(last, current);
                return make_int(to_int(l) * to_int(r));
            case Div:
                l = eval_expr(first, current);
                r = eval_expr(last, current);
                if (to_int(r) == 0) {
                    cerr << "ERROR: integer division or modulo by zero" << endl;
                    std::exit(1);
                }
                return make_int(to_int(l) / to_int(r));
            case Mod:
                l = eval_expr(first, current);
                r = eval_expr(last, current);
                if (to_int(r) == 0) {
                    cerr << "ERROR: integer division or modulo by zero" << endl;
                    std::exit(1);
                }
                return make_int(to_int(l) % to_int(r));
            case Apply: {
                var = current->get_var(first->name);
                const FuncValue *fv = static_cast<const FuncValue*>(var.get());
                Scope *s = new Scope(fv->outer, fv->visible);
                #if DEBUG_MODE
                cout << "Apply <" << first->name << "> scope: " << s->id <<endl;
                #endif
                init_params(fv, t, s, current);
                Term *func = fv->value();
                auto res = execute_program(func->sons.back(), s);
                return res;
            }
        }
    }
    cerr << "ERROR: Invalid expr: " << t->kind << endl;
    std::exit(1);
}

void Zitp::init_params(const FuncValue *fv, Term *argus, Scope *born, Scope *current) {
    Term *params = fv->value();
    // Function has a Block
    if (params->sons.size() - 1 != argus->sons.size()) {
        cerr << "ERROR: Different size:" << endl;
        cerr << "vars size: " << params->sons.size() - 1 << endl;
        cerr << "exprs size: " << argus->sons.size() << endl;
        std::exit(1);
    }
    auto vit = ++params->sons.begin();
    auto eit = ++argus->sons.begin();
    for (auto i = argus->sons.size();
         i > 1;
         --i, ++vit, ++eit)
    {
        born->decl_var((*vit)->name);
        born->set_var((*vit)->name, eval_expr(*eit, current));
    }
}

shared_ptr<Value> Zitp::execute_program(Term *t, Scope *root) {
    if (t->kind != Block) {
        cerr << "ERROR: Not a Block" << endl;
        std::exit(1);
    }

    for (auto &cmd : t->sons) {
        if (cmd->kind == Function) {
            root->decl_var(cmd->sons.front()->name);
            root->set_var(cmd->sons.front()->name,
                    std::make_shared<FuncValue>(root, cmd));
        }
        else if (cmd->kind == Command) {
            if (cmd->subtype == Declaration) {
                for (auto &var : cmd->sons) {
                    root->decl_var(var->name);
                }
            }
            else if (cmd->subtype == While) {
                auto expr = eval_expr(cmd->sons.front(), root);
                while (to_bool(expr)) {
                    Scope *born = new Scope(root, root->count_vars());
                    #if DEBUG_MODE
                    cout << "While block scope: " << born->id <<endl;
                    #endif
                    auto res = execute_program(cmd->sons.back(), born);
                    if (res) {
                        root->unlink();
                        return res;
                    }

                    expr = eval_expr(cmd->sons.front(), root);
                }
            }
            else if (cmd->subtype == If) {
                auto it = cmd->sons.begin();
                auto expr = eval_expr(*it, root);
                Scope *born = new Scope(root, root->count_vars());
                #if DEBUG_MODE
                cout << "If block scope: " << born->id <<endl;
                #endif
                shared_ptr<Value> res;
                if (to_bool(expr)) {
                    res = execute_program(*++it, born);
                } else {
                    std::advance(it, 2);
                    res = execute_program(*it, born);
                }
                if (res) {
                    root->unlink();
                    return res;
                }
            }
            else if (cmd->subtype == Return) {
                shared_ptr<Value> expr = eval_expr(cmd->sons.front(), root);
                switch (expr->kind) {
                    case Integer:
                        root->unlink();
                    // fallthrough
                    case Func:
                        return expr;
                    // unreachable
                    default:
                        root->unlink();
                        cerr << "ERROR: Return unexpected value" << endl;
                        return nullptr;
                }
            }
            else if (cmd->subtype == Assign) {
                auto it = cmd->sons.begin();
                const string& name = (*it)->name;
                root->set_var(name, eval_expr(*++it, root));
            }
            else if (cmd->subtype == Call) {
                auto var = root->get_var(cmd->sons.front()->name);
                const FuncValue *fv = static_cast<const FuncValue*>(var.get());
                Scope *s = new Scope(fv->outer, fv->visible);
                #if DEBUG_MODE
                cout << "Call <" << cmd->sons.front()->name << "> scope: " << s->id <<endl;
                #endif
                init_params(fv, cmd, s, root);
                Term *func = fv->value();
                execute_program(func->sons.back(), s);
            }
            else if (cmd->subtype == Read) {
                root->set_var(cmd->sons.front()->name, make_int(read_int()));
            }
            else if (cmd->subtype == Print) {
                auto res = eval_expr(cmd->sons.front(), root);
                if (res->kind == Integer) {
                    print_int(to_int(res.get()));
                }
            }
        }
    }
    root->unlink();
    if (t->father && t->father->kind == Function) {
        return make_int(0);
    }
    return nullptr;
}


i32 Zitp::read_int() {
    if (!_input.is_open()) {
        _input = ifstream(input_file);
        if (!_input) {
            cerr << "ERROR: Failed to open " << input_file << endl;
            std::exit(1);
        }
    }
    i32 n = 0;
    _input >> n;
    return n;
}

void Zitp::print_int(i32 val) {
    static bool first = true;
    if (!_output.is_open()) {
        _output = ofstream(output_file);
        if (!_output) {
            cerr << "ERROR: Failed to open " << output_file << endl;
            std::exit(1);
        }
    }
    if (!first) {
        _output << ' ' << val;
    } else {
        _output << val;
        first = false;
    }
}


void Zitp::run() {
    if (ast == nullptr) {
        cerr << "ERROR: No AST" << endl;
        std::exit(1);
    }
    Scope *top = new Scope(nullptr, 0);
    #if DEBUG_MODE
    cout << "Global scope: " << top->id << endl;
    #endif
    auto res = execute_program(ast, top);
    _output << endl;
    return;
}
