#pragma once

union u_fi
{
   float fvalue;
   int ivalue;
};

union ExtInt
{
#ifdef _WIN32
	__int64 value64;
#else
	__int64_t value64;
#endif
	int value32[2];
};
