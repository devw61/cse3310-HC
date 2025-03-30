// gcc a2-logical-counting_LOC.c -o a2-logical-counting_LOC && ./a2-logical-counting_LOC

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int check_char_in_str(char *str, char c) {
    while (*str) {
        if (*str == c) {
            return 1;
        }
        str++;
    }
    return 0;
}

char* trim(char *str){
    int trim_start = 0, trim_end = strlen(str) - 1;

    while (str[trim_start] == ' ') trim_start++; // where do leading whitespaces stop
    
    while (str[trim_end] == ' ') trim_end--; // where do ending whitespaces stop
    
    for (int i = 0; i <= (trim_end - trim_start); i++) str[i] = str[trim_start + i]; // set new_str to difference

    str[trim_end - trim_start + 1] = '\0'; // terminate new_str
    return str;
}

int check_str_is_comment(char *str) {
    if (str[0] == '/' && str[1] == '/') return 1;
    return 0;
}

// postmortem
void export_data(char *user_file, int count) {
    FILE *file = fopen("processed_data.txt", "a+");
    if (file == NULL)
    {
        printf("file not found\n");
        return;
    }

    time_t timer;
    char buffer[26];
    struct tm *tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(file, "\n%s\n", buffer);
    fprintf(file, "Filename: %s\nLOC: %d\n", user_file, count);
    fprintf(file, "--------------------");
    fclose(file);
}

int main() {
    int count = 1; // start at 1 since last line is usually not terminated by \n
    char filename[50], line_str[100], trimmed_line_str[100];

    printf("enter file name: ");
    scanf("%s",filename);

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error opening file\n");
        return 1;
    }

    // iterate through the file
    while(fgets(line_str,100,fp) != NULL) { 
        strcpy(trimmed_line_str,trim(line_str));
        if (check_char_in_str(trimmed_line_str, '\n') && trimmed_line_str[0] != '\n' && !check_str_is_comment(trimmed_line_str)){
            count++;
        }
    }

    printf("%s has %d lines of code\n", filename, count);
    export_data(filename, count);
    
    fclose(fp);
    return 0;
}
