#include <stdio.h>
#include <stdint.h>


// 16비트 half-float 구조체
typedef union {
    uint16_t bits;
    struct {
        uint16_t mantissa : 10;
        uint16_t exponent : 5;
        uint16_t sign : 1;
    } parts;
} half_float;


extern const char* _ADDER_VERSION_;
extern const char* _MULTI_VERSION_;

unsigned int f16_adder(unsigned int x, unsigned int y);
unsigned int f16_adder_ai(unsigned int x, unsigned int y);
unsigned int f16_adder_opt(unsigned int x, unsigned int y);

unsigned int f16_multi(unsigned int x, unsigned int y);

void printbits(__int64 value);
double hextodec(unsigned int hex_input);