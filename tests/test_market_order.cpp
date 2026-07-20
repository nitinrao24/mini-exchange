#include "matching_engine.hpp"
#include <gtest/gtest.h>

TEST(MarketOrderTest, MarketBuy_MatchesBestAsk) {
    MatchingEngine e; e.submit_limit(1, Side::Sell, 10030, 5);
    auto r = e.submit_market(2, Side::Buy, 5);
    ASSERT_EQ(r.trades.size(), 1u); EXPECT_EQ(r.trades[0].qty, 5u);
    EXPECT_EQ(r.trades[0].sell_order_id, 1u); EXPECT_FALSE(r.best_ask.has_value());
}
TEST(MarketOrderTest, MarketSell_MatchesBestBid) {
    MatchingEngine e; e.submit_limit(1, Side::Buy, 10025, 5);
    auto r = e.submit_market(2, Side::Sell, 5);
    ASSERT_EQ(r.trades.size(), 1u); EXPECT_EQ(r.trades[0].qty, 5u);
    EXPECT_EQ(r.trades[0].buy_order_id, 1u); EXPECT_FALSE(r.best_bid.has_value());
}
TEST(MarketOrderTest, MarketBuy_EmptyBook_NoTrades) {
    MatchingEngine e; EXPECT_TRUE(e.submit_market(1, Side::Buy, 10).trades.empty());
}
TEST(MarketOrderTest, MarketSell_EmptyBook_NoTrades) {
    MatchingEngine e; EXPECT_TRUE(e.submit_market(1, Side::Sell, 10).trades.empty());
}
TEST(MarketOrderTest, MarketBuy_SweepsMultipleLevels) {
    MatchingEngine e;
    e.submit_limit(1, Side::Sell, 10025, 5); e.submit_limit(2, Side::Sell, 10030, 5);
    e.submit_limit(3, Side::Sell, 10035, 5);
    auto r = e.submit_market(4, Side::Buy, 12);
    ASSERT_EQ(r.trades.size(), 3u);
    EXPECT_EQ(r.trades[0].price_ticks, 10025); EXPECT_EQ(r.trades[0].qty, 5u);
    EXPECT_EQ(r.trades[1].price_ticks, 10030); EXPECT_EQ(r.trades[1].qty, 5u);
    EXPECT_EQ(r.trades[2].price_ticks, 10035); EXPECT_EQ(r.trades[2].qty, 2u);
    EXPECT_EQ(r.best_ask->total_qty, 3u);
}
TEST(MarketOrderTest, MarketSell_SweepsMultipleLevels) {
    MatchingEngine e;
    e.submit_limit(1, Side::Buy, 10035, 5); e.submit_limit(2, Side::Buy, 10030, 5);
    e.submit_limit(3, Side::Buy, 10025, 5);
    auto r = e.submit_market(4, Side::Sell, 12);
    ASSERT_EQ(r.trades.size(), 3u);
    EXPECT_EQ(r.trades[0].price_ticks, 10035); EXPECT_EQ(r.trades[2].qty, 2u);
}
TEST(MarketOrderTest, MarketBuy_PartialLiquidity_RemainderDiscarded) {
    MatchingEngine e; e.submit_limit(1, Side::Sell, 10030, 3);
    auto r = e.submit_market(2, Side::Buy, 10);
    ASSERT_EQ(r.trades.size(), 1u); EXPECT_EQ(r.trades[0].qty, 3u);
    EXPECT_FALSE(r.best_bid.has_value()); EXPECT_FALSE(r.best_ask.has_value());
}
TEST(MarketOrderTest, MarketOrder_TradePriceIsAlwaysRestingPrice) {
    MatchingEngine e; e.submit_limit(1, Side::Sell, 10030, 5);
    auto r = e.submit_market(2, Side::Buy, 5);
    ASSERT_EQ(r.trades.size(), 1u); EXPECT_EQ(r.trades[0].price_ticks, 10030);
}
