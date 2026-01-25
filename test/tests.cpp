#include <fstream>
#include <functional>
#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include "package.hxx"
#include "reports.hxx"
#include "storage_types.hxx"
#include "nodes.hxx"
#include "factory.hxx"
#include "simulation.hxx"

TEST(SanityTest, BasicMathWorks)
{
    EXPECT_EQ(2 + 2, 4);
}

TEST(PackageTest, NewPackagesHaveDifferentIDs) {
    Package p1;
    Package p2;

    EXPECT_NE(p1.get_id(), p2.get_id());
}

TEST(PackageTest, FirstPackageHasIDOne) {
    Package p1;
    Package p2;
    Package p3;

    EXPECT_EQ(p1.get_id(), 1);
    EXPECT_EQ(p2.get_id(), 2);
    EXPECT_EQ(p3.get_id(), 3);
}

TEST(PackageTest, FreedIDIsReused) {
    ElementID freed_id;
    {
        Package p;
        freed_id = p.get_id();
    }

    Package p2;
    EXPECT_EQ(p2.get_id(), freed_id);
}

TEST(PackageTest, MoveConstructorTransfersID) {
    Package p1;
    ElementID id = p1.get_id();

    Package p2(std::move(p1));

    EXPECT_EQ(p2.get_id(), id);
}

TEST(StorageTypesTest,QueueType) {
    PackageQueue q1(PackageQueueType::FIFO);
    PackageQueue q2(PackageQueueType::LIFO);
    EXPECT_EQ(q1.get_queue_type(),PackageQueueType::FIFO);
    EXPECT_EQ(q2.get_queue_type(),PackageQueueType::LIFO);

}

TEST(PackageQueueTest, PushIncreasesSize) {
    PackageQueue q(PackageQueueType::FIFO);
    EXPECT_TRUE(q.empty());
    q.push(Package{});
    EXPECT_FALSE(q.empty());
    EXPECT_EQ(q.size(), 1);
    q.push(Package{});
    EXPECT_EQ(q.size(), 2);
}

TEST(PackageQueueTest, LIFOOrderIsCorrect) {
    PackageQueue q(PackageQueueType::LIFO);

    Package p1;
    Package p2;
    Package p3;

    auto id1 = p1.get_id();
    auto id2 = p2.get_id();
    auto id3 = p3.get_id();

    q.push(std::move(p1));
    q.push(std::move(p2));
    q.push(std::move(p3));

    EXPECT_EQ(q.pop().get_id(), id3);
    EXPECT_EQ(q.pop().get_id(), id2);
    EXPECT_EQ(q.pop().get_id(), id1);
}

TEST(PackageQueueTest, FIFOOrderIsCorrect) {
    PackageQueue q(PackageQueueType::FIFO);

    Package p1;
    Package p2;
    Package p3;

    auto id1 = p1.get_id();
    auto id2 = p2.get_id();
    auto id3 = p3.get_id();

    q.push(std::move(p1));
    q.push(std::move(p2));
    q.push(std::move(p3));

    EXPECT_EQ(q.pop().get_id(), id1);
    EXPECT_EQ(q.pop().get_id(), id2);
    EXPECT_EQ(q.pop().get_id(), id3);
}

TEST(PackageQueueTest, PopOnEmptyQueueThrows) {
    PackageQueue q(PackageQueueType::FIFO);

    EXPECT_THROW(q.pop(), std::runtime_error);
}

TEST(PackageSenderTest, BufferClearAfterSending) {
	PackageSender ps;
	Package p;
	std::unique_ptr<PackageQueue> q = std::make_unique<PackageQueue>(PackageQueueType::FIFO);

	auto w = std::make_unique<Worker>(1, 1, std::move(q));
	ps.receiver_preferences.add_receiver(w.get());
	ps.push_package(std::move(p));

	ps.send_package();
	EXPECT_EQ(ps.get_sending_buffer(), std::nullopt);
	EXPECT_FALSE(ps.get_sending_buffer().has_value());
}

double func1() {
	return 0.4;
}

double func2() {
	return 0.6;
}

TEST(ReceiverPreferencesTest, ProbabilityScaling) {
	std::function<double()> testFunc = func1;
	ReceiverPreferences rp = ReceiverPreferences(testFunc);

	
	std::unique_ptr<PackageQueue> q1 = std::make_unique<PackageQueue>(PackageQueueType::FIFO);
	std::unique_ptr<PackageQueue> q2 = std::make_unique<PackageQueue>(PackageQueueType::FIFO);

	auto w1 = std::make_unique<Worker>(1, 1, std::move(q1));
	auto w2 = std::make_unique<Worker>(1, 1, std::move(q2));

	auto Pw1 = w1.get();
	auto Pw2 = w2.get();

	const auto& prefs = rp.get_preferences();

	rp.add_receiver(Pw1);
	EXPECT_DOUBLE_EQ(prefs.at(Pw1), 1.0);

	rp.add_receiver(Pw2);
	EXPECT_DOUBLE_EQ(prefs.at(Pw1), 0.5);
	EXPECT_DOUBLE_EQ(prefs.at(Pw2), 0.5);

	rp.remove_receiver(Pw1);
	EXPECT_DOUBLE_EQ(prefs.at(Pw2), 1.0);

}

TEST(ReceiverPreferencesTest, ChoosingReceiver) {

	std::function<double()> testFunc1 = func1;
	std::function<double()> testFunc2 = func2;
	ReceiverPreferences rp1 = ReceiverPreferences(testFunc1);
	ReceiverPreferences rp2 = ReceiverPreferences(testFunc2);

	std::unique_ptr<PackageQueue> q1 = std::make_unique<PackageQueue>(PackageQueueType::FIFO);
	std::unique_ptr<PackageQueue> q2 = std::make_unique<PackageQueue>(PackageQueueType::FIFO);

	auto w1 = std::make_unique<Worker>(1, 1, std::move(q1));
	auto w2 = std::make_unique<Worker>(1, 1, std::move(q2));

	auto Pw1 = w1.get();
	auto Pw2 = w2.get();

	const auto& prefs1 = rp1.get_preferences();
	const auto& prefs2 = rp2.get_preferences();

	rp1.add_receiver(Pw1);
	rp1.add_receiver(Pw2);

	rp2.add_receiver(Pw1);
	rp2.add_receiver(Pw2);

	for (std::size_t i = 0; i < 10; i++) {
		EXPECT_EQ(rp1.choose_receiver(), Pw1);
		EXPECT_EQ(rp2.choose_receiver(), Pw2);
	}
}

TEST(RampTest, DeliverGoods) {
	Ramp r = Ramp(1, 2);
	r.deliver_goods(1);
	
	EXPECT_FALSE(r.get_sending_buffer().has_value());
	
	r.deliver_goods(2);

	EXPECT_TRUE(r.get_sending_buffer().has_value());
}

TEST(WorkerTest, IsReceivedProperly) {

	std::unique_ptr<PackageQueue> q = std::make_unique<PackageQueue>(PackageQueueType::FIFO);

	auto w = std::make_unique<Worker>(1, 1, std::move(q));
	Package p1;
	Package p2;
	Package p3;
	unsigned int i;
	std::vector<Package*> p_vec = {&p1, &p2, &p3};

	w->receive_package(std::move(p1));
	i = 1;

	for (auto it = w->cbegin(); it != w->cend(); it++, i++) {
		EXPECT_EQ(it->get_id(), i);
	}

	w->receive_package(std::move(p2));
	i = 1;

	for (auto it = w->cbegin(); it != w->cend(); it++, i++) {
		EXPECT_EQ(it->get_id(), i);
	}
	w->receive_package(std::move(p3));

	i = 1;
	for (auto it = w->cbegin(); it != w->cend(); it++, i++) {
		EXPECT_EQ(it->get_id(), i);
	}
}

TEST(WorkerTest, IsWorkingProperly) {
	
	std::unique_ptr<PackageQueue> q = std::make_unique<PackageQueue>(PackageQueueType::LIFO);

	auto w = std::make_unique<Worker>(1, 2, std::move(q));
	
	Package p1;
	Package p2;
	Package p3;

	w->receive_package(std::move(p1));
	w->receive_package(std::move(p2));

	w->do_work(1);
	EXPECT_EQ(w->get_current_buffer().value().get_id(), 2);
	EXPECT_FALSE(w->get_sending_buffer());

	w->receive_package(std::move(p3));

	w->do_work(2);
	EXPECT_FALSE(w->get_current_buffer());
	EXPECT_EQ(w->get_sending_buffer().value().get_id(), 2);


}

TEST(StorehouseTest, IsReceivedProperly) {
	std::unique_ptr<PackageQueue> q1 = std::make_unique<PackageQueue>(PackageQueueType::FIFO);

	Storehouse sh = Storehouse(1, std::move(q1));

	Package p1;
	Package p2;
	unsigned int i;
	sh.receive_package(std::move(p1));
	sh.receive_package(std::move(p2));

	i = 1;

	for (auto it = sh.cbegin(); it != sh.cend(); it++, i++) {
		EXPECT_EQ(it->get_id(), i);
	}
	
}

static std::unique_ptr<IPackageQueue> make_queue(PackageQueueType t = PackageQueueType::FIFO) {
    return std::make_unique<PackageQueue>(t);
}
static std::unique_ptr<IPackageStockpile> make_stockpile() {
    return std::make_unique<PackageQueue>(PackageQueueType::FIFO);
}

TEST(NodeCollectionTest, FindByIdReturnsCorrectIterator) {
    NodeCollection<Ramp> c;
    c.add(Ramp(1, 1));
    c.add(Ramp(7, 1));
    c.add(Ramp(3, 1));

    auto it = c.find_by_id(7);
    ASSERT_NE(it, c.end());
    EXPECT_EQ(it->get_id(), 7);

    auto it2 = c.find_by_id(999);
    EXPECT_EQ(it2, c.end());
}

TEST(NodeCollectionTest, RemoveByIdRemovesCorrectElement) {
    NodeCollection<Ramp> c;
    c.add(Ramp(1, 1));
    c.add(Ramp(7, 1));
    c.add(Ramp(3, 1));

    c.remove_by_id(7);

    EXPECT_EQ(c.find_by_id(7), c.end());
    EXPECT_NE(c.find_by_id(1), c.end());
    EXPECT_NE(c.find_by_id(3), c.end());
}

TEST(NodeCollectionTest, ConstFindByIdWorks) {
    NodeCollection<Ramp> c;
    c.add(Ramp(10, 1));
    c.add(Ramp(20, 1));

    const auto& cc = c;

    auto it = cc.find_by_id(20);
    ASSERT_NE(it, cc.cend());
    EXPECT_EQ(it->get_id(), 20);

    auto it2 = cc.find_by_id(999);
    EXPECT_EQ(it2, cc.cend());
}

TEST(FactoryTest, AddAndFindRampWorkerStorehouse) {
    Factory f;

    f.add_ramp(Ramp(1, 3));
    f.add_worker(Worker(2, 2, make_queue(PackageQueueType::FIFO)));
    f.add_storehouse(Storehouse(3, make_stockpile()));

    EXPECT_NE(f.find_ramp_by_id(1), f.ramp_cend());
    EXPECT_NE(f.find_worker_by_id(2), f.worker_cend());
    EXPECT_NE(f.find_storehouse_by_id(3), f.storehouse_cend());

    EXPECT_EQ(f.find_ramp_by_id(999), f.ramp_cend());
    EXPECT_EQ(f.find_worker_by_id(999), f.worker_cend());
    EXPECT_EQ(f.find_storehouse_by_id(999), f.storehouse_cend());
}

TEST(FactoryTest, IsConsistentFalseWhenRampHasNoReceivers) {
    Factory f;
    f.add_ramp(Ramp(1, 1));
    f.add_storehouse(Storehouse(1, make_stockpile()));

    EXPECT_FALSE(f.is_consistent());
}

TEST(FactoryTest, IsConsistentTrueForRampToStorehouse) {
    Factory f;

    f.add_ramp(Ramp(1, 1));
    f.add_storehouse(Storehouse(1, make_stockpile()));

    auto r_it = f.find_ramp_by_id(1);
    auto s_it = f.find_storehouse_by_id(1);
    ASSERT_NE(r_it, f.ramp_cend());
    ASSERT_NE(s_it, f.storehouse_cend());

    r_it->get_receiver_preferences().add_receiver(&(*s_it));

    EXPECT_TRUE(f.is_consistent());
}

TEST(FactoryTest, IsConsistentTrueForRampToWorkerToStorehouse) {
    Factory f;

    f.add_ramp(Ramp(1, 1));
    f.add_worker(Worker(1, 1, make_queue(PackageQueueType::FIFO)));
    f.add_storehouse(Storehouse(1, make_stockpile()));

    auto r_it = f.find_ramp_by_id(1);
    auto w_it = f.find_worker_by_id(1);
    auto s_it = f.find_storehouse_by_id(1);

    ASSERT_NE(r_it, f.ramp_cend());
    ASSERT_NE(w_it, f.worker_cend());
    ASSERT_NE(s_it, f.storehouse_cend());

    r_it->get_receiver_preferences().add_receiver(&(*w_it));
    w_it->get_receiver_preferences().add_receiver(&(*s_it));

    EXPECT_TRUE(f.is_consistent());
}

TEST(FactoryTest, RemoveWorkerRemovesConnectionsFromRampsAndWorkers) {
    Factory f;

    // ramp(1) -> worker(2)
    // worker(3) -> worker(2)
    f.add_ramp(Ramp(1, 1));
    f.add_worker(Worker(2, 1, make_queue()));
    f.add_worker(Worker(3, 1, make_queue()));

    auto r_it = f.find_ramp_by_id(1);
    auto w2_it = f.find_worker_by_id(2);
    auto w3_it = f.find_worker_by_id(3);

    ASSERT_NE(r_it, f.ramp_cend());
    ASSERT_NE(w2_it, f.worker_cend());
    ASSERT_NE(w3_it, f.worker_cend());

    IPackageReceiver* w2_ptr = &(*w2_it);

    r_it->get_receiver_preferences().add_receiver(w2_ptr);
    w3_it->get_receiver_preferences().add_receiver(w2_ptr);

    EXPECT_NE(r_it->get_receiver_preferences().get_preferences().find(w2_ptr),
              r_it->get_receiver_preferences().get_preferences().end());
    EXPECT_NE(w3_it->get_receiver_preferences().get_preferences().find(w2_ptr),
              w3_it->get_receiver_preferences().get_preferences().end());

    f.remove_worker(2);

    EXPECT_EQ(f.find_worker_by_id(2), f.worker_cend());

    r_it = f.find_ramp_by_id(1);
    w3_it = f.find_worker_by_id(3);
    ASSERT_NE(r_it, f.ramp_cend());
    ASSERT_NE(w3_it, f.worker_cend());

    EXPECT_EQ(r_it->get_receiver_preferences().get_preferences().find(w2_ptr),
              r_it->get_receiver_preferences().get_preferences().end());
    EXPECT_EQ(w3_it->get_receiver_preferences().get_preferences().find(w2_ptr),
              w3_it->get_receiver_preferences().get_preferences().end());
}

TEST(FactoryTest, RemoveStorehouseRemovesConnectionsFromRampsAndWorkers) {
    Factory f;

    f.add_ramp(Ramp(1, 1));
    f.add_worker(Worker(2, 1, make_queue()));
    f.add_storehouse(Storehouse(5, make_stockpile()));

    auto r_it = f.find_ramp_by_id(1);
    auto w_it = f.find_worker_by_id(2);
    auto s_it = f.find_storehouse_by_id(5);

    ASSERT_NE(r_it, f.ramp_cend());
    ASSERT_NE(w_it, f.worker_cend());
    ASSERT_NE(s_it, f.storehouse_cend());

    IPackageReceiver* s_ptr = &(*s_it);

    r_it->get_receiver_preferences().add_receiver(s_ptr);
    w_it->get_receiver_preferences().add_receiver(s_ptr);

    EXPECT_NE(r_it->get_receiver_preferences().get_preferences().find(s_ptr),
              r_it->get_receiver_preferences().get_preferences().end());
    EXPECT_NE(w_it->get_receiver_preferences().get_preferences().find(s_ptr),
              w_it->get_receiver_preferences().get_preferences().end());

    f.remove_storehouse(5);

    EXPECT_EQ(f.find_storehouse_by_id(5), f.storehouse_cend());

    r_it = f.find_ramp_by_id(1);
    w_it = f.find_worker_by_id(2);
    ASSERT_NE(r_it, f.ramp_cend());
    ASSERT_NE(w_it, f.worker_cend());

    EXPECT_EQ(r_it->get_receiver_preferences().get_preferences().find(s_ptr),
              r_it->get_receiver_preferences().get_preferences().end());
    EXPECT_EQ(w_it->get_receiver_preferences().get_preferences().find(s_ptr),
              w_it->get_receiver_preferences().get_preferences().end());
}

TEST(LoadSaveTest, LoadFactoryStructure) {
	std::ifstream file("factory.txt");

	if (!file.is_open()) throw std::runtime_error("Cannot open factory.txt");

	Factory f = load_factory_structure(file);

	
	auto rit = f.find_ramp_by_id(1);

	ASSERT_NE(rit, f.ramp_cend());
	EXPECT_EQ(rit->get_id(), 1);

	rit = f.find_ramp_by_id(2);

	ASSERT_NE(rit, f.ramp_cend());
	EXPECT_EQ(rit->get_id(), 2);


	auto wit = f.find_worker_by_id(1);

	ASSERT_NE(wit, f.worker_cend());
	EXPECT_EQ(wit->get_id(), 1);
	

	wit = f.find_worker_by_id(2);

	ASSERT_NE(wit, f.worker_cend());
	EXPECT_EQ(wit->get_id(), 2);
	
	auto sit = f.find_storehouse_by_id(1);

	ASSERT_NE(sit, f.storehouse_cend());
	EXPECT_EQ(sit->get_id(), 1);

	


	EXPECT_TRUE(f.is_consistent());

}

TEST(LoadSaveTest, SaveFactoryStructure) {

	std::ifstream file("factory.txt");

	if (!file.is_open()) throw std::runtime_error("Cannot open factory.txt");

	Factory f = load_factory_structure(file);

	std::ofstream save_file1("output1.txt");
	if (!save_file1.is_open()) throw std::runtime_error("Cannot open output1.txt");

	std::ofstream save_file2("output2.txt");
	if (!save_file2.is_open()) throw std::runtime_error("Cannot open output2.txt");

	std::ofstream save_file3("output3.txt");
	if (!save_file3.is_open()) throw std::runtime_error("Cannot open output3.txt");

	save_factory_structure(f, save_file1);
	generate_structure_report(f, save_file2);
	generate_structure_turn_report(f, save_file3, 1);

}

TEST(ReportNotifierTest, IsSpecificTurnCorrect) {
	SpecificTurnsReportNotifier notifier = SpecificTurnsReportNotifier(std::set<Time>{2,3,5});
	
	EXPECT_TRUE(notifier.should_generate_report(2));
	EXPECT_TRUE(notifier.should_generate_report(3));
	EXPECT_TRUE(notifier.should_generate_report(5));

	EXPECT_FALSE(notifier.should_generate_report(1));
	EXPECT_FALSE(notifier.should_generate_report(4));
}

TEST(ReportNotifierTest, IsIntervalCorrect) {
	IntervalReportNotifier notifier = IntervalReportNotifier(2);
	
	EXPECT_TRUE(notifier.should_generate_report(1));
	EXPECT_TRUE(notifier.should_generate_report(3));
	EXPECT_TRUE(notifier.should_generate_report(5));

	EXPECT_FALSE(notifier.should_generate_report(2));
	EXPECT_FALSE(notifier.should_generate_report(4));
}

TEST(SimulationTest, IsRunningCorrectly) {

	std::ifstream file("factory.txt");

	if (!file.is_open()) throw std::runtime_error("Cannot open factory.txt");

	Factory factory = load_factory_structure(file);

	SpecificTurnsReportNotifier spec_notifier(std::set<Time>{1,6});
	generate_structure_report(factory, std::cout);
	std::cout << "\n\n";
	simulate(factory, 6, [&spec_notifier](Factory& f, Time t_offset) {
		if (spec_notifier.should_generate_report(t_offset)) {
			generate_structure_turn_report(f, std::cout, t_offset);
		}
	});
}
