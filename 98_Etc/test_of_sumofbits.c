#include <stdio.h>
#include <math.h>

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

int main() {
    int x = 16;    // 시작 지수
    int n = 11;    // 비트의 개수
    int expo = 5;  // 지수부의 비트 수
    for(int i=0; i<pow(2, expo); i++)
    {
        double result = sumofbits(x-i, n);
        printf("first Expo: 2^%+03d, restul: %+19.10f\n", x-i, result);
    }
    return 0;
}