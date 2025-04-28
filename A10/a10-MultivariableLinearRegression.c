// gcc A10/a10-MultivariableLinearRegression.c -o A10/a10-MultivariableLinearRegression && ./A10/a10-MultivariableLinearRegression

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MY_NULL -99999

typedef struct betaConstant {
	int beta_num;
	float beta_val, constant;
} betaConstant;

typedef struct node {
    struct node *next;
    int vals[4]; // W, X, Y, Z
} node;

typedef struct Averages {
    float avgs[3];
} Averages;

node *create_linked_list(char line1[], char line2[], char line3[], char line4[], node *head);
FILE *open_file(char *filename);
void close_list(node *head);
void export_data(float beta_arr[], int expected_vals[], float Z_k, float std_dev, float range_sev, float t_dist_sev, float UPL_sev, float LPL_sev, float range_nine, float t_dist_nine, float UPL_nine, float LPL_nine);

Averages calc_avgs(node *head, int N);
float Width(float X_high, float X_low, int N);
int get_coef(int i, int N);
float gamma(float x);
float calc_t_val(int N, float alpha, float E, int DoF);
float t_dist_summ(int N, float X_low, float W, int DoF);
float calc_range(node *head, int N, float avgs[], int expected_vals[], float std_dev, float t_val);
float calc_std_dev(node *head, int N, float beta_arr[]);
void init_matrix(betaConstant matrix[4][5], node *head, int N);
float constant_summ(node *head, int row, int col);
float *Guass_elim(betaConstant matrix[4][5]);
void gauss_recr(betaConstant matrix[4][5], int eq_0);
void multiply_eq_by_constant(betaConstant matrix[4][5], float constant, int eq, float result_arr[]);
void subtract_two_eqs(betaConstant matrix[4][5], float mult_arr[], int eq2);
float solve_for_beta(betaConstant matrix[4][5], int beta_num);
void set_beta_val(betaConstant matrix[4][5], int beta_num, float beta_val);

int main(){
    node *head = NULL;
    betaConstant matrix[4][5]; // Declare the matrix array
    int N = 6, expected_vals[3], DoF = 4; // 6-4
    float std_dev, t_dist_sev, t_dist_nine, Z_k, W, UPL_sev, LPL_sev, UPL_nine, LPL_nine;
    char filename[100], line1[100], line2[100], line3[100], line4[100];

    printf("Enter Filepath + name: ");
    scanf("%s", filename);
    printf("Enter the value of New LOC: ");
    scanf("%d", &expected_vals[0]);
    printf("Enter the value of Reused LOC: ");
    scanf("%d", &expected_vals[1]);
    printf("Enter the value of Modified LOC: ");
    scanf("%d", &expected_vals[2]);

    FILE *file = open_file(filename);
    fgets(line1, sizeof(line1), file); // W
    fgets(line2, sizeof(line2), file); // X
    fgets(line3, sizeof(line3), file); // Y
    fgets(line4, sizeof(line4), file); // Z
    head = create_linked_list(line1, line2, line3, line4, head);

    init_matrix(matrix, head, N);
    Averages avgs = calc_avgs(head, N);

    float *beta_arr = Guass_elim(matrix);
    for (int i = 0; i < 4; i++){
        Z_k += (i == 0) ? beta_arr[i] : beta_arr[i] * expected_vals[i - 1];
    }

    std_dev = calc_std_dev(head, N, beta_arr);

    t_dist_sev = calc_t_val(N, .15, .85, N-4); 
    t_dist_nine = calc_t_val(N, .1, .95, N-4); 

    float range_sev = calc_range(head, N, avgs.avgs, expected_vals, std_dev, t_dist_sev);
    float range_nine = calc_range(head, N, avgs.avgs, expected_vals, std_dev, t_dist_nine);

    UPL_sev = Z_k + range_sev;
    LPL_sev = Z_k - range_sev;
    UPL_nine = Z_k + range_nine;
    LPL_nine = Z_k - range_nine;

    printf("\nStandard Deviation: %.3f\n\n", std_dev);
    printf("range(.85): %.0f\n", range_sev);
    printf("t-dist(.85): %.3f\n", t_dist_sev);
    printf("UPL: %.0f\n", UPL_sev);
    printf("LPL: %.0f\n\n", LPL_sev);
    printf("range(.95): %.0f\n", range_nine);
    printf("t-dist(.95): %.3f\n", t_dist_nine);
    printf("UPL: %.0f\n", UPL_nine);
    printf("LPL: %.0f\n", LPL_nine);

    export_data(beta_arr, expected_vals, Z_k, std_dev, range_sev, t_dist_sev, UPL_sev, LPL_sev, range_nine, t_dist_nine, UPL_nine, LPL_nine);

    close_list(head);

    return 0;
}

float calc_range(node *head, int N, float avgs[], int expected_vals[], float std_dev, float t_val){
    node *current;
    float range = 0, range_tmp = 0, summations[3], numerators[3], total_summ = 0; 

    for (int i = 0; i < 3; i++){
        current = head;
        numerators[i] = pow(expected_vals[i] - avgs[i],2);
        while (current != NULL) {
            summations[i] += pow(current->vals[i] - avgs[i], 2);
            current = current->next;
        }
    }

    for (int i = 0; i < 3; i++){
        total_summ += numerators[i] / summations[i];
    }

    range_tmp = sqrt(1 + (1 / (float)N) + total_summ);
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

float calc_std_dev(node *head, int N, float beta_arr[]) {
    node *current = head;
    float std_dev, variance, summation = 0, temp_res = 0, Z_i; 

    while (current != NULL) {
        temp_res = 0;
        for (int i = 0; i<4; i++){
            temp_res += (i == 0) ? beta_arr[i] : beta_arr[i] * current->vals[i-1];
        }
        Z_i = current->vals[3];
        summation += pow(Z_i - temp_res, 2);
        current = current->next;
    }
    variance = summation / (N - 4);
    std_dev = sqrt(variance);

    return std_dev;
}

float* Guass_elim(betaConstant matrix[4][5]){
    float *beta_arr = (float *)malloc(4 * sizeof(float)); 
    if (beta_arr == NULL) {
        printf("Memory allocation failed for beta_arr\n");
        exit(1);
    }

    gauss_recr(matrix, 0);
    
    for (int i = 0; i < 4; i++) {
        beta_arr[i] = matrix[0][i].beta_val;
    }
    
    return beta_arr; 
}

void gauss_recr(betaConstant matrix[4][5], int eq_0) {
    static float temp_arr[4];
    float beta_val;

    for (int eq = eq_0+1; eq<4; eq++){
        float constant = matrix[eq][eq_0].constant / matrix[eq_0][eq_0].constant;
        multiply_eq_by_constant(matrix, constant, eq_0, temp_arr);
        subtract_two_eqs(matrix, temp_arr, eq);
    }

    if (eq_0 == 3) {
        beta_val = matrix[3][4].constant / matrix[3][3].constant;
        set_beta_val(matrix, 3, beta_val);
        return;
    } 

    gauss_recr(matrix, eq_0 + 1);
    beta_val = solve_for_beta(matrix, eq_0);
    set_beta_val(matrix, eq_0, beta_val);

    return;
}

float solve_for_beta(betaConstant matrix[4][5], int beta_num) {
    float sum = 0, result;

    for (int j = beta_num+1; j<4; j++){
        sum += matrix[beta_num][j].beta_val * matrix[beta_num][j].constant;
    }
    
    result = (matrix[beta_num][4].constant - sum) / matrix[beta_num][beta_num].constant;

    return result;
}

void set_beta_val(betaConstant matrix[4][5], int beta_num, float beta_val) {
    for (int i = 0; i<4; i++){
        if (matrix[i][beta_num].beta_num == beta_num){
            matrix[i][beta_num].beta_val = beta_val;
        }
    }
}

void multiply_eq_by_constant(betaConstant matrix[4][5], float constant, int eq, float result_arr[]) {
    for (int i = 0; i<5; i++){
        result_arr[i] = matrix[eq][i].constant * constant;
    }
}

void subtract_two_eqs(betaConstant matrix[4][5], float mult_arr[], int eq2) {
    for (int i = 0; i<5; i++){
        matrix[eq2][i].constant -= mult_arr[i];
    }
}

void init_matrix(betaConstant matrix[4][5], node *head, int N) {
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 5; j++){
            matrix[i][j].beta_num = (j == 4) ? MY_NULL : j;
            matrix[i][j].beta_val = (j == 4) ? MY_NULL : 0;
            matrix[i][j].constant = (i == 0 && j == 0) ? N : constant_summ(head, i, j);
        }
    }
}

float constant_summ(node *head, int row, int col){
    node *current = head;
    float summation = 0;

    if (row == 0){
        while (current != NULL){
            summation += current->vals[col-1];
            current = current->next;
        }
    } else if (col == 0){
        while (current != NULL){
            summation += current->vals[row-1];
            current = current->next;
        }
    } 
    else {
        while (current != NULL){
            summation += current->vals[row-1] * current->vals[col-1];
            current = current->next;
        }
    }
    return summation;
}

Averages calc_avgs(node *head, int N) {
    Averages avgs;
    node *current;
    int sum;

    for (int i = 0; i<3; i++){
        current = head;
        sum = 0;
        while (current != NULL){
            sum += current->vals[i];
            current = current->next;
        }
        avgs.avgs[i] = sum / N;
    }

    return avgs;
}

float Width(float X_high, float X_low, int N){
    return (X_high - X_low) / N;
}

float gamma(float x){
    return tgamma(x);
}

int get_coef(int i, int N){
    if (i == 0 || i == N) return 1; // if x = X_low or X_high
    if (i % 2 == 0) return 2;
    return 4;
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

node *create_linked_list(char line1[], char line2[], char line3[], char line4[], node *head) {
    // init linked list and add X values
    char *W_num = strtok(line1, ","); 
    while (W_num != NULL){
        struct node *current = (struct node *)malloc(sizeof(node));

        if (current == NULL){ 
            printf("Failed to allocate memory in create_linked_list\n");
            exit(0);
        }
        
        current->vals[0] = atoi(W_num); 
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

        W_num = strtok(NULL, ","); 
    }

    // add rest of the values
    // CANNOT be in the previous while loop because strtok will override itself
    char *X_num = strtok(line2, ",");
    struct node *temp = head;
    while (X_num != NULL){
        temp->vals[1] = atoi(X_num);
        temp = temp->next;
        X_num = strtok(NULL, ",");
    }
    
    temp = head;
    char *Y_num = strtok(line3, ",");
    while (Y_num != NULL){
        temp->vals[2] = atoi(Y_num);
        temp = temp->next;
        Y_num = strtok(NULL, ",");
    }
    
    temp = head;
    char *Z_num = strtok(line4, ",");
    while (Z_num != NULL){
        temp->vals[3] = atoi(Z_num);
        temp = temp->next;
        Z_num = strtok(NULL, ",");
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

void export_data(float beta_arr[], int expected_vals[], float Z_k, float std_dev, float range_sev, float t_dist_sev, float UPL_sev, float LPL_sev, float range_nine, float t_dist_nine, float UPL_nine, float LPL_nine) {
    FILE *file = fopen("A10/processed_data.txt", "a+");
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
    fprintf(file, "Beta Coefficients:\n");
    for (int i = 0; i < 4; i++){
        fprintf(file, "B_%d: %.3f  ", i, beta_arr[i]);
    }
    fprintf(file, "\n\nExpected Values:\n");
    for (int i = 0; i < 3; i++){
        fprintf(file, "%d  ", expected_vals[i]);
    }
    fprintf(file, "\n\nStandard Deviation: %.3f\n\n", std_dev);
    fprintf(file, "range(.70): %.0f | t_value: %.4f\n", range_sev, t_dist_sev);
    fprintf(file, "UPL: %.0f | LPL: %.0f\n\n", UPL_sev, LPL_sev);
    fprintf(file, "range(.95): %.0f | t_value: %.4f\n", range_nine, t_dist_nine);
    fprintf(file, "UPL: %.0f | LPL: %.0f\n", UPL_nine, LPL_nine);
    fprintf(file, "--------------------");
    fclose(file);
}