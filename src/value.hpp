#ifndef ZITP_VALUE_H
#define ZITP_VALUE_H

#include <memory>
#include <string>
#include <iostream>
#include <unordered_map>

#include "Term.hpp"

typedef int32_t i32;
typedef uint32_t u32;

enum ValueKind {
    Boolean,
    Integer,
    Func
};

class Value {
    public:
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

class Scope {
    private:
        u32 ref;
        std::unordered_map<std::string, std::shared_ptr<Value>> map;
        void destroy() {
            #if DEBUG_MODE
            std::cout << "Destroying scope " << id << std::endl;
            #endif
            delete this;
        }

    public:
        Scope* outer;
        #if DEBUG_MODE
        u32 id;
        #endif
        Scope(Scope *s);

        void unlink();
        void link();
        void set_var(const std::string& key, std::shared_ptr<Value> v);
        void decl_var(const std::string& name);
        std::shared_ptr<Value> get_var(const std::string &key) const;
};

class FuncValue : public Value {
    Term* val;
    public:
    Scope* outer;
    FuncValue(Scope *s, Term* v = nullptr): val(v), outer(s) {
        kind = Func;
    }
    Term* value() const { return val; }
};

#endif
