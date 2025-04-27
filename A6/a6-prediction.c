// gcc A6/a6-prediction.c -o A6/a6-prediction && ./A6/a6-prediction

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

node *create_linked_list(char line1[], char line2[], node *head);
FILE *open_file(char *filename);
void close_list(node *head);
void export_data(float B_1, float B_0, float Y_k, int X_k, float std_dev, float range_sev, float t_dist_sev, float UPL_sev, float LPL_sev, float range_nine, float t_dist_nine, float UPL_nine, float LPL_nine);

Averages calc_X_Y_avg(node *head, int N);
float Width(float X_high, float X_low, int N);
int get_coef(int i, int N);
float gamma(float x);
float calc_t_val(int N, float alpha, float E, int DoF);
float t_dist_summ(int N, float X_low, float W, int DoF);
float B_1(node *head, int N);
float calc_B_1_numerator(node *head, int N, float X_avg, float Y_avg);
float calc_B_1_denomenator(node *head, int N, float X_avg);
float B_0(node *head, int N, float B1);
float calc_range(node *head, int N, float X_avg, int X_k, float std_dev, float t_val);
float calc_std_dev(node *head, int N, float B_0, float B_1);


int main() {
    node *head = NULL;
    int N = 10, X_k, DoF;
    float X_avg, B1, B0, std_dev, t_dist_sev, t_dist_nine, Y_k, W, UPL_sev, LPL_sev, UPL_nine, LPL_nine;
    char filename[100], line1[100], line2[100];

    printf("Enter Filepath + name: ");
    scanf("%s", filename);
    printf("Enter the value of X_k: ");
    scanf("%d", &X_k);

    FILE *file = open_file(filename);
    fgets(line1, sizeof(line1), file);
    fgets(line2, sizeof(line2), file);
    head = create_linked_list(line1, line2, head);

    // Linear Regression
    B1 = B_1(head, N);
    B0 = B_0(head, N, B1);
    
    Y_k = (B0 + (B1 * X_k));
    
    // Prediction
    Averages avgs = calc_X_Y_avg(head, N);
    X_avg = avgs.X_avg;

    std_dev = calc_std_dev(head, N, B0, B1);
    W = Width(X_k, X_avg, N);
    t_dist_sev = calc_t_val(N, .15, .85, N-2); // ɑ = 30%
    t_dist_nine = calc_t_val(N, .05, .95, N-2); // ɑ = 10%

    float range_sev = calc_range(head, N, X_avg, X_k, std_dev, t_dist_sev);
    float range_nine = calc_range(head, N, X_avg, X_k, std_dev, t_dist_nine);

    UPL_sev = Y_k + range_sev;
    LPL_sev = Y_k - range_sev;
    UPL_nine = Y_k + range_nine;
    LPL_nine = Y_k - range_nine;

    printf("---------------------\n");
    printf("B1: %.4f\n", B1);
    printf("B0: %.4f\n", B0);
    printf("Estimated LOC: %.0f\n", Y_k);
    printf("Standard Deviation: %.3f\n\n", std_dev);
    printf("range(.70): %.0f\n", range_sev);
    printf("t-dist(.70): %.3f\n", t_dist_sev);
    printf("UPL: %.0f\n", UPL_sev);
    printf("LPL: %.0f\n\n", LPL_sev);
    printf("range(.95): %.0f\n", range_nine);
    printf("t-dist(.95): %.3f\n", t_dist_nine);
    printf("UPL: %.0f\n", UPL_nine);
    printf("LPL: %.0f\n", LPL_nine);

    export_data(B1, B0, Y_k, X_k, std_dev, range_sev, t_dist_sev, UPL_sev, LPL_sev, range_nine, t_dist_nine, UPL_nine, LPL_nine);

    close_list(head);

    return 0;
}

float calc_range(node *head, int N, float X_avg, int X_k, float std_dev, float t_val){
    node *current = head;
    float range = 0, range_tmp = 0, summation = 0, Y_i, X_i; 

    while (current != NULL) {
        Y_i = current->Y;
        X_i = current->X;
        summation += pow(X_i - X_avg, 2);
        current = current->next;
    }

    range_tmp = sqrt(1 + (1 / (float)N) + (pow(X_k - X_avg,2) / summation));
    range = t_val * std_dev * range_tmp;

    return range;
}

float calc_t_val(int N, float alpha, float E, int DoF) {
    float Result = 0, OldResult = 0, iterations = 0, max_iterations = 10;
    float X_low = 0, X_high = 0, W = 0, trial_t = 0;

    while (Result < E){
        X_high += .001;
        W = Width(X_high, X_low, N);
        while (iterations < max_iterations){
            Result = t_dist_summ(N, X_low, W, DoF);
            if (fabs(Result - OldResult) <= E){
                break;
            }
            OldResult = Result;
            N *= 2;
            W = Width(X_high, X_low, N);
            iterations++;
        }

        Result = (X_high > 0) ? .5 + Result : fabs(.5 - fabs(Result));
    }

    return X_high;
}

float t_dist_summ(int N, float X_low, float W, int DoF){
    float sum, summ_arr[N + 1], X_i, result = 0;
    float t_dist_coef = ((gamma((DoF + 1) / 2.0) / (sqrt(DoF * M_PI) * gamma(DoF / 2.0)))); // Γ((DoF + 1)/2) / (√(DoFπ) * Γ(DoF/2))
    int coef;

    // x^(DoF/2 - 1) * e^(-x/2)
    for (int i = 0; i < N + 1; i++) {
        X_i = X_low + i * W;
        coef = get_coef(i, N);
        summ_arr[i] = (W / 3) * coef * t_dist_coef * pow((1 + (pow(X_i,2) / DoF)), -(DoF + 1) / 2.0);
    }

    for (int j = 0; j < N + 1; j++) {
        result += summ_arr[j] ;
    }

    return result;
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

float calc_std_dev(node *head, int N, float B_0, float B_1){
    node *current = head;
    float std_dev, variance, summation = 0, Y_i, X_i; 

    while (current != NULL) {
        Y_i = current->Y;
        X_i = current->X;
        summation += pow(Y_i - B_0 - B_1 * X_i, 2);
        current = current->next;
    }
    variance = summation / (N - 2);
    std_dev = sqrt(variance);

    return std_dev;
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

int get_coef(int i, int N){
    if (i == 0 || i == N) return 1; // if x = X_low or X_high
    if (i % 2 == 0) return 2;
    return 4;
}

float Width(float X_high, float X_low, int N){
    return (X_high - X_low) / N;
}

float gamma(float x){
    return tgamma(x);
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

void export_data(float B_1, float B_0, float Y_k, int X_k, float std_dev, float range_sev, float t_dist_sev, float UPL_sev, float LPL_sev, float range_nine, float t_dist_nine, float UPL_nine, float LPL_nine) {
    FILE *file = fopen("A6/processed_data.txt", "a+");
    if (file == NULL){
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
    fprintf(file, "B1 + B0 * X_k = Y_k | %.4f + %.4f * %d = %.0f\n", B_1, B_0, X_k, Y_k);
    fprintf(file, "Standard Deviation: %.3f\n\n", std_dev);
    fprintf(file, "range(.70): %.0f | t_value: %.4f\n", range_sev, t_dist_sev);
    fprintf(file, "UPL: %.0f | LPL: %.0f\n\n", UPL_sev, LPL_sev);
    fprintf(file, "range(.95): %.0f | t_value: %.4f\n", range_nine, t_dist_nine);
    fprintf(file, "UPL: %.0f | LPL: %.0f\n", UPL_nine, LPL_nine);
    fprintf(file, "--------------------");
    fclose(file);
}