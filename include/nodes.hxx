#pragma once

#include <list>
#include <memory>
#include "package.hxx"
#include "types.hxx"
#include "storage_types.hxx"
#include <map>

class IPackageQueue;
class IPackageStockpile;


enum class ReceiverType {

};


class IPackageReceiver {
public:
	void receive_package(Package&& p);
	ElementID get_id() const;

};


//

class Ramp {
public:
	explicit Ramp(ElementID id, TimeOffset di): id_(id), di_(di) {};

	void deliver_goods(Time t);
	TimeOffset get_delivery_interval() const;
	ElementID get_id() const;

private:
	ElementID id_;
	TimeOffset di_;
};


class Worker {
public:
	explicit Worker(ElementID id, TimeOffset pd, std::unique_ptr<IPackageQueue> q): id_(id), pd_(pd), q_(std::move(q)) {}; 
	void do_work(Time t);
	TimeOffset get_processing_duration() const;
	Time get_package_processing_start_time() const;




private:
	ElementID id_;
	TimeOffset pd_;
	std::unique_ptr<IPackageQueue> q_;

};


class Storehouse {
public:
	explicit Storehouse(ElementID id, std::unique_ptr<IPackageStockpile> d): id_(id), d_(std::move(d)) {};



private:
	ElementID id_;
	std::unique_ptr<IPackageStockpile> d_; 
};



class ReceiverPreferences {
	using preferences_t = std::map<IPackageReceiver*, double>;
	using const_iterator = preferences_t::const_iterator;

public:
	ReceiverPreferences(ProbabilityGenerator pg): pg_(pg) {};

	void add_receiver(IPackageReceiver* r);
	void remove_receiver(IPackageReceiver* r);
	IPackageReceiver* choose_receiver();


private:
	ProbabilityGenerator pg_;
	std::vector<IPackageReceiver*> r_;
};





class PackageSender {
public:
	


};


