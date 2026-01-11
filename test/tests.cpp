#include <gtest/gtest.h>
#include "package.hxx"

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