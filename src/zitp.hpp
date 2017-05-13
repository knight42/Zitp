#ifndef ZITP_CPP
#define ZITP_CPP

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <iterator>
#include <unordered_map>
#include <stack>
#include <string>
#include <memory>
using std::unique_ptr;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::unordered_map;
using std::ifstream;
using std::ofstream;

typedef int32_t i32;
typedef uint32_t u32;

#include "Term.h"

namespace {
    enum ValueKind {
        Null,
        Boolean,
        Integer,
        Func
    };
    class Value {
        public:
            virtual ~Value() {}
            ValueKind kind;
    };

    class NullValue : public Value {
        public:
            NullValue() {
                kind = Null;
            }
    };

    class IntValue : public Value {
        i32 val;
        public:
            IntValue(i32 v = 0): val(v) {
                kind = Integer;
            }
            i32 value() const { return val; }
            unique_ptr<IntValue> copy() const { return std::make_unique<IntValue>(val); }
    };

    class BoolValue : public Value {
        bool val;
        public:
            BoolValue(bool v = false): val(v) {
                kind = Boolean;
            }
            bool value() const { return val; }
            unique_ptr<BoolValue> copy() const { return std::make_unique<BoolValue>(val); }
    };

    typedef unordered_map<string, unique_ptr<Value>> var_t;
    typedef vector<unique_ptr<var_t>> var_list_t;

    typedef unordered_map<string, Value*> _var_t;

    #define to_int(v)  (static_cast<const IntValue*>(v)->value())
    #define to_bool(v) (static_cast<const BoolValue*>(v)->value())
    #define to_func(v) (static_cast<const FuncValue*>(v)->value())

    #define make_int(v)  (std::make_unique<IntValue>(v))
    #define make_bool(v) (std::make_unique<BoolValue>(v))
    #define make_func(v) (std::make_unique<FuncValue>(v))

    static u32 sid = 0;
    class Scope {
        private:
            u32 ref;
            unique_ptr<var_t> map;
            void destroy() {
                cout << "Destroying scope " << id << endl;
                delete this;
            }

        public:
            Scope* outer;
            u32 id;
            /*~Scope() {
                cout << "Killing scope " << id << endl;
            }*/
            Scope(Scope *s) : ref(1), map(std::make_unique<var_t>()), outer(s) {
                id = sid++;
                if (s) {
                    cout << "scope " << id << " link scope " << s->id << endl;
                    s->link();
                }
            }

            void unlink() {
                if (ref == 0) {
                    cerr << "Why ref == 0?" << endl;
                    return;
                }
                if (--ref == 0) {
                    if (outer) {
                        cout << "scope " << id << " unlink scope " << outer->id << endl;
                        outer->unlink();
                    }
                    return destroy();
                }
            }
            void link() {
                ref++;
            }

            void set_var(const string& key, unique_ptr<Value> v) {
                const Scope *current = this;
                while (current) {
                    auto search = current->map->find(key);
                    if (search != current->map->end()) {
                        (*current->map)[key] = std::move(v);
                        return;
                    }
                    current = current->outer;
                }
                cerr << "Cannot assign var: " << key << endl;
            }

            void decl_var(const string& name) {
                (*map)[name] = std::make_unique<NullValue>();
            }

            const Value* get_var(const string &key) const {
                const Scope *current = this;
                while (current) {
                    auto search = current->map->find(key);
                    if (search != current->map->end()) {
                        return search->second.get();
                    }
                    current = current->outer;
                }
                cerr << "Cannot find var: " << key << endl;
                return nullptr;
            }
    };

    class FuncValue : public Value {
        Term* val;
        public:
            Scope* outer;
            FuncValue(Scope *s, Term* v = nullptr): val(v), outer(s) {
                kind = Func;
            }
            Term* value() const { return val; }
            unique_ptr<FuncValue> copy() const {
                cout << "Copying scope " << outer->id << endl;
                return std::make_unique<FuncValue>(outer, val);
            }
    };
}

class Zitp {
private:
    string input_file = "input.txt";
    string output_file = "output.txt";
    string prog_file = "program.txt";
    ifstream _input;
    ofstream _output;

    bool _feed(ifstream &ifs, const std::string &fp) {
        if (!ifs) {
            cerr << fp << " cannot be found" << endl;
            return false;
        }
        //ast = parse(ifs);
        ast = nullptr;
        return true;
    }

    i32 read_int() {
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

    void print_int(i32 val) {
        if (!_output.is_open()) {
            _output = ofstream(output_file);
            if (!_output) {
                cerr << "Failed to open " << output_file << endl;
                return;
            }
        }
        _output << val << endl;
    }

    unique_ptr<Value> eval_expr(Term *t, Scope *current) {

        unique_ptr<Value> l, r;
        auto first = t->sons.front();
        auto last = t->sons.back();
        const Value *var;
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
                    // FIXME: Overhead
                    if (var->kind == Integer) {
                        cout << "Var " << t->name << ": " << to_int(var) << endl;
                        return static_cast<const IntValue*>(var)->copy();
                    }
                    else if (var->kind == Boolean) {
                        return static_cast<const BoolValue*>(var)->copy();
                    }
                    else if (var->kind == Func) {
                        return static_cast<const FuncValue*>(var)->copy();
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
                    const FuncValue *fv = static_cast<const FuncValue*>(var);
                    Scope *s = new Scope(fv->outer);
                    cout << "Apply <" << first->name << "> scope: " << s->id <<endl;
                    init_params(fv, t, s, current);
                    Term *func = to_func(var);
                    auto res = execute_program(func->sons.back(), s);
                    if (res->kind == Integer) {
                        cout << "Application result: " << to_int(res.get()) << endl;
                    }
                    return res;
                 }
            }
        }
        cerr << "Invalid expr: " << t->kind << endl;
        return nullptr;
    }

    void init_params(const FuncValue *fv, Term *argus, Scope *born, Scope *current) {
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

    void exec_trivial_command(Term *t, Scope *scope) {
        if (t->kind != Command) return;

        if (t->subtype == Assign) {
            auto it = t->sons.begin();
            const string& name = (*it)->name;
            scope->set_var(name, eval_expr(*++it, scope));
        }
        else if (t->subtype == Call) {
            auto var = scope->get_var(t->sons.front()->name);
            const FuncValue *fv = static_cast<const FuncValue*>(var);
            Scope *s = new Scope(fv->outer);
            cout << "Call scope: " << s->id <<endl;
            init_params(fv, t, s, scope);
            Term *func = to_func(var);
            execute_program(func->sons.back(), s);
        }
        else if (t->subtype == Read) {
            scope->set_var(t->sons.front()->name, make_int(read_int()));
        }
        else if (t->subtype == Print) {
            print_int(to_int(eval_expr(t->sons.front(), scope).get()));
        }
    }

    unique_ptr<Value> execute_program(Term *t, Scope *root) {
        if (t->kind != Block) {
            cerr << "Not a Block" << endl;
            return nullptr;
        }

        for (auto &i : t->sons) {
            if (i->kind == Function) {
                root->decl_var(i->sons.front()->name);
                root->set_var(i->sons.front()->name,
                        std::make_unique<FuncValue>(root, i));
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
                        cout << "While block scope: " << born->id <<endl;
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
                    cout << "If block scope: " << born->id <<endl;
                    unique_ptr<Value> res;
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
                    unique_ptr<Value> expr = eval_expr(cmd->sons.front(), root);
                    switch (expr->kind) {
                        case Integer:
                            root->unlink();
                            //[[clang::fallthrough]];
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

                else {
                    exec_trivial_command(cmd, root);
                }

            }
        }
        root->unlink();
        if (t->father && t->father->kind == Function) {
            return make_int(0);
        }
        return nullptr;
    }

public:
    Term *ast;

    Zitp(const char *prog, const char *in, const char *out):
        ast(nullptr)
    {
        if (prog) {
            prog_file = string(prog);
        }
        if (in) {
            input_file = string(in);
        }
        if (out) {
            output_file = string(out);
        }
    }

    bool feed() {
        std::ifstream ifs(prog_file);
        return _feed(ifs, prog_file);
    }

    void run() {
        if (ast == nullptr) {
            cerr << "No AST" << endl;
            return;
        }
        Scope *top = new Scope(nullptr);
        cout << "Global scope: " << top->id << endl;
        auto res = execute_program(ast, top);
        return;
    }
};
#endif
