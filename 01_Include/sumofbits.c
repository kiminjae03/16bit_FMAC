#include <stdio.h>
#include <math.h>
#include "f16_arith.h"

// #define BIAS 15
// #define EXPONENT 5
// #define MANTISSA 10
// #define SIGNIFICAND (1+MANTISSA)


// x: 첫 비트의 2의 승수
// n: 총 비트 수
double sumofbits(int x, int n) {
    double total = 0;
    int i;
    for (i = 0; i < n; i++) {
        total += (double)pow(2, x - i);
    }
    return total;
}