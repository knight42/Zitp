#include "value.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::shared_ptr;

#if DEBUG_MODE
static u32 sid = 0;
#endif
Scope::Scope(Scope *s) : ref(1), map(std::make_unique<var_t>()), outer(s) {
    #if DEBUG_MODE
    id = sid++;
    #endif
    if (s) {
        #if DEBUG_MODE
        cout << "Scope " << id << " link Scope " << s->id << endl;
        #endif
        s->link();
    }
}

void Scope::link() {
    ref++;
}

void Scope::decl_var(const string& name) {
    (*map)[name] = std::make_shared<IntValue>(0);
}

shared_ptr<Value> Scope::get_var(const string &key) const {
    const Scope *current = this;
    while (current) {
        auto search = current->map->find(key);
        if (search != current->map->end()) {
            return search->second;
        }
        current = current->outer;
    }
    cerr << "Cannot find var: " << key << endl;
    return nullptr;
}

void Scope::set_var(const string& key, shared_ptr<Value> v) {
    const Scope *current = this;
    while (current) {
        auto search = current->map->find(key);
        if (search != current->map->end()) {
            if (search->second->kind == Func && search->second.use_count() == 2) {
                auto captured = std::static_pointer_cast<FuncValue>(search->second)->outer;
                if (captured != this) captured->unlink();
            }
            (*current->map)[key] = v;
            return;
        }
        current = current->outer;
    }
    cerr << "Cannot assign var: " << key << endl;
}

void Scope::unlink() {
    if (ref == 0) {
        cerr << "Why ref == 0?" << endl;
        return;
    }
    if (--ref == 0) {
        if (outer) {
            #if DEBUG_MODE
            cout << "Scope " << id << " unlink Scope " << outer->id << endl;
            #endif
            outer->unlink();
        }
        return destroy();
    }
}
