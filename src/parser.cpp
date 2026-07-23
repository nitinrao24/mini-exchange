#include "parser.hpp"
#include <iostream>
#include <limits>
#include <sstream>

static std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> parts;
    std::istringstream ss(s);
    std::string part;
    while (std::getline(ss, part, delim)) parts.push_back(part);
    return parts;
}

static std::string trim(std::string s) {
    if (!s.empty() && s.back() == '\r') s.pop_back();
    auto start = s.find_first_not_of(" \t");
    if (start == std::string::npos) return {};
    auto end = s.find_last_not_of(" \t");
    return s.substr(start, end - start + 1);
}

static bool parse_u64(const std::string& s, std::uint64_t& out) {
    try { std::size_t pos; out = std::stoull(s, &pos); return pos == s.size(); }
    catch (...) { return false; }
}
static bool parse_i64(const std::string& s, std::int64_t& out) {
    try { std::size_t pos; out = std::stoll(s, &pos); return pos == s.size(); }
    catch (...) { return false; }
}
static bool parse_u32(const std::string& s, std::uint32_t& out) {
    std::uint64_t v; if (!parse_u64(s, v) || v > 0xFFFFFFFFu) return false;
    out = static_cast<std::uint32_t>(v); return true;
}

std::optional<Command> Parser::parse_line(const std::string& raw) {
    std::string line = trim(raw);
    if (line.empty() || line[0] == '#') return std::nullopt;
    auto fields = split(line, ',');
    if (fields.empty()) return std::nullopt;
    const std::string cmd = trim(fields[0]);

    if (cmd == "SNAPSHOT")    return Command{CommandType::Snapshot,  std::nullopt, std::nullopt};
    if (cmd == "TOP_OF_BOOK") return Command{CommandType::TopOfBook, std::nullopt, std::nullopt};

    if (cmd == "CANCEL") {
        if (fields.size() < 2) { std::cerr << "Parser: CANCEL missing id: " << line << '\n'; return std::nullopt; }
        OrderId id; if (!parse_u64(trim(fields[1]), id)) { std::cerr << "Parser: bad id: " << line << '\n'; return std::nullopt; }
        Command c; c.type = CommandType::Cancel; c.cancel_id = id; return c;
    }

    if (cmd == "PLACE") {
        if (fields.size() < 5) { std::cerr << "Parser: PLACE missing fields: " << line << '\n'; return std::nullopt; }
        OrderId id; if (!parse_u64(trim(fields[1]), id)) { std::cerr << "Parser: bad id: " << line << '\n'; return std::nullopt; }
        Side side;
        std::string ss = trim(fields[2]);
        if      (ss == "BUY")  side = Side::Buy;
        else if (ss == "SELL") side = Side::Sell;
        else { std::cerr << "Parser: bad side: " << line << '\n'; return std::nullopt; }
        std::string ts = trim(fields[3]);
        if (ts == "LIMIT") {
            if (fields.size() < 6) { std::cerr << "Parser: LIMIT missing price/qty: " << line << '\n'; return std::nullopt; }
            Price p; Qty q;
            if (!parse_i64(trim(fields[4]), p)) { std::cerr << "Parser: bad price: " << line << '\n'; return std::nullopt; }
            if (!parse_u32(trim(fields[5]), q)) { std::cerr << "Parser: bad qty: "   << line << '\n'; return std::nullopt; }
            Command c; c.type = CommandType::Place; c.order = Order{id, side, OrderType::Limit, p, q, 0}; return c;
        }
        if (ts == "MARKET") {
            Qty q; if (!parse_u32(trim(fields[4]), q)) { std::cerr << "Parser: bad qty: " << line << '\n'; return std::nullopt; }
            Command c; c.type = CommandType::Place; c.order = Order{id, side, OrderType::Market, 0, q, 0}; return c;
        }
        std::cerr << "Parser: bad type: " << line << '\n'; return std::nullopt;
    }
    std::cerr << "Parser: unknown command: " << line << '\n'; return std::nullopt;
}

std::vector<Command> Parser::parse_stream(std::istream& in) {
    std::vector<Command> cmds; std::string line;
    while (std::getline(in, line)) { auto c = parse_line(line); if (c) cmds.push_back(*c); }
    return cmds;
}
