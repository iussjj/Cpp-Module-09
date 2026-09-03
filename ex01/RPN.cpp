#include "RPN.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <cctype>

RPN::RPN() {}
RPN::~RPN() {}
RPN::RPN(const RPN& source) : stack_(source.stack_) {}
RPN& RPN::operator=(const RPN& source) {
	if (this != &source) {
		stack_ = source.stack_;
	}
	return *this;
}

void RPN::calculate(const std::string& expression) {
	std::istringstream iss(expression);
	
	// variable to store chunks extracted from stream
	std::string token;

	while (iss >> token) {
		if (token.length() > 1) throw std::runtime_error("Error");

		if (std::isdigit(token[0])) {
			if (stack_.size() < 2) {
				throw std::runtime_error("Error");
			}
			stack_.push(std::stoi(token));
		}

	}
}