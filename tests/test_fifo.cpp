#include "matching_engine.hpp"
#include <gtest/gtest.h>

TEST(FIFOTest, OlderOrderAtSamePriceFillsFirst) {
    MatchingEngine engine;
    engine.submit_limit(1, Side::Sell, 10030, 5);
    engine.submit_limit(2, Side::Sell, 10030, 5);
    auto result = engine.submit_limit(3, Side::Buy, 10030, 5);
    ASSERT_EQ(result.trades.size(), 1u);
    EXPECT_EQ(result.trades[0].sell_order_id, 1u);
    EXPECT_EQ(result.trades[0].qty, 5u);
}
TEST(FIFOTest, OlderBidFillsFirst) {
    MatchingEngine e; e.submit_limit(1, Side::Buy, 10025, 5); e.submit_limit(2, Side::Buy, 10025, 5);
    auto r = e.submit_limit(3, Side::Sell, 10025, 5);
    ASSERT_EQ(r.trades.size(), 1u); EXPECT_EQ(r.trades[0].buy_order_id, 1u);
}
TEST(FIFOTest, ThreeOrdersSamePrice_FillInArrivalOrder) {
    MatchingEngine e;
    e.submit_limit(1, Side::Sell, 10030, 3); e.submit_limit(2, Side::Sell, 10030, 3);
    e.submit_limit(3, Side::Sell, 10030, 3);
    { auto r = e.submit_limit(4, Side::Buy, 10030, 3); EXPECT_EQ(r.trades[0].sell_order_id, 1u); }
    { auto r = e.submit_limit(5, Side::Buy, 10030, 3); EXPECT_EQ(r.trades[0].sell_order_id, 2u); }
    { auto r = e.submit_limit(6, Side::Buy, 10030, 3); EXPECT_EQ(r.trades[0].sell_order_id, 3u); }
}
TEST(FIFOTest, SweepTwoOrdersAtSameLevel_OldestFirst) {
    MatchingEngine e; e.submit_limit(1, Side::Sell, 10030, 4); e.submit_limit(2, Side::Sell, 10030, 4);
    auto r = e.submit_limit(3, Side::Buy, 10030, 8);
    ASSERT_EQ(r.trades.size(), 2u);
    EXPECT_EQ(r.trades[0].sell_order_id, 1u); EXPECT_EQ(r.trades[1].sell_order_id, 2u);
}
TEST(FIFOTest, PartialFillOfOlderOrder_ThenYoungerOrder) {
    MatchingEngine e; e.submit_limit(1, Side::Sell, 10030, 6); e.submit_limit(2, Side::Sell, 10030, 6);
    { auto r = e.submit_limit(3, Side::Buy, 10030, 4);
      EXPECT_EQ(r.trades[0].sell_order_id, 1u); EXPECT_EQ(r.trades[0].qty, 4u); }
    { auto r = e.submit_limit(4, Side::Buy, 10030, 4);
      ASSERT_EQ(r.trades.size(), 2u);
      EXPECT_EQ(r.trades[0].sell_order_id, 1u); EXPECT_EQ(r.trades[0].qty, 2u);
      EXPECT_EQ(r.trades[1].sell_order_id, 2u); EXPECT_EQ(r.trades[1].qty, 2u); }
}
TEST(FIFOTest, CancelMiddleOrder_RemainingStillFIFO) {
    MatchingEngine e;
    e.submit_limit(1, Side::Sell, 10030, 5); e.submit_limit(2, Side::Sell, 10030, 5);
    e.submit_limit(3, Side::Sell, 10030, 5); e.cancel(2);
    auto r = e.submit_limit(4, Side::Buy, 10030, 10);
    ASSERT_EQ(r.trades.size(), 2u);
    EXPECT_EQ(r.trades[0].sell_order_id, 1u); EXPECT_EQ(r.trades[1].sell_order_id, 3u);
}
