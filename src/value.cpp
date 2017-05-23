#include "value.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::shared_ptr;

#ifndef DEBUG_MODE
#define DEBUG_MODE 0
#endif

#if DEBUG_MODE
static u32 sid = 0;
#endif
Scope::Scope(Scope *s, usize seen) : ref(1), outer(s), visible(seen) {
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
    static auto dummy = std::make_shared<Value>();
    for (auto &search : map) {
        if (search.first == name) return;
    }
    map.push_back(std::make_pair(name, dummy));
}

shared_ptr<Value> Scope::get_var(const string &key) const {
    const Scope *out = this;
    usize before = this->map.size();
    while (out) {
        auto it = out->map.begin();
        auto cnt = std::min(before, out->map.size());
        while (cnt != 0) {
            if (it->first == key) {
                return it->second;
            }
            ++it; --cnt;
        }
        before = out->visible;
        out = out->outer;
    }
    cerr << "ERROR: Cannot find var: " << key << endl;
    std::exit(1);
}

// TODO: Almost the same as Scope::get_var
// How can we reuse the code?
void Scope::set_var(const string& key, shared_ptr<Value> v) {
    Scope *out = this;
    usize before = this->map.size();
    while (out) {
        auto it = out->map.begin();
        auto cnt = std::min(before, out->map.size());
        while (cnt != 0) {
            if (it->first == key) {
                // Reclaim unused scope
                if (it->second->kind == Func &&
                    it->second.use_count() == 2)
                {
                    auto captured = std::static_pointer_cast<FuncValue>(it->second)->outer;
                    if (captured != this) captured->unlink();
                }
                it->second = v;
                return;
            }
            ++it; --cnt;
        }
        before = out->visible;
        out = out->outer;
    }
    cerr << "ERROR: Cannot assign var: " << key << endl;
    std::exit(1);
}

void Scope::unlink() {
    if (ref == 0) {
        #if DEBUG_MODE
        cerr << "Scope " << id << ": ";
        #endif
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
