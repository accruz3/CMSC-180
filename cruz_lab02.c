/*
Author: Angelo John C. Cruz
Section: CMSC 180 T-6L
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <unistd.h> 
#include <pthread.h> 

pthread_mutex_t lock;
double* r;

typedef struct threadargs {
	int** matrix;
	int* y;
	int n;
	int tid;
	int start;
	int end;
}ARGS;

// function for actual pearson correlation coefficient factor computation (row-wise)
void *row_pearson_corr(void *args){
	double sum_x, sum_x2, sum_y, sum_y2, sum_xy, ans;
	
	ARGS* temp = (ARGS*) args;
	
	int tid = temp->tid;
	int n = temp->n; 
	int start = temp->start;
	int end = temp->end;
	int* y = temp->y; 
	int** matrix = temp->matrix;
	
	for(int i=start; i<end; i++){
		sum_x = sum_x2 = sum_y = sum_y2 = sum_xy = ans = 0;
		
		for(int j=0; j<n; j++){
			sum_x += matrix[i][j];
			sum_x2 += (matrix[i][j] * matrix[i][j]);
			sum_y += y[j];
			sum_y2 += (y[j] * y[j]);
			sum_xy += (matrix[i][j] * y[j]);
		}
		
		ans = (n*sum_xy - (sum_x * sum_y)) / sqrt(((n*sum_x2) - (sum_x * sum_x)) * ((n*sum_y2) - (sum_y * sum_y)));

		if(isnan(ans) || ans > 1 || ans < -1){
				printf("Submatrix %d: %f, %f, %f, %f, %f\n", tid, sum_x, sum_x2, sum_y, sum_y2, sum_xy);
				break;
		}	
		
		pthread_mutex_lock(&lock);
		r[i] = ans;
		pthread_mutex_unlock(&lock);
	}

	
	pthread_exit(NULL);
}

// function for actual pearson correlation coefficient factor computation (column-wise)
void *pearson_corr(void *args){
	double sum_x, sum_x2, sum_y, sum_y2, sum_xy, ans;
	
	ARGS* temp = (ARGS*) args;
	
	int tid = temp->tid;
	int n = temp->n; 
	int start = temp->start;
	int end = temp->end;
	int* y = temp->y; 
	int** matrix = temp->matrix;

	for(int i=start; i<end; i++){
		sum_x = sum_x2 = sum_y = sum_y2 = sum_xy = ans = 0;
		
		for(int j=0; j<n; j++){
			sum_x += matrix[j][i];
			sum_x2 += (matrix[j][i] * matrix[j][i]);
			sum_y += y[j];
			sum_y2 += (y[j] * y[j]);
			sum_xy += (matrix[j][i] * y[j]);
		}
		
		ans = (n*sum_xy - (sum_x * sum_y)) / sqrt(((n*sum_x2) - (sum_x * sum_x)) * ((n*sum_y2) - (sum_y * sum_y)));

		if(isnan(ans) || ans > 1 || ans < -1){
				printf("Submatrix %d: %f, %f, %f, %f, %f\n", tid, sum_x, sum_x2, sum_y, sum_y2, sum_xy);
				break;
		}	
		
		pthread_mutex_lock(&lock);
		r[i] = ans;
		pthread_mutex_unlock(&lock);
	}
	
	pthread_exit(NULL);
}

int main(){
	int n, t, randnum, wise, bound = 0, start = 0, remainder = 0, end = 0;
	struct timeval begin, stop;
	
	// asking for matrix size
	printf("enter matrix size: ");
	
	scanf("%d", &n);
	
	// asking for thread number
	printf("enter # of threads: ");
	
	scanf("%d", &t);
	
	// asking for column-wise or row-wise
	printf("column-wise (0) or row-wise (1)? ");
	
	scanf("%d", &wise);
	
	// initialization of mutex
	if (pthread_mutex_init(&lock, NULL) != 0) { 
      printf("Mutex initialization failed\n"); 
      return 0; 
  } 
    
	// initialization of arrays
	int** matrix = (int**) malloc (sizeof(int*) * n); // main matrix
	ARGS* params = (ARGS*) malloc (sizeof(ARGS) * t); // parameters for thread function
	pthread_t* tid = (pthread_t*) malloc (sizeof(pthread_t) * t); // array for thread id
	r = (double*) malloc (sizeof(double) * n); // vector r

	if(matrix == NULL || params == NULL || tid == NULL || r == NULL){
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
	
	// FOR CHECKING: prints original matrix	and vector y
	/*
	for(int i=0; i<n; i++){
	 	for(int j=0; j<n; j++){
	 		printf("%d\t", matrix[i][j]);
	 	}
	 	printf("\n");
	}
	
	printf("\n");

	for(int j=0; j<n; j++){
 		printf("%d\t", y[j]);
	}

	printf("\n");
	*/
	
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
		params[i].start = start; // starting column
		params[i].end = start+bound; // ending column
		params[i].tid = i;
		
		start += bound;
	}
		
 	gettimeofday(&begin, NULL);

	// creation of threads
	if(wise == 0){
		for(int i=0; i<t; i++){
			pthread_create(&tid[i], NULL, pearson_corr, (void *)&params[i]);
		}
	} else{
		for(int i=0; i<t; i++){
			pthread_create(&tid[i], NULL, row_pearson_corr, (void *)&params[i]);
		}
	}
	

	// thread joining
	for(int i=0; i<t; i++){
		pthread_join(tid[i], NULL);
	}

	gettimeofday(&stop, NULL);
	
	printf("time elapsed: %f\n", (double)((stop.tv_sec - begin.tv_sec) * 1000000 + stop.tv_usec - begin.tv_usec)/1000000);

	/*
	int count = 0;
	for(int i=0; i<n; i++){
		if(!isnan(r[i]) && 1 >= r[i] && r[i] >= -1 && r[i] != 0){
			count += 1;
		}
	}
	
	printf("count: %d\n", count);
	*/
	
	// memory deallocation
	for(int i=0; i<n; i++){
		free(matrix[i]);
	}
			
	free(y);
	free(matrix);
	free(params);
	pthread_mutex_destroy(&lock); 
	
	return 0;
}
