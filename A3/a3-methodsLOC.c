#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

FILE* open_file(){
    char filename[100];
    printf("Enter filename: ");
    scanf("%s", filename);

    FILE *fp = fopen(filename, "r"); 
    
    while (fp == NULL){
        printf("Please enter a valid filename: ");
        scanf("%s", filename);
        fp = fopen(filename, "r");
    }

    return fp;
}

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

char** get_non_OOP_LOC() {
    char usr_input[100], line[1000], func_name[100]; 
    FILE *fp = open_file();
    int func_count = get_func_count(fp), func_LOC = 0, program_loc = 0;
    char **final_str_arr = init_str_arr(func_count) ;
    func_count = 0;

    while (fgets(line, sizeof(line), fp) != NULL) { 
        if (check_str_is_func(line)) { 
            char *func_name = get_func_name(line);
            func_LOC = count_non_OOP_LOC(fp, func_name, line);
            snprintf(final_str_arr[func_count], 200, "Function %d: %30s | LOC: %4d", func_count+1, func_name, func_LOC); // Format the string
            free(func_name); // Free dynamically allocated memory for func_name
            func_count++;
        }
    }

    fclose(fp);

    return final_str_arr;
}


int main(){
    char **result, prog_type[10];
    int program_loc = 0;

    result = get_non_OOP_LOC();
    for (int i = 0; result[i] != NULL; i++) {
        printf("%s\n", result[i]);
    }
    

}

