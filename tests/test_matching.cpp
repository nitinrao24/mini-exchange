#include "matching_engine.hpp"
#include <gtest/gtest.h>

TEST(MatchingTest, LimitBuyCrossesAsk_FullFill) {
    MatchingEngine e; e.submit_limit(1, Side::Sell, 10030, 5);
    auto r = e.submit_limit(2, Side::Buy, 10030, 5);
    ASSERT_EQ(r.trades.size(), 1u); EXPECT_EQ(r.trades[0].qty, 5u);
    EXPECT_EQ(r.trades[0].buy_order_id, 2u); EXPECT_EQ(r.trades[0].sell_order_id, 1u);
    EXPECT_FALSE(r.best_bid.has_value()); EXPECT_FALSE(r.best_ask.has_value());
}
TEST(MatchingTest, LimitSellCrossesBid_FullFill) {
    MatchingEngine e; e.submit_limit(1, Side::Buy, 10025, 8);
    auto r = e.submit_limit(2, Side::Sell, 10025, 8);
    ASSERT_EQ(r.trades.size(), 1u); EXPECT_EQ(r.trades[0].qty, 8u);
    EXPECT_EQ(r.trades[0].buy_order_id, 1u); EXPECT_EQ(r.trades[0].sell_order_id, 2u);
}
TEST(MatchingTest, LimitBuy_NoCrossWhenPriceBelow) {
    MatchingEngine e; e.submit_limit(1, Side::Sell, 10030, 5);
    auto r = e.submit_limit(2, Side::Buy, 10025, 5);
    EXPECT_TRUE(r.trades.empty());
    EXPECT_EQ(r.best_bid->price_ticks, 10025); EXPECT_EQ(r.best_ask->price_ticks, 10030);
}
TEST(MatchingTest, LimitSell_NoCrossWhenPriceAbove) {
    MatchingEngine e; e.submit_limit(1, Side::Buy, 10025, 5);
    auto r = e.submit_limit(2, Side::Sell, 10030, 5);
    EXPECT_TRUE(r.trades.empty());
    EXPECT_TRUE(r.best_bid.has_value()); EXPECT_TRUE(r.best_ask.has_value());
}
TEST(MatchingTest, PartialFill_IncomingLarger_RemainderRests) {
    MatchingEngine e; e.submit_limit(1, Side::Sell, 10030, 4);
    auto r = e.submit_limit(2, Side::Buy, 10030, 10);
    ASSERT_EQ(r.trades.size(), 1u); EXPECT_EQ(r.trades[0].qty, 4u);
    ASSERT_TRUE(r.best_bid.has_value());
    EXPECT_EQ(r.best_bid->price_ticks, 10030); EXPECT_EQ(r.best_bid->total_qty, 6u);
    EXPECT_FALSE(r.best_ask.has_value());
}
TEST(MatchingTest, PartialFill_IncomingSmaller_RestingRemains) {
    MatchingEngine e; e.submit_limit(1, Side::Sell, 10030, 10);
    auto r = e.submit_limit(2, Side::Buy, 10030, 4);
    ASSERT_EQ(r.trades.size(), 1u); EXPECT_EQ(r.trades[0].qty, 4u);
    EXPECT_EQ(r.best_ask->total_qty, 6u); EXPECT_FALSE(r.best_bid.has_value());
}
TEST(MatchingTest, SellSide_PartialFill_RemainderRests) {
    MatchingEngine e; e.submit_limit(1, Side::Buy, 10025, 3);
    auto r = e.submit_limit(2, Side::Sell, 10025, 8);
    ASSERT_EQ(r.trades.size(), 1u); EXPECT_EQ(r.trades[0].qty, 3u);
    EXPECT_EQ(r.best_ask->price_ticks, 10025); EXPECT_EQ(r.best_ask->total_qty, 5u);
}
TEST(MatchingTest, TradePriceIsRestingAskPrice_NotIncomingBuyPrice) {
    MatchingEngine e; e.submit_limit(1, Side::Sell, 10025, 5);
    auto r = e.submit_limit(2, Side::Buy, 10030, 5);
    ASSERT_EQ(r.trades.size(), 1u); EXPECT_EQ(r.trades[0].price_ticks, 10025);
}
TEST(MatchingTest, TradePriceIsRestingBidPrice_NotIncomingSellPrice) {
    MatchingEngine e; e.submit_limit(1, Side::Buy, 10030, 5);
    auto r = e.submit_limit(2, Side::Sell, 10025, 5);
    ASSERT_EQ(r.trades.size(), 1u); EXPECT_EQ(r.trades[0].price_ticks, 10030);
}
TEST(MatchingTest, MultilevelSweep_BuySide) {
    MatchingEngine e;
    e.submit_limit(1, Side::Sell, 10025, 5); e.submit_limit(2, Side::Sell, 10030, 5);
    e.submit_limit(3, Side::Sell, 10035, 5);
    auto r = e.submit_limit(4, Side::Buy, 10035, 15);
    ASSERT_EQ(r.trades.size(), 3u);
    EXPECT_EQ(r.trades[0].price_ticks, 10025); EXPECT_EQ(r.trades[1].price_ticks, 10030);
    EXPECT_EQ(r.trades[2].price_ticks, 10035); EXPECT_FALSE(r.best_ask.has_value());
}
TEST(MatchingTest, MultilevelSweep_SellSide) {
    MatchingEngine e;
    e.submit_limit(1, Side::Buy, 10035, 5); e.submit_limit(2, Side::Buy, 10030, 5);
    e.submit_limit(3, Side::Buy, 10025, 5);
    auto r = e.submit_limit(4, Side::Sell, 10025, 15);
    ASSERT_EQ(r.trades.size(), 3u);
    EXPECT_EQ(r.trades[0].price_ticks, 10035); EXPECT_FALSE(r.best_bid.has_value());
}
TEST(MatchingTest, EmptyLevelRemovedAfterFullFill) {
    MatchingEngine e; e.submit_limit(1, Side::Sell, 10030, 5);
    e.submit_limit(2, Side::Buy, 10030, 5);
    EXPECT_TRUE(e.snapshot(10).asks.empty());
}
TEST(MatchingTest, MultipleLevelsSweep_EmptyLevelsAllRemoved) {
    MatchingEngine e;
    e.submit_limit(1, Side::Sell, 10025, 3); e.submit_limit(2, Side::Sell, 10030, 3);
    e.submit_limit(3, Side::Buy, 10030, 6);
    EXPECT_TRUE(e.snapshot(10).asks.empty());
}
