#include "RPN.hpp"

#include <iostream>

int main(int argc, char** argv) {
	if (argc != 2) {
		std::cerr 	<< "Invalid arguments: please enter one valid expression\n"
					<< "Example:  \"7 7 * 7 -\""
					<< std::endl;
		return 1;
	}
	try {
		RPN calculator;
		calculator.calculate(argv[1]);
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}