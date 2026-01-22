#include "factory.hxx"

#include <stdexcept>

void Factory::remove_worker(ElementID id){
    auto it = cont_w.find_by_id(id);
    if (it == cont_w.end()) return;
    Worker* node = &(*it);

    std::for_each(cont_w.begin(), cont_w.end(), [node](Worker& worker) {
        worker.PackageSender_.receiver_preferences.remove_receiver(node);
    });

    std::for_each(cont_r.begin(), cont_r.end(), [node](Ramp& ramp) {
        ramp.PackageSender_.receiver_preferences.remove_receiver(node);
    });

    cont_w.remove_by_id(id);
}

void Factory::remove_storehouse(ElementID id)
{
    auto it = cont_s.find_by_id(id);
    if (it == cont_s.end()) return;

    Storehouse* node = &(*it);

    for (auto& worker : cont_w) {
        worker.PackageSender_.receiver_preferences.remove_receiver(node);
    }

    for (auto& ramp : cont_r) {
        ramp.PackageSender_.receiver_preferences.remove_receiver(node);
    }

    cont_s.remove_by_id(id);
}

void Factory::do_deliveries(Time t) {
    for (auto &ramp : cont_r)
        ramp.deliver_goods(t);
}

void Factory::do_work(Time t ) {
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
            const PackageSender* sender =  &item.PackageSender_;
            color[sender] = NodeColor::UNVISITED;
        }
    };

    set_unvisited_colors(cont_w);
    set_unvisited_colors(cont_r);

    try {
        for (const auto& ramp : cont_r) {
            has_reachable_storehouse(&ramp.PackageSender_, color);
        }
    } catch (const std::logic_error&) {
        return false;
    }
    return true;
}


bool has_reachable_storehouse(const PackageSender* sender, std::map<const PackageSender*, NodeColor>& node_colors) {
    if (node_colors[sender] == NodeColor::VERIFIED) {
        return true;
    }

    node_colors[sender] = NodeColor::VISITED;

    if (sender->receiver_preferences.get_preferences().empty()) {
        throw std::logic_error("Sender does not have any receivers");
    }

    for (const auto& receiver : sender->receiver_preferences.get_preferences()) {
        if (receiver.first->get_receiver_type() == ReceiverType::STOREHOUSE) {
            return true;
        } else if (receiver.first->get_receiver_type() == ReceiverType::WORKER) {
            auto* worker_ptr = dynamic_cast<Worker*>(receiver.first);
            if (!worker_ptr) continue;

            PackageSender* sendrecv_ptr = &worker_ptr->PackageSender_;

            if (sendrecv_ptr == sender) continue;

            if (node_colors[sendrecv_ptr] == NodeColor::UNVISITED &&
                has_reachable_storehouse(sendrecv_ptr, node_colors)) {
                return true;
                }
        }
    }

    node_colors[sender] = NodeColor::VERIFIED;
    return false;
}