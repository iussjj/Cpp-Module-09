#include "BitcoinExchange.hpp"
#include <cctype>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

	/*
		checks for correct date format, used in processing both
		database and input files
		correct format: YYYY-MM-DD
	*/
	bool isValidDate(const std::string& date) {

		if (date.length() != 10) return false;

		if (date[4] != '-' || date[7] != '-') return false;

		for (int i = 0; i < 10; ++i) {
			if (i == 4 || i == 7) continue;
			
			if (!isdigit(static_cast<unsigned char>(date[i]))) return false;
		}

		int year, month, day;
		try {
			year = std::stoi(date.substr(0, 4));
			month = std::stoi(date.substr(5, 2));
			day = std::stoi(date.substr(8, 2));
		} catch (...) {
			return false;
		}

		if (month < 1 || month > 12 || day < 1 || day > 31) return false;

		int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

		if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
			daysInMonth[2] = 29;
		}

		if (day > daysInMonth[month]) return false;

		return true;
	}

	/*
		used to validate and return the value of a single database price data cell
		-returns nullopt in all invalid cases
	*/
	std::optional<double> parseDatabasePrice(const std::string& number) {
		
		//stod doesn't consume trailing invalid characters,
		//so pos is compared against number.length()
		std::size_t pos = 0;
		double price = 0.0;

		try {
			price = std::stod(number, &pos);
		}
		// catches all stod exceptions (out of range, invalid characters)
		catch (...) {
			return std::nullopt;
		}

		// catches trailing invalid characters and negative numbers
		if (pos < number.length() || price < 0 || !std::isfinite(price)) {
			return std::nullopt;
		}

		return price;
	}

	/*
		validates a single csv database line and returns its values as a pair
	*/
	std::pair<std::string, double> processDatabaseLine(const std::string& line) {

		// check for at least one delineator
		std::size_t pos = line.find(',');
		if (pos == std::string::npos) {
			throw std::runtime_error(std::string("Invalid database."));
		}

		// check that value before delineator is a valid date
		std::string date = line.substr(0, pos);
		if (!isValidDate(date)) {
			throw std::runtime_error(std::string("Invalid database."));
		}

		// check that value after delineator is a valid number, and save it
		std::optional<double> price = parseDatabasePrice(line.substr(pos + 1));

		if (!price) {
			// catches invalid price input
			throw std::runtime_error(std::string("Invalid database."));
		}

		// here the dereference * is used to unwrap the raw value from std::optional wrapper
		return {date, *price};
	}

	double parseInputValue(const std::string& valueString, const std::string& wholeLine) {
		std::size_t pos = 0;
		double value = 0.0;

		try {
			value = std::stod(valueString, &pos);
		}
		// catches completely invalid inputs like 'lol XD'
		catch (const std::invalid_argument&) {
			throw std::runtime_error("Error: bad input => " + wholeLine);
		}
		catch (const std::out_of_range&) {
			throw std::runtime_error("Error: too large a number.");
		}
		
		//catch trailing invalid characters
		if (pos != valueString.length()) {
			throw std::runtime_error("Error: bad input => " + wholeLine);
		}

		//handle nan, inf and -infinity edge case
		if (!std::isfinite(value)) {
			throw std::runtime_error("Error: bad input => " + wholeLine);
		}

		if (value < 0) {
			throw std::runtime_error("Error: not a positive number.");
		}

		if (value > 1000) {
			throw std::runtime_error("Error: too large a number.");
		}

		return value;
	}

	std::pair<std::string, double> processInputLine(const std::string& line) {
		std::size_t pos = line.find(" | ");
		if (pos == std::string::npos) {
			throw std::runtime_error("Error: bad input => " + line);
		}
		std::string date = line.substr(0, pos);
		if (!isValidDate(date)) {
			throw std::runtime_error("Error: bad input => " + line);
		}
		double value = parseInputValue(line.substr(pos + 3), line);
		return {date, value};

	}

} //namespace

/*
	iterates through all database csv lines, validates and extracts the values,
	and populates the priceData_ map with them. 
*/
void BitcoinExchange::parsePriceData(const std::string& filename) {

	std::ifstream file(filename);
	if (!file.is_open()) {
		throw std::runtime_error("Error: could not read database.");
	}

	std::string line;
	// skip csv header line
	std::getline(file, line);

	// getline takes an input file stream to read from,
	// and a string to store the extracted line in, as inputs
	while (getline(file, line)) {
			//protect against trailing empty line (also ignores intermediary empty lines)
			if (line.empty()) {
				continue;
			}
			// structured binding maps composit types like std::pair and std::tuple
			// to named local variables
			auto [date, price] = processDatabaseLine(line);

			// NOTE! no try-catch block needed here: stack unwinding means that
			// an exception deep in the call stack will be caught by a catch
			// block at the top, without needing intermediate catch blocks

			//insert values into map
			priceData_[date] = price;
	}
	if (priceData_.empty()) {
		throw std::runtime_error("Error: database contains no price data.");
	}
}

// default constructor calls parametrized constructor
BitcoinExchange::BitcoinExchange() : BitcoinExchange("data.csv") {}

BitcoinExchange::BitcoinExchange(const std::string& databaseFilename) {
	parsePriceData(databaseFilename);
}

BitcoinExchange::~BitcoinExchange() {}

BitcoinExchange::BitcoinExchange(const BitcoinExchange& source) : priceData_(source.priceData_) {}

BitcoinExchange& BitcoinExchange::operator=(const BitcoinExchange& source) {
	if (this != &source) {
		priceData_ = source.priceData_;
	}
	return *this;
}

// reads user-provided file
void BitcoinExchange::processFile(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		throw std::runtime_error("Error: could not open file.");
	}

	std::string line;
	// skip input header line
	std::getline(file, line);

	while (getline(file, line)) {
		if (line.empty()) {
			continue;
		}
		try {
			auto [date, value] = processInputLine(line);

			// exact date OR closest date after it
			// NOTE: subject calls for closest date BEFORE
			auto it = priceData_.lower_bound(date);

			// if input date is earlier than the first database entry
			if (it == priceData_.begin() && it -> first != date) {
				std::cerr	<< "Error: requested date " << date << " predates earliest database entry "
							<< it->first << std::endl;
				continue;
			}

			//if lower_bound() didn't find an exact match
			if (it == priceData_.end() || it->first != date) {
				// decrement iterator to match subject requirement
				--it;
			}

			double rate = it->second;

			// to access map values, use .at(date) NOT [date]
			// with [], nonexistent keys are created with zero values
			std::cout	<< date << " => " << value << " = "
						<< (rate * value) << std::endl;
		} catch (const std::exception& e) {
			// catches errors and outputs the message
			std::cerr << e.what() << std::endl;
		}
	}
}