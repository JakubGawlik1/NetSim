#include "factory.hxx"

#include <stdexcept>

void Factory::remove_worker(ElementID id) {
    auto it = cont_w.find_by_id(id);
    if (it == cont_w.end()) return;

    Worker* node = &(*it);

    for (auto& w : cont_w) {
        auto& prefs = w.get_receiver_preferences().get_preferences();
        if (prefs.find(node) != prefs.end()) {
            w.get_receiver_preferences().remove_receiver(node);
        }
    }

    for (auto& r : cont_r) {
        auto& prefs = r.get_receiver_preferences().get_preferences();
        if (prefs.find(node) != prefs.end()) {
            r.get_receiver_preferences().remove_receiver(node);
        }
    }

    cont_w.remove_by_id(id);
}

void Factory::remove_storehouse(ElementID id) {
    auto it = cont_s.find_by_id(id);
    if (it == cont_s.end()) return;

    Storehouse* node = &(*it);

    for (auto& ramp : cont_r) {
        auto& prefs = ramp.get_receiver_preferences().get_preferences();
        if (prefs.find(node) != prefs.end()) {
            ramp.get_receiver_preferences().remove_receiver(node);
        }
    }

    for (auto& worker : cont_w) {
        auto& prefs = worker.get_receiver_preferences().get_preferences();
        if (prefs.find(node) != prefs.end()) {
            worker.get_receiver_preferences().remove_receiver(node);
        }
    }

    cont_s.remove_by_id(id);
}

void Factory::do_deliveries(Time t) {
    for (auto &ramp : cont_r)
        ramp.deliver_goods(t);
}

void Factory::do_work(Time t) {
    for (auto& worker : cont_w)
        worker.do_work(t);
}

void Factory::do_package_passing() {
    for (auto &ramp : cont_r)
        ramp.send_package();

    for (auto &worker : cont_w)
        worker.send_package();
}

bool Factory::is_consistent() const {
    std::map<const PackageSender*, NodeColor> color;

    auto set_unvisited_colors = [&color](const auto& container) {
        for (const auto& item : container) {
            const PackageSender* sender = &item.get_package_sender();
            color[sender] = NodeColor::UNVISITED;
        }
    };

    set_unvisited_colors(cont_w);
    set_unvisited_colors(cont_r);

    try {
        for (const auto& ramp : cont_r) {
            has_reachable_storehouse(ramp.get_package_sender(), color);
        }
    } catch (const std::logic_error&) {
        return false;
    }

    return true;
}


bool has_reachable_storehouse(const PackageSender& sender,
                              std::map<const PackageSender*, NodeColor>& node_colors)
{
    const PackageSender* sender_key = &sender;

    if (node_colors[sender_key] == NodeColor::VERIFIED) return true;
    if (node_colors[sender_key] == NodeColor::VISITED)  return false;

    node_colors[sender_key] = NodeColor::VISITED;

    const auto& prefs = sender.receiver_preferences.get_preferences();
    if (prefs.empty()) {
        throw std::logic_error("Sender does not have any receivers");
    }

    for (const auto& [recv, _prob] : prefs) {
        if (recv->get_receiver_type() == ReceiverType::STOREHOUSE) {
            node_colors[sender_key] = NodeColor::VERIFIED;
            return true;
        }

        if (recv->get_receiver_type() == ReceiverType::WORKER) {
            auto* worker_ptr = dynamic_cast<Worker*>(recv);
            if (!worker_ptr) continue;

            const PackageSender& next_sender = worker_ptr->get_package_sender();

            if (has_reachable_storehouse(next_sender, node_colors)) {
                node_colors[sender_key] = NodeColor::VERIFIED;
                return true;
            }
        }
    }

    node_colors[sender_key] = NodeColor::VERIFIED;
    return false;
}