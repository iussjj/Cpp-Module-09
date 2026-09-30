#include "PmergeMe.hpp"

#include <iostream>

int main(int argc, char** argv) {
	try {
		PmergeMe sorter(argc, argv);
		std::cout << "Before: ";
		sorter.printVec();
		sorter.sort();
		std::cout << "After: ";
		sorter.printVec(); // deque sequence identical

	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}