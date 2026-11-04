#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "IMinesweeperGame.h"

TEST(SanityCheck, GMockWorks) {
    EXPECT_EQ(1 + 1, 2);
    EXPECT_THAT(2, testing::Eq(2));
}