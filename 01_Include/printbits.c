#include "f16_arith.h"

#define BIT_SIZE 16  // Floating Point Bit Size

// 비트를 출력하는 함수
void printbits(__int64 value) {
    printf("\t");
    for (int i = BIT_SIZE - 1; i >= 0; i--){
        printf("%d", i / 10);  // 비트의 10의 자리 출력
        if (i % 4 == 0) printf(" ");  // 가독성을 위해 4비트마다 공백 추가
    }
    printf("  <- 10's Bit Position");
    printf("\n");

    printf("\t");
    for (int i = BIT_SIZE - 1; i >= 0; i--){
        printf("%d", i % 10);  // 비트의 1의 자리 출력
        if (i % 4 == 0) printf(" ");  // 가독성을 위해 4비트마다 공백 추가
    }
    printf("  <-  1's Bit Position");
    printf("\n");

    printf("\t");
    for (int i = BIT_SIZE - 1; i >= 0; i--){
        printf("-");  // 구분선 출력
        if (i % 4 == 0) printf(" ");  // 가독성을 위해 4비트마다 공백 추가
    }
    printf("\n");

    printf("\t");
    for (int i = BIT_SIZE - 1; i >= 0; i--){
        printf("%d", (value >> i) & 1);  // 각 비트를 출력
        if (i % 4 == 0) printf(" ");  // 가독성을 위해 4비트마다 공백 추가
    }
    printf("  <- Bit Value");
    printf("\n");
}