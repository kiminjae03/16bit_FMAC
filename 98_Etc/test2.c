#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "f16_arith.h"



int main() {
    unsigned int hex_input = 0x2680;
    const unsigned int sign_bit = (hex_input >> 15) & 0x1;
    const unsigned short exponent = (hex_input >> 10) & 0x1F;
    const unsigned int mantissa = hex_input & 0x3FF;
    int tzc = 0;

    tzc = tzcofmantissa(hex_input);
    printf("%d\n", tzc);    
    printf("%f\n", pow(2, exponent - 15 + tzc - 10));
    printf("%f\n", calerror(hex_input));
    return 0;
}
