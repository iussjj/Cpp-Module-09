#pragma once

#include <vector>
#include <deque>

class PmergeMe {
private:
	std::vector<int> input_;
	std::vector<int> vec_;
	std::deque<int> deq_;

	void parseInput_(int argc, char** argv);
	void sortVec_();
	void sortDeq_();

public:
	PmergeMe();
	~PmergeMe();
	PmergeMe(const PmergeMe& src);
	PmergeMe& operator=(const PmergeMe& src);

	PmergeMe(int argc, char** argv);

	void sort();
} ;