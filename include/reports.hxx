#pragma once

#include "factory.hxx"
#include "nodes.hxx"
#include "types.hxx"
#include <types.hxx>

extern void receivers_id_sorting(const ReceiverPreferences::preferences_t& prefs, std::vector<IPackageReceiver*> receivers);

extern std::string enum_to_string(ReceiverType type);
extern std::string enum_to_string(PackageQueueType type);

class SpecificTurnsReportNotifier {
public:
	SpecificTurnsReportNotifier(std::set<Time> turns): turns_(turns) {}
	bool should_generate_report(Time t);



private:
	std::set<Time> turns_;
};



class IntervalReportNotifier {
public:
	IntervalReportNotifier(TimeOffset to): to_(to) {}
	bool should_generate_report(Time t);


private:
	TimeOffset to_;
};




extern void generate_structure_report(const Factory& f, std::ostream& os);
extern void generate_structure_turn_report(const Factory& f, std::ostream& os, Time t);


