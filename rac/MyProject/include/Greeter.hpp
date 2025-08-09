#pragma once
#include <string>

class Greeter {
public:
    Greeter();
    ~Greeter();
    std::string greet(const std::string& name) const;
};
