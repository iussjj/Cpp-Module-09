#pragma once

#include <map>
#include <string>

class BitcoinExchange {
private:
	// store data extracted from data.csv: date (YYYY-MM-DD) mapped to price (#.#####)
	std::map<std::string, double> priceData_;

	void parsePriceData(const std::string& filename);

public:
	BitcoinExchange();
	BitcoinExchange(const std::string& databaseFilename);
	~BitcoinExchange();
	BitcoinExchange(const BitcoinExchange& source);
	BitcoinExchange& operator=(const BitcoinExchange& source);

	// reads user-provided file
	void readFile(std::string_view filename);

};

/*
	NOTES:
	std::string_view is efficient for read-only: no memory allocation and near-instant
	substring operations like .substr(), .remove_prefix() or remove_suffix().
*/
