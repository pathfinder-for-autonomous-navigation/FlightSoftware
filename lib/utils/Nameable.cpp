#include "Nameable.hpp"

Nameable::Nameable(const std::string& name) : _name(name) {}

const std::string& Nameable::name() const { return _name; }