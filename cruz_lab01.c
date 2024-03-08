/*
Author: Angelo John C. Cruz
Section: CMSC 180 T-6L
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// function for checking pearson correlation coefficient factor computation
void pearson_corr_sample(){
	double sum_x, sum_x2, sum_y, sum_y2, sum_xy, r, numerator, denominator1, denominator2;
	
	// matrix
	double sample[10][10] = {
		{3.63,0,0,0,0,0,0,0,0,0}, 
		{3.02,0,0,0,0,0,0,0,0,0}, 
		{3.82,0,0,0,0,0,0,0,0,0}, 
		{3.42,0,0,0,0,0,0,0,0,0},
		{3.59,0,0,0,0,0,0,0,0,0}, 
		{2.87,0,0,0,0,0,0,0,0,0}, 
		{3.03,0,0,0,0,0,0,0,0,0}, 
		{3.46,0,0,0,0,0,0,0,0,0}, 
		{3.36,0,0,0,0,0,0,0,0,0}, 
		{3.3,0,0,0,0,0,0,0,0,0}
	};
	
	// vector
	double y[10] = {53.1, 49.7, 48.4, 54.2, 54.9, 43.7, 47.2, 45.2, 54.4, 50.4};

	// computation
	for(int j=0; j<10; j++){
		sum_x = sum_x + sample[j][0];
		sum_x2 = sum_x2 + sample[j][0] * sample[j][0];
		sum_y = sum_y + y[j];
		sum_y2 = sum_y2 + y[j] * y[j];
		sum_xy = sum_xy + (sample[j][0] * y[j]);
	}
	
	numerator = (10*sum_xy - (sum_x * sum_y));
	denominator1 = ((10*sum_x2) - (sum_x * sum_x));
	denominator2 = ((10*sum_y2) - (sum_y * sum_y));
	r = numerator / sqrt(denominator1 * denominator2);
	
	printf("check: %f\n", r);
}

// function for actual pearson correlation coefficient factor computation
void pearson_corr(int** matrix, int* y, int m, int n){
	int sum_x, sum_y, sum_x2, sum_y2, sum_xy;
	double numerator, denominator1, denominator2;
	
	double* r = (double*) malloc (sizeof(double) * n);
		
	for(int i=0; i<n; i++){
		r[i] = 0;
		sum_x = sum_x2 = sum_y = sum_y2 = sum_xy = numerator = denominator1 = denominator2 = 0;
		
		for(int j=0; j<n; j++){
			sum_x = sum_x + matrix[j][i];
			sum_x2 = sum_x2 + matrix[j][i] * matrix[j][i];
			sum_y = sum_y + y[j];
			sum_y2 = sum_y2 + y[j] * y[j];
			sum_xy = sum_xy + (matrix[j][i] * y[j]);
		}
		
		numerator = (n*sum_xy - (sum_x * sum_y));
		denominator1 = ((n*sum_x2) - (sum_x * sum_x));
		denominator2 = ((n*sum_y2) - (sum_y * sum_y));
		r[i] = numerator / sqrt(denominator1 * denominator2);
	}
}

int main(){
	int n, randnum;
	
	// asking for matrix size
	printf("enter matrix size: ");
	
	scanf("%d", &n);
	
	// initialization of 2d array
	int** matrix = (int**) malloc (sizeof(int*) * n);
	
	if(matrix == NULL){
		printf("Memory allocation failed\n");
		return 0;
	}
	
	for(int i=0; i<n; i++){
		matrix[i] = (int*) malloc (sizeof(int) * n);
		
		if(matrix[i] == NULL){
			printf("Memory allocation failed\n");
			return 0;
		}
	}
	
	// filling up matrix with values
	srand(time(NULL));
	
	for(int i=0; i<n; i++){
		for(int j=0; j<n; j++){
			do{
				randnum = rand() % 16;
			}while(randnum == 0);
			
			matrix[i][j] = randnum;
			randnum = 0;
		}
	}
	
	// initialization of vector y
	int* y = (int*) malloc (sizeof(int) * n);
	
	if(y == NULL){
		printf("Memory allocation failed\n");
		return 0;
	}
	
	// filling up vector y with random values
	for(int i=0; i<n; i++){
		do{
			randnum = rand() % 16;
		}while(randnum == 0);
		
		y[i] = randnum;
		randnum = 0;
	}
	
	//pearson_corr_sample();
	
	clock_t begin = clock();
	pearson_corr(matrix, y, n, n);
	clock_t end = clock();
	
	double time_elapsed = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("time elapsed: %f\n", time_elapsed);
	
	// memory deallocation
	for(int i=0; i<n; i++){
		free(matrix[i]);
	}
	
	free(y);
	free(matrix);
	
	return 0;
}
