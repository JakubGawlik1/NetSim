#include <nodes.hxx>



class Ramp {

	void deliver_goods(Time t);

	Ramp::~Ramp() = default;


class Worker : public IPackageReceiver {

	void do_work(Time t);
	Time get_package_processing_start_time() const;


	void receive_package(Package&& p) override;

	~Worker() = default;





	void receive_package(Package&& p) override;

	~Storehouse() = default;



void ReceiverPreferences::add_receiver(IPackageReceiver* r);
void ReceiverPreferences::remove_receiver(IPackageReceiver* r);
IPackageReceiver* ReceiverPreferences::choose_receiver();
preferences_t& ReceiverPreferences::get_preferences() const; 







void PackageSender::send_package();
std::optional<Package>& PackageSender::get_sending_buffer() const;
void PackageSender::push_package(Package&& p);


