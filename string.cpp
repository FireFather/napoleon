#include "string.h"

string int32ToStr(const int32_t & val)
	{
	stringstream s;
	s << val;
	return s.str();
	}

string int64ToStr(const int64_t & val)
	{
	stringstream s;
	s << val;
	return s.str();
	}

string doubleToStr(const double & val)
	{
	stringstream s;
	s << val;
	return s.str();
	}