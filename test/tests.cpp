#include <gtest/gtest.h>
#include "package.hxx"
#include "storage_types.hxx"

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