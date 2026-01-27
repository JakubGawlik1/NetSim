#pragma once

#include <string>
#include <vector>
#include <set>
#include <types.hxx>

enum class ReportMode {
	None,
	Interval,
	Specific
};


struct Config {
	std::string factory_config;
	int turns = 1;
	int interval_turns = 0;
	ReportMode mode = ReportMode::None;
	std::set<Time> specific_turns;
};

Config parse_args(int argc, char** argv);
