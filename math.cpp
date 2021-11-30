#include <string>
#include <valarray>
#include "math.h"

double Math::Log2(double x)
    {
    return std::log(x) / std::log(2.);
    }