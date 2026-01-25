#include "factory.hxx"
#include "types.hxx"
#include <functional>


extern void simulate(Factory &f, TimeOffset d, std::function<void (Factory&, Time)> rf);
