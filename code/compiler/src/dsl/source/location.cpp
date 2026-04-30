#include "dsl/source/location.hpp"

namespace dsl {

Location::Location(const Position& begin_position, const Position& end_position)
    : begin(begin_position), end(end_position) {}

Location::Location(const Position& position) : begin(position), end(position) {}

Location::Location(filename_type* filename, counter_type line, counter_type column)
    : begin(filename, line, column), end(filename, line, column) {}

void Location::initialize(filename_type* filename, counter_type line, counter_type column) {
    begin.initialize(filename, line, column);
    end = begin;
}

void Location::step() { begin = end; }

void Location::columns(counter_type count) { end += count; }

void Location::lines(counter_type count) { end.lines(count); }

}  // namespace dsl
