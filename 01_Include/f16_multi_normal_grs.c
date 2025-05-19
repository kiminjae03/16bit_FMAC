#include "f16_arith.h"
const char* _MULTI_NORMAL_GRS_VERSION_ = "250508.01";

#define NORMAL    0
#define OVERFLOW  1
#define ZERO      2
#define UNDERFLOW 2
// #define DENORMAL  3

#define BIAS 15


unsigned int f16_multi_normal_grs(unsigned int x, unsigned int y) {
    unsigned int flag = NORMAL;         // Noraml: 0, Overflow: 1, Underflow: 2, Zero: 2, Denoraml: 3
    unsigned int sign_x, sign_y, sign;  // Sign(1-bit)
    int          expo_x, expo_y, expo;  // Exponent(5-bit)
    unsigned int mant_x, mant_y, mant;  // Mantissa(10-bit)

    unsigned int sigf_x, sigf_y;        // Significand(11-bit) : Hidden 1-bit + Mantissa
    unsigned int aligned;               // Aligned Significand(14-bit) : Significand + GRS-bit
    
    int expo_add = 0;
    unsigned int lsb;      // LSB of Aligned
    unsigned int g, r, s;  // Guard-bit, Round-bit, Sticky-bit of Aligned
    unsigned int result;   // Sign + Exponent + Mantissa

    // Seperate (Sign / Expnent / Mantissa)
    sign_x = (x >> 15) & 0x1;
    sign_y = (y >> 15) & 0x1;

    expo_x = (x >> 10) & 0x1F;
    expo_y = (y >> 10) & 0x1F;

    mant_x = (x) & 0x3FF;
    mant_y = (y) & 0x3FF;

    // Sign
    sign = (sign_x ^ sign_y) & 0x1;

    // Zero = (? x 0) or (0 x ?)
    if(expo_x == 0 || expo_y == 0) flag = ZERO;

    // Hidden 1-bit + Mantissa (11-bit)
    sigf_x = mant_x | 1 << 10;   // Normal
    sigf_y = mant_y | 1 << 10;   // Normal

    // Multiply: significand_x * significand_y = aligned(22-bit)
    aligned = (sigf_x * sigf_y) & 0x3FFFFF;

    // 22-bit -> 14-bit (including Sticky-bit)
    if((aligned & 0x200000) != 0){
        unsigned int mask = (1 << 8) - 1;
        unsigned int sticky = (aligned & mask) ? 1 : 0;
        aligned = aligned >> 8 | sticky;
		aligned = aligned & 0x3FFF;
		expo_add = 1;
	}else{
        unsigned int mask = (1 << 7) - 1;
        unsigned int sticky = (aligned & mask) ? 1 : 0;
        aligned = aligned >> 7 | sticky;
		aligned = aligned & 0x3FFF;
		expo_add = 0;
	}

    // Exponent
    expo = (expo_x + expo_y) - BIAS + expo_add;

    // Normalizing
    if(32 <= expo){         // Overflow
        flag = OVERFLOW;
    }else if(expo <= 0) {   // Underflow
        flag = UNDERFLOW;
    }else{;}                // Normal

    // 1st Rounding
    lsb = (aligned & 0x8) >> 3;
    g = (aligned & 0x4) >> 2;
    r = (aligned & 0x2) >> 1;
    s = aligned & 0x1;

    aligned = (aligned >> 3);
    if(g == 0){;}                                // GRS : 0xx
    else if( g == 1 && ((r != 0) || (s != 0))){  // GRS : 11x or 1x1
        aligned = aligned + 1;
    }else{                                       // GRS : 100
        if(lsb == 1) {
            aligned = aligned + 1;
        }else{;}
    }

    // 2nd Rounding
    if(aligned & 0x800 && expo == 0x1F) {        // Overflow
        flag = OVERFLOW;
    }else if(aligned & 0x800){                   // Normal
        expo = expo + 1;
        aligned = aligned >> 1;
    }

    mant = aligned & 0x3FF;

    if(flag == NORMAL)                         result = (sign << 15) | (expo << 10) |  mant;  // Normal
    else if(flag == OVERFLOW)                  result = (sign << 15) | (0x1F << 10) | 0x3FF;  // Overflow
    else if(flag == UNDERFLOW || flag == ZERO) result = (sign << 15) | (0x00 << 10) | 0x000;  // Underflow

    return result;
}
