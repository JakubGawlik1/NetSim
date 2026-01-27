#include "reports.hxx"
#include <iostream>
#include <factory.hxx>
#include <fstream>
#include <cli.hxx>
#include <memory>
#include <stdexcept>
#include <simulation.hxx>


int main(int argc, char** argv)
{
	
	auto cfg = parse_args(argc, argv);

	std::ifstream file(cfg.factory_config);
	
	if (!file.is_open()) {
		std::cerr << "Cannot open file: " << cfg.factory_config << "\n";
		return 1;
	}

	Factory f = load_factory_structure(file);

	std::unique_ptr<IReportNotifier> notifier;
	switch (cfg.mode) {
		case ReportMode::Interval: {
			notifier = std::make_unique<IntervalReportNotifier>(cfg.interval_turns);
			break;
		}

		case ReportMode::Specific: {
			notifier = std::make_unique<SpecificTurnsReportNotifier>(cfg.specific_turns);
			break;
		}

		case ReportMode::None: {
			throw std::runtime_error("Wrong report mode");
		}
	}
	
	generate_structure_report(f, std::cout);
	simulate(f, cfg.turns, [&notifier] (Factory &factory, Time time) {
		if (notifier->should_generate_report(time)) {
			generate_structure_turn_report(factory, std::cout, time);
		}
		
	});

}
