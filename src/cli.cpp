#include <cli.hxx>
#include <cstring>
#include <stdexcept>
#include <iostream>

Config parse_args(int argc, char **argv) {
	if (argc < 2) {
		std::cerr << ("Usage: Netsim <config_file> [options]\n");
		std::cerr << ("Use --help for more informations\n");
		exit(1);
	}

	if (argc == 2 && strcmp(argv[1], "--help") == 0) {
		std::cout << "\n--turns <int>: Length of the simulation\n";
		std::cout << "--report <type> <value>: Report notifier type\n";
		std::cout << "Available types:\n";
		std::cout << "\tinterval <int> e.g. interval 5\n";
		std::cout << "\tspecific <std::vector<int> e.g. specific 2 4 5 6\n";
		exit(1);
	}

	Config cfg;

	cfg.factory_config = argv[1];

	for (int i = 2; i < argc; i++) {
		std::string arg = argv[i];

		if (arg == "--turns") {
			if (++i > argc) throw std::runtime_error("Missing value for --turns");
			cfg.turns = std::stoi(argv[i]);
		}
		else if (arg == "--report") {
			if (++i > argc) throw std::runtime_error("Missing report mode");
			
			std::string mode = argv[i];

			if (mode == "interval") {
				cfg.mode = ReportMode::Interval;
				if (++i > argc) throw std::runtime_error("Missing value for interval");

				cfg.interval_turns = std::stoi(argv[i]);
			}
			else if (mode == "specific") {
				cfg.mode = ReportMode::Specific;
				while (i + 1 < argc && argv[i + 1][0] != '-') {
					cfg.specific_turns.insert(std::stoi(argv[++i]));
				}
			}
			else throw std::runtime_error("Wrong mode");
		}
	}
	return cfg;
}
