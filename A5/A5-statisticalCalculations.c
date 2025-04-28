// gcc A5/A5-statisticalCalculations.c -o A5/A5-statisticalCalculations && ./A5/A5-statisticalCalculations

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

int get_coef(int i, int N);
float gamma(float x);
float Width(float X_high, float X_low, int N);
float calc_normal_dist(int N, float X_low, float X_high, float W, float E, char X_low_str[]);
float normal_dist_summ(int N, float X_low, float W);
float calc_chi_squared(int N, float X_low, float X_high, float W, float E, int DoF);
float chi_square_summ(int N, float X_low, float W, int DoF);
float calc_t_dist(int N, float X_low, float X_high, float W, float E, int DoF, char X_low_str[]);
float t_dist_summ(int N, float X_low, float W, int DoF);
bool is_numeric(const char *str);
void export_data(float lower_limit, float upper_limit, int intervales, int DoF, float tolerance, float normal_dist, float chi_squared, float t_dist);

int main(){
    char input_X_high[20], input_X_low[20];
    float X_high, X_low, E = 0, OldResult = 0, Result, W;
    int N = 20, DoF = 9;

    printf("Enter the lower limit (∞ = inf): ");
    scanf("%s", input_X_low);
    printf("Enter the upper limit: ");
    scanf("%s", input_X_high);
   

    while(!(is_numeric(input_X_high)) && !(is_numeric(input_X_low) || strcmp(input_X_low, "inf") == 0)){
        printf("Invalid input. Please enter numeric values for limits.\n");
        printf("Enter the upper limit: ");
        scanf("%s", input_X_high);
        printf("Enter the lower limit (∞ = inf): ");
        scanf("%s", input_X_low);
    }
    
    if (strcmp(input_X_low, "inf") == 0 ){
        X_high = 0;
        X_low = atof(input_X_high);
    } else {
        X_high = atof(input_X_high);
        X_low = atof(input_X_low);
    }

    if (X_high == X_low) {
        printf("Integration will be 0, no need to calculate.\n");
        return 0;
    } 
    
    W = Width(X_high, X_low, N);

    printf("--------------------\n");
    printf("Normal Distribution: %.6f\n", calc_normal_dist(N, X_low, X_high, W, E, input_X_low));
    printf("Chi-squared: %.6f\n", calc_chi_squared(N, X_low, X_high, W, E, DoF));
    printf("t-distribution: %.6f\n", calc_t_dist(N, X_low, X_high, W, E, DoF, input_X_low));

    export_data(X_low, X_high, N, DoF, E, 
                calc_normal_dist(N, X_low, X_high, W, E, input_X_low),
                calc_chi_squared(N, X_low, X_high, W, E, DoF),
                calc_t_dist(N, X_low, X_high, W, E, DoF, input_X_low));
    return 0;
}

float calc_t_dist(int N, float X_low, float X_high, float W, float E, int DoF, char X_low_str[]){
     float Result, OldResult = 0, iterations = 0, max_iterations = 10;

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

    if (strcmp(X_low_str, "inf") == 0){
        Result = (X_high > 0) ? .5 + Result : fabs(.5 - fabs(Result));
    }

    return Result;
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

float calc_chi_squared(int N, float X_low, float X_high, float W, float E, int DoF) {
    float Result, OldResult = 0, iterations = 0, max_iterations = 10;

    while (iterations < max_iterations){
        Result = chi_square_summ(N, X_low, W, DoF);
        if (fabs(Result - OldResult) <= E){
            break;
        }
        OldResult = Result;
        N *= 2;
        W = Width(X_high, X_low, N);
        iterations++;
    }

    return Result;
}

float chi_square_summ(int N, float X_low, float W, int DoF){
    float sum, summ_arr[N + 1], X_i, result = 0;
    float chi_squared_coef = (1 / (pow(2, DoF / 2.0) * gamma(DoF / 2.0))); // 1 / (2^(DoF/2) * Γ(DoF/2))
    int coef;

    // x^(DoF/2 - 1) * e^(-x/2)
    for (int i = 0; i < N + 1; i++) {
        X_i = X_low + i * W;
        coef = get_coef(i, N);
        summ_arr[i] = (W / 3) * coef * chi_squared_coef * pow(X_i, (DoF / 2.0 - 1)) * exp(-X_i / 2.0);
    }

    for (int j = 0; j < N + 1; j++) {
        result += summ_arr[j] ;
    }

    return result;
}

float calc_normal_dist(int N, float X_low, float X_high, float W, float E, char X_low_str[]){
    float Result, OldResult = 0, iterations = 0, max_iterations = 10;

    while (iterations < max_iterations){
        Result = normal_dist_summ(N, X_low, W);
        if (fabs(Result - OldResult) <= E){
            break;
        }
        OldResult = Result;
        N *= 2;
        W = Width(X_high, X_low, N);
        iterations++;
    }

    Result = (X_high > 0) ? .5 + Result : fabs(.5 - fabs(Result));

    return Result;
}

float normal_dist_summ(int N, float X_low, float W){
    float sum, summ_arr[N+1], X_i, result = 0; 
    float pi_coef = 1 / (sqrt(2 * M_PI)); // 1/√(2π)
    int coef;

    // ∑ (coef * e^(-0.5 * x^2) * (pi_coef)
    for (int i = 0; i < N+1; i++){
        X_i = X_low + i * W;
        coef = get_coef(i, N);
        summ_arr[i] = coef * exp(-0.5 * pow(X_i, 2));    
    }

    for (int j = 0; j < N+1; j++){
        result += summ_arr[j] * (pi_coef);
    }
    result *= (W / 3) ;

    return result;
}

float gamma(float x){
    return tgamma(x);
}

float Width(float X_high, float X_low, int N){
    return (X_high - X_low) / N;
}

int get_coef(int i, int N){
    if (i == 0 || i == N) return 1; // if x = X_low or X_high
    if (i % 2 == 0) return 2;
    return 4;
}

bool is_numeric(const char *str) {
    if (str == NULL) {
        return false;
    }

    while (*str != '\0') {
        if (!isdigit(*str)) {
            return false;
        }
        str++;
    }
    return true;
}

void export_data(float lower_limit, float upper_limit, int intervales, int DoF, float tolerance, float normal_dist, float chi_squared, float t_dist) {
    FILE *file = fopen("A5/processed_data.txt", "a+");
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
    fprintf(file, "lower_limit: %f | upper_limit: %f\nintervals: %d | degrees of freedom: %d, tolerance: %f\n", lower_limit, upper_limit, intervales, DoF, tolerance);
    fprintf(file, "Normal Distribution: %.6f\n", normal_dist);
    fprintf(file, "Chi-squared: %.6f\n", chi_squared);
    fprintf(file, "t-distribution: %.6f\n", t_dist);
    fprintf(file, "--------------------");
    fclose(file);
}