#include <nodes.hxx>
#include <stdexcept>




void Ramp::deliver_goods(Time t) {
	if(t % di_ == 0) {
		PackageSender_.push_package(Package());
	}
}



void Worker::do_work(Time t) {
	if(t % pd_ == 0) {
		processing_start_time_ = t;
		PackageSender_.push_package(q_->pop());
	}

}


void Worker::receive_package(Package&& p) {
	q_->push(std::move(p));
}

void Storehouse::receive_package(Package&& p) {
	d_->push(std::move(p));
}

void ReceiverPreferences::add_receiver(IPackageReceiver* r) {
	auto& prefs = r_preferences_;
	double odds = ReceiverPreferences::pg_();

	for (auto & [_, weight] : prefs) {
		weight *= ratio_;
	}

	prefs.insert({r, odds});

	ratio_ = 0;
	for (const auto & [_, weight] : prefs) {
		ratio_ += weight;
	}

	for (auto & [_, weight] : prefs) {
		weight *= 1.0/ratio_;
	}
}

void ReceiverPreferences::remove_receiver(IPackageReceiver* r) {
	auto& prefs = r_preferences_;
	double ratio = 0;

	if (prefs.erase(r) == 0) {
		throw std::runtime_error("No such receiver");
	}

	for (const auto & [_, weight] : prefs) {
		ratio += weight;
	}

	for (auto & [_, weight] : prefs) {
		weight *= 1.0/ratio;
	}

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


