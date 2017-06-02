#ifndef ZITP_VALUE_H
#define ZITP_VALUE_H

#include <memory>
#include <string>
#include <iostream>
#include <algorithm>
#include <utility>
#include <cstdlib>
#include <vector>
#include <functional>
#include <unordered_map>

#include "Term.hpp"

#ifndef DEBUG_MODE
#define DEBUG_MODE 0
#endif

typedef int32_t i32;
typedef uint32_t u32;
typedef uintptr_t usize;

enum ValueKind {
    Null,
    Boolean,
    Integer,
    Func
};

class Value {
    public:
    Value(): kind(Null) {}
    virtual ~Value() {}
    ValueKind kind;
};

class IntValue : public Value {
    i32 val;
    public:
    IntValue(i32 v = 0): val(v) {
        kind = Integer;
    }
    i32 value() const { return val; }
};

class BoolValue : public Value {
    bool val;
    public:
    BoolValue(bool v = false): val(v) {
        kind = Boolean;
    }
    bool value() const { return val; }
};

typedef std::pair<std::string, std::shared_ptr<Value>> var_t;
class Scope {
    private:
        u32 ref;
        bool top;
        std::vector<var_t>::iterator find_var(Scope *root, const std::string& key) const;
        void destroy() {
            #if DEBUG_MODE
            std::cout << "Destroying scope " << id << std::endl;
            #endif
            delete this;
        }

    public:
        Scope* outer;
        usize visible;
        #if DEBUG_MODE
        u32 id;
        #endif
        std::vector<var_t> map;
        Scope(Scope *s, usize seen);

        void mark_top() { top = true; }
        void link() { ++ref; }
        usize count_vars() const { return map.size(); }
        void unlink();
        void free2top();
        void decl_var(const std::string& name);
        void set_var(const std::string& key, std::shared_ptr<Value> v);
        std::shared_ptr<Value> get_val(const std::string &key);
};

class FuncValue : public Value {
    Term* val;
    public:
    Scope* outer;
    usize visible;
    usize ref;
    FuncValue(Scope *s, Term* v = nullptr) :
        val(v), outer(s), visible(s->count_vars()), ref(1)
    {
        kind = Func;
    }
    Term* value() const { return val; }
};

#endif
