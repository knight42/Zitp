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

std::vector<var_t>::iterator Scope::find_var(Scope *root, const std::string& key) const {
    usize before = root->map.size();
    while (root) {
        auto it = root->map.begin();
        auto cnt = std::min(before, root->map.size());
        while (cnt != 0) {
            if (it->first == key) {
                return it;
            }
            ++it; --cnt;
        }
        before = root->visible;
        root = root->outer;
    }
    cerr << "ERROR: Cannot find " << key << endl;
    std::exit(1);
}

shared_ptr<Value> Scope::get_val(const string &key) {
    auto it = find_var(this, key);
    return it->second;
}

void Scope::set_var(const string& key, shared_ptr<Value> v) {
    auto it = find_var(this, key);
    // Reclaim unused scope
    if (it->second->kind == Func &&
        it->second.use_count() == 2)
    {
        auto captured = std::static_pointer_cast<FuncValue>(it->second)->outer;
        if (captured != this) captured->unlink();
    }
    it->second = v;
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
