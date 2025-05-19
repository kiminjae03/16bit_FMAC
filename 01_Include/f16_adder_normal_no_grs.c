#include "f16_arith.h"
const char* _ADDER_NORMAL_NO_GRS_VERSION_ = "250517.01";

#define NORMAL    0
#define OVERFLOW  1
#define ZERO      2
#define UNDERFLOW 2
// #define DENORMAL  3


unsigned int f16_adder_normal_no_grs(unsigned int x, unsigned int y) {
    unsigned int flag = NORMAL;         // Noraml: 0, Overflow: 1, Underflow: 2, Zero: 2, Denoraml: 3
    unsigned int sign_x, sign_y, sign;  // Sign(1-bit)
    int          expo_x, expo_y, expo;  // Exponent(5-bit)
    unsigned int mant_x, mant_y, mant;  // Mantissa(10-bit)

    unsigned int great_value;                    // Magnitude Compare (Exponent + Mantissa)
    unsigned int sign_l, sign_s, sign_xor;       // Large/Small Sign, Sign Xor
    int          expo_l, expo_s, expo_diff;      // Large/Small Exponent, Large - Small Exponent
    unsigned int mant_l, mant_s;                 // Large/Small Mantissa
    unsigned int sigf_l, sigf_s;                 // Large/Small Significand(11-bit) : Hidden 1-bit + Mantissa
    unsigned int aligned_l, aligned_s, aligned;  // Large/Small Aligned Significand(14-bit) : Significand + GRS-bit
    
    int shift_exp = 0;     // Zero Leading Count
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

    // Magnitude Compare : {Exp_x + Man_x} <> {Exp_y + Man_y}
    great_value = (x & 0x7FFF) >= (y & 0x7FFF) ? 1 : 0;

    // Sign
    sign_xor = (sign_x ^ sign_y) & 0x1;
    sign = (sign_x == sign_y) ? sign_x : 
                                great_value ? sign_x : 
                                              sign_y;

    // Large Magnitude: l, Small Magnitude: s
    if(great_value){
        sign_l = sign_x;
        sign_s = sign_y;
        expo_l = expo_x;
        expo_s = expo_y;
        mant_l = mant_x;
        mant_s = mant_y;
    }else{
        sign_l = sign_y;
        sign_s = sign_x;
        expo_l = expo_y;
        expo_s = expo_x;
        mant_l = mant_y;
        mant_s = mant_x;
    }

    // Exponent Difference
    expo_diff = (expo_l - expo_s) & 0x3F;

    // Hidden 1-bit + Mantissa (11-bit)
    if(expo_l == 0) sigf_l = 0x0;    // Denormal
    else sigf_l = mant_l | 1 << 10;  // Normal
    if(expo_s == 0) sigf_s = 0x0;    // Denormal
    else sigf_s = mant_s | 1 << 10;  // Normal

    // Aligning
    aligned_l = sigf_l;
    if((1 <= expo_diff) && (expo_diff <= 10) && (expo_s != 0)){        // Normal
        aligned_s = sigf_s >> expo_diff;
    }else if(11 <= expo_diff) {                                        // Mantissa Underflow
        aligned_s = 0;
    }else{                                                             // No Aligning
        aligned_s = sigf_s;
    }

    // 2's Complement and Add
    if( sign_xor == 0x1){                           // Sign is difference
        aligned_s = (~aligned_s + 1) & 0x7FF;
        aligned = (aligned_l + aligned_s) & 0x7FF;
    }else if(sign_xor == 0x0 && expo_l != 0){       // Sign is same 
        aligned = (aligned_l + aligned_s) & 0xFFF;  // 덧셈할 때 올림 발생 가능, 12-bit 
    }

    if(aligned == 0){  // Zero
        flag = ZERO;
    }

    // Leading Zero Counter
    if      (aligned & 0x800) shift_exp = -1;
    else if (aligned & 0x400) shift_exp =  0;
    else if (aligned & 0x200) shift_exp =  1;
    else if (aligned & 0x100) shift_exp =  2;
    else if (aligned &  0x80) shift_exp =  3;
    else if (aligned &  0x40) shift_exp =  4;
    else if (aligned &  0x20) shift_exp =  5;
    else if (aligned &  0x10) shift_exp =  6;
    else if (aligned &   0x8) shift_exp =  7;
    else if (aligned &   0x4) shift_exp =  8;
    else if (aligned &   0x2) shift_exp =  9;
    else if (aligned &   0x1) shift_exp = 10;
    else                      shift_exp =  0;

    // Normalizing
    expo = expo_l - shift_exp;
    if (32 <= expo) {                          // Overflow
        flag = OVERFLOW;
    } else if ((1 <= expo) && (expo <= 31)) {  // Normal
        if (shift_exp == -1) {
            aligned = aligned >> 1;
        } else {
            aligned = aligned << (shift_exp);
        }
    } else if (expo <= 0) {                    // Underflow(expo <= 0)
        // expo = 0;
        flag = UNDERFLOW;
    }

    // 1st Rounding
    // lsb = (aligned & 0x8) >> 3;
    // g = (aligned & 0x4) >> 2;
    // r = (aligned & 0x2) >> 1;
    // s = aligned & 0x1;

    // aligned = (aligned >> 3);
    // if(g == 0){;}                                // GRS : 0xx
    // else if( g == 1 && ((r != 0) || (s != 0))){  // GRS : 11x or 1x1
    //     aligned = aligned + 1;
    // }else{                                       // GRS : 100
    //     if(lsb == 1) {
    //         aligned = aligned + 1;
    //     }else{
    //         ;
    //     }
    // }

    // // 2nd Rounding
    // if(aligned & 0x800 && expo == 0x1F) {        // Overflow
    //     flag = OVERFLOW;
    // }else if(aligned & 0x800){                   // Normal
    //     expo = expo + 1;
    //     aligned = aligned >> 1;
    // }else if((aligned & 0x400) && expo == 0x0){  // Denormal
    //     expo = expo + 1;
    // }

    mant = aligned & 0x3FF;

    if(flag == NORMAL)                         result = (sign << 15) | (expo << 10) |  mant;  // Normal
    else if(flag == OVERFLOW)                  result = (sign << 15) | (0x1F << 10) | 0x3FF;  // Overflow
    else if(flag == UNDERFLOW || flag == ZERO) result = (sign << 15) | (0x00 << 10) | 0x000;  // Underflow

    return result;
}
