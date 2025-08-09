#include "Greeter.hpp"

Greeter::Greeter() = default;
Greeter::~Greeter() = default;

std::string Greeter::greet(const std::string& name) const {
    return std::string("Hello, ") + name + "!";
}
