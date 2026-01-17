#pragma once

#include <iostream>
#include <list>
#include <memory>
#include <map>
#include <optional>
#include "helpers.hxx"
#include "package.hxx"
#include "types.hxx"
#include "storage_types.hxx"


class IPackageQueue;
class IPackageStockpile;
class PackageSender;


enum class ReceiverType {
	WORKER,
	STOREHOUSE
};


class IPackageReceiver {
public:
	virtual void receive_package(Package&& p) = 0;
	virtual ElementID get_id() const = 0;
	virtual ReceiverType get_receiver_type() const = 0;

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

};


class PackageSender {
public:
	PackageSender() = default; 
	explicit PackageSender(ReceiverPreferences prefs): receiver_preferences_(std::move(prefs)) {} // na razie zbędne, może się przyda w przyszłości


	PackageSender(PackageSender&&) = default;
	PackageSender& operator = (PackageSender&&) = default;



	PackageSender(const PackageSender&) = delete;
	PackageSender& operator=(const PackageSender&) = delete;

	const std::optional<Package>& get_sending_buffer() const { return buffer_; }
	void push_package(Package&& p) { buffer_ = std::move(p); }
	void send_package();

	~PackageSender() = default;

private:
	ReceiverPreferences receiver_preferences_;
	std::optional<Package> buffer_;
};


//

class Ramp {
public:
	explicit Ramp(ElementID id, TimeOffset di): id_(id), di_(di) {};

	void deliver_goods(Time t);
	TimeOffset get_delivery_interval() const { return di_; }
	ElementID get_id() const { return id_; }

	~Ramp() = default;
private:
	ElementID id_;
	TimeOffset di_;
	PackageSender PackageSender_;

};


class Worker : public IPackageReceiver {
public:
	explicit Worker(ElementID id, TimeOffset pd, std::unique_ptr<IPackageQueue> q): id_(id), pd_(pd), q_(std::move(q)) {}

	void do_work(Time t);
	TimeOffset get_processing_duration() const { return pd_; }
	Time get_package_processing_start_time() const;


	void receive_package(Package&& p) override;
	ElementID get_id() const override {return id_; }
	ReceiverType get_receiver_type() const override { return ReceiverType::WORKER; }

	~Worker() = default;

private:
	ElementID id_;
	TimeOffset pd_;
	std::unique_ptr<IPackageQueue> q_;

};


class Storehouse : public IPackageReceiver {
public:
	explicit Storehouse(ElementID id, std::unique_ptr<IPackageStockpile> d): id_(id), d_(std::move(d)) {};


	ReceiverType get_receiver_type() const override { return ReceiverType::STOREHOUSE; }

	void receive_package(Package&& p) override;
	ElementID get_id() const override {return id_; }

	~Storehouse() = default;

private:
	ElementID id_;
	std::unique_ptr<IPackageStockpile> d_; 
};









