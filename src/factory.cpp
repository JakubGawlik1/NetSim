#include "factory.hxx"
#include "nodes.hxx"
#include <iostream>
#include <istream>
#include <stdexcept>
#include <string>
#include <sstream>

PackageQueueType string_to_enum(std::string type);
std::string enum_to_string(PackageQueueType type);
std::string enum_to_string(ReceiverType type);

void generate_structure_report(const Factory& f, std::ostream& os);
void generate_structure_turn_report(const Factory& f, std::ostream& os, Time t);


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

void linking(const Factory &f, std::ostream &os, std::string type) {
	if (type == "ramp") {
		for (auto it  = f.ramp_cbegin(); it != f.ramp_cend(); it++) {
			auto rp = it -> get_receiver_preferences().get_preferences();
			
			for (auto __it = rp.cbegin(); __it != rp.cend(); __it++) {
				std::string t = enum_to_string(__it->first->get_receiver_type());
				os << "LINK src=ramp-" << it -> get_id();
				os << " dest=" << t << "-" << __it->first->get_id();
				os << "\n";
			}
			os << "\n";
		}
	}
	else if (type == "worker") {
		for (auto it  = f.worker_cbegin(); it != f.worker_cend(); it++) {
			auto rp = it -> get_receiver_preferences().get_preferences();
			
			for (auto __it = rp.cbegin(); __it != rp.cend(); __it++) {
				std::string t = enum_to_string(__it->first->get_receiver_type());
				os << "LINK src=worker-" << it -> get_id();
				os << " dest=" << t << "-" << __it->first->get_id();
				os << "\n";
			}
			os << "\n";
		}
	}
	else throw std::logic_error("Wrong source type!");
} // dodać do factory.hxx


Factory load_factory_structure(std::istream &is) {
	Factory factory;
	std::string line;
	ParsedLineData data;

	ElementID id;
	TimeOffset to;
	TimeOffset pd;

	while (std::getline(is, line)) {
		if (line.empty() || line[0] == ';') continue;
		data = parse_line(line);

		switch (data.element_type) {
			case ElementType::RAMP: {
				id = std::stoi(data.parameters["id"]);
				to = std::stoi(data.parameters["delivery-interval"]);
				Ramp r = Ramp(id, to);
				factory.add_ramp(std::move(r));
				break;
			}

			case ElementType::WORKER: {
				id = std::stoi(data.parameters["id"]);
				pd = std::stoi(data.parameters["processing-time"]);
				
				PackageQueueType qt = string_to_enum(data.parameters["queue-type"]);
				auto q = std::make_unique<PackageQueue>(qt);
				
				Worker w = Worker(id, pd, std::move(q));
				factory.add_worker(std::move(w));
				break;
			}

			case ElementType::STOREHOUSE: {
				id = std::stoi(data.parameters["id"]);
				auto q = std::make_unique<PackageQueue>(PackageQueueType::LIFO); //LIFO jako default, ponieważ nie ma to znaczenia dla storehouse

				Storehouse s = Storehouse(id, std::move(q));
				factory.add_storehouse(std::move(s));
				break;
			}

			case ElementType::LINK: {
				std::string src = data.parameters["src"];
				std::vector<std::string> src_id;

				std::string dest = data.parameters["dest"];
				std::vector<std::string> dest_id;

				std::istringstream src_token(src);
				std::istringstream dest_token(dest);

				while(std::getline(src_token, src, '-')) {
					src_id.push_back(src);
				}

				while(std::getline(dest_token, dest, '-')) {
					dest_id.push_back(dest);
				}
				// src/dest_id[0] - ramp/worker
				// src/dest_id[1] - ID

				id = std::stoi(dest_id[1]);
				IPackageReceiver* __dest;

				if (dest_id[0] == "worker") __dest = &(*factory.find_worker_by_id(id));
				else if (dest_id[0] == "store") __dest = &(*factory.find_storehouse_by_id(id));
				else throw std::logic_error("Wrong destination!");


				id = std::stoi(src_id[1]);

				if (src_id[0] == "ramp") {
					ReceiverPreferences& src = factory.find_ramp_by_id(id)->get_receiver_preferences();
					src.add_receiver(__dest);
				}
				else if (src_id[0] == "worker") {
					ReceiverPreferences& src = factory.find_worker_by_id(id)->get_receiver_preferences();
					if(__dest != &(*factory.find_worker_by_id(id))) src.add_receiver(__dest);
				}
				else throw std::logic_error("Wrong source!");

				break;
			}
		}
	}
	return factory;
}

void save_factory_structure(Factory &factory, std::ostream &os) {
	std::map<ElementID, std::ostringstream> id_streams;
	
	auto concatenate = [&id_streams, &os] () {
	
		for (const auto& [_, s] : id_streams) {
			os << s.str();
		}
		id_streams.clear();
	};

	os << "\n; == LOADING RAMPS ==\n\n";

	for (auto it = factory.ramp_cbegin(); it != factory.ramp_cend(); it++) {
		auto& dstream = id_streams[it->get_id()];

		dstream << "LOADING_RAMP";
		dstream << " id=" << it -> get_id();
		dstream << " delivery-interval=" << it -> get_delivery_interval();
		dstream << "\n";
	}

	concatenate();


	os << "\n; == WORKERS ==\n\n";

	for (auto it = factory.worker_cbegin(); it != factory.worker_cend(); it++) {
		auto& dstream = id_streams[it->get_id()];

		dstream << "WORKER";
		dstream << " id=" << it -> get_id();
		dstream << " processing-time=" << it -> get_processing_duration();
		dstream << " queue-type=" << enum_to_string(it -> get_queue_type());
		dstream << "\n";
	}

	concatenate();

	os << "\n; == STOREHOUSES ==\n\n";

	for (auto it = factory.storehouse_cbegin(); it != factory.storehouse_cend(); it++) {
		auto& dstream = id_streams[it->get_id()];

		dstream << "STOREHOUSE";
		dstream << " id=" << it -> get_id();
		dstream << "\n";
	}

	concatenate();

	os << "\n; == LINKS ==\n";

	linking(factory, os, "ramp");
	linking(factory, os, "worker");


}
ParsedLineData parse_line(std::string &line) {
	ParsedLineData data;

	std::vector<std::string> tokens;
	std::string token;

	std::istringstream token_stream(line);

	while (std::getline(token_stream, token, ' ')) {
		tokens.push_back(token);
	}
	
	const std::string& etype = tokens[0];
	if (etype == "LOADING_RAMP") data.element_type = ElementType::RAMP;
	else if (etype == "WORKER") data.element_type = ElementType::WORKER;
	else if (etype == "STOREHOUSE") data.element_type = ElementType::STOREHOUSE;
	else if (etype == "LINK") data.element_type = ElementType::LINK;
	else throw std::runtime_error("Wrong element type");


	for (auto it = tokens.begin(); it != tokens.end(); it++) {
		if (it == tokens.begin()) continue;
		std::istringstream it_stream(*it);
		
		std::vector<std::string> pairs;

		while (std::getline(it_stream, token, '=')) {
			pairs.push_back(token);
		}
		data.parameters[pairs[0]] = pairs[1];
		pairs.clear();
	}
	
	return data;
}
