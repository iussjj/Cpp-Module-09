#pragma once

#include <string>
#include <stack>
#include <list>

class RPN {
private:
	//by default, stack wraps around a deque -here we use a list instead
	std::stack<long long, std::list<long long> > stack_;
	void performMath(char operand);

public:
	RPN();
	~RPN();
	RPN(const RPN& source);
	RPN& operator=(const RPN& source);

	void calculate(const std::string& expression);
} ;