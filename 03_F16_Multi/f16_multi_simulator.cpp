#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "f16_arith.h"

#define BIT_SIZE 16  // Floating Point Bit Size
#define MAX_LINE_LEN 256  // 입력할 줄의 최대 글자 길이

#define BLACK    "\033[30m]"
#define RED      "\033[31m"
#define GREEN    "\033[32m"
#define YELLOW   "\033[33m"
#define BLUE     "\033[34m"
#define MAGENTA  "\033[35m"
#define CYAN     "\033[36m"
#define WHITE    "\033[37m"
#define RESET    "\033[0m"


// ## Exception Flag List ##
// 0 0 0 0 0  <- bit code
// - - - - -
// | | | | |-> bit 0 : Inexact Exception
// | | | |-> bit 1 : Underflow Excepton
// | | |-> bit 2 : Overflow Exception
// | |-> bit 3 : Infinite Exception (“divide by zero”)
// |-> bit 4 : Invalid Exception
// Hex| Bin   | Exception
// ----------------------------------------------------
// 00 | 00000 | Normal
// 01 | 00001 | Inexact Exception
// 02 | 00010 | Underflow Exception
// 03 | 00011 | Inexact Exception + Underflow Exception
// 04 | 00100 | Overflow Exception
// 05 | 00101 | Inexact Exception + Overflow Exception
// 08 | 01000 | Infinite Exception (“divide by zero”)
// 10 | 10000 | Invalid Exception
//
// ## Exception Decription ##
// Inexact Exception (근사값으로 자리맞춤)
// → 연산 결과가 표현 가능한 부동소수점으로 정확하게 표현 불가. 자리맞춤이 일어날 때 발생.
// Overflow (오버플로우)
// → 결과값이 표현 가능한 최대 범위를 초과할 때 발생.
// → Max 값으로 satuation
// Underflow (언더플로우)
// → 결과값이 너무 작아서 정규화된 부동소수점으로 표현할 수 없을 때 발생.
// → 0 값으로 satuation
// Zero Divide (0으로 나눔) (미발생으로 제외)
// → 0으로 나누었을 때 발생.
// Invalid Operation (잘못된 연산)
// → 연산이 수학적으로 정의되지 않은 경우 발생 (예: NaN 연산, ∞−∞, 0×∞ 등)

using namespace std;
// #define DIR_PATH "C:\\Users\\sejong_WW\\OneDrive - Sejong University\\03_FP_Arithmetic\\"  // Lab PC 경로
// #define DIR_PATH "C:\\Users\\LeeJooHong\\OneDrive - Sejong University\\03_FP_Arithmetic\\"  // Notebook 경로

#define REF_FILENAME "03_mul_ref.txt"
#define INPUT_FILENAME "04_mul_input.txt"


int main()
{
    clock_t start, end;
    double elapsed_sec, elapsed_ms;
    int minutes, seconds, milliseconds;
    start = clock();

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char ref_fullpath[512];
    char input_fullpath[512];
    char output_fullpath[512];
    char output_filename[512];
    unsigned int failcase[10000];
    unsigned int failx[10000];
    unsigned int faily[10000];
    unsigned int failexpectedoutput[10000];
    unsigned int failexpectedflag[10000];
    unsigned int failactualoutput[10000];
    unsigned int failcase_cnt = 0;

    unsigned int x, y, expected_output, expected_flag, actual_output;
    unsigned int ref_x, ref_y, ref_expected_output, ref_actual_output;
    
    unsigned int testfloat_pass_cnt = 0;
    unsigned int testfloat_fail_cnt = 0;
    unsigned int real_pass_cnt = 0;

    // 파일명에 "01_output_YYMMDD_HHMMSS_.txt" 형식으로 현재 시각을 포함
    strftime(output_filename, sizeof(output_filename), "05_mul_output_%y%m%d_%H%M%S_.txt", t);

    // 경로와 파일명을 합쳐 전체 경로 생성
    // snprintf(ref_fullpath, sizeof(ref_fullpath), "%s%s", DIR_PATH, REF_FILENAME);
    // snprintf(input_fullpath, sizeof(input_fullpath), "%s%s", DIR_PATH, INPUT_FILENAME);
    // snprintf(output_fullpath, sizeof(output_fullpath), "%s%s", DIR_PATH, output_filename);

    FILE *input_file = fopen(INPUT_FILENAME, "r");
    if (input_file == NULL) {
        perror("파일 열기 실패");
        return 1;
    }

    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        perror("파일 열기 실패");
        return 1;
    }

    printf("\n");
    printf(RED   "#################################################\n" RESET);
    printf(GREEN "##  FLOATING POINT                             ##\n" RESET);
    printf(BLUE  "##  MULTIPLER             VERSION : %s  ##\n" RESET, _MULTI_VERSION_);
    printf(CYAN  "#################################################\n\n" RESET);
    // fprintf(output_file, "\n");
    fprintf(output_file, "#################################################\n");
    fprintf(output_file, "##  FLOATING POINT                             ##\n");
    fprintf(output_file, "##  MULTIPLER             VERSION : %s  ##\n", _MULTI_VERSION_);
    fprintf(output_file, "#################################################\n\n");

    // for(int i=0; i<46570; i++) {
    for(int i=0; i<10; i++) {
        char line[MAX_LINE_LEN];
        char *pos = line;
        int testfloat_pass = 0;
        int real_pass = 0;
        int ref_match = 0;

        printf(GREEN "#%05d  " RESET, i+1);  // #00001
        fprintf(output_file, "#%05d  ", i+1);  // #00001

        if(fgets(line, sizeof(line), input_file) == NULL){
            break;
        }
        line[strcspn(line, "\n")] = '\0';  // 개행 문자 제거

        x               = strtoul(pos, &pos, 16);
        y               = strtoul(pos, &pos, 16);
        expected_output = strtoul(pos, &pos, 16);
        expected_flag   = strtoul(pos, &pos, 16);

        // printf("%04u %04u %04u %04u\n", x, y, expected_output, expected_flag); // 10진수 출력
        printf("%04X %04X %04X %02X", x, y, expected_output, expected_flag); // 16진수 출력
        fprintf(output_file, "%04X %04X %04X %02X", x, y, expected_output, expected_flag); // 16진수 출력


        // 16-bit Floating Point Adder  ///////////////////////////////////////////////////////////////////////////////////////////////////
        // actual_output = f16_adder(x, y);
        
        // 16-bit Floating Point Multi  ///////////////////////////////////////////////////////////////////////////////////////////////////
        actual_output = f16_multi(x, y);


        // TestFloat 결과와 비교
        // testfloat_pass = 1 : expected_output == actual_output 로 동일
        // testfloat_pass = 0 : expected_output != actual_output 로 다름
        if(expected_output == actual_output){  // pass
            testfloat_pass = 1;
        }else{  // fail
            testfloat_pass = 0;
        }

        if(testfloat_pass == 1){  // TestFloat 결과와 동일
            testfloat_pass_cnt++;
            printf("   <------  %s ", "PASS");
            fprintf(output_file, "   <------  %s      ", "PASS");
        }else if(testfloat_pass == 0){  // TestFloat 결과와 다르지만 00_ref.txt 와 matching 함
            testfloat_fail_cnt++;
            printf(MAGENTA "   <-  %s %04X " RESET, "FAIL" , actual_output);
            fprintf(output_file, "   <------  %s %04X ", "FAIL" , actual_output);
        }

        // Denormalized number is supported. (exponent == 0x0 && mantissa != 0)
        if (((x >> 10) & 0x1F) == 0 && (x & 0x3FF) != 0){
            printf("  <- X is Denormal!");
            fprintf(output_file, "  <- X is Denormal!");
            // continue;
        }
        if (((y >> 10) & 0x1F) == 0 && (y & 0x3FF) != 0){
            printf("  <- Y is Denormal!");
            fprintf(output_file, "  <- Y is Denormal!");
            // continue;
        }

        // NaN is NOT supported. (exponent == 0x1F && mantissa !=0)
        // But, The calculation range includes exponent 0x1F.
        if (((x >> 10) & 0x1F) == 0x1F && (x & 0x3FF) != 0){
            printf("  <- X is NaN!");
            fprintf(output_file, "  <- X is NaN!");
            // continue;
        }
        if (((y >> 10) & 0x1F) == 0x1F && (y & 0x3FF) != 0){
            printf("  <- Y is NaN!");
            fprintf(output_file, "  <- Y is NaN!");
            // continue;
        }

        // +-infinite is NOT supported. (exponent == 0x1F && mantissa ==0)
        // But, The calculation range includes exponent 0x1F.
        if (((x >> 10) & 0x1F) == 0x1F && (x & 0x3FF) == 0){
            printf("  <- X is +-Inf!");
            fprintf(output_file, "  <- X is +-Inf!");
            // continue;
        }
        if (((y >> 10) & 0x1F) == 0x1F && (y & 0x3FF) == 0){
            printf("  <- Y is +-Inf!");
            fprintf(output_file, " <- Y is +-Inf!");
            // continue;
        }

        // Denormalized number is supported. (exponent == 0x0 && mantissa != 0)
        if (((expected_output >> 10) & 0x1F) == 0 && (expected_output & 0x3FF) != 0){
            printf("  <- Expected is Denormal!");
            fprintf(output_file, "  <- Expected is Denormal!");
            // continue;
        }

        // NaN is NOT supported. (exponent == 0x1F && mantissa !=0)
        // But, The calculation range includes exponent 0x1F.
        if (((expected_output >> 10) & 0x1F) == 0x1F && (expected_output & 0x3FF) != 0){
            printf("  <- Expected is NaN!");
            fprintf(output_file, "  <- Expected is NaN!");
            // continue;
        }
        
        // +-infinite is NOT supported. (exponent == 0x1F && mantissa ==0)
        // But, The calculation range includes exponent 0x1F.
        if (((expected_output >> 10) & 0x1F) == 0x1F && (expected_output & 0x3FF) == 0){
            printf("  <- Expected is +-Inf!");
            fprintf(output_file, "  <- Expected is +-Inf!");
            // continue;
        }
        printf("\n");
        fprintf(output_file, "\n");
        
        double x_dec, y_dec, expected_dec, actual_dec, real_dec;
        double r_e, r_a;

        x_dec = hextodec(x);
        y_dec = hextodec(y);
        expected_dec = hextodec(expected_output);
        actual_dec = hextodec(actual_output);
        real_dec = x_dec * y_dec;
        r_e = fabs(fabs(real_dec) - fabs(expected_dec));
        r_a = fabs(fabs(real_dec) - fabs(actual_dec));

        printf("-------- X --------   -------- Y --------   - Expected_Output -   -- Actual_Output --   --- Real_result ---\n");
        // printf("----------------X  ----------------Y  --Expected_Output  ----Actual_Output  ------Real_result\n");
        // printf("X----------------  Y----------------  Expected_Output--  Actual_Output----  Real_result------\n");
        printf("%+19.10f   %+19.10f   %+19.10f   %+19.10f   %+19.10f\n", x_dec, y_dec, expected_dec, actual_dec, real_dec);
        
        printf("--- |Real-Expected|   ----- |Real-Actual|\n");
        printf("%19.10f   %19.10f", r_e, r_a);

        if( r_a <= r_e ) real_pass = 1;
        else             real_pass = 0;

        // 00_ref.txt 검색
        // TestFloat 기준으로 결과값이 다르지만, 확인된 Case에 대해 Pass 처리용
        if(real_pass == 0){
            FILE *ref_file = fopen(REF_FILENAME, "r");
            if (ref_file == NULL) {
                perror("파일 열기 실패");
                return 1;
            }

            for(int j=0; j<5000; j++)
            {
                char ref_line[MAX_LINE_LEN];
                char *ref_pos = ref_line;
                
                if(fgets(ref_line, sizeof(ref_line), ref_file) == NULL){
                    break;  // 파일 끝(EOF)이면 for문 종료
                }
                ref_line[strcspn(ref_line, "\n")] = '\0';  // 개행 문자 제거

                ref_x               = strtoul(ref_pos, &ref_pos, 16);
                ref_y               = strtoul(ref_pos, &ref_pos, 16);
                ref_expected_output = strtoul(ref_pos, &ref_pos, 16);
                ref_actual_output   = strtoul(ref_pos, &ref_pos, 16);

                // 00_ref.txt matching 여부
                // ref_match = 1 : 00_ref.txt 와 matching 함
                // ref_match = 0 : 00_ref.txt 와 matching 안함
                if((ref_x == x) && (ref_y==y) && (ref_expected_output==expected_output) && (ref_actual_output==actual_output)){
                    ref_match = 1;
                    break;
                }
            }
            fclose(ref_file);
        }

        if(real_pass == 1){  // 10진수 계산 기준 Pass
            real_pass_cnt++;
            printf(BLUE "   <-------  REAL PASS  \n" RESET);
            fprintf(output_file, "                            <------  REAL PASS\n");
        }else if(real_pass == 0 && ref_match == 1){  // 10진수 계산 기준 틀리지만, 00_ref.txt 와 matching 함
            real_pass_cnt++;
            printf(YELLOW "   <-------  REAL PASS(REF MATCH)  \n" RESET);
            fprintf(output_file, "                            <------  REAL PASS(REF MATCH)\n");
        }else if(real_pass == 0 && ref_match == 0){  // 10진수 계산 기준 틀리고, 00_ref.txt 와 matching 안함
            printf(RED "   <-------  REAL FAIL  \n" RESET);
            fprintf(output_file, "                            <------  REAL FAIL   <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
            failcase[failcase_cnt] = i+1;
            failx[failcase_cnt] = x;
            faily[failcase_cnt] = y;
            failexpectedoutput[failcase_cnt] = expected_output;
            failexpectedflag[failcase_cnt] = expected_flag;
            failactualoutput[failcase_cnt] = actual_output;
            failcase_cnt++;
        }

        if(testfloat_pass == 0 || real_pass == 0){
            printf("\tExpected_Output(%04X)\n", expected_output);
            // fprintf(output_file, "Expected_Output\n");
            printbits(expected_output);

            printf("\tActual_Output(%04X)\n", actual_output);
            // fprintf(output_file, "\nActual_Output\n");
            printbits(actual_output);
        }
        printf("\n");
        fprintf(output_file, "\n");
    }

    printf("TestFloat Pass Rate : %.2f%% (%d/%d)\n", (float(testfloat_pass_cnt)*100/(testfloat_pass_cnt+testfloat_fail_cnt)), testfloat_pass_cnt, (testfloat_pass_cnt+testfloat_fail_cnt));
    fprintf(output_file, "TestFloat Pass Rate : %.2f%% (%d/%d)\n", (float(testfloat_pass_cnt)*100/(testfloat_pass_cnt+testfloat_fail_cnt)), testfloat_pass_cnt, (testfloat_pass_cnt+testfloat_fail_cnt));
    
    printf("Real Pass Rate : %.2f%% (%d/%d)\n", (float(real_pass_cnt)*100/(testfloat_pass_cnt+testfloat_fail_cnt)), real_pass_cnt, (testfloat_pass_cnt+testfloat_fail_cnt));
    fprintf(output_file, "Real Pass Rate : %.2f%% (%d/%d)\n", (float(real_pass_cnt)*100/(testfloat_pass_cnt+testfloat_fail_cnt)), real_pass_cnt, (testfloat_pass_cnt+testfloat_fail_cnt));
    
    fprintf(output_file, "\n# Real Fail Case Number #\n");
    for(int i=0; i<failcase_cnt; i++){
        fprintf(output_file, "Real Fail Case Number : %5d   <-  %04X %04X %04X %02X %04X\n", failcase[i], failx[i], faily[i], failexpectedoutput[i], failexpectedflag[i],failactualoutput[i]);
    }
    fprintf(output_file, "Real Fail Cnt : %d\n", failcase_cnt);


    end = clock();
    elapsed_sec = (double)(end - start) / CLOCKS_PER_SEC;  // 전체 초 단위 실행 시간
    elapsed_ms = elapsed_sec * 1000.0;  // 밀리초 단위 시간

    // 분, 초, 밀리초로 분해
    minutes = (int)(elapsed_sec / 60);
    seconds = (int)elapsed_sec % 60;
    milliseconds = (int)(elapsed_ms) % 1000;
    printf("Elapsed Time : %f Sec (%d:%02d.%03d)\n", elapsed_sec, minutes, seconds, milliseconds);
    fprintf(output_file, "\nElapsed Time : %f Sec (%d:%02d.%03d)\n", elapsed_sec, minutes, seconds, milliseconds);


    fclose(input_file);
    fclose(output_file);

    return 0;
}
