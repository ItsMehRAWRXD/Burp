#include <iostream>
#include "Greeter.hpp"

int main() {
    Greeter greeter;
    std::cout << greeter.greet("World") << std::endl;
    return 0;
}
