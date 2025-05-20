#include <stdio.h>
#include <math.h>
#include "f16_arith.h"

// #define BIAS 15
// #define EXPONENT 5
// #define MANTISSA 10
// #define SIGNIFICAND (1+MANTISSA)


unsigned int tzcofmantissa(unsigned int hex_input) {
    const unsigned int sign_bit = (hex_input >> 15) & 0x1;
    const unsigned int exponent = (hex_input >> 10) & 0x1F;
    const unsigned int mantissa = hex_input & 0x3FF;

    unsigned int x = 0;
    int tzc = 0;
    if(exponent == 0){
        x = mantissa | 0x0 << 10;
    }else{
        x = mantissa | 0x1 << 10;
    }
    x = ~x + 1;

    // Trailing Zero Counter
    if      ((x) & 0x001) tzc = 0;
    else if ((x) & 0x002) tzc = 1;
    else if ((x) & 0x004) tzc = 2;
    else if ((x) & 0x008) tzc = 3;
    else if ((x) & 0x010) tzc = 4;
    else if ((x) & 0x020) tzc = 5;
    else if ((x) & 0x040) tzc = 6;
    else if ((x) & 0x080) tzc = 7;
    else if ((x) & 0x100) tzc = 8;
    else if ((x) & 0x200) tzc = 9;
    else if ((x) & 0x400) tzc = 10;
    else                  tzc = 11;
    
    return tzc;
}

// 가장 작은 유효숫자값 계산
// pow(2, Biased exponent - 15 + tzc - 10)
double calerror(unsigned int hex_input) {
    // unsigned int hex_input = 0x2680;
    const unsigned int sign_bit = (hex_input >> 15) & 0x1;
    const unsigned short exponent = (hex_input >> 10) & 0x1F;
    const unsigned int mantissa = hex_input & 0x3FF;
    int tzc = 0;

    tzc = tzcofmantissa(hex_input);
    // printf("%d\n", tzc);
    // printf("%f\n", pow(2, exponent - 15 + tzc - 10));
    return pow(2, exponent - 15 + tzc - 10);
}