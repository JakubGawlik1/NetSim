#include <functional>
#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include "package.hxx"
#include "storage_types.hxx"
#include "nodes.hxx"
#include "factory.hxx"

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
