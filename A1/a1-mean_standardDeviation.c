// gcc a1-mean_standardDeviation.c -o a1-mean_standardDeviation && ./a1-mean_standardDeviation

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

typedef struct node
{
    struct node *next;
    float value;
} node;

float mean(node *head, int N);
float std_dev(node *head, float mean, int N);
void close_list(node *head);
void export_data(float mean, float std_dev);

int main(){
    node head = {NULL, 0};
    char input;

    printf("read from file or keyboard? (f/k) ");
    scanf("%c", &input);

    if (input == 'f'){
        char filename[20] = "data.txt", row[40], line[300];
        int N = 10; // number of nums

        FILE *file = fopen(filename, "r");
        if (file == NULL){
            printf("file not found\n");
            return 1;
        }

        printf("do you want Object_LOC, New_and_Changed_LOC, or Dev_hrs? ");
        scanf("%s", row);
        
        // initialize linked list
        while (fgets(line, sizeof(line), file)){
            char *token = strtok(line, ","); // get row header
            if (strcmp(token, row) == 0){ 
                token = strtok(NULL, ","); // get the first number for the row
                while (token != NULL){
                    struct node *current = (struct node *)malloc(sizeof(node));
                    current->value = atof(token); // convert string to float
                    current->next = NULL;

                    // get last node in list 
                    if (head.next == NULL){
                        head.next = current;
                    } else {
                        struct node *temp = head.next;
                        while (temp->next != NULL){
                            temp = temp->next;
                        }
                        temp->next = current;
                    }

                    token = strtok(NULL, ","); // returns a pointer to the next token
                }
            }
        }
        float mean_num = mean(head.next, N);
        printf("mean: %.2f\n", mean_num);
        float std_dev_num = std_dev(head.next, mean_num, N);
        printf("std_dev: %.2f\n", std_dev_num);
        export_data(mean_num, std_dev_num);

        fclose(file);
        close_list(head.next);
    } else {
        char input[20] = "1";
        int N = 0;
        while (1) {
            struct node *current = (struct node *)malloc(sizeof(node));

            printf("Enter a number (q to quit): ");
            scanf("%s", input);

            if (strcmp(input, "q") == 0) {break;}
            
            current->value = atof(input);
            current->next = NULL;
            N += 1;

            if (head.next == NULL){
                head.next = current;
            } else {
                struct node *temp = head.next;
                while (temp->next != NULL){
                    temp = temp->next;
                }
                temp->next = current;
            }
        }
        float mean_num = mean(head.next, N);
        printf("mean: %.2f\n", mean_num);
        float std_dev_num = std_dev(head.next, mean_num, N);
        printf("std_dev: %.2f\n", std_dev_num);
        export_data(mean_num, std_dev_num);

        close_list(head.next);
    }
}

float mean(node *head, int N) {
    float mean = 0, sum = 0;

    node *current = head;
    while (current != NULL) {
        sum += current->value;
        current = current->next;
    }
    mean = sum / N;
    return mean;
}

float std_dev(node *head, float mean, int N) {
    float std_dev = 0, summation = 0;

    node *current = head;
    while (current != NULL) {
        summation += pow(current->value - mean, 2);
        current = current->next;
    }
    std_dev = sqrt(summation / (N-1));
    return std_dev;
}

void close_list(node *head) {
    node *current = head;
    while (current != NULL) {
        node *temp = current;
        current = current->next;
        free(temp);
    }
}

// postmortem
void export_data(float mean, float std_def){
    FILE *file = fopen("processed_data.txt", "a+");
    if (file == NULL){
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
    fprintf(file, "mean: %.2f\nstandard deviation:%.2f\n", mean, std_def);
    fprintf(file, "--------------------");
    fclose(file);
}