#pragma once

#include <ostream>

#include "dsl/source/position.hpp"

namespace dsl {

struct Location {
    Position begin;
    Position end;

    using filename_type = Position::filename_type;
    using counter_type = Position::counter_type;

    Location(const Position& begin_position, const Position& end_position);
    explicit Location(const Position& position = Position());
    explicit Location(filename_type* filename, counter_type line = 1, counter_type column = 1);

    void initialize(filename_type* filename = nullptr, counter_type line = 1, counter_type column = 1);
    void step();

    void columns(counter_type count = 1);
    void lines(counter_type count = 1);
};

inline Location& operator+=(Location& location, const Location& end_location) {
    location.end = end_location.end;
    return location;
}

inline Location operator+(Location location, const Location& end_location) { return location += end_location; }

inline Location& operator+=(Location& location, const Location::counter_type width) {
    location.columns(width);
    return location;
}

inline Location operator+(Location location, const Location::counter_type width) { return location += width; }

inline Location& operator-=(Location& location, const Location::counter_type width) { return location += -width; }

inline Location operator-(Location location, const Location::counter_type width) { return location -= width; }

template <typename CharT>
std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const Location& location) {
    const bool same_line = location.begin.line == location.end.line;
    const bool same_column = location.begin.column == location.end.column;

    os << location.begin;
    if (!same_line || !same_column) {
        os << '-';
        if (location.begin.filename != location.end.filename) {
            os << location.end;
        } else if (same_line) {
            os << location.end.column;
        } else {
            os << location.end.line << '.' << location.end.column;
        }
    }

    return os;
}

}  // namespace dsl
