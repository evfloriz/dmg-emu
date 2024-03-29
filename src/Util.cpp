#include <iostream>
#include <fstream>

#include "Util.h"

// TODO: Make a macro for prints that does the correct one automatically
#ifdef VITA
#include <psp2/kernel/processmgr.h>
#include "debugScreen.h"
#define printf psvDebugScreenPrintf
#endif

namespace util {
	// Initialize options with default values
	std::unordered_map<std::string, std::string> options = {
		{"romPath", ""},
		{"palette", "mgb"},
		{"pixelScale", "1"},
		{"displayFPS", "0"},
		{"debugMode", "0"},
		{"fontPath", ""},
		{"logPath", "logs/log.txt"},
	};

	std::string romPath = options["romPath"];
	std::string palette = options["palette"];
	int pixelScale = std::stoi(options["pixelScale"]);
	int displayFPS = std::stoi(options["displayFPS"]);
	int debugMode = std::stoi(options["debugMode"]);
	std::string fontPath = options["fontPath"];
	std::string logPath = options["logPath"];
}

int util::readOptionsFile() {
	// Overwrite options with input from options.txt
	// If an unexpected option is present, print an error message and return

	// TODO: Make sure options.txt is valid
	
	std::ifstream ifs;
	ifs.open("options.txt");
	if (!ifs) {
		printf("Error opening options.txt\n");
		return -1;
	}

	std::string line;
	while (std::getline(ifs, line)) {
		// Ignore commented out lines
		if (line.find("#") == 0) {
			continue;
		}
		
		size_t pos = line.find("=");
		
		if (pos == std::string::npos) {
			printf((line + " is an invalid line in options.txt\n").c_str());
			ifs.close();
			return -1;
		}

		std::string key = line.substr(0, pos);
		std::string value = line.substr(pos + 1, line.length());

		// Strip \r off of value if it's present
		if (value[value.length() - 1] == '\r') {
			value = value.substr(0, value.length() - 1);
		}

		if (util::options.count(key) == 0) {
			printf((key + " is an invalid option in options.txt\n").c_str());
			ifs.close();
			return -1;
		}

		if ((key == "pixelScale" || key == "displayFPS" || key == "debugMode") && !isNumber(value)) {	
			printf((value + " is an invalid value for " + key + " in options.txt\n").c_str());
			ifs.close();
			return -1;
		}

		if (key == "pixelScale" && std::stoi(value) < 1) {
			printf((value + " is an invalid pixel scale\n").c_str());
			ifs.close();
			return -1;
		}

		if (key == "palette" && !(value == "mgb" || value == "dmg")) {
			printf((value + " is an unrecognized palette\n").c_str());
			ifs.close();
			return -1;
		}

		util::options[key] = value;
	}

	romPath = options["romPath"];
	palette = options["palette"];
	pixelScale = std::stoi(options["pixelScale"]);
	displayFPS = std::stoi(options["displayFPS"]);
	debugMode = std::stoi(options["debugMode"]);
	fontPath = options["fontPath"];
	logPath = options["logPath"];

	// Check for fontPath if displayFPS is 1
	if (displayFPS && fontPath == "") {
		printf("displayFPS=1 requires fontPath\n");
		ifs.close();
		return -1;
	}
	
	ifs.close();
	return 0;
}
