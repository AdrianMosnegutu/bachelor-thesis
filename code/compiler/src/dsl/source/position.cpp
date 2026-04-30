#include "dsl/source/position.hpp"

namespace dsl {

Position::Position(filename_type* f, counter_type l, counter_type c) : filename(f), line(l), column(c) {}

void Position::initialize(filename_type* f, counter_type l, counter_type c) {
    filename = f;
    line = l;
    column = c;
}

void Position::lines(counter_type count) {
    if (count != 0) {
        column = 1;
        line = add(line, count);
    }
}

void Position::columns(counter_type count) { column = add(column, count); }

Position::counter_type Position::add(Position::counter_type lhs, Position::counter_type rhs) {
    const Position::counter_type result = lhs + rhs;
    return result < 1 ? 1 : result;
}

}  // namespace dsl
