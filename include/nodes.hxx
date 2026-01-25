#pragma once

#include <memory>
#include <map>
#include <optional>
#include "helpers.hxx"
#include "types.hxx"
#include "package.hxx"
#include "storage_types.hxx"



enum class ReceiverType {
	WORKER,
	STOREHOUSE
};


class IPackageReceiver {
public:
	virtual void receive_package(Package&& p) = 0;
	virtual ElementID get_id() const = 0;
	virtual ReceiverType get_receiver_type() const = 0;
	
	virtual IPackageStockpile::const_iterator cbegin() const = 0;
	virtual IPackageStockpile::const_iterator cend() const = 0;

	virtual ~IPackageReceiver() = default;
};


class ReceiverPreferences {
public:
	using preferences_t = std::map<IPackageReceiver*, double>;
	using const_iterator = preferences_t::const_iterator;

	ReceiverPreferences(ProbabilityGenerator pg = probability_generator): pg_(pg) {};


	void add_receiver(IPackageReceiver* r);
	void remove_receiver(IPackageReceiver* r);
	IPackageReceiver* choose_receiver();
	const preferences_t& get_preferences() const { return r_preferences_; }

	~ReceiverPreferences() = default;

private:
	ProbabilityGenerator pg_;
	preferences_t r_preferences_;
	double ratio_ = 1; //aktualny mnożnik potrzebny do zsumowania pradopodobieństw do 1

};


class PackageSender {
public:
	ReceiverPreferences receiver_preferences;

	PackageSender() = default; 
	explicit PackageSender(ReceiverPreferences prefs): receiver_preferences(std::move(prefs)) {} // na razie zbędne, może się przyda w przyszłości


	PackageSender(PackageSender&&) = default;
	PackageSender& operator = (PackageSender&&) = default;



	PackageSender(const PackageSender&) = delete;
	PackageSender& operator=(const PackageSender&) = delete;

	const std::optional<Package>& get_sending_buffer() const { return buffer_; }
	void push_package(Package&& p) { buffer_ = std::move(p); }
	void send_package();

	~PackageSender() = default;

private:
	std::optional<Package> buffer_;
};


//

class Ramp {
public:
	explicit Ramp(ElementID id, TimeOffset di): id_(id), di_(di) {};
	Ramp(Ramp&&) = default;

	void deliver_goods(Time t);
	TimeOffset get_delivery_interval() const { return di_; }
	ElementID get_id() const { return id_; }

	ReceiverPreferences& get_receiver_preferences() { return PackageSender_.receiver_preferences; }
	const ReceiverPreferences& get_receiver_preferences() const { return PackageSender_.receiver_preferences; }
	
	const PackageSender& get_package_sender() const { return PackageSender_; }

	void send_package() {PackageSender_.send_package(); }
	const std::optional<Package>& get_sending_buffer() const {return PackageSender_.get_sending_buffer(); }

	~Ramp() = default;

private:
	ElementID id_;
	TimeOffset di_;
	std::optional<Time> producing_start_time_;
	PackageSender PackageSender_;

};


class Worker : public IPackageReceiver {
public:
	explicit Worker(ElementID id, TimeOffset pd, std::unique_ptr<IPackageQueue> q): id_(id), pd_(pd), q_(std::move(q)) {}
	Worker(Worker&&) = default;

	IPackageStockpile::const_iterator cbegin() const override { return q_->cbegin(); }
	IPackageStockpile::const_iterator cend() const override { return q_->cend(); }

	ReceiverPreferences& get_receiver_preferences() { return PackageSender_.receiver_preferences; }
	const ReceiverPreferences& get_receiver_preferences() const { return PackageSender_.receiver_preferences; }

	const PackageSender& get_package_sender() const { return PackageSender_; }

	void do_work(Time t);
	TimeOffset get_processing_duration() const { return pd_; }
	Time get_package_processing_start_time() const { return processing_start_time_; }


	void receive_package(Package&& p) override;
	ElementID get_id() const override {return id_; }
	ReceiverType get_receiver_type() const override { return ReceiverType::WORKER; }


	const std::optional<Package>& get_sending_buffer() const {return PackageSender_.get_sending_buffer(); }
	
	const std::optional<Package>& get_current_buffer() const { return buffer_; }
	PackageQueueType get_queue_type() const { return q_->get_queue_type(); }

	void send_package() {PackageSender_.send_package(); }
	~Worker() = default;

private:
	ElementID id_;
	Time processing_start_time_;
	TimeOffset pd_;
	std::unique_ptr<IPackageQueue> q_;
	std::optional<Package> buffer_;
	PackageSender PackageSender_;

};


class Storehouse : public IPackageReceiver {
public:
	explicit Storehouse(ElementID id, std::unique_ptr<IPackageStockpile> d = std::make_unique<PackageQueue>(PackageQueueType::FIFO)): id_(id), d_(std::move(d)) {};
	Storehouse(Storehouse&&) = default;

	ReceiverType get_receiver_type() const override { return ReceiverType::STOREHOUSE; }

	void receive_package(Package&& p) override;
	ElementID get_id() const override {return id_; }

	IPackageStockpile::const_iterator cbegin() const override { return d_->cbegin(); }
	IPackageStockpile::const_iterator cend() const override { return d_->cend(); }


	~Storehouse() = default;

private:
	ElementID id_;
	std::unique_ptr<IPackageStockpile> d_; 
};









