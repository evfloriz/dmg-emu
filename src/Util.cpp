#include <iostream>
#include <fstream>

#include "Util.h"

namespace util {
	// Initialize options with default values
	std::unordered_map<std::string, std::string> options = {
		{"romPath", ""},
		{"palette", "mgb"},
		{"pixelScale", "1"},
		{"displayFPS", "0"},
		{"debugMode", "0"},
		{"logPath", "logs/log.txt"},
	};

	std::string romPath = options["romPath"];
	std::string palette = options["palette"];
	int pixelScale = std::stoi(options["pixelScale"]);
	int displayFPS = std::stoi(options["displayFPS"]);
	int debugMode = std::stoi(options["debugMode"]);
	std::string logPath = options["logPath"];
}


int util::readOptionsFile() {
	// Overwrite options with input from options.txt
	// If an unexpected option is present, print an error message and return

	// TODO: Make sure options.txt is valid
	
	std::ifstream ifs;
	ifs.open("options.txt");
	if (!ifs) {
		std::cout << "Error opening options.txt" << std::endl;
		return -1;
	}

	std::string line;
	while (std::getline(ifs, line)) {
		// Ignore commented out lines
		if (line.find("#") == 0) {
			continue;
		}
		
		size_t pos = line.find("=");
		if (pos != std::string::npos) {
			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1, line.length());

			if (util::options.count(key)) {
				util::options[key] = value;
			}
			else {
				std::cout << key << " is an invalid option in options.txt" << std::endl;
				ifs.close();
				return -1;
			}

			if ((key == "pixelScale" || key == "displayFPS" || key == "debugMode") && !isNumber(value)) {
				std::cout << value << " is an invalid value for " << key << " in options.txt" << std::endl;
				ifs.close();
				return -1;
			}

			if (key == "pixelScale" && std::stoi(value) < 1) {
				std::cout << value << " is an invalid pixel scale" << std::endl;
				ifs.close();
				return -1;
			}

			if (key == "palette" && !(value == "mgb" || value == "dmg")) {
				std::cout << value << " is an unrecognized palette" << std::endl;
				ifs.close();
				return -1;
			}
		}
		else {
			std::cout << line << " is an invalid line in options.txt" << std::endl;
			ifs.close();
			return -1;
		}
	}

	romPath = options["romPath"];
	palette = options["palette"];
	pixelScale = std::stoi(options["pixelScale"]);
	displayFPS = std::stoi(options["displayFPS"]);
	debugMode = std::stoi(options["debugMode"]);
	logPath = options["logPath"];
	
	ifs.close();
	return 0;
}
