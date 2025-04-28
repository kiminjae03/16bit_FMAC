#include <stdio.h>
#include <stdint.h>
#include <math.h>

// 16비트 부동소수점 구조체
typedef union {
    uint16_t bits;
    struct {
        uint16_t mantissa : 10;
        uint16_t exponent : 5;
        uint16_t sign : 1;
    } parts;
} half_float;

// double → half-float 변환 (오버플로우 시 0x7FFF, denormal 지원, NaN/Inf는 일반 숫자로 처리)
half_float double_to_half(double value) {
    half_float result = {0};

    // 부호 처리
    result.parts.sign = (value < 0) ? 1 : 0;
    value = fabs(value);

    // 너무 작은 값은 0 처리
    if (value < pow(2, -24)) {
        result.parts.exponent = 0;
        result.parts.mantissa = 0;
        return result;
    }

    int exp = 0;
    double mantissa = value;

    // 정규화
    if (mantissa >= 1.0) {
        while (mantissa >= 2.0 && exp < 30) {
            mantissa /= 2.0;
            exp++;
        }
    } else {
        while (mantissa < 1.0 && exp > -14) {
            mantissa *= 2.0;
            exp--;
        }
    }

    uint16_t half_exp;
    uint16_t half_mantissa;

    if (exp < -14) {
        // 비정규화 처리
        half_exp = 0;
        mantissa *= pow(2, exp + 14);
        half_mantissa = (uint16_t)(mantissa * 1024.0 + 0.5);
        if (half_mantissa > 0x3FF) half_mantissa = 0x3FF; // 최대 비정규화 수로 클램핑
    } else {
        // 정규화 처리
        half_exp = exp + 15;
        mantissa -= 1.0; // 숨겨진 1 제거
        half_mantissa = (uint16_t)(mantissa * 1024.0 + 0.5);

        // 반올림 처리
        if (half_mantissa > 0x3FF) {
            half_mantissa = 0;
            half_exp++;
        }

        // 오버플로우 처리 (0x7FFF로 클램핑)
        if (half_exp > 30) {
            result.parts.sign = 0; // 양수 최대값
            result.parts.exponent = 31;
            result.parts.mantissa = 0x3FF;
            return result;
        }
    }

    result.parts.exponent = half_exp;
    result.parts.mantissa = half_mantissa;
    return result;
}

// half-float → double 변환 (지수 31도 일반 숫자로 처리)
double half_to_double(half_float hf) {
    double result;
    int sign = hf.parts.sign ? -1 : 1;
    int exp = hf.parts.exponent;
    uint16_t mant = hf.parts.mantissa;

    if (exp == 0) {
        // 비정규화 수
        result = sign * (mant / 1024.0) * pow(2, -14);
    } else {
        // 모든 값을 정규화된 숫자로 처리 (지수가 31이더라도)
        result = sign * (1.0 + mant / 1024.0) * pow(2, exp - 15);
    }
    return result;
}

// 16비트 부동소수점 덧셈 (오버플로우 시 0x7FFF로 클램핑)
half_float half_add(uint16_t a_bits, uint16_t b_bits) {
    half_float a = {.bits = a_bits};
    half_float b = {.bits = b_bits};

    double a_val = half_to_double(a);
    double b_val = half_to_double(b);

    double result_val = a_val + b_val;

    // 오버플로우 시 0x7FFF로 클램핑
    double max_half = half_to_double((half_float){.bits = 0x7BFF}); // 65504.0
    if (result_val > max_half) result_val = max_half;
    if (result_val < -max_half) result_val = -max_half;

    return double_to_half(result_val);
}

// int main() {
//     // 입력 예시: 0BCE, 03FE
//     uint16_t a_hex = 0x0BCE;
//     uint16_t b_hex = 0x03FE;

//     half_float a = {.bits = a_hex};
//     half_float b = {.bits = b_hex};
//     half_float result = half_add(a_hex, b_hex);

//     printf("입력1: 0x%04X (값: %.15g)\n", a_hex, half_to_double(a));
//     printf("입력2: 0x%04X (값: %.15g)\n", b_hex, half_to_double(b));
//     printf("결과 : 0x%04X (값: %.15g)\n", result.bits, half_to_double(result));

//     // 가장 큰 비정규화 수 테스트
//     half_float max_denorm = {.bits = 0x03FF};
//     printf("\n가장 큰 비정규화 수: 0x%04X (값: %.15g)\n", max_denorm.bits, half_to_double(max_denorm));

//     // 오버플로우 테스트
//     double over = 1e10;
//     half_float over_hf = double_to_half(over);
//     printf("\n오버플로우(1e10) 변환: 0x%04X (값: %.15g)\n", over_hf.bits, half_to_double(over_hf));

//     return 0;
// }


int main() {
    // 테스트할 값들
    double test_values[] = {
        0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 
        0.00000005960464477539063,  // 가장 작은 정규화 수
        0.000000059,  // 정규화 수 근처 값
        0.000000029802322387695312, // 가장 큰 비정규화 수
        0.0000000001,  // 작은 비정규화 수
        65504.0,  // 최대 표현 가능 값
        -65504.0, // 최소 표현 가능 값
        100000.0,  // 오버플로우 값
        +0.0000915527
    };
    
    for (int i = 0; i < sizeof(test_values)/sizeof(test_values[0]); i++) {
        half_float hf = double_to_half(test_values[i]);
        printf("입력: %+-17.10f -> 16비트: 0x%04X (부호=%d, 지수=%2d, 가수=0x%03X)\n", 
               test_values[i], hf.bits, hf.parts.sign, hf.parts.exponent, hf.parts.mantissa);
    }
    
    return 0;
}
