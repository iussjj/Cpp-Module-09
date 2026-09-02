#include <iostream>

int main (int argc, char** argv) {

	if (argc != 2) {
		std::cout 	<< "Invalid arguments: please specify exactly one source file."
					<< std::endl;
	}
	
	return 0;
}