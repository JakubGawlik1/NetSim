#include <gtest/gtest.h>
#include "package.hxx"

TEST(SanityTest, BasicMathWorks)
{
    EXPECT_EQ(2 + 2, 4);
}

TEST(New_Product,IsIDCorrect) {
    Package p1(1);
    Package p2(3);
    EXPECT_EQ(p1.get_id(),1);
    EXPECT_EQ(p2.get_id(),3);

}