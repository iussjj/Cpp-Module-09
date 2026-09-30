#include "PmergeMe.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

/*
	Pairs give us known upper bounds.
	Binary search places the pending elements.
	Jacobsthal chooses the order that makes those binary searches cheap.
*/

namespace {

	//std::size_t comparisonCount = 0;

	struct Item {
		int value;
		std::size_t id;
	};

	struct Pair {
		Item small;
		Item big;
	};

	struct Pending {
		Item small;
		// Id of big value that the small value was paired with
		std::optional<std::size_t> partnerId;
	};

	template <typename Container>
	void printSequence(const Container& values) {
		for (int value : values) {
			std::cout << value << ' ';
		}
		std::cout << std::endl;
	}

	bool compareItems(const Item& a, const Item& b) {
		//++comparisonCount;
		return a.value < b.value;
	}

	/*
		Returns the end of the allowed search range, defined by the position of the item's original
		partner in mainChain
	*/
	std::vector<Item>::iterator findPartner(std::vector<Item>& mainChain, const Pending& pending) {

		// handle straggler case: whole mainChain needs to be searched
		if (!pending.partnerId.has_value()) {
			return mainChain.end();
		}

		// search mainchain for id whose value matches current pending item's partnerId
		for (auto it = mainChain.begin(); it != mainChain.end(); ++it) {
			if (it->id == pending.partnerId.value()) {
				return it;
			}
		}

		// not a straggler but still no partner in mainChain: bug
		throw std::runtime_error("Error");
	}

	std::deque<Item>::iterator findPartner(std::deque<Item>& mainChain, const Pending& pending) {

		if (!pending.partnerId.has_value()) {
			return mainChain.end();
		}

		for (auto it = mainChain.begin(); it != mainChain.end(); ++it) {
			if (it->id == pending.partnerId.value()) {
				return it;
			}
		}

		throw std::runtime_error("Error");
	}

	void insertPendingItem(std::vector<Item>& mainChain, const Pending& pending) {
		
		// find upper limit of search (position of item's initial partner)
		auto searchEnd = findPartner(mainChain, pending);

		// binary search the allowed range for correct insertion pos
		// argument 3: object to insert, 4: how to compare elements
		auto insertPos = std::lower_bound(mainChain.begin(), searchEnd, pending.small, compareItems);
		
		mainChain.insert(insertPos, pending.small);
	}

	void insertPendingItem(std::deque<Item>& mainChain, const Pending& pending) {
		
		auto searchEnd = findPartner(mainChain, pending);

		auto insertPos = std::lower_bound(mainChain.begin(), searchEnd, pending.small, compareItems);
		
		mainChain.insert(insertPos, pending.small);
	}

	/*
		Returns a vector of pending chain indices, used to determine the order
		in which elements should be inserted into the main chain.
		The purpose is to ensure that *binary searches are as cheap as possible*
		-fewer comparisons, not necessarily fewer moves or les total runtime
	*/
	std::vector<std::size_t> buildJacobsthalOrderVec(std::size_t pendingCount) {
		std::vector<std::size_t> order;

		if (pendingCount == 0){
			return order;
		}

		// prevBoundary 1 is small #1, boundary 3 is small #3
		// the following means all elements after 1, up to and including 3
		std::size_t prevBoundary = 1;
		std::size_t boundary = 3;
		// small # of last item in pending
		const std::size_t lastSmall = pendingCount + 1;

		while (boundary <= lastSmall) {

			for (std::size_t small = boundary; small > prevBoundary; --small) {
				// small #2 == pending[0]
				// small #3 == pending[1]
				std::size_t index = small - 2;
				

				order.push_back(index);
			}

			// Jacobsthal recurrence: next = current + 2 * previous
			std::size_t nextBoundary = boundary + 2 * prevBoundary;
			prevBoundary = boundary;
			boundary = nextBoundary;
		}

		// handle incomplete final group (not enough smalls for full
		// Jacobsthal range remaining)
		for (std::size_t small = lastSmall; small > prevBoundary; --small) {
			std::size_t index = small - 2;
			order.push_back(index);
		}
		return order;
	}

	std::deque<std::size_t> buildJacobsthalOrderDeq(std::size_t pendingCount) {
	std::deque<std::size_t> order;

		if (pendingCount == 0){
			return order;
		}

		std::size_t prevBoundary = 1;
		std::size_t boundary = 3;
		const std::size_t lastSmall = pendingCount + 1;

		while (boundary <= lastSmall) {

			for (std::size_t small = boundary; small > prevBoundary; --small) {
				std::size_t index = small - 2;
				

				order.push_back(index);
			}

			std::size_t nextBoundary = boundary + 2 * prevBoundary;
			prevBoundary = boundary;
			boundary = nextBoundary;
		}

		for (std::size_t small = lastSmall; small > prevBoundary; --small) {
			std::size_t index = small - 2;
			order.push_back(index);
		}
		return order;
	}

	std::vector<Item> mergeInsertionSortVec(std::vector<Item> input) {
		
		// base case to stop recursion
		if (input.size() <= 1) {
			return input;
		}

		bool hasStraggler = (input.size() % 2 != 0);
		Item straggler {};
		if (hasStraggler) {
			straggler = input.back();
		}

		// group input into pairs so that each pair's first.value <= second.value
		std::vector<Pair> pairs;
		for (std::size_t i = 0; i + 1 < input.size(); i += 2) {
			Item first = input[i];
			Item second = input[i + 1];
			//++comparisonCount;
			if (first.value > second.value) {
				std::swap(first, second);
			}
			pairs.push_back({first, second});
		}

		// collect larger element from each pair
		std::vector<Item> winners;
		for (const Pair& pair : pairs) {
			winners.push_back(pair.big);
		}

		// further sort winners recursively
		winners = mergeInsertionSortVec(winners);

		// reorder pairs based on sorted winners
		std::vector<Pair> sortedPairs;
		for (const Item& winner : winners) {
			for (const Pair& pair : pairs) {
				if (pair.big.id == winner.id) {
					sortedPairs.push_back(pair);
					break; // matching pair found, move to next winner
				}
			}
		}

		// build starting main chain
		std::vector<Item> mainChain;
		mainChain.push_back(sortedPairs[0].small);
		for (const Pair& pair : sortedPairs) {
			mainChain.push_back(pair.big);
		}

		// build pending chain and insert straggler if there is one
		std::vector<Pending> pendingChain;
		for (std::size_t i = 1; i < sortedPairs.size(); ++i) {
			pendingChain.push_back({ sortedPairs[i].small, sortedPairs[i].big.id });
		}
		if (hasStraggler) {
			pendingChain.push_back({ straggler, std::nullopt });
		}

		// generate order in which to insert elements from pending to main chain
		std::vector<std::size_t> order = buildJacobsthalOrderVec(pendingChain.size());

		// insert each pending item according to the jacobsthal order
		for (std::size_t index : order) {
			insertPendingItem(mainChain, pendingChain[index]);
		}

		return mainChain;
	}

	std::deque<Item> mergeInsertionSortDeq(std::deque<Item> input) {
		
		if (input.size() <= 1) {
			return input;
		}

		bool hasStraggler = (input.size() % 2 != 0);
		Item straggler {};
		if (hasStraggler) {
			straggler = input.back();
		}

		std::deque<Pair> pairs;
		for (std::size_t i = 0; i + 1 < input.size(); i += 2) {
			Item first = input[i];
			Item second = input[i + 1];
			//++comparisonCount;
			if (first.value > second.value) {
				std::swap(first, second);
			}
			pairs.push_back({first, second});
		}

		std::deque<Item> winners;
		for (const Pair& pair : pairs) {
			winners.push_back(pair.big);
		}

		winners = mergeInsertionSortDeq(winners);

		std::deque<Pair> sortedPairs;
		for (const Item& winner : winners) {
			for (const Pair& pair : pairs) {
				if (pair.big.id == winner.id) {
					sortedPairs.push_back(pair);
					break;
				}
			}
		}

		std::deque<Item> mainChain;
		mainChain.push_back(sortedPairs[0].small);
		for (const Pair& pair : sortedPairs) {
			mainChain.push_back(pair.big);
		}

		std::deque<Pending> pendingChain;
		for (std::size_t i = 1; i < sortedPairs.size(); ++i) {
			pendingChain.push_back({ sortedPairs[i].small, sortedPairs[i].big.id });
		}
		if (hasStraggler) {
			pendingChain.push_back({ straggler, std::nullopt });
		}

		std::deque<std::size_t> order = buildJacobsthalOrderDeq(pendingChain.size());

		for (std::size_t index : order) {
			insertPendingItem(mainChain, pendingChain[index]);
		}

		return mainChain;
	}

} //namespace

PmergeMe::PmergeMe() {}
PmergeMe::~PmergeMe() {}
PmergeMe::PmergeMe(const PmergeMe& src) : input_(src.input_), vec_(src.vec_), deq_(src.deq_) {}

PmergeMe& PmergeMe::operator=(const PmergeMe& src) {
	if (this != &src) {
		input_ = src.input_;
		vec_ = src.vec_;
		deq_ = src.deq_;
	}
	return *this;
}

void	PmergeMe::parseInput_(int argc, char** argv) {
	if (argc < 2) {
		throw std::runtime_error("Error");
	}
	std::string input;
	for (int i = 1; i < argc; i++) {
		input += argv[i];
		input += " ";
	}

	std::istringstream iss(input);
	std::string token;

	while (iss >> token) {
		size_t start = 0;
		if (token[0] == '+') {
			if (token.length() == 1) {
				throw std::runtime_error("Error");
			}
			start = 1;
		}
		for (size_t i = start; i < token.length(); i++) {
			if (!std::isdigit(token[i])) {
				throw std::runtime_error("Error");
			}
		}
		try {
			long long val = std::stoll(token);
			if (val <= 0 || val > std::numeric_limits<int>::max()) {
				throw std::runtime_error("Error");
			}
			input_.push_back(static_cast<int>(val));
		} catch (const std::exception& e) {
			throw std::runtime_error("Error");
		}
	}
	if (input_.empty()) {
		throw std::runtime_error("Error");
	}
}

PmergeMe::PmergeMe(int argc, char** argv) {
	parseInput_(argc, argv);
}

void PmergeMe::sortVec_() {

	//comparisonCount = 0;

	// construct input: assign each value an id (to identify duplicate values)
	std::vector<Item> input;
	for (std::size_t i = 0; i < vec_.size(); i++) {
		input.push_back({vec_[i], i});
	}

	// recursively ford-johnson input
	input = mergeInsertionSortVec(input);

	// overwrite vec_ with sorted values
	for (std::size_t i = 0; i < input.size(); ++i) {
		vec_[i] = input[i].value;
	}

	//std::cout << "Vector implementation comparison count: " << comparisonCount << std::endl;
}

void PmergeMe::sortDeq_() {

	//comparisonCount = 0;

	std::deque<Item> input;
	for (std::size_t i = 0; i < deq_.size(); i++) {
		input.push_back({deq_[i], i});
	}

	input = mergeInsertionSortDeq(input);

	for (std::size_t i = 0; i < input.size(); ++i) {
		deq_[i] = input[i].value;
	}

	//std::cout << "Deque implementation comparison count: " << comparisonCount << std::endl;
}
/*
	container filling is handled here, so that all data management is included in
	the timed interval
	
*/
void PmergeMe::sort() {
	std::cout << "Before: ";
	printSequence(input_);
	auto vecStart = std::chrono::steady_clock::now();
	vec_ = input_;
	sortVec_();
	auto vecEnd = std::chrono::steady_clock::now();
	double vecTime = std::chrono::duration<double, std::micro>(vecEnd - vecStart).count();
	auto deqStart = std::chrono::steady_clock::now();
	deq_.assign(input_.begin(), input_.end());
	sortDeq_();
	auto deqEnd = std::chrono::steady_clock::now();
	double deqTime = std::chrono::duration<double, std::micro>(deqEnd - deqStart).count();
	std::cout << "After: ";
	printSequence(vec_);
	std::cout	<< "Time to process a range of " << vec_.size()
				<< " elements with std::vector : " << vecTime << " µs" << std::endl;
	std::cout	<< "Time to process a range of " << deq_.size()
				<< " elements with std::deque : " << deqTime << " µs" << std::endl;
}
