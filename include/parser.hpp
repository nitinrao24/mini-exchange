#pragma once

#include "command.hpp"
#include <istream>
#include <optional>
#include <string>
#include <vector>

class Parser {
public:
    static std::optional<Command> parse_line(const std::string& line);
    static std::vector<Command>   parse_stream(std::istream& in);
};
