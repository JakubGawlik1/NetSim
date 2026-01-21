#include <nodes.hxx>
#include <optional>
#include <stdexcept>




void Ramp::deliver_goods(Time t) {
	if (!producing_start_time_) producing_start_time_ = t;
	
	if(t - *producing_start_time_ + 1 >= di_) {
		producing_start_time_.reset();
		PackageSender_.push_package(Package());
	}
}



void Worker::do_work(Time t) {
	if (!buffer_) {
		buffer_ = q_->pop();
		processing_start_time_ = t;
	}

	if(t - processing_start_time_ + 1 >= pd_) { // + 1 ponieważ przetwarzanie rozpoczyna się jeszcze w tej samej turze
		PackageSender_.push_package(std::move(*buffer_));
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
	auto prefs = ReceiverPreferences::get_preferences();

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
	IPackageReceiver* receiver = PackageSender::receiver_preferences.choose_receiver();

	receiver -> receive_package(std::move(*buffer_));
	buffer_.reset();
}


