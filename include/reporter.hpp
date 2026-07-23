#pragma once

#include "matching_engine.hpp"
#include "snapshot.hpp"
#include <optional>
#include <ostream>

class Reporter {
public:
    explicit Reporter(std::ostream& out);
    void report_result(const EngineResult& result);
    void report_snapshot(const BookSnapshot& snap, std::size_t depth = 10);
    void report_top_of_book(const std::optional<LevelInfo>& bid,
                             const std::optional<LevelInfo>& ask);
private:
    std::ostream& out_;
    void print_bbo(const std::optional<LevelInfo>& bid,
                   const std::optional<LevelInfo>& ask);
};
