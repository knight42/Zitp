#include "zitp.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::unordered_map;
using std::shared_ptr;
using std::ifstream;
using std::ofstream;

typedef int32_t i32;
typedef uint32_t u32;

#define to_int(v)  (static_cast<const IntValue*>(v)->value())
#define to_bool(v) (static_cast<const BoolValue*>(v)->value())
#define to_func(v) (static_cast<const FuncValue*>(v)->value())

#define make_int(v)  (std::make_shared<IntValue>(v))

shared_ptr<BoolValue> make_bool(bool v) {
    static shared_ptr<BoolValue> arr[2] = {
        std::make_shared<BoolValue>(false),
        std::make_shared<BoolValue>(true),
    };
    return v ? arr[1] : arr[0];
}

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
                return make_bool(to_int(l.get()) < to_int(r.get()));
            case Gt:
                l = eval_expr(first, current);
                r = eval_expr(last, current);
                return make_bool(to_int(l.get()) > to_int(r.get()));
            case Eq:
                l = eval_expr(first, current);
                r = eval_expr(last, current);
                return make_bool(to_int(l.get()) == to_int(r.get()));
            case And:
                l = eval_expr(first, current);
                if (!to_bool(l.get())) {
                    return make_bool(false);
                }
                r = eval_expr(last, current);
                return make_bool(to_bool(r.get()));
            case Or:
                l = eval_expr(first, current);
                if (to_bool(l.get())) {
                    return make_bool(true);
                }
                r = eval_expr(last, current);
                return make_bool(to_bool(r.get()));
            case Negb:
                l = eval_expr(first, current);
                return make_bool(!to_bool(l.get()));
        }
    }
    else if (t->kind == Expr) {
        switch (t->subtype) {
            case Number:
                return make_int(t->number);
            case VarName:
                var = current->get_var(t->name);
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
                cerr << "Invalid kind of var: " << t->sons.front()->name <<endl;
                return nullptr;
            case Plus:
                l = eval_expr(first, current);
                r = eval_expr(last, current);
                return make_int(to_int(l.get()) + to_int(r.get()));
            case Minus:
                l = eval_expr(first, current);
                r = eval_expr(last, current);
                return make_int(to_int(l.get()) - to_int(r.get()));
            case Mult:
                l = eval_expr(first, current);
                r = eval_expr(last, current);
                return make_int(to_int(l.get()) * to_int(r.get()));
            case Div:
                l = eval_expr(first, current);
                r = eval_expr(last, current);
                return make_int(to_int(l.get()) / to_int(r.get()));
            case Mod:
                l = eval_expr(first, current);
                r = eval_expr(last, current);
                return make_int(to_int(l.get()) % to_int(r.get()));
            case Apply: {
                var = current->get_var(first->name);
                const FuncValue *fv = static_cast<const FuncValue*>(var.get());
                Scope *s = new Scope(fv->outer);
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
    cerr << "Invalid expr: " << t->kind << endl;
    return nullptr;
}

void Zitp::init_params(const FuncValue *fv, Term *argus, Scope *born, Scope *current) {
    Term *params = fv->value();
    // Function has a Block
    if (params->sons.size() - 1 != argus->sons.size()) {
        cerr << "Different size:" << endl;
        cerr << "vars size: " << params->sons.size() - 1 << endl;
        cerr << "exprs size: " << argus->sons.size() << endl;
        return;
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
        cerr << "Not a Block" << endl;
        return nullptr;
    }

    for (auto &i : t->sons) {
        if (i->kind == Function) {
            root->decl_var(i->sons.front()->name);
            root->set_var(i->sons.front()->name,
                    std::make_shared<FuncValue>(root, i));
        }
        else if (i->kind == Command) {
            auto cmd = i;

            if (cmd->subtype == Declaration) {
                for (auto &var : cmd->sons) {
                    root->decl_var(var->name);
                }
            }
            else if (cmd->subtype == While) {
                auto expr = eval_expr(i->sons.front(), root);
                while (to_bool(expr.get())) {
                    Scope *born = new Scope(root);
                    #if DEBUG_MODE
                    cout << "While block scope: " << born->id <<endl;
                    #endif
                    auto res = execute_program(i->sons.back(), born);
                    if (res) {
                        root->unlink();
                        return res;
                    }

                    expr = eval_expr(i->sons.front(), root);
                }
            }
            else if (cmd->subtype == If) {
                auto it = i->sons.begin();
                auto expr = eval_expr(*it, root);
                Scope *born = new Scope(root);
                #if DEBUG_MODE
                cout << "If block scope: " << born->id <<endl;
                #endif
                shared_ptr<Value> res;
                if (to_bool(expr.get())) {
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
                        cerr << "Return unexpected value" << endl;
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
                Scope *s = new Scope(fv->outer);
                #if DEBUG_MODE
                cout << "Call scope: " << s->id <<endl;
                #endif
                init_params(fv, cmd, s, root);
                Term *func = fv->value();
                execute_program(func->sons.back(), s);
            }
            else if (cmd->subtype == Read) {
                root->set_var(cmd->sons.front()->name, make_int(read_int()));
            }
            else if (cmd->subtype == Print) {
                print_int(to_int(eval_expr(cmd->sons.front(), root).get()));
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
            cerr << "Failed to open " << input_file << endl;
            return -1;
        }
    }
    i32 n = 0;
    _input >> n;
    return n;
}

void Zitp::print_int(i32 val) {
    if (!_output.is_open()) {
        _output = ofstream(output_file);
        if (!_output) {
            cerr << "Failed to open " << output_file << endl;
            return;
        }
    }
    _output << val << endl;
}


void Zitp::run() {
    if (ast == nullptr) {
        cerr << "No AST" << endl;
        return;
    }
    Scope *top = new Scope(nullptr);
    #if DEBUG_MODE
    cout << "Global scope: " << top->id << endl;
    #endif
    auto res = execute_program(ast, top);
    return;
}
