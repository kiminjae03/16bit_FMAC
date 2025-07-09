#include "f16_arith.h"
const char* _FMAC_NORMAL_NO_GRS_VERSION_ = "250605.01";

#define NORMAL    0
#define OVERFLOW  1
#define ZERO      2
#define UNDERFLOW 2
// #define DENORMAL  3

#define BIAS 15


unsigned int f16_fmac_normal_no_grs(unsigned int x, unsigned int y, unsigned int z) {
    unsigned int flag = NORMAL;         // Noraml: 0, Overflow: 1, Underflow: 2, Zero: 2, Denoraml: 3
    unsigned int t_flag = NORMAL;       // Noraml: 0, Overflow: 1, Underflow: 2, Zero: 2, Denoraml: 3

    unsigned int sign_x, sign_y, sign_z, sign_t, sign;  // Sign(1-bit)
    int          expo_x, expo_y, expo_z, expo_t, expo;  // Exponent(5-bit)
    unsigned int mant_x, mant_y, mant_z,         mant;  // Mantissa(10-bit)

    unsigned int great_value;                    // Magnitude Compare (Exponent + Mantissa)
    unsigned int sign_xor;                       // Sign Xor(sign_t ^ sign_z)
    unsigned int sigf_x, sigf_y, sigf_z;         // Significand(11-bit/21-bit) : Hidden 1-bit + Mantissa
    int          expo_l, expo_s, expo_diff;      // Large/Small Exponent, Large - Small Exponent
    unsigned int sigf_l, sigf_s;                 // Large/Small Significand(21-bit) : Hidden 1-bit + Mantissa
    unsigned int aligned_l, aligned_s, aligned_t, aligned;  // Large/Small Aligned Significand(22-bit)
    
    int shift_exp = 0;     // Leading Zero Count
    int expo_add = 0;
    unsigned int result;   // Sign + Exponent + Mantissa

    // Seperate (Sign / Expnent / Mantissa)
    sign_x = (x >> 15) & 0x1;
    sign_y = (y >> 15) & 0x1;
    sign_z = (z >> 15) & 0x1;

    expo_x = (x >> 10) & 0x1F;
    expo_y = (y >> 10) & 0x1F;
    expo_z = (((z >> 10) & 0x1F) + BIAS) & 0x3F;  // 6-bit(Biased -> Real Exponent)

    mant_x = (x) & 0x3FF;
    mant_y = (y) & 0x3FF;
    mant_z = (z) & 0x3FF;

    // Temp Sign
    sign_t = (sign_x ^ sign_y) & 0x1;

    // Hidden 1-bit + Mantissa (11-bit)
    sigf_x = mant_x | 1 << 10;   // Normal
    sigf_y = mant_y | 1 << 10;   // Normal
    if(expo_z == BIAS) sigf_z = 0;                        // Normal(22-bit)
    else               sigf_z = (1 << 10 | mant_z) << 11;   // Normal(22-bit)

    // Zero = (? x 0) or (0 x ?)
    if(expo_x == 0 || expo_y == 0) t_flag = ZERO;

    // Multiply: significand_x * significand_y = aligned_t(22-bit)
    if(t_flag == ZERO){
        aligned_t = 0;
    }else{
        aligned_t = (sigf_x * sigf_y) & 0x3FFFFF;
    }

    // 22-bit
    if((aligned_t & 0x200000) != 0){
        expo_add = 1;
    }
    else{
        aligned_t = (aligned_t << 1) & 0x3FFFFF;
    }

    // Exponent
    if(t_flag == ZERO){
        expo_t = 0;
    }else{
        expo_t = ((expo_x + expo_y) + expo_add) & 0x3F;  // 6-bit
    }

    // Magnitude Compare : {Exp_t + aligned_t} <> {Exp_z + sigf_z}
    great_value = ((expo_t << 22 | aligned_t) & 0xFFFFFFF) >= ((expo_z << 22 | sigf_z) & 0xFFFFFFF) ? 1 : 0;

    // Sign
    sign_xor = (sign_t ^ sign_z) & 0x1;
    sign = (sign_t == sign_z) ? sign_t : 
                                great_value ? sign_t : 
                                              sign_z;

    // Large Magnitude: l, Small Magnitude: s
    if(great_value){
        expo_l = expo_t;
        expo_s = expo_z;
        sigf_l = aligned_t;
        sigf_s = sigf_z;
    }else{
        expo_l = expo_z;
        expo_s = expo_t;
        sigf_l = sigf_z;
        sigf_s = aligned_t;
    }

    // Exponent Difference(6-bit)
    expo_diff = (expo_l - expo_s) & 0x3F;

    // Aligning
    aligned_l = sigf_l;
    if((1 <= expo_diff) && (expo_diff <= 21)){  // Normal
        aligned_s = sigf_s >> expo_diff;
    }else if(22 <= expo_diff) {                 // Mantissa Underflow
        aligned_s = 0;
    }else{                                      // No Aligning
        aligned_s = sigf_s;
    }

    // 2's Complement and Add
    if( sign_xor == 0x1){                              // Sign is difference
        aligned_s = (~aligned_s + 1) & 0x3FFFFF;
        aligned = (aligned_l + aligned_s) & 0x3FFFFF;
    }else if(sign_xor == 0x0){                         // Sign is same 
        aligned = (aligned_l + aligned_s) & 0x7FFFFF;  // 덧셈할 때 올림 발생 가능, 23-bit
    }

    if(aligned == 0){  // Zero
        flag = ZERO;
    }

    // Leading Zero Counter(23-bit)
    if      (aligned & 0x400000) shift_exp = -1;
    else if (aligned & 0x200000) shift_exp =  0;
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
    else if (aligned &    0x200) shift_exp = 12;
    else if (aligned &    0x100) shift_exp = 13;
    else if (aligned &     0x80) shift_exp = 14;
    else if (aligned &     0x40) shift_exp = 15;
    else if (aligned &     0x20) shift_exp = 16;
    else if (aligned &     0x10) shift_exp = 17;
    else if (aligned &      0x8) shift_exp = 18;
    else if (aligned &      0x4) shift_exp = 19;
    else if (aligned &      0x2) shift_exp = 20;
    else if (aligned &      0x1) shift_exp = 21;
    else                         shift_exp =  0;

    // Normalizing
    expo = expo_l - shift_exp - BIAS;
    if (32 <= expo) {                          // Overflow
        flag = OVERFLOW;
    } else if ((1 <= expo) && (expo <= 31)) {  // Normal
        if(shift_exp == -1){
            aligned = aligned >> 1;
        } else {
            aligned = aligned << (shift_exp);
        }
    } else if (expo <= 0) {                    // Underflow(expo <= 0)
        flag = UNDERFLOW;
    }

    // Normalizing
    if(32 <= expo){         // Overflow
        flag = OVERFLOW;
    }else if(expo <= 0) {   // Underflow
        flag = UNDERFLOW;
    }else{;}                // Normal

    mant = (aligned >> 11) & 0x3FF;

    if(flag == NORMAL)                         result = (sign << 15) | (expo << 10) |  mant;  // Normal
    else if(flag == OVERFLOW)                  result = (sign << 15) | (0x1F << 10) | 0x3FF;  // Overflow
    else if(flag == UNDERFLOW || flag == ZERO) result = (sign << 15) | (0x00 << 10) | 0x000;  // Underflow

    return result;
}