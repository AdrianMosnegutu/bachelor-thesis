#include "dsl/source/position.hpp"

namespace dsl::source {

Position::Position(filename_type* f, const counter_type l, const counter_type c) : filename(f), line(l), column(c) {}

void Position::initialize(filename_type* f, const counter_type l, const counter_type c) {
    filename = f;
    line = l;
    column = c;
}

void Position::lines(const counter_type count) {
    if (count != 0) {
        column = 1;
        line = add(line, count);
    }
}

void Position::columns(const counter_type count) { column = add(column, count); }

Position::counter_type Position::add(const counter_type lhs, const counter_type rhs) {
    const counter_type result = lhs + rhs;
    return result < 1 ? 1 : result;
}

}  // namespace dsl::source
