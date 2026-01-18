#include "factory.hxx"

void Factory::remove_worker(ElementID id){
    Worker* node = &(*cont_w.find_by_id(id));

    std::for_each(cont_w.begin(), cont_w.end(), [node](Worker& worker) {
        worker.receiver_preferences_.remove_receiver(node);
    });

    std::for_each(cont_r.begin(), cont_r.end(), [node](Ramp& ramp) {
        ramp.receiver_preferences_.remove_receiver(node);
    });

    cont_w.remove_by_id(id);
}

void Factory::remove_storehouse(ElementID id)
{
    Storehouse* node = &(*cont_s.find_by_id(id));
    std::for_each(cont_w.begin(), cont_w.end(), [&node](Worker& ramp) {
        ramp.receiver_preferences_.remove_receiver(node);
    });

    std::for_each(cont_w.begin(), cont_w.end(), [&node](Worker& worker) {
        worker.receiver_preferences_.remove_receiver(node);
    });
    cont_s.remove_by_id(id);
}


