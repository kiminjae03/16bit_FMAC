#include "f16_arith.h"
#include <math.h>

double hextodec(unsigned int hex_input) {
    const unsigned int sign_bit = (hex_input >> 15) & 0x1;
    const unsigned int exponent = (hex_input >> 10) & 0x1F;
    const unsigned int mantissa = hex_input & 0x3FF;

    double result;

    if (exponent == 0) { // 비정규화 수/0 처리
        result = (mantissa / 1024.0) * pow(2, -14); // 2^(-14) * (0 + m/2^10)
    } 
    else { // 정규화 수 (모든 지수값 처리)
        double normalized_mantissa = 1.0 + (mantissa / 1024.0);
        int exp_value = exponent - 15; // bias=15
        result = normalized_mantissa * pow(2, exp_value);
    }

    // Inf/NaN → 일반 숫자 처리 (부호 적용)
    if (sign_bit) result = -result;

    // printf("\n10진수 변환 결과:\n");
    // printf("과학 표기법: %.15e\n", result);
    // printf("일반 표기법: %.15f\n", result);

    return result;
}