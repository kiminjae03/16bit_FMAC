#include <stdio.h>
#include "DataStruct.h"
#include "../Common/DataType.h"
#include "bitconv.h"

float f_f32f24(float x)
{
	union u_fi fi;
	int sign;
	int exponent;
	int mantisa;

	fi.fvalue = x;


	sign = fi.ivalue & 0x80000000;
	exponent = fi.ivalue & 0x7F800000;
	mantisa = fi.ivalue & 0x7FFFFF;


	sign = sign >> 31;
	exponent = exponent >> 23;

	if (exponent <= 96) exponent = 0;
	else if (exponent >= 160) exponent = 63;
	//   else exponent -=64;

	mantisa = mantisa;
	sign = sign << 31;
	exponent = exponent << 23;

	fi.ivalue = sign + exponent + mantisa;

	return fi.fvalue;
}
#if defined(SoundFloat24)
// 1_8_23 ==> 1_6_17
 unsigned int x_f32f24(float x)
{
	union u_fi fi;
	int sign;
	int exponent;
	int mantisa;
	fi.fvalue = x;


	sign     = fi.ivalue & 0x80000000;
	exponent = fi.ivalue & 0x7F800000;
	mantisa  = fi.ivalue & 0x7FFFFF;


	sign = sign >> 31;
	sign = sign&1;
	exponent = exponent >> 23;

	if (exponent <= 96) { 
		exponent = 0;
		mantisa = 0;
		sign = 0;
	}
	else if (exponent >= 160 ){
		exponent = 63;
		mantisa = 0x7FFFFF;
	}
	else exponent -=96;

	mantisa = mantisa >> 6;
	sign = sign << 23;
	exponent = exponent << 17;

	fi.ivalue = sign + exponent + mantisa;

	return fi.ivalue;   
}
#endif
unsigned int x_f32f24_test(float x)
{
	union u_fi fi;
	int sign;
	int exponent;
	int mantisa;
	fi.fvalue = x;

	sign = fi.ivalue & 0x80000000;
	exponent = fi.ivalue & 0x7F800000;
	mantisa = fi.ivalue & 0x7FFFFF;

	sign = sign >> 31;
	sign = sign & 1;
	exponent = exponent >> 23;

	if (exponent <= 96) {
		exponent = 0;
		mantisa = 0;
		sign = 0;
	}
	else if (exponent >= 160) {
		exponent = 63;
		mantisa = 0x7FFFFF;
	}
	else exponent -= 96;

	mantisa = mantisa >> 6;
	sign = sign << 23;
	exponent = exponent << 17;

	fi.ivalue = sign + exponent + mantisa;

	return fi.ivalue;
}


unsigned int x_f32f16(float x)
{
   union u_fi fi;
   int sign;
   int exponent;
   int mantisa;
   fi.fvalue = x;

   
   sign     = fi.ivalue & 0x80000000;
   exponent = fi.ivalue & 0x7F800000;
   mantisa  = fi.ivalue & 0x7FFFFF;

   
   sign = sign >> 31;
   sign = sign&1;
   exponent = exponent >> 23;
   
   if (exponent <= 96) { 
      exponent = 0;
      mantisa = 0;
      sign = 0;
   }
   else if (exponent >= 160 ){
      exponent = 63;
      mantisa = 0x7FFFFF;
   }
   else exponent -=96;
   mantisa = (mantisa >> 6)&0x1FFE0;
   sign = sign << 23;
   exponent = exponent << 17;

   fi.ivalue = sign + exponent + mantisa;
   
   return fi.ivalue;   
}
unsigned int x_x32x24(unsigned int x)
{
   union u_fi fi;
   int sign;
   int exponent;
   int mantisa;
   fi.ivalue = x;

   
   sign     = fi.ivalue & 0x80000000;
   exponent = fi.ivalue & 0x7F800000;
   mantisa  = fi.ivalue & 0x7FFFFF;

   
   sign = sign >> 31;
   sign = sign&1;
   exponent = exponent >> 23;
   exponent = exponent&0xFF;

   if (exponent <= 96) { 
      exponent = 0;
      mantisa = 0;
      sign = 0;
   }
   else if (exponent >= 160 ){
      exponent = 63;
      mantisa = 0x7fffff;
   }
   else exponent -=96;
   mantisa = mantisa >> 6;
   sign = sign << 23;
   exponent = exponent << 17;

   fi.ivalue = sign + exponent + mantisa;
   
   return fi.ivalue;   
}

#if defined(SoundFloat24)
// 1_8_23 ==> 1_7_16
float f24f32(int x)
{
   union u_fi fi;
   int sign;
   int exponent;
   int mantisa;

   fi.ivalue = x;

   sign     = fi.ivalue & 0x800000;
   exponent = fi.ivalue & 0x7E0000;
   mantisa  = fi.ivalue & 0x1FFFF;

   sign = sign >> 23;
   exponent = exponent >> 17;
   exponent += 96;
   

   sign = sign << 31;
   exponent = exponent << 23;
   mantisa = mantisa << 6;
   if((fi.ivalue & 0x7E0000) ==0)
      fi.ivalue=0;
   else fi.ivalue = sign + exponent + mantisa;

   return fi.fvalue;   
}

unsigned int f24Int(unsigned int x)
{
	union u_fi fi;
	int exponent;
	int mantisa;

	fi.ivalue = x;

	exponent = fi.ivalue & 0x7E0000;
	mantisa = fi.ivalue & 0x1FFFF;

	exponent = exponent >> 17;
	if (exponent < 0x1f)
		return 0;

	exponent -= 0x1f;

	mantisa = mantisa + 0x20000;
	mantisa = mantisa << exponent;
	mantisa = mantisa >> 17;
	return mantisa;
}

#endif



int f24x32(int x)
{
	union u_fi fi;
	int sign;
	int exponent;
	int mantisa;

	fi.ivalue = x;

	sign = fi.ivalue & 0x800000;
	exponent = fi.ivalue & 0x7E0000;
	mantisa = fi.ivalue & 0x1FFFF;

	sign = sign >> 23;
	exponent = exponent >> 17;
	exponent += 96;


	sign = sign << 31;
	exponent = exponent << 23;
	mantisa = mantisa << 6;
	if ((fi.ivalue & 0x7E0000) == 0)
		fi.ivalue = 0;
	else fi.ivalue = sign + exponent + mantisa;

	return fi.ivalue;
}

///////////////old/////////////////
float f_f32f24_old(float x)
{
	union u_fi fi;
	int sign;
	int exponent;
	int mantisa;

	fi.fvalue = x;


	sign = fi.ivalue & 0x80000000;
	exponent = fi.ivalue & 0x7F800000;
	mantisa = fi.ivalue & 0x7FFFFF;


	sign = sign >> 31;
	exponent = exponent >> 23;

	if (exponent <= 64) exponent = 0;
	else if (exponent >= 192) exponent = 127;
	//   else exponent -=64;

	mantisa = mantisa;
	sign = sign << 31;
	exponent = exponent << 23;

	fi.ivalue = sign + exponent + mantisa;

	return fi.fvalue;
}

// 1_8_23 ==> 1_7_16
unsigned int x_f32f24_old(float x)
{
	union u_fi fi;
	int sign;
	int exponent;
	int mantisa;
	fi.fvalue = x;


	sign = fi.ivalue & 0x80000000;
	exponent = fi.ivalue & 0x7F800000;
	mantisa = fi.ivalue & 0x7FFFFF;


	sign = sign >> 31;
	sign = sign & 1;
	exponent = exponent >> 23;
	exponent = exponent & 0xFF;

	if (exponent <= 64) {
		exponent = 0;
		mantisa = 0;
		sign = 0;
	}
	else if (exponent >= 192) {
		exponent = 127;
		mantisa = 0x7fffff;
	}
	else exponent -= 64;
	mantisa = mantisa >> 7;
	sign = sign << 23;
	exponent = exponent << 16;

	fi.ivalue = sign + exponent + mantisa;

	return fi.ivalue;
}
unsigned int x_f32f16_old(float x)
{
	union u_fi fi;
	int sign;
	int exponent;
	int mantisa;
	fi.fvalue = x;


	sign = fi.ivalue & 0x80000000;
	exponent = fi.ivalue & 0x7F800000;
	mantisa = fi.ivalue & 0x7FFFFF;


	sign = sign >> 31;
	sign = sign & 1;
	exponent = exponent >> 23;
	exponent = exponent & 0xFF;

	if (exponent <= 64) {
		exponent = 0;
		mantisa = 0;
		sign = 0;
	}
	else if (exponent >= 192) {
		exponent = 127;
		mantisa = 0x7fffff;
	}
	else exponent -= 64;
	mantisa = (mantisa >> 7) & 0xfff0;
	sign = sign << 23;
	exponent = exponent << 16;

	fi.ivalue = sign + exponent + mantisa;

	return fi.ivalue;
}
unsigned int x_x32x24_old(unsigned int x)
{
	union u_fi fi;
	int sign;
	int exponent;
	int mantisa;
	fi.ivalue = x;


	sign = fi.ivalue & 0x80000000;
	exponent = fi.ivalue & 0x7F800000;
	mantisa = fi.ivalue & 0x7FFFFF;


	sign = sign >> 31;
	sign = sign & 1;
	exponent = exponent >> 23;
	exponent = exponent & 0xFF;

	if (exponent <= 64) {
		exponent = 0;
		mantisa = 0;
		sign = 0;
	}
	else if (exponent >= 192) {
		exponent = 127;
		mantisa = 0x7fffff;
	}
	else exponent -= 64;
	mantisa = mantisa >> 7;
	sign = sign << 23;
	exponent = exponent << 16;

	fi.ivalue = sign + exponent + mantisa;

	return fi.ivalue;
}


// 1_8_23 ==> 1_7_16
float f24f32_old(int x)
{
	union u_fi fi;
	int sign;
	int exponent;
	int mantisa;

	fi.ivalue = x;

	sign = fi.ivalue & 0x800000;
	exponent = fi.ivalue & 0x7F0000;
	mantisa = fi.ivalue & 0xFFFF;

	sign = sign >> 23;
	exponent = exponent >> 16;
	exponent += 64;


	sign = sign << 31;
	exponent = exponent << 23;
	mantisa = mantisa << 7;
	if (x == 0)
		fi.ivalue = 0;
	else fi.ivalue = sign + exponent + mantisa;

	return fi.fvalue;
}
