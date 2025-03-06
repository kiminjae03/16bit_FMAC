#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

#define MAX_LINES 10000
#define MAX_LINE_LENGTH 30

int main() {
	char lines[MAX_LINES][MAX_LINE_LENGTH];
	char flag[3];
	int num;

	printf("Exception Flag: ");
	scanf("%s", flag);
	printf("개수: ");
	scanf("%d", &num);


    FILE* rdFile = _wfopen(L"f16_mulAdd.txt", L"rb");
    wchar_t temp[MAX_LINE_LENGTH];
    char temp2[MAX_LINE_LENGTH];
    int line_count = 0;

    while (fgetws(temp, MAX_LINE_LENGTH, rdFile) != NULL) {
        if (line_count == 0) {
            // BOM 제거
            if (temp[0] == 0xFEFF) {
                memmove(temp, temp + 1, (wcslen(temp) + 1) * sizeof(wchar_t));
            }
        }

        size_t len = wcslen(temp);
        wcstombs(temp2, temp, len + 1);

        if (strncmp(temp2 + 20, flag, 2) == 0 || strcmp(flag, "99") == 0) {
            strcpy(lines[line_count], temp2);
            lines[line_count++][22] = NULL;
        }
        if (line_count >= MAX_LINES) break;
    }
    fclose(rdFile);

    FILE* wrFile1 = fopen("f16_mulAdd_C_test.txt", "w");
    FILE* wrFile2 = fopen("f16_mulAdd_Verilog_test.txt", "w");

    srand(time(NULL));
    for (int i = 0; i < num; i++) {
        int random_index = rand() % line_count;
        fprintf(wrFile1, "%s\n", lines[random_index]);
        fprintf(wrFile2, "%s\n", lines[random_index]);
    }

    fclose(wrFile1);
    fclose(wrFile2);

	return 0;
}