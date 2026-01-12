#pragma once

#include <list>
#include <memory>
#include "package.hxx"
#include "types.hxx"

//Package to polprodukt z ID

//typ kolejki
enum class PackageQueueType {
    FIFO, //pierwsza ktora przyszla (bierz z dolu)
    LIFO //ostatnia ktora przyszla (bierz z gory)
};

class IPackageStockpile {
    using const_iterator = std::list<Package>::const_iterator;

    // virtual const_iterator begin() const = 0;
    // virtual const_iterator end() const = 0;
    // virtual const_iterator cbegin() const = 0;
    // virtual const_iterator cend() const = 0;
    //
    // virtual ~IPackageStockpile() = default;
};

class IPackageQueue : public IPackageStockpile {

};

class IPackageReceiver {
public:
	void receive_package(Package&& p);
	ElementID get_id() const;

};


//
class PackageQueue: public IPackageQueue {

};



class Ramp {
public:
	Ramp(ElementID id, TimeOffset di): id_(id), di_(di) {};

	void deliver_goods(Time t);
	TimeOffset get_delivery_interval() const;
	ElementID get_id() const;

private:
	ElementID id_;
	TimeOffset di_;
};


class Worker {
public:
	Worker(ElementID id, TimeOffset pd, std::unique_ptr<IPackageQueue> q): id_(id), pd_(pd), q_(q) {}; 
	void do_work(Time t);
	TimeOffset get_processing_duration() const;
	Time get_package_processing_start_time() const;




private:
	ElementID id_;
	TimeOffset pd;
	std::unique_ptr<IPackageQueue> q_;

};


class Storehouse {
public:
	Storehouse(ElementID id, std::unique_ptr<IPackageStockpile> d): id_(id), d_(d) {};



private:
	ElementID id_;
	std::unique_ptr<IPackageStockpile> d_; 
};



class ReceiverPreferences {
	using preferences_t = std::map<IPackageReceiver*, double>;
	using const_iterator = preferences_t::const_iterator;

public:
	ReceiverPreferences(ProbabilityGenerator pg): preferences_t(pg) {};
	
	void add_receiver(IPackageReceiver* r);
	void remove_receiver(IPackageReceiver* r);
	IPackageReceiver* choose_receiver();
	

private:
	ProbabilityGenerator preferences_t;
};





class PackageSender {
public:
	


};


