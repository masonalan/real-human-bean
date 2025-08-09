//
// Created by James Pickering on 8/9/25.
//

#include "log.hpp"

#include <fstream>
#include <iostream>

auto writeStdOutToFile(const std::string& file) -> void {
	static std::streambuf* originalCoutBuffer =
		nullptr;  // To store the original buffer

	if (originalCoutBuffer == nullptr) {		 // If not already redirected
		originalCoutBuffer = std::cout.rdbuf();	 // Save the original streambuf
	}

	static std::ofstream outputFile;  // Use static to keep it alive
	if (outputFile.is_open()) {
		outputFile.close();	 // Close previous file if open
	}
	outputFile.open(file);

	if (outputFile.is_open()) {
		std::cout.rdbuf(outputFile.rdbuf());  // Redirect std::cout to the file
	} else {
		// Handle error if file cannot be opened
		std::cerr << "Error: Could not open log file: " << file << std::endl;
		std::cout.rdbuf(
			originalCoutBuffer);  // Restore original cout if redirection failed
	}
}
