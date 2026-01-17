#include <cstddef>
#include <nodes.hxx>




void Ramp::deliver_goods(Time t) {}




void Worker::do_work(Time t) {}
Time Worker::get_package_processing_start_time() const {}


void Worker::receive_package(Package&& p) {}






void Storehouse::receive_package(Package&& p) {}




void ReceiverPreferences::add_receiver(IPackageReceiver* r) {
	preferences_t prefs = ReceiverPreferences::get_preferences();
	std::size_t map_size = prefs.size();
	double odds = ReceiverPreferences::pg_();
	for (auto & pair : prefs) pair.second *= map_size;

	prefs.insert({r, odds});

	for (auto &pair : prefs) pair.second /= map_size + 1;
}

void ReceiverPreferences::remove_receiver(IPackageReceiver* r) {
	preferences_t prefs = ReceiverPreferences::get_preferences();
	std::size_t map_size = prefs.size();

	for (auto & pair : prefs) pair.second *= map_size;

	try {
		prefs.erase(r);
		throw("No such receiver");
	}
	catch(std::string s) {
		std::cout << "Error occurred: " << s;
	}	

	for (auto &pair : prefs) pair.second /= map_size - 1;
}

IPackageReceiver* ReceiverPreferences::choose_receiver() {
	double value = ReceiverPreferences::pg_();
	preferences_t prefs = ReceiverPreferences::get_preferences();

	double current_value = 0;
	for (const auto& pair : prefs) {
		if (value >= current_value && value <= current_value + value) {
			return pair.first;
		}
		else {
			current_value += value;
		}
	}
	return nullptr;
}


void PackageSender::send_package() {
	if (!buffer_) return;
	IPackageReceiver* receiver = PackageSender::receiver_preferences_.choose_receiver();

	receiver -> receive_package(std::move(*buffer_));
}


