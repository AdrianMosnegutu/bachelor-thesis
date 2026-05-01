#pragma once

#include <ostream>
#include <string>

namespace dsl::source {

struct Position {
    using filename_type = const std::string;
    using counter_type = int;

    filename_type* filename;
    counter_type line;
    counter_type column;

    explicit Position(filename_type* f = nullptr, counter_type l = 1, counter_type c = 1);

    void initialize(filename_type* f = nullptr, counter_type l = 1, counter_type c = 1);

    void lines(counter_type count = 1);
    void columns(counter_type count = 1);

   private:
    static counter_type add(counter_type lhs, counter_type rhs);
};

inline Position& operator+=(Position& position, const Position::counter_type width) {
    position.columns(width);
    return position;
}

inline Position operator+(Position position, const Position::counter_type width) { return position += width; }

inline Position& operator-=(Position& position, const Position::counter_type width) { return position += -width; }

inline Position operator-(Position position, const Position::counter_type width) { return position -= width; }

template <typename CharT>
std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const Position& position) {
    if (position.filename != nullptr) {
        os << *position.filename << ':';
    }
    return os << position.line << ':' << position.column;
}

}  // namespace dsl::source
