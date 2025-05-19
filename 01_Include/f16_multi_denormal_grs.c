#include "f16_arith.h"
const char* _MULTI_DENORMAL_GRS_VERSION_ = "250516.01";

#define NORMAL    0
#define OVERFLOW  1
#define ZERO      2
#define UNDERFLOW 2
#define DENORMAL  3

#define BIAS 15


unsigned int f16_multi_denormal_grs(unsigned int x, unsigned int y) {
    unsigned int flag = NORMAL;         // Noraml: 0, Overflow: 1, Underflow: 2, Zero: 2, Denoraml: 3
    unsigned int sign_x, sign_y, sign;  // Sign(1-bit)
    int          expo_x, expo_y, expo;  // Exponent(5-bit)
    unsigned int mant_x, mant_y, mant;  // Mantissa(10-bit)

    unsigned int sigf_x, sigf_y;        // Significand(11-bit) : Hidden 1-bit + Mantissa
    unsigned int aligned;               // Aligned Significand(22bit -> 14-bit) : Significand + GRS-bit
    
    int denormal_input_flag = 0;        // Flag of denormal input
    int shift_exp = 0;                  // Leading Zero Count
    int denormal_shift = 0;             // Shift value of aligned for denormal
    unsigned int lsb;                   // LSB of Aligned
    unsigned int g, r, s;               // Guard-bit, Round-bit, Sticky-bit of Aligned
    unsigned int result;                // Sign + Exponent + Mantissa

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
    if(expo_x == 0 && mant_x == 0) flag = ZERO;
    if(expo_y == 0 && mant_y == 0) flag = ZERO;

    // Hidden 1-bit + Mantissa (11-bit)
    if(expo_x == 0) sigf_x = mant_x;  // Denormal
    else sigf_x = mant_x | 1 << 10;   // Normal
    if(expo_y == 0) sigf_y = mant_y;  // Denormal
    else sigf_y = mant_y | 1 << 10;   // Normal


    if(expo_x == 0 || expo_y == 0) denormal_input_flag = 1;
    if(expo_x == 0 && expo_y == 0) flag = UNDERFLOW;

    // Multiply: significand_x * significand_y = aligned(22-bit)
    aligned = (sigf_x * sigf_y) & 0x3FFFFF;

    // Leading Zero Counter(22-bit기준: Hidden + Mantissa + 11-bit)
    if      (aligned & 0x200000) shift_exp =  0;
    else if (aligned & 0x100000) shift_exp =  1;
    else if (aligned &  0x80000) shift_exp =  2;
    else if (aligned &  0x40000) shift_exp =  3;
    else if (aligned &  0x20000) shift_exp =  4;
    else if (aligned &  0x10000) shift_exp =  5;
    else if (aligned &   0x8000) shift_exp =  6;
    else if (aligned &   0x4000) shift_exp =  7;
    else if (aligned &   0x2000) shift_exp =  8;
    else if (aligned &   0x1000) shift_exp =  9;
    else if (aligned &    0x800) shift_exp = 10;
    else if (aligned &    0x400) shift_exp = 11;
    // else if (aligned &    0x200) shift_exp = 12;
    // else if (aligned &    0x100) shift_exp = 13;
    // else if (aligned &     0x80) shift_exp = 14;
    // else if (aligned &     0x40) shift_exp = 15;
    // else if (aligned &     0x20) shift_exp = 16;
    // else if (aligned &     0x10) shift_exp = 17;
    // else if (aligned &      0x8) shift_exp = 18;
    // else if (aligned &      0x4) shift_exp = 19;
    // else if (aligned &      0x2) shift_exp = 20;
    // else if (aligned &      0x1) shift_exp = 21;
    else                          shift_exp = 0;

    // Calculate Exponent 
    expo = expo_x + expo_y - BIAS - shift_exp + denormal_input_flag;
    if(0 <= expo){  // Normal
        expo = expo + 1;
    }else{          // Denormal
        denormal_shift = (-expo) - shift_exp;
    }

    // Normalizing
    if (32 <= expo) {                           // Overflow
        flag = OVERFLOW;
    } else if((1 <= expo) && (expo <= 31)) {    // Normal
        aligned = aligned << (shift_exp);
        unsigned int mask = (1 << 8) - 1;
        unsigned int sticky = (aligned & mask) ? 1 : 0;
        aligned = (aligned >> 8 | sticky) & 0x3FFF;
    } else if ((-13 <= expo) && (expo <= 0)) {  // Denormal(-13 <= expo <= 0)
        if(0 <= denormal_shift){                // if (denormal_shift == +): Right Shift
            unsigned int mask = (1 << denormal_shift) - 1;
            unsigned int sticky = (aligned & mask) ? 1 : 0;
            aligned = (aligned >> denormal_shift | sticky);
            mask = (1 << 8) - 1;
            sticky = (aligned & mask) ? 1 : 0;
            aligned = (aligned >> 8 | sticky) & 0x3FFF;
        }else{                                  // if (denormal_shift == -): Left Shift
            aligned = aligned << -denormal_shift;
            unsigned int mask = (1 << 8) - 1;
            unsigned int sticky = (aligned & mask) ? 1 : 0;
            aligned = (aligned >> 8 | sticky) & 0x3FFF;
        }
        expo = 0;
    } else if (expo <= -14) {                   // Underflow
        flag = UNDERFLOW;
    }

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
        }else{
            ;
        }
    }

    // 2nd Rounding
    if(aligned & 0x800 && expo == 0x1F) {        // Overflow
        flag = OVERFLOW;
    }else if(aligned & 0x800){                   // Normal
        expo = expo + 1;
        aligned = aligned >> 1;
    }else if((aligned & 0x400) && expo == 0x0){  // Denormal
        expo = expo + 1;
    }

    mant = aligned & 0x3FF;

    if(flag == NORMAL)                         result = (sign << 15) | (expo << 10) |  mant;  // Normal
    else if(flag == OVERFLOW)                  result = (sign << 15) | (0x1F << 10) | 0x3FF;  // Overflow
    else if(flag == UNDERFLOW || flag == ZERO) result = (sign << 15) | (0x00 << 10) | 0x000;  // Underflow
    else if(flag == DENORMAL)                  result = (sign << 15) | (expo << 10) |  mant;  // Denormal

    return result;
}
