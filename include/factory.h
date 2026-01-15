#pragma once
#include "storage_types.hxx"

class nodes;

#include <algorithm>
#include <list>


template<class Node>
class NodeCollection {
public:
    using container_t = typename std::list<Node>;
    using iterator = typename container_t::iterator;
    using const_iterator = typename container_t::const_iterator;

    void add(Node&& node) { container.emplace_back(std::move(node)); }

    void remove_by_id(ElementID id)
    {
        container.remove_if([id](const Node& elem) { return elem.get_id() == id; });
    }

    NodeCollection<Node>::iterator find_by_id(ElementID id) {
        return std::find_if(container.begin(), container.end(), [id](const Node& elem) {
            return elem.get_id() == id;
        });
    }

    NodeCollection<Node>::const_iterator find_by_id(ElementID id) const {
        return std::find_if(container.begin(), container.end(), [id](const Node& elem) {
            return elem.get_id() == id;
        });
    }

    iterator begin() { return container.begin(); }

    iterator end() { return container.end(); }

    const_iterator begin() const { return container.cbegin(); }

    const_iterator end() const { return container.cend(); }

    const_iterator cbegin() const { return container.cbegin(); }

    const_iterator cend() const { return container.cend(); }

private:
    container_t container;

};


class Factory {
private:

    template<class Node>
    void remove_receiver(NodeCollection<Node>& collection, ElementID id);

    NodeCollection<Ramp> cont_r;
    NodeCollection<Worker> cont_w;
    NodeCollection<Storehouse> cont_s;

public:

    void add_ramp(Ramp&& r) { cont_r.add(std::move(r)); }

    void remove_ramp(ElementID id) { cont_r.remove_by_id(id); }

    void add_worker(Worker&& w) { cont_w.add(std::move(w)); }

    void remove_worker(ElementID id);

    void add_storehouse(Storehouse&& s) { cont_s.add(std::move(s)); }

    void remove_storehouse(ElementID id);

    bool is_consistent() const;

    void do_deliveries(Time);

    void do_package_passing();

    void do_work(Time);

    NodeCollection<Ramp>::iterator find_ramp_by_id(ElementID id) { return cont_r.find_by_id(id); }

    NodeCollection<Ramp>::const_iterator find_ramp_by_id(ElementID id) const { return cont_r.find_by_id(id); }

    NodeCollection<Ramp>::const_iterator ramp_cbegin() const { return cont_r.cbegin(); }

    NodeCollection<Ramp>::const_iterator ramp_cend() const { return cont_r.cend(); }

    NodeCollection<Worker>::iterator find_worker_by_id(ElementID id) { return cont_w.find_by_id(id); }

    NodeCollection<Worker>::const_iterator find_worker_by_id(ElementID id) const { return cont_w.find_by_id(id); }

    NodeCollection<Worker>::const_iterator worker_cbegin() const { return cont_w.cbegin(); }

    NodeCollection<Worker>::const_iterator worker_cend() const { return cont_w.cend(); }

    NodeCollection<Storehouse>::iterator find_storehouse_by_id(ElementID id) { return cont_s.find_by_id(id); }

    NodeCollection<Storehouse>::const_iterator find_storehouse_by_id(ElementID id) const {
        return cont_s.find_by_id(id);
    }

    NodeCollection<Storehouse>::const_iterator storehouse_cbegin() const { return cont_s.cbegin(); }

    NodeCollection<Storehouse>::const_iterator storehouse_cend() const { return cont_s.cend(); }
};
