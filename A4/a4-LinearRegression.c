// gcc A4/a4-LinearRegression.c -o A4/a4-LinearRegression && A4/a4-LinearRegression

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

typedef struct node {
    struct node *next;
    int X, Y;
} node;

typedef struct Averages {
    float X_avg, Y_avg;
} Averages;

node* create_linked_list(char line1[], char line2[], node* head);
void close_list(node *head);
void export_data(float B_o, float B_1, int estimated, int actual);
float B_1(node *head, int N);
float B_0(node *head, int N, float B1);
Averages calc_X_Y_avg(node *head, int N);
float calc_B_1_numerator(node *head, int N, float X_avg, float Y_avg);
float calc_B_1_denomenator(node *head, int N, float X_avg);
FILE* open_file(char *filename);

int main() {
    node *head = NULL;
    int N = 10, Y_k, X_k; // number of nums in data set
    float B1, B0;
    char line1[100], line2[100], choice;
    
    do {
        printf("Have you input values for X and Y into data.txt (y/n)? ");
        scanf(" %c", &choice);
    } while (choice != 'y');
    
    // initialize linked list 
    FILE *file = open_file("A4/data.txt");
    fgets(line1, sizeof(line1), file);
    fgets(line2, sizeof(line2), file);
    head = create_linked_list(line1,line2,head);

    B1 = B_1(head, N);
    B0 = B_0(head,N,B1);

    printf("B0 = %f | B1 = %f\n", B0, B1);

    printf("Enter Estimated Object size: ");
    scanf("%d", &X_k);

    Y_k = (B0 + (B1*X_k));

    printf("The actual outcome will likely be %d LOC\n", Y_k);

    export_data(B0,B1,X_k,Y_k);
    
    return 0;
}

float B_1(node *head, int N){
    Averages avgs = calc_X_Y_avg(head, N);
    float X_avg, Y_avg, numerator, denominator, B1;

    X_avg = avgs.X_avg;
    Y_avg = avgs.Y_avg;

    numerator = calc_B_1_numerator(head, N, X_avg, Y_avg);
    denominator = calc_B_1_denomenator(head, N, X_avg);

    B1 = numerator/denominator;

    return B1;
}

float calc_B_1_numerator(node *head, int N, float X_avg, float Y_avg){
    node *current = head;
    float final_result = 0, summ_arr[N], X_i, Y_i;

    for(int i = 0; current != NULL; i++){
        X_i = current->X;
        Y_i = current->Y;
        summ_arr[i] = (X_i * Y_i); // ∑ (X_i * Y_i)
        current = current->next;
    }

    for(int j = 0; j < 10; j++){
        final_result += summ_arr[j];
    }

    final_result -= (N * X_avg * Y_avg); // ∑ (X_i * Y_i) - (N*X_avg*Y_avg)

    return final_result;
}

float calc_B_1_denomenator(node *head, int N, float X_avg){
    node *current = head;
    float final_result = 0, summ_arr[N], X_i;

    for(int i = 0; current != NULL; i++){
        X_i = current->X;
        summ_arr[i] = (pow(current->X, 2)); // ∑ (X_i^2)
        current = current->next;
    }

    for(int j = 0; j < 10; j++){
        final_result += summ_arr[j];
    }

    final_result -= (N * pow(X_avg, 2)); // ∑ (X_i^2) - (n*X_avg^2)

    return final_result;
}

float B_0(node *head, int N, float B1){ 
    Averages avgs = calc_X_Y_avg(head, N);
    float X_avg, Y_avg, B0;

    X_avg = avgs.X_avg;
    Y_avg = avgs.Y_avg;

    B0 = (Y_avg - (B1*X_avg));
    
    return B0; 
}

Averages calc_X_Y_avg(node *head, int N) {
    node *current = head;
    Averages avgs;
    float sum_X = 0, sum_Y =0;
    while(current != NULL){
        sum_X += current->X;
        sum_Y += current->Y;
        current = current->next;
    }

    avgs.X_avg = sum_X / N;
    avgs.Y_avg = sum_Y / N;

    return avgs;
}

node *create_linked_list(char line1[], char line2[], node* head){
    // init linked list and add X values
    char *X_num = strtok(line1, ","); 
    while (X_num != NULL){
        struct node *current = (struct node *)malloc(sizeof(node));

        if (current == NULL){ 
            printf("Failed to allocate memory in create_linked_list\n");
            exit(0);
        }
        
        // convert string to integer
        current->X = atoi(X_num); 
        current->next = NULL;

        // add node to the linked list
        if (head == NULL){
            head = current;
        } else {
            struct node *temp = head;
            while (temp->next != NULL){
                temp = temp->next;
            }
            temp->next = current;
        }

        // get next X and Y
        X_num = strtok(NULL, ","); 
    }

    // add Y values
    // CANNOT be in the previous while loop because of how strtok works
    char *Y_num = strtok(line2, ",");
    struct node *temp = head;
    while (Y_num != NULL){
        temp->Y = atoi(Y_num);
        temp = temp->next;
        Y_num = strtok(NULL, ",");
    }

    return head;
}

void close_list(node *head) {
    node *current = head;
    while (current != NULL) {
        node *temp = current;
        current = current->next;
        free(temp);
    }
}

FILE* open_file(char *filename) {
    FILE *fp = fopen(filename, "r"); 
    
    while (fp == NULL){
        printf("Please enter a valid filename: ");
        scanf("%s", filename);
        fp = fopen(filename, "r");
    }

    return fp;
}

// postmortem
void export_data(float B_o, float B_1, int estimated, int actual) {
    FILE *file = fopen("A4/processed_data.txt", "a+");
    if (file == NULL)
    {
        printf("file not found. Could not save results.\n");
        return;
    }

    time_t timer;
    char buffer[26];
    struct tm *tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(file, "\n%s\n", buffer);
    fprintf(file, "B_o: %.2f | B_1:%.2f\n", B_o, B_1);
    fprintf(file, "Estimated: %.2d | Calculated Actual: %.2d\n", estimated, actual);
    fprintf(file, "--------------------");
    fclose(file);
}