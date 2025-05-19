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


// int main() {
//     // 테스트할 값들
//     double test_values[] = {
//         0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 
//         0.00000005960464477539063,  // 가장 작은 정규화 수
//         0.000000059,  // 정규화 수 근처 값
//         0.000000029802322387695312, // 가장 큰 비정규화 수
//         0.0000000001,  // 작은 비정규화 수
//         65504.0,  // 최대 표현 가능 값
//         -65504.0, // 최소 표현 가능 값
//         100000.0,  // 오버플로우 값
//         +0.0000915527
//     };
    
//     for (int i = 0; i < sizeof(test_values)/sizeof(test_values[0]); i++) {
//         half_float hf = double_to_half(test_values[i]);
//         printf("입력: %+-17.10f -> 16비트: 0x%04X (부호=%d, 지수=%2d, 가수=0x%03X)\n", 
//                test_values[i], hf.bits, hf.parts.sign, hf.parts.exponent, hf.parts.mantissa);
//     }

//     printf("%f\n", 2^(-14));
//     printf("%f\n", pow(2, -14));
    
//     return 0;
// }


#include <stdint.h>
#include <stdio.h>

typedef uint16_t float16;

float16 float16_mul_1(float16 a, float16 b) {
    // Sign 계산
    uint16_t sign = ((a >> 15) ^ (b >> 15)) << 15;
    
    // 특수값 처리 (0 처리)
    if(a == 0 || b == 0) return sign;
    
    // 지수 및 가수 추출
    uint16_t a_exp = (a >> 10) & 0x1F;
    uint16_t b_exp = (b >> 10) & 0x1F;
    uint16_t a_man = a & 0x3FF;
    uint16_t b_man = b & 0x3FF;
    
    uint16_t de = 0;
    if(a_exp == 0 && b_exp != 0) de = 1;
    if(a_exp != 0 && b_exp == 0) de = 1;
    if(a_exp == 0 && b_exp == 0) de = 2;


    // Denormal 처리
    int32_t a_real_exp = a_exp ? a_exp - 15 : -14;
    int32_t b_real_exp = b_exp ? b_exp - 15 : -14;
    uint32_t a_real_man = a_exp ? a_man | 0x400 : a_man;
    uint32_t b_real_man = b_exp ? b_man | 0x400 : b_man;
    
    // 가수 곱셈 (22비트 결과)
    uint32_t man_product = a_real_man * b_real_man;
    
    // 정규화
    int32_t product_exp = a_real_exp + b_real_exp;
    uint32_t shift = 0;
    
    if(de == 0){
        if(man_product & 0x00200000) {      // 22번째 비트 체크
            shift = 1;
            product_exp += 1;
            man_product >>= 1;
        } else {
            while(!(man_product & 0x00080000) && man_product) {
                man_product <<= 1;
                shift++;
            }
            product_exp -= shift;
        }
    }
    if(de == 1){
        if(man_product & 0x00100000) {      // 22번째 비트 체크
            shift = 1;
            product_exp += 1;
            man_product >>= 1;
        } else {
            while(!(man_product & 0x00040000) && man_product) {
                man_product <<= 1;
                shift++;
            }
            product_exp -= shift;
        }
    }
    
    // 지수 오버플로우 처리
    if(product_exp > 16) {
        // 최대값 (0x7BFF)
        return sign | 0x7FFF;
    }
    
    // 언더플로우 처리 (denormal 지원)
    if(product_exp < -24) return sign;  // 0 반환
    if(product_exp < -14) {
        if(de == 0){
            // Denormal로 변환
            int denormal_shift = -15 - product_exp;
            man_product >>= (denormal_shift);
            // man_product >>= (denormal_shift + 1);
            product_exp = -15;
        }
        if(de == 1){
            // Denormal로 변환
            int denormal_shift = -15 - product_exp;
            man_product >>= (denormal_shift);
            // man_product >>= (denormal_shift + 1);
            product_exp = -15;
        }
    }
    
    // 가수 라운딩 (10비트로)
    uint16_t man_result = (man_product >> 12) & 0x3FF;
    
    // 최종 지수 계산
    uint16_t exp_result = product_exp + 15;
    if(exp_result >= 0x1F) exp_result = 0x1F - 1;
    
    return sign | (exp_result << 10) | man_result;
}


// float16 곱셈 (denormal 지원, inf/nan 미지원)
float16 float16_mul_2(float16 a, float16 b) {
    // 1. 부호 추출
    uint16_t sign = ((a ^ b) & 0x8000);

    // 2. 지수 및 가수 추출
    uint16_t exp_a = (a >> 10) & 0x1F;
    uint16_t exp_b = (b >> 10) & 0x1F;
    uint16_t man_a = a & 0x3FF;
    uint16_t man_b = b & 0x3FF;

    // 3. 0 처리 (denormal 포함)
    if ((exp_a == 0 && man_a == 0) || (exp_b == 0 && man_b == 0)) {
        return sign; // 0
    }

    // 4. 가수 준비 (hidden bit)
    uint32_t m_a, m_b;
    int e_a, e_b;
    if (exp_a == 0) { // denormal
        m_a = man_a;
        e_a = -14;
    } else {
        m_a = man_a | 0x400;
        e_a = exp_a - 15;
    }
    if (exp_b == 0) { // denormal
        m_b = man_b;
        e_b = -14;
    } else {
        m_b = man_b | 0x400;
        e_b = exp_b - 15;
    }

    // 5. 곱셈
    uint64_t mantissa = (uint64_t)m_a * m_b; // 최대 21비트 * 21비트 = 42비트
    int exp = e_a + e_b;

    // 6. 정규화 (m_a, m_b는 최대 11비트, mantissa는 최대 22비트)
    // 곱셈 결과는 [21:0] (최대 22비트)
    // 정규화: 최상위 11비트(0x80000) 체크
    int shift = 0;
    if (mantissa & (1 << 21)) {
        // 이미 정규화됨 (1.xxxx 형태)
        shift = 11;
        exp += 1;
    } else {
        // 0.xxxx 형태, 10비트 시프트
        shift = 10;
    }
    // 7. 라운딩 (round to nearest even)
    uint32_t round_guard_sticky = (mantissa >> (shift - 1)) & 0x3; // guard, round bit
    uint32_t sticky = (mantissa & ((1U << (shift - 1)) - 1)) ? 1 : 0;
    uint32_t mantissa_10b = (mantissa >> shift) & 0x3FF;
    // round to nearest
    if ((round_guard_sticky & 0x2) && ((round_guard_sticky & 0x1) || sticky || (mantissa_10b & 1))) {
        mantissa_10b++;
        if (mantissa_10b == 0x400) { // overflow
            mantissa_10b = 0x200;
            exp += 1;
        }
    }

    // 8. 지수 보정 및 언더/오버플로우 처리
    int final_exp = exp + 15;
    if (final_exp >= 31) {
        // 오버플로우: 최대값 반환
        return sign | (0x1F - 1) << 10 | 0x3FF;
    }
    if (final_exp <= 0) {
        // 언더플로우: denormal로 변환
        if (final_exp < -9) return sign; // 완전 언더플로우(0)
        // denormal: 가수를 우측 시프트
        mantissa_10b = mantissa_10b >> (1 - final_exp);
        return sign | (mantissa_10b & 0x3FF);
    }

    // 9. 결과 결합
    return sign | (final_exp << 10) | (mantissa_10b & 0x3FF);
}

float16 float16_mul_3(float16 a, float16 b) {
    uint16_t sign = ((a ^ b) & 0x8000);

    uint16_t exp_a = (a >> 10) & 0x1F;
    uint16_t exp_b = (b >> 10) & 0x1F;
    uint16_t man_a = a & 0x3FF;
    uint16_t man_b = b & 0x3FF;

    // Zero check
    if ((exp_a == 0 && man_a == 0) || (exp_b == 0 && man_b == 0))
        return sign;

    // Unpack
    uint32_t m_a, m_b;
    int e_a, e_b;
    if (exp_a == 0) {
        m_a = man_a;
        e_a = -14;
    } else {
        m_a = man_a | 0x400;
        e_a = exp_a - 15;
    }
    if (exp_b == 0) {
        m_b = man_b;
        e_b = -14;
    } else {
        m_b = man_b | 0x400;
        e_b = exp_b - 15;
    }

    // Multiply mantissas (22 bits)
    uint64_t product = (uint64_t)m_a * m_b;
    int exp = e_a + e_b;

    // Normalize product
    int shift;
    if (product & (1 << 21)) {
        shift = 11;
        exp += 1;
    } else {
        shift = 10;
    }

    // Rounding: round-to-nearest-even
    uint32_t mant = (product >> shift) & 0x3FF;
    uint32_t g = (product >> (shift - 1)) & 1;
    uint32_t r = (product >> (shift - 2)) & 1;
    uint32_t s = (product & ((1U << (shift - 2)) - 1)) ? 1 : 0;

    if (g && (r || s || (mant & 1))) {
        mant += 1;
        if (mant == 0x400) { // mantissa overflow
            mant = 0x200;
            exp += 1;
        }
    }

    int final_exp = exp + 15;
    if (final_exp >= 31) {
        // Overflow: return max finite value
        return sign | 0x7BFF;
    }
    if (final_exp <= 0) {
        // Denormal or underflow
        if (final_exp < -9)
            return sign;
        mant = mant >> (1 - final_exp);
        return sign | (mant & 0x3FF);
    }
    return sign | ((final_exp & 0x1F) << 10) | (mant & 0x3FF);
}


// 테스트용 출력 함수
void print_float16(float16 f) {
    printf("0x%04X: ", f);
    printf("%c", (f >> 15) ? '-' : '+');
    
    uint16_t exp = (f >> 10) & 0x1F;
    uint16_t man = f & 0x3FF;
    
    if(exp == 0) {
        printf("0.%010lld * 2^-14", (long long)man);
    } else {
        printf("1.%010lld * 2^%d", (long long)man, exp - 15);
    }
    printf("\n");
}

int main() {
    // 테스트 케이스
    float16 a = 0x0809; // 1.0
    float16 b = 0xACFE; // 2.0
    float16 r = float16_mul_1(a, b);
    printf("0x%04X * 0x%04X = 0x%04X\n", a, b, r); // 0x80A1 기대

    a = 0xBB94; // 1.0
    b = 0x03EF; // 2.0
    r = float16_mul_1(a, b);
    printf("0x%04X * 0x%04X = 0x%04X\n", a, b, r); // 0x80A1 기대


    // 추가 테스트
    // print_float16(a);
    // print_float16(b);
    // print_float16(r);
    
    return 0;
}
