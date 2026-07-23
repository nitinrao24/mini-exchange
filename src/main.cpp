#include "matching_engine.hpp"
#include "parser.hpp"
#include "reporter.hpp"

#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    const std::string path = (argc > 1) ? argv[1] : "data/sample_orders.csv";
    std::ifstream file(path);
    if (!file.is_open()) { std::cerr << "Cannot open: " << path << '\n'; return 1; }

    MatchingEngine engine;
    Reporter       reporter(std::cout);
    std::string    line;
    int            line_num = 0;

    while (std::getline(file, line)) {
        ++line_num;
        auto cmd = Parser::parse_line(line);
        if (!cmd) continue;

        std::cout << line << '\n';
        auto result = engine.process(*cmd);

        if      (cmd->type == CommandType::Snapshot)  reporter.report_snapshot(engine.snapshot(10));
        else if (cmd->type == CommandType::TopOfBook) reporter.report_top_of_book(result.best_bid, result.best_ask);
        else                                          reporter.report_result(result);

        std::cout << '\n';
    }
    std::cout << "--- Replay complete: " << line_num << " lines processed ---\n";
    return 0;
}
