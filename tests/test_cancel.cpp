#include "matching_engine.hpp"
#include <gtest/gtest.h>

TEST(CancelTest, CancelBeforeMatch_ReturnsTrue) {
    MatchingEngine e; e.submit_limit(1, Side::Buy, 10025, 10); EXPECT_TRUE(e.cancel(1));
}
TEST(CancelTest, CancelBeforeMatch_RemovesFromBook) {
    MatchingEngine e; e.submit_limit(1, Side::Buy, 10025, 10); e.cancel(1);
    EXPECT_FALSE(e.best_bid().has_value());
}
TEST(CancelTest, CancelNonExistent_ReturnsFalse) { MatchingEngine e; EXPECT_FALSE(e.cancel(999)); }
TEST(CancelTest, DoubleCancelReturnsFalseSecondTime) {
    MatchingEngine e; e.submit_limit(1, Side::Buy, 10025, 5);
    EXPECT_TRUE(e.cancel(1)); EXPECT_FALSE(e.cancel(1));
}
TEST(CancelTest, CancelAfterFullFill_ReturnsFalse) {
    MatchingEngine e; e.submit_limit(1, Side::Sell, 10030, 5); e.submit_limit(2, Side::Buy, 10030, 5);
    EXPECT_FALSE(e.cancel(1));
}
TEST(CancelTest, CancelAfterPartialFill_ReturnsTrue) {
    MatchingEngine e; e.submit_limit(1, Side::Sell, 10030, 10); e.submit_limit(2, Side::Buy, 10030, 4);
    EXPECT_TRUE(e.cancel(1)); EXPECT_FALSE(e.best_ask().has_value());
}
TEST(CancelTest, CancelOneSideDoesNotAffectOtherSide) {
    MatchingEngine e; e.submit_limit(1, Side::Buy, 10025, 5); e.submit_limit(2, Side::Sell, 10030, 5);
    e.cancel(1); EXPECT_FALSE(e.best_bid().has_value());
    EXPECT_EQ(e.best_ask()->price_ticks, 10030);
}
TEST(CancelTest, CancelReducesLevelQtyCorrectly) {
    MatchingEngine e; e.submit_limit(1, Side::Buy, 10025, 5); e.submit_limit(2, Side::Buy, 10025, 8);
    e.cancel(1); EXPECT_EQ(e.best_bid()->total_qty, 8u); EXPECT_EQ(e.best_bid()->order_count, 1u);
}
