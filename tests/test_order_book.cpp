#include "order_book.hpp"
#include <gtest/gtest.h>

static Order make_order(OrderId id, Side side, Price price, Qty qty, SeqNo seq = 0) {
    return Order{id, side, OrderType::Limit, price, qty, seq};
}

TEST(OrderBookTest, EmptyBookNoBestBid) { OrderBook b; EXPECT_FALSE(b.best_bid().has_value()); }
TEST(OrderBookTest, EmptyBookNoBestAsk) { OrderBook b; EXPECT_FALSE(b.best_ask().has_value()); }
TEST(OrderBookTest, EmptySnapshotHasNoLevels) {
    OrderBook b; auto s = b.snapshot(10);
    EXPECT_TRUE(s.bids.empty()); EXPECT_TRUE(s.asks.empty());
}
TEST(OrderBookTest, AddBidSetsBestBid) {
    OrderBook b; b.add_resting_order(make_order(1, Side::Buy, 10025, 10));
    auto bb = b.best_bid(); ASSERT_TRUE(bb.has_value());
    EXPECT_EQ(bb->price_ticks, 10025); EXPECT_EQ(bb->total_qty, 10u); EXPECT_EQ(bb->order_count, 1u);
}
TEST(OrderBookTest, AddAskSetsBestAsk) {
    OrderBook b; b.add_resting_order(make_order(1, Side::Sell, 10030, 5));
    auto ba = b.best_ask(); ASSERT_TRUE(ba.has_value());
    EXPECT_EQ(ba->price_ticks, 10030); EXPECT_EQ(ba->total_qty, 5u);
}
TEST(OrderBookTest, BestBidIsHighestPrice) {
    OrderBook b;
    b.add_resting_order(make_order(1, Side::Buy, 10020, 5));
    b.add_resting_order(make_order(2, Side::Buy, 10025, 5));
    b.add_resting_order(make_order(3, Side::Buy, 10015, 5));
    EXPECT_EQ(b.best_bid()->price_ticks, 10025);
}
TEST(OrderBookTest, BestAskIsLowestPrice) {
    OrderBook b;
    b.add_resting_order(make_order(1, Side::Sell, 10035, 5));
    b.add_resting_order(make_order(2, Side::Sell, 10025, 5));
    b.add_resting_order(make_order(3, Side::Sell, 10030, 5));
    EXPECT_EQ(b.best_ask()->price_ticks, 10025);
}
TEST(OrderBookTest, AddingSameSideDoesNotAffectOtherSide) {
    OrderBook b;
    b.add_resting_order(make_order(1, Side::Buy, 10020, 5));
    b.add_resting_order(make_order(2, Side::Buy, 10025, 5));
    EXPECT_FALSE(b.best_ask().has_value());
}
TEST(OrderBookTest, TwoOrdersSamePriceAggregatesQty) {
    OrderBook b;
    b.add_resting_order(make_order(1, Side::Buy, 10025, 5));
    b.add_resting_order(make_order(2, Side::Buy, 10025, 3));
    EXPECT_EQ(b.best_bid()->total_qty, 8u); EXPECT_EQ(b.best_bid()->order_count, 2u);
}
TEST(OrderBookTest, ThreeOrdersSamePriceSummedCorrectly) {
    OrderBook b;
    b.add_resting_order(make_order(1, Side::Sell, 10030, 4));
    b.add_resting_order(make_order(2, Side::Sell, 10030, 6));
    b.add_resting_order(make_order(3, Side::Sell, 10030, 10));
    EXPECT_EQ(b.best_ask()->total_qty, 20u); EXPECT_EQ(b.best_ask()->order_count, 3u);
}
TEST(OrderBookTest, HasOrderTrueAfterAdd) {
    OrderBook b; b.add_resting_order(make_order(42, Side::Buy, 10025, 5));
    EXPECT_TRUE(b.has_order(42));
}
TEST(OrderBookTest, HasOrderFalseForUnknownId) { OrderBook b; EXPECT_FALSE(b.has_order(999)); }
TEST(OrderBookTest, DuplicateOrderIdRejected) {
    OrderBook b;
    EXPECT_TRUE(b.add_resting_order(make_order(1, Side::Buy, 10025, 10)));
    EXPECT_FALSE(b.add_resting_order(make_order(1, Side::Buy, 10025, 5)));
    EXPECT_EQ(b.best_bid()->total_qty, 10u);
}
TEST(OrderBookTest, CancelExistingOrderReturnsTrueAndRemovesFromIndex) {
    OrderBook b; b.add_resting_order(make_order(1, Side::Buy, 10025, 10));
    EXPECT_TRUE(b.cancel_order(1)); EXPECT_FALSE(b.has_order(1));
}
TEST(OrderBookTest, CancelReducesBestBidQty) {
    OrderBook b;
    b.add_resting_order(make_order(1, Side::Buy, 10025, 10));
    b.add_resting_order(make_order(2, Side::Buy, 10025, 5));
    b.cancel_order(1);
    EXPECT_EQ(b.best_bid()->total_qty, 5u); EXPECT_EQ(b.best_bid()->order_count, 1u);
}
TEST(OrderBookTest, CancelLastOrderAtLevelRemovesLevel) {
    OrderBook b; b.add_resting_order(make_order(1, Side::Buy, 10025, 10));
    b.cancel_order(1); EXPECT_FALSE(b.best_bid().has_value());
}
TEST(OrderBookTest, CancelLowerBidLeavesHigherBidIntact) {
    OrderBook b;
    b.add_resting_order(make_order(1, Side::Buy, 10025, 10));
    b.add_resting_order(make_order(2, Side::Buy, 10020, 5));
    b.cancel_order(2);
    ASSERT_TRUE(b.best_bid().has_value()); EXPECT_EQ(b.best_bid()->price_ticks, 10025);
}
TEST(OrderBookTest, CancelAskReducesAskQty) {
    OrderBook b;
    b.add_resting_order(make_order(1, Side::Sell, 10030, 8));
    b.add_resting_order(make_order(2, Side::Sell, 10030, 4));
    b.cancel_order(2); EXPECT_EQ(b.best_ask()->total_qty, 8u);
}
TEST(OrderBookTest, CancelNonExistentOrderReturnsFalse) {
    OrderBook b; EXPECT_FALSE(b.cancel_order(999));
}
TEST(OrderBookTest, DoubleCancelReturnsFalseSecondTime) {
    OrderBook b; b.add_resting_order(make_order(1, Side::Buy, 10025, 10));
    EXPECT_TRUE(b.cancel_order(1)); EXPECT_FALSE(b.cancel_order(1));
}
TEST(OrderBookTest, SnapshotRespectsDepthLimit) {
    OrderBook b;
    for (int i = 0; i < 8; ++i)
        b.add_resting_order(make_order(i + 1, Side::Buy, 10000 - i * 5, 10));
    EXPECT_EQ(b.snapshot(3).bids.size(), 3u);
}
TEST(OrderBookTest, SnapshotBidsDescendingOrder) {
    OrderBook b;
    b.add_resting_order(make_order(1, Side::Buy, 10010, 5));
    b.add_resting_order(make_order(2, Side::Buy, 10020, 5));
    b.add_resting_order(make_order(3, Side::Buy, 10015, 5));
    auto s = b.snapshot(10); ASSERT_EQ(s.bids.size(), 3u);
    EXPECT_GT(s.bids[0].price_ticks, s.bids[1].price_ticks);
    EXPECT_GT(s.bids[1].price_ticks, s.bids[2].price_ticks);
}
TEST(OrderBookTest, SnapshotAsksAscendingOrder) {
    OrderBook b;
    b.add_resting_order(make_order(1, Side::Sell, 10030, 5));
    b.add_resting_order(make_order(2, Side::Sell, 10020, 5));
    b.add_resting_order(make_order(3, Side::Sell, 10025, 5));
    auto s = b.snapshot(10); ASSERT_EQ(s.asks.size(), 3u);
    EXPECT_LT(s.asks[0].price_ticks, s.asks[1].price_ticks);
    EXPECT_LT(s.asks[1].price_ticks, s.asks[2].price_ticks);
}
TEST(OrderBookTest, SnapshotQtyMatchesSumOfOrders) {
    OrderBook b;
    b.add_resting_order(make_order(1, Side::Sell, 10030, 7));
    b.add_resting_order(make_order(2, Side::Sell, 10030, 3));
    auto s = b.snapshot(1); ASSERT_EQ(s.asks.size(), 1u);
    EXPECT_EQ(s.asks[0].total_qty, 10u); EXPECT_EQ(s.asks[0].order_count, 2u);
}
TEST(OrderBookTest, SnapshotDepthLargerThanBookReturnsAll) {
    OrderBook b;
    b.add_resting_order(make_order(1, Side::Buy, 10025, 5));
    b.add_resting_order(make_order(2, Side::Buy, 10020, 5));
    EXPECT_EQ(b.snapshot(100).bids.size(), 2u);
}
