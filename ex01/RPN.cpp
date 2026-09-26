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

void RPN::performMath(char operand) {
	if (stack_.size() < 2) {
		throw std::runtime_error("Error");
	}

	int right = stack_.top();
	stack_.pop();

	int left = stack_.top();
	stack_.pop();

	switch (operand) {
		case '+':
			stack_.push(left + right);
			break;
		case '-':
			stack_.push(left - right);
			break;
		case '*':
			stack_.push(left * right);
			break;
		case '/':
			if (right == 0) {
				throw std::runtime_error("Error");
			}
			stack_.push(left / right);
			break;
		default:
			throw std::runtime_error("Error");
	}
}

void RPN::calculate(const std::string& expression) {
	std::istringstream iss(expression);
	
	// variable to store chunks extracted from stream
	std::string token;

	while (iss >> token) {
		if (token.length() > 1) throw std::runtime_error("Error");

		if (std::isdigit(token[0])) {
			stack_.push(std::stoi(token));
		}

		if (token == "+" || token == "-" || token == "*" || token == "/") {
			performMath(token[0]);
		}
		

	}
}
