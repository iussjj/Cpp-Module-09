#include "BitcoinExchange.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <string>
#include <optional>

namespace {

	/*
		checks for correct date format, used in processing both
		database and input files
	*/
	bool isValidDate(const std::string& date) {

		if (date.length() != 10) {
			return false;
		}

		// wrap date string in an input stream object
		std::istringstream date_stream(date);
		
		//declare C++20 calendar object with specified format
		std::chrono::year_month_day ymd;

		// feed input stream into parse function, which populates calendar object
		// %F is a shortcut for %Y-%m-%d which corresponds to YYYY-MM-DD format
		// if input invalid, stream state is set to false (invalid)
		date_stream >> std::chrono::parse("%F", ymd);

		// in case of invalid format, date_stream return false
		// ymd.ok() checks validity of date against gregorian calendar
		return date_stream && ymd.ok();
	}

	/*
		used to validate and return the value of a single database price data cell
		-returns nullopt in all invalid cases
	*/
	std::optional<double> parseDatabasePrice(const std::string& number) {
		
		//stod doesn't consume trailing invalid characters,
		//so pos is compared against number.length()
		size_t pos = 0;
		double price = 0.0;

		try {
			price = std::stod(number, &pos);
		}
		// catches all stod exceptions (out of range, invalid characters)
		catch (...) {
			return std::nullopt;
		}

		// catches trailing invalid characters and negative numbers
		if (pos < number.length() || price < 0) {
			return std::nullopt;
		}

		return price;
	}

	/*
		validates a single csv database line and returns its values as a pair
	*/
	std::pair<std::string, double> processDatabaseLine(const std::string& line) {

		// check for at least one delineator
		size_t pos = line.find(',');
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

}
/*
	iterates through all database csv lines, validates and extracts the values,
	and populates the priceData_ map with them. 
*/
void BitcoinExchange::parsePriceData(const std::string& filename) {

	std::ifstream file(filename);
	if (!file.is_open()) {
		throw std::runtime_error("Could not open database file.");
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
void BitcoinExchange::readFile(std::string_view filename) {

}