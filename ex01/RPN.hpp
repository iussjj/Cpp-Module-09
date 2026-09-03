#pragma once

#include <string>
#include <stack>
#include <list>

class RPN {
private:
	//by default, stack wraps around a deque -here we use a list instead
	std::stack<int, std::list<int> > stack_;

public:
	RPN();
	~RPN();
	RPN(const RPN& source);
	RPN& operator=(const RPN& source);

	void calculate(const std::string& expression);
} ;