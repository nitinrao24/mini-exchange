#include "matching_engine.hpp"
#include "parser.hpp"
#include <gtest/gtest.h>
#include <sstream>

TEST(ParserTest, PlaceLimit_Buy) {
    auto c = Parser::parse_line("PLACE,1,BUY,LIMIT,10025,10");
    ASSERT_TRUE(c.has_value()); ASSERT_TRUE(c->order.has_value());
    EXPECT_EQ(c->order->order_id, 1u); EXPECT_EQ(c->order->side, Side::Buy);
    EXPECT_EQ(c->order->type, OrderType::Limit);
    EXPECT_EQ(c->order->price_ticks, 10025); EXPECT_EQ(c->order->qty, 10u);
}
TEST(ParserTest, PlaceLimit_Sell) {
    auto c = Parser::parse_line("PLACE,3,SELL,LIMIT,10030,8");
    ASSERT_TRUE(c.has_value()); ASSERT_TRUE(c->order.has_value());
    EXPECT_EQ(c->order->side, Side::Sell); EXPECT_EQ(c->order->price_ticks, 10030);
}
TEST(ParserTest, PlaceMarket_Buy) {
    auto c = Parser::parse_line("PLACE,2,BUY,MARKET,5");
    ASSERT_TRUE(c.has_value()); ASSERT_TRUE(c->order.has_value());
    EXPECT_EQ(c->order->type, OrderType::Market); EXPECT_EQ(c->order->qty, 5u);
}
TEST(ParserTest, PlaceMarket_Sell) {
    auto c = Parser::parse_line("PLACE,7,SELL,MARKET,12");
    ASSERT_TRUE(c.has_value()); EXPECT_EQ(c->order->side, Side::Sell); EXPECT_EQ(c->order->qty, 12u);
}
TEST(ParserTest, Cancel) {
    auto c = Parser::parse_line("CANCEL,42");
    ASSERT_TRUE(c.has_value()); EXPECT_EQ(c->type, CommandType::Cancel);
    ASSERT_TRUE(c->cancel_id.has_value()); EXPECT_EQ(*c->cancel_id, 42u);
}
TEST(ParserTest, Snapshot) {
    auto c = Parser::parse_line("SNAPSHOT");
    ASSERT_TRUE(c.has_value()); EXPECT_EQ(c->type, CommandType::Snapshot);
}
TEST(ParserTest, TopOfBook) {
    auto c = Parser::parse_line("TOP_OF_BOOK");
    ASSERT_TRUE(c.has_value()); EXPECT_EQ(c->type, CommandType::TopOfBook);
}
TEST(ParserTest, BlankLine_ReturnsNullopt)     { EXPECT_FALSE(Parser::parse_line("").has_value()); }
TEST(ParserTest, WhitespaceLine_ReturnsNullopt){ EXPECT_FALSE(Parser::parse_line("   ").has_value()); }
TEST(ParserTest, CommentLine_ReturnsNullopt)   { EXPECT_FALSE(Parser::parse_line("# comment").has_value()); }
TEST(ParserTest, UnknownCommand_ReturnsNullopt){ EXPECT_FALSE(Parser::parse_line("FOOBAR,1").has_value()); }
TEST(ParserTest, WindowsLineEnding_ParsedCorrectly) {
    auto c = Parser::parse_line("PLACE,1,BUY,LIMIT,10025,10\r");
    ASSERT_TRUE(c.has_value()); EXPECT_EQ(c->order->price_ticks, 10025);
}
TEST(ParserTest, ParseStream_SkipsBlanksAndComments) {
    std::istringstream ss("# comment\n\nPLACE,1,BUY,LIMIT,10025,5\n   \nPLACE,2,SELL,LIMIT,10030,5\n");
    auto cmds = Parser::parse_stream(ss);
    EXPECT_EQ(cmds.size(), 2u);
}
TEST(ParserTest, ParseStream_MixedCommandTypes) {
    std::istringstream ss("PLACE,1,BUY,LIMIT,10025,5\nCANCEL,1\nSNAPSHOT\nTOP_OF_BOOK\n");
    auto cmds = Parser::parse_stream(ss);
    ASSERT_EQ(cmds.size(), 4u);
    EXPECT_EQ(cmds[0].type, CommandType::Place);   EXPECT_EQ(cmds[1].type, CommandType::Cancel);
    EXPECT_EQ(cmds[2].type, CommandType::Snapshot); EXPECT_EQ(cmds[3].type, CommandType::TopOfBook);
}
TEST(ReplayTest, SingleLimitMatch_OneTrade) {
    std::istringstream ss("PLACE,1,SELL,LIMIT,10030,5\nPLACE,2,BUY,LIMIT,10030,5\n");
    MatchingEngine engine; int trades = 0;
    for (const auto& cmd : Parser::parse_stream(ss)) trades += (int)engine.process(cmd).trades.size();
    EXPECT_EQ(trades, 1);
}
TEST(ReplayTest, Deterministic_SameInputSameOutput) {
    const std::string csv = "PLACE,1,SELL,LIMIT,10030,5\nPLACE,2,SELL,LIMIT,10030,5\nPLACE,3,BUY,LIMIT,10030,8\n";
    auto run = [&]() {
        std::istringstream ss(csv); MatchingEngine e; std::vector<Trade> all;
        for (const auto& cmd : Parser::parse_stream(ss))
            for (const auto& t : e.process(cmd).trades) all.push_back(t);
        return all;
    };
    auto t1 = run(), t2 = run(); ASSERT_EQ(t1.size(), t2.size());
    for (std::size_t i = 0; i < t1.size(); ++i)
        EXPECT_EQ(t1[i].sell_order_id, t2[i].sell_order_id);
}
TEST(ReplayTest, CancelViaCsv_RemovesOrder) {
    std::istringstream ss("PLACE,1,BUY,LIMIT,10025,10\nCANCEL,1\n");
    MatchingEngine e; auto cmds = Parser::parse_stream(ss);
    e.process(cmds[0]); auto r = e.process(cmds[1]);
    EXPECT_TRUE(r.accepted); EXPECT_FALSE(r.best_bid.has_value());
}
TEST(ReplayTest, SnapshotCommand_ReturnsBBO) {
    std::istringstream ss("PLACE,1,BUY,LIMIT,10025,5\nPLACE,2,SELL,LIMIT,10030,8\nSNAPSHOT\n");
    MatchingEngine e; auto cmds = Parser::parse_stream(ss);
    e.process(cmds[0]); e.process(cmds[1]); auto r = e.process(cmds[2]);
    EXPECT_EQ(r.best_bid->price_ticks, 10025); EXPECT_EQ(r.best_ask->price_ticks, 10030);
}
TEST(ReplayTest, MarketOrderViaCsv_MatchesBook) {
    std::istringstream ss("PLACE,1,SELL,LIMIT,10030,5\nPLACE,2,BUY,MARKET,5\n");
    MatchingEngine e; auto cmds = Parser::parse_stream(ss);
    e.process(cmds[0]); auto r = e.process(cmds[1]);
    ASSERT_EQ(r.trades.size(), 1u); EXPECT_EQ(r.trades[0].price_ticks, 10030);
}
TEST(ReplayTest, FullScenario_FIFOPreservedAcrossReplay) {
    std::istringstream ss("PLACE,1,SELL,LIMIT,10030,5\nPLACE,2,SELL,LIMIT,10030,5\nPLACE,3,BUY,LIMIT,10030,5\n");
    MatchingEngine e; auto cmds = Parser::parse_stream(ss);
    e.process(cmds[0]); e.process(cmds[1]); auto r = e.process(cmds[2]);
    ASSERT_EQ(r.trades.size(), 1u); EXPECT_EQ(r.trades[0].sell_order_id, 1u);
}
TEST(ReplayTest, PartialFillThenCancel_LeavesBookClean) {
    std::istringstream ss("PLACE,1,SELL,LIMIT,10030,10\nPLACE,2,BUY,LIMIT,10030,4\nCANCEL,1\n");
    MatchingEngine e;
    for (const auto& cmd : Parser::parse_stream(ss)) e.process(cmd);
    EXPECT_FALSE(e.best_bid().has_value()); EXPECT_FALSE(e.best_ask().has_value());
}
