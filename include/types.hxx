#pragma once
#include <functional>

using ElementID = int;
using Time = unsigned int;
using TimeOffset = unsigned int;
using ProbabilityGenerator = std::function<double()>;
