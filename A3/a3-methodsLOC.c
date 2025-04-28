// gcc A3/a3-methodsLOC.c -o A3/a3 && ./A3/a3

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
FILE* open_file(char *filename) {
    FILE *fp = fopen(filename, "r"); 
    
    while (fp == NULL){
        printf("Please enter a valid filename: ");
        scanf("%s", filename);
        fp = fopen(filename, "r");
    }

    return fp;
}

// get functions loc
char **init_str_arr(int num_strings){
    char **str_array = (char **)malloc(num_strings * sizeof(char *));
    if (str_array == NULL){
        return NULL; 
    }

    for (int i = 0; i < num_strings; i++){
        str_array[i] = (char *)malloc(100 * sizeof(char)); 
        if (str_array[i] == NULL){
            for (int j = 0; j < i; j++){
                free(str_array[j]);
            }
            free(str_array);
            return NULL;
        }
    }
    return str_array;
}

int check_str_is_func(char *str){
    if ((strstr(str, "void") || strstr(str, "int") || strstr(str, "char") || strstr(str, "float")) && (!strstr(str, "while") && !strstr(str, "if"))){
        if (strchr(str, '{') || !strchr(str, ';')) {
            return 1; 
        }
    }
    return 0; 
}

int get_func_count(FILE *fp){
    char line[1000];
    int func_count = 0; 

    while(fgets(line, sizeof(line), fp) != NULL) { 
        if (check_str_is_func(line)) { 
            char *start = strstr(line, " ");
            if (start != NULL) {
                start++; 
                char *end = strchr(start, '(');
                if (end != NULL) {
                    func_count++;
                }
            }
           
        }
    }

    rewind(fp); // Reset the file pointer for the beginning

    return func_count;
}

char *get_func_name(char *line) {
    char *start = strstr(line, " ");
    if (start != NULL) {
        start++; 
        char *end = strchr(start, '(');
        if (end != NULL) {
            int length = end - start;
            char *func_name = (char *)malloc((length + 1) * sizeof(char));
            if (func_name == NULL) {
                return NULL; 
            }
            strncpy(func_name, start, length);
            func_name[length] = '\0'; 
            return func_name;
        }
    }
    return NULL; 
}

int count_non_OOP_LOC(FILE *fp, char *func_name, char *line) {
    int func_loc = 0; 
    char new_line[1000];
    int stack = 0; 

    stack = strchr(line, '{') ? 1 : 0; // if { is on same line as function definition, start with stack = 1
    while (fgets(new_line, sizeof(new_line), fp) != NULL){
        if (strchr(new_line, '{')){
            stack++; // Push to stack
        }
        if (strchr(new_line, '}')) {
            stack--; // Pop from stack
        }
        func_loc++;
        if (stack == 0) {
            break; 
        }
    }

    return func_loc;
}

char **get_non_OOP_LOC(char *filename) {
    char usr_input[100], line[1000], func_name[100]; 
    FILE *fp = open_file(filename);
    int func_count = get_func_count(fp), func_LOC = 0, program_loc = 0;
    char **final_str_arr = init_str_arr(func_count) ;
    func_count = 0;

    while (fgets(line, sizeof(line), fp) != NULL) { 
        if (check_str_is_func(line)) { 
            char *func_name = get_func_name(line);
            func_LOC = count_non_OOP_LOC(fp, func_name, line);
            snprintf(final_str_arr[func_count], 200, "Function %2d: %30s | LOC: %4d", func_count+1, func_name, func_LOC); // Format the string
            free(func_name); // Free dynamically allocated memory for func_name
            func_count++;
        }
    }
    final_str_arr[func_count] = NULL; // Null-terminate the array to indicate the end of strings
    for (int i = 0; final_str_arr[i] != NULL; i++) {
        printf("%s\n", final_str_arr[i]);
    }

    fclose(fp);
    return final_str_arr; 
}

// Get Porgram LOC
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

int check_char_in_str(char *str, char c) {
    while (*str) {
        if (*str == c) {
            return 1;
        }
        str++;
    }
    return 0;
}

int get_program_loc(char *filename){
    int count = 0; 
    char line_str[100], trimmed_line_str[100];
    FILE *fp = open_file(filename);
   
    while(fgets(line_str,100,fp) != NULL) { 
        strcpy(trimmed_line_str,trim(line_str));
        if (check_char_in_str(trimmed_line_str, '\n') && trimmed_line_str[0] != '\n' && !check_str_is_comment(trimmed_line_str)){
            count++;
        }
    }

    printf("Total Program LOC: %d\n", count); // Print the total LOC for the program

    fclose(fp); // Close the file after reading

    return count;
}

//postmortem
void export_data(char *user_file, char **final_str_arr, int count) {
    FILE *file = fopen("A3/postmortem.txt", "a+");
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
    for (int i = 0; final_str_arr[i] != NULL; i++) {
        fprintf(file, "%s\n", final_str_arr[i]);
    }
    fprintf(file, "--------------------");
    fclose(file);
}


int main(){
    char **func_loc, prog_type[10], filename[100];
    int program_loc = 0;

    printf("Enter filename: ");
    scanf("%s", filename);

    func_loc = get_non_OOP_LOC(filename);

    program_loc = get_program_loc(filename);

    export_data(filename, func_loc, program_loc); 

    return 0;
}

