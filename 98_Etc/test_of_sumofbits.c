#include <stdio.h>
#include <math.h>
#include "f16_arith.h"

#define BIAS 15
#define EXPONENT 5
#define MANTISSA 10
#define SIGNIFICAND (1+MANTISSA)

// x: 첫 비트의 2의 승수
// n: 총 비트 수
// double sumofbits(int x, int n) {
//     double total = 0;
//     int i;
//     for (i = 0; i < n; i++) {
//         total += (double)pow(2, x - i);
//     }
//     return total;
// }

int main() {
    int x = pow(2, EXPONENT) - 1  - BIAS;    // 시작 지수
    int n = SIGNIFICAND;    // 비트의 개수
    int expo = EXPONENT;  // 지수부의 비트 수
    printf("##   Real Expo       Max Real Value      Min Real Value                Step\n");
    printf("---------------------------------------------------------------------------\n");
    for(int i=0; i<pow(2, expo); i++){
        if((int)(pow(2, expo)-1-i) != 0){
            printf("%02d:    2^(%+03d)  %+19.10f %+19.10f %+19.10f\n", (int)(pow(2, expo)-1-i), x-i, sumofbits(x-i, n), pow(2, x-i), pow(2, x-i-10) );
        }
        else{
            printf("%02d:          0  %+19.10f %+19.10f %+19.10f\n", (int)(pow(2, expo)-1-i), 0.0, 0.0, 0.0);
        }
    }
    return 0;
}