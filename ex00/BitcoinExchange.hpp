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

	// reads user-provided file and outputs data
	void processFile(const std::string& filename);

};