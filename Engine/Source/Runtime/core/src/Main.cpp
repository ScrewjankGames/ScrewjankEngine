// STD Headers
#include <iostream>

// Engine Headers
#include "math/Vector.hpp"

int main() {
	math::Vector3 test{ 1,2,3 };
	std::cout << "Test: " << test * 2 << std::endl;

}