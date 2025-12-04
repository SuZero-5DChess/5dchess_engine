#ifndef GAMETREE_H
#define GAMETREE_H

#include <functional>
#include <optional>
#include "action.h"
#include "state.h"

template<typename T>
class gnode {
    gnode<T> *parent;
    std::optional<state> s;
    action act;
    T info;
    std::vector<std::unique_ptr<gnode>> children;

    gnode(gnode<T>* parent, std::optional<state> s, const action &act, const T &info) : parent(parent), s(s), act(act), info(info), children() {}
public:
    static std::unique_ptr<gnode<T>> create_root(const state &s, const T &info)
    {
        return std::unique_ptr<gnode<T>>(new gnode<T>(nullptr, s, action{}, info));
    }
    
    static std::unique_ptr<gnode<T>> create_child(gnode<T>* parent, std::optional<state> s, const action &act, const T &info)
    {
        return std::unique_ptr<gnode<T>>(new gnode<T>(parent, s, act, info));
    }
    state get_state()
    {
        if(s)
        {
            return *s;
        }
        else if(parent)
        {
            s = *parent->get_state().can_apply(act);
            return *s;
        }
        else
        {
            throw std::runtime_error("gnode::get_state(): Root gnode has no state");
        }
    }
    const action& get_action() const {
        return act;
    }
    const T& get_info() const {
        return info;
    }
    gnode<T>* get_parent() const {
        return parent;
    }
    gnode<T>* add_child(std::unique_ptr<gnode<T>> child) {
        children.push_back(std::move(child));
        return children.back().get();
    }
    const std::vector<std::unique_ptr<gnode>>& get_children() const {
        return children;
    }
    gnode<T>* find_child(const action &a)
    {
        for(const auto &child : children)
        {
            if(a == child->act)
            {
                return child.get();
            }
        }
        return nullptr;
    }
};

#endif /* GAMETREE_H */
