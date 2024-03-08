/*
Author: Angelo John C. Cruz
Section: CMSC 180 T-6L
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h> 
#include <pthread.h> 

typedef struct threadargs {
	int** matrix;
	int* y;
	int n;
	int m;
	int tid;
	int startcol;
}ARGS;

// function for actual pearson correlation coefficient factor computation
void *pearson_corr(void *args){
	double sum_x, sum_x2, sum_y, sum_y2, sum_xy, ans;
	
	ARGS* temp = (ARGS*) args;
	
	int tid = temp->tid;
	int n = temp->n; 
	int m = temp->m;
	int startcol = temp->startcol;
	int* y = temp->y; 
	int** matrix = temp->matrix;

	// for(int i=0; i<n; i++){
	// 	temp2 = startcol;

	// 	for(int j=0; j<m; j++){
	// 		printf("%d\t", matrix[i][temp2++]);
	// 	}
	// 	printf("\n");
	// }

	// for(int i=0; i<m; i++){
	// 	temp2 = startcol;

	// 	for(int j=0; j<n; j++){
	// 		printf("%d\t", matrix[j][i+temp2]);
	// 	}

	// 	printf("\n");
	// }

	// for when main matrix is subdivided into n x n/t submatrices
	for(int i=startcol; i<m; i++){
		sum_x = sum_x2 = sum_y = sum_y2 = sum_xy = 0;

		for(int j=0; j<n; j++){
			sum_x += matrix[j][i];
			sum_x2 += (matrix[j][i] * matrix[j][i]);
			sum_y += y[j];
			sum_y2 += (y[j] * y[j]);
			sum_xy += (matrix[j][i] * y[j]);

			ans = (n*sum_xy - (sum_x * sum_y)) / sqrt(((n*sum_x2) - (sum_x * sum_x)) * ((n*sum_y2) - (sum_y * sum_y)));

			if(isnan(ans) || ans > 1 || ans < -1){
					printf("Submatrix %d: %f, %f, %f, %f, %f\n", tid, sum_x, sum_x2, sum_y, sum_y2, sum_xy);
					break;
			}	
		}
	}

	/*
	for(int i=startcol; i<m; i++){
		double sum_x = help_sum_x(matrix, y, n, i);
		double sum_x2 = help_sum_x2(matrix, y, n, i);
		double sum_y = help_sum_y(matrix, y, n, i);
		double sum_y2 = help_sum_y2(matrix, y, n, i);
		double sum_xy = help_sum_xy(matrix, y, n, i);
	
		// numerator = (n*sum_xy - (sum_x * sum_y));
		// denominator1 = ((n*sum_x2) - (sum_x * sum_x));
		// denominator2 = ((n*sum_y2) - (sum_y * sum_y));
		// ans = numerator / sqrt(denominator1 * denominator2);

		ans = (n*sum_xy - (sum_x * sum_y)) / sqrt(((n*sum_x2) - (sum_x * sum_x)) * ((n*sum_y2) - (sum_y * sum_y)));
		
		if(isnan(ans) || ans > 1 || ans < -1){
			printf("Submatrix %d: %f, %f, %f, %f, %f\n", tid, sum_x, sum_x2, sum_y, sum_y2, sum_xy);
			break;
		}
	}
	*/

	pthread_exit(NULL);
}

int main(){
	int n, t, randnum, bound = 0, startcol = 0, remainder = 0;

	// asking for matrix size
	printf("enter matrix size: ");
	
	scanf("%d", &n);
	
	// asking for thread number
	printf("enter # of threads: ");
	
	scanf("%d", &t);
	
	// initialization of arrays
	int** matrix = (int**) malloc (sizeof(int*) * n); // main matrix
	ARGS* params = (ARGS*) malloc (sizeof(ARGS) * t); // parameters for thread function
	pthread_t* tid = (pthread_t*) malloc (sizeof(pthread_t) * t); // array for thread id

	if(matrix == NULL || params == NULL){
		printf("Memory allocation failed\n");
		return 0;
	}
	
	// 2d arrays
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
	
	// FOR CHECKING: prints original matrix	
	// for(int i=0; i<n; i++){
	// 	for(int j=0; j<n; j++){
	// 		printf("%d\t", matrix[i][j]);
	// 	}
	// 	printf("\n");
	// }
	
	// printf("\n");

	// for(int j=0; j<n; j++){
	// 	printf("%d\t", y[j]);
	// }

	// printf("\n");

	// computing remainder
	remainder = n % t;

	// initialization of params
	for(int i=0; i<t; i++){
		if(i < remainder && remainder != 0){
			bound = n/t + 1;
		} else {
			bound = n/t;
		}
		
		params[i].matrix = matrix;
		params[i].y = y;
		params[i].n = n;
		params[i].m = bound;
		params[i].startcol = startcol;
		params[i].tid = i;
		
		startcol += bound;
	}

	clock_t begin = clock();

	// creation of threads
	for(int i=0; i<t; i++){
		pthread_create(&tid[i], NULL, pearson_corr, (void *)&params[i]);
	}

	// thread joining
	for(int i=0; i<t; i++){
		pthread_join(tid[i], NULL);
	}

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
