#include <iostream>
#include "reports.hxx"
#include "nodes.hxx"
#include "factory.hxx"
#include "storage_types.hxx"
#include "types.hxx"
#include <sstream>
#include <stdexcept>


void receivers_id_sorting(const ReceiverPreferences::preferences_t& prefs, std::vector<const IPackageReceiver*> receivers) {
	
	for (const auto& [r, _] : prefs) {
		receivers.push_back(r);
	}

	std::sort(receivers.begin(), receivers.end(),
			[](const IPackageReceiver& a, const IPackageReceiver& b) {
				if (a.get_receiver_type() != b.get_receiver_type()) {
					return a.get_receiver_type() < b.get_receiver_type(); //Worker jest zdefiniowany przed Storehouse w enum class
			}

			return a.get_id() < b.get_id();
		});
}

std::string enum_to_string(ReceiverType type) {
    switch (type) {
        case ReceiverType::WORKER:		return "worker";
        case ReceiverType::STOREHOUSE:	return "storehouse";
    }
    return "unknown";
}
std::string enum_to_string(PackageQueueType type) {
	switch (type) {
		case PackageQueueType::FIFO:	return "FIFO";
		case PackageQueueType::LIFO:	return "LIFO";

	}
	return "unknown";
}

bool SpecificTurnsReportNotifier::should_generate_report(Time t) {
	auto it = turns_.find(t);

	return (it != turns_.cend());
}

bool IntervalReportNotifier::should_generate_report(Time t) {
	if (to_ == 1) return true;

	return (t % to_ == 1);
}


void generate_structure_report(const Factory& f, std::ostream& os) {
	std::vector<const IPackageReceiver*> receivers;
	std::map<ElementID, std::ostringstream> id_streams;


	os << "\n  == LOADING RAMPS ==\n";
	
	auto concatenate = [&id_streams, &os] () {
	
		for (const auto& [_, s] : id_streams) {
			os << s.str();
		}
		id_streams.clear();
	};

	for (auto it = f.ramp_cbegin(); it != f.ramp_cend(); it++) {
		auto& dstream = id_streams[it->get_id()];
		
		
		dstream << "LOADING RAMP #" << it->get_id() << "\n";
		dstream << "  Delivery interval: " << it->get_delivery_interval() << "\n";
		dstream << "  Receivers:\n";

		const auto& prefs = it->get_receiver_preferences().get_preferences();
		receivers_id_sorting(prefs, receivers);

		for (const auto& r : receivers) {
			dstream << "    worker #" << r->get_id() << "\n";
		}

		receivers.clear();

	}

	concatenate();

	os << "\n == WORKERS ==\n";

	for (auto it = f.worker_cbegin(); it != f.worker_cend(); it++) {
		auto& dstream = id_streams[it->get_id()];
		dstream << "WORKER #" << it->get_id() << "\n";
		dstream << "  Processing time: " << it->get_processing_duration() << "\n";
		dstream << "  Queue type: " << enum_to_string(it->get_queue_type()) << "\n";
		dstream << "  Receivers:\n";


		const auto& prefs = it->get_receiver_preferences().get_preferences();
		receivers_id_sorting(prefs, receivers);

		for (const auto& r : receivers) {
			dstream << "    " << enum_to_string(r->get_receiver_type());
			dstream << " #" << r->get_id() << "\n";

		}
		receivers.clear();
	}

	concatenate();

	os << "\n == STOREHOUSES ==\n";

	for (auto it = f.storehouse_cbegin(); it != f.storehouse_cend(); it++) {
		auto& dstream = id_streams[it->get_id()];
		dstream << "STOREHOUSE #" << it->get_id() << "\n";
	}

	concatenate();
}


void generate_structure_turn_report(const Factory& f, std::ostream& os, Time t) {
	std::map<ElementID, std::ostringstream> id_streams;

	os << "=== [ Turn: " << t << " ] ===";
	

	auto concatenate = [&id_streams, &os] () {
	
		for (const auto& [_, s] : id_streams) {
			os << s.str();
		}
		id_streams.clear();
	};

	os << "\n == WORKERS ==\n";

	for (auto it = f.worker_cbegin(); it != f.worker_cend(); it++) {
		auto& dstream = id_streams[it->get_id()];
		dstream << "WORKER #" << it->get_id() << "\n";
		dstream << "  PBuffer: ";
		
		const std::optional<Package>& cbuff = it -> get_current_buffer();
		if (cbuff.has_value()) {
			dstream << "#" << cbuff->get_id() << " (pt = " << t % it -> get_package_processing_start_time() << ")\n";
		}
		else {
			dstream << "(empty)\n";
		}

		dstream << "  Queue: ";
		
		if (it -> cbegin() == it -> cend()) dstream << "(empty)\n";
		else {
			for(auto __it = it -> cbegin(); __it != it -> cend(); __it++) {
				dstream << "#" << __it->get_id();
				if (__it != std::prev(it -> cend())) dstream << ",";
			}
			dstream << "\n";
		}

		dstream << "  SBuffer: ";
		const std::optional<Package>& sbuff = it -> get_sending_buffer();
		if (sbuff.has_value()) {
			dstream << "#" << cbuff->get_id() << "\n";
		}
		else {
			dstream << "(empty)\n";
		}
	}

	concatenate();

	os << "\n == STOREHOUSES ==\n";

	for (auto it = f.storehouse_cbegin(); it != f.storehouse_cend(); it++) {
		auto& dstream = id_streams[it->get_id()];
		dstream << "STOREHOUSE #" << it->get_id() << "\n";
		dstream << "  Stock: ";

		if (it -> cbegin() == it -> cend()) dstream << "(empty)\n";
		else {
			for(auto __it = it -> cbegin(); __it != it -> cend(); __it++) {
				dstream << "#" << __it->get_id();
				if (__it != std::prev(it -> cend())) dstream << ",";
			}
			dstream << "\n";
		}
	}

	concatenate();
}


PackageQueueType string_to_enum(std::string type) {
	if (type == "LIFO") return PackageQueueType::LIFO;
	else if (type == "FIFO") return PackageQueueType::FIFO;
	else throw std::logic_error("Wrong type");

}
