/*
Author: Angelo John C. Cruz
Section: CMSC 180 T-6L
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/time.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>
#include <math.h>
#include <unistd.h> 
#include <pthread.h>
#include <sched.h> 
#define SA struct sockaddr

pthread_mutex_t lock;
double* r;

typedef struct threadargs {
	int** matrix;
	int n;
	int tid;
	int start;
	int end;
}ARGS;

typedef struct clientargs {
	int** matrix;
	int n;
	int tid;
	int start;
	int end;
	int connfd;
} CLIENTARGS;

typedef struct params {
	char* ip;
	int count;
	int port;
} PARAMS;

typedef struct serverargs {
	int sockfd;
	struct sockaddr_in servaddr;	
} SERVERARGS; 

struct timeval begin;

void* handle_acknowledgements(void* args) {
	  int sockfd = *((int*)args);
	  char ack[4];
	  
	  printf("%d acknowledged\n", sockfd);
	  read(sockfd, ack, sizeof(ack)); 
	  close(sockfd);
	  pthread_exit(NULL);
}

void* handle_writes(void* args) {
	CLIENTARGS* temp = (CLIENTARGS*) args;
	
	int tid = temp->tid;
	int n = temp->n; 
	int start = temp->start;
	int end = temp->end;
	int connfd = temp->connfd;
	int** matrix = temp->matrix;
	
	write(connfd, &start, sizeof(int));
	write(connfd, &end, sizeof(int));
	write(connfd, &n, sizeof(int));
	write(connfd, &tid, sizeof(int));
	
	for(int i=start; i<end; i++) {
		for(int j=0; j<n; j++) {
			int element = matrix[j][i];
			write(connfd, &element, sizeof(int));
		}
	}
		
	close(connfd);
	pthread_exit(NULL);
}

void* server_thread(void* args) {
	SERVERARGS* temp = (SERVERARGS*) args;
	
	int sockfd = temp->sockfd;
	struct sockaddr_in servaddr = temp->servaddr;
	
}

void server(char* IP, int count, int port, ARGS* params, int* ports){
	int sockfd[count], connfd[count], len, clientnum = 0; 
	struct sockaddr_in servaddr[count], cli[count]; 
	pthread_t readThreads[count], writeThreads[count], serverThreads[count];
	CLIENTARGS clientarg[count];
	SERVERARGS serverarg[count];
	//const char* IP = "10.0.2.15";
	char ack[4];
 	
 	for(int i=0; i<count; i++){
 		// socket create and verification 
		sockfd[i] = socket(AF_INET, SOCK_STREAM, 0); 
		if (sockfd[i] == -1) { 
				printf("socket creation failed...\n"); 
				exit(0); 
		} 
		else {
				printf("Socket successfully created..\n"); 
		}
		
		bzero(&serverarg[i].servaddr, sizeof(&serverarg[i].servaddr)); 
	 	
		// assign IP, PORT 
		serverarg[i].servaddr.sin_family = AF_INET; 
		serverarg[i].servaddr.sin_addr.s_addr = inet_addr(IP); 
		serverarg[i].servaddr.sin_port = htons(ports[i]); 
	 	
	 	// Binding newly created socket to given IP and verification 
		if ((bind(sockfd[i], (SA*)&serverarg[i].servaddr, sizeof(serverarg[i].servaddr))) != 0) { 
				perror("socket bind failed...\n"); 
				exit(0); 
		} 
		else
				printf("Socket successfully binded..\n"); 
	 
		// Now server is ready to listen and verification 
		if ((listen(sockfd[i], 1)) != 0) { 
				printf("Listen failed...\n"); 
				exit(0); 
		} 
		else {
				printf("Server listening..\n"); 
		}	
	
	 	//pthread_create(&serverThreads[i], NULL, server_thread, (void*)&serverarg[i]);
 	}
		
	while(clientnum < count){
		len = sizeof(cli[clientnum]); 

		// Accept the data packet from client and verification 
		connfd[clientnum] = accept(sockfd[clientnum], (SA*)&cli, &len); 
		if (connfd < 0) { 
				printf("server accept failed...\n"); 
				exit(0); 
		} 
		else {
				printf("server accept the client... \n");
				clientnum += 1;
		} 
	} 
	
	clientnum = 0;
	printf("all clients accepted...\n");
	gettimeofday(&begin, NULL);

	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	int num_cores = sysconf(_SC_NPROCESSORS_ONLN); // get number of cores
	
	// setting of cpu affinity
	for(int i=0; i<num_cores-1; i++){
		CPU_SET(i, &cpuset);
	}
	
	
	while(clientnum < count) {				
		clientarg[clientnum].tid = clientnum;
		clientarg[clientnum].matrix = params[clientnum].matrix;
		clientarg[clientnum].start = params[clientnum].start;
		clientarg[clientnum].end = params[clientnum].end;
		clientarg[clientnum].n = params[clientnum].n;
		clientarg[clientnum].connfd = connfd[clientnum];
		
		pthread_create(&writeThreads[clientnum], NULL, handle_writes, &clientarg[clientnum]);
		pthread_setaffinity_np(writeThreads[clientnum], sizeof(cpu_set_t), &cpuset);
		
		read(sockfd[clientnum], ack, sizeof(ack));
		clientnum += 1;
		
		//pthread_create(&readThreads[clientnum], NULL, handle_acknowledgements, &connfd[clientnum]);
		//printf("received client %d acknowledgement...\n", clientnum);
	}
	
	for(int i = 0; i < count; i++) {
		//pthread_join(serverThreads[i], NULL);
	  pthread_join(writeThreads[i], NULL);
	}
}

void* client(char* ip, int count, int port){
	int sockfd, connfd, n, start, end, element, tid;
	ARGS submatrix;
	struct sockaddr_in servaddr, cli;
	char ack[4];
	
	/*
	PARAMS* temp = (PARAMS*) args;
	
	char* ip = temp->ip;
	int count = temp->count;
	int port = temp->port;
	*/
	
	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		  printf("socket creation failed...\n");
		  exit(0);
	}
	else
		  printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);
	servaddr.sin_port = htons(port);

	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		  perror("connection with the server failed...\n");
		  exit(0);
	}
	else {
		  printf("connected to the server..\n");
		  
			gettimeofday(&begin, NULL);
		  		
		  read(sockfd, &start, sizeof(int));
		  read(sockfd, &end, sizeof(int));
		  read(sockfd, &n, sizeof(int));
  	  read(sockfd, &tid, sizeof(int));
  	  
		  int** matrix = (int**) malloc (sizeof(int*) * n);
		  if(matrix == NULL){
		  	perror("Memory allocation failed!\n");
		  }

		  for(int i=0; i<n; i++){
				if(end != start) {
					matrix[i] = (int*) malloc (sizeof(int) * (end-start));
				} else{
					matrix[i] = (int*) malloc (sizeof(int) * 1);
				}
				
				if(matrix[i] == NULL){
					perror("Memory allocation failed\n");
				}
			}
		  
		  for(int i=0; i<(end-start); i++) {
				for(int j=0; j<n; j++) {
					read(sockfd, &element, sizeof(int));
					matrix[j][i] = element;
				}
			}
		 
		 	// FOR CHECKING 
		  
		  for(int i=0; i<n; i++) {
				for(int j=0; j<(end-start); j++) {
					printf("%d\t", matrix[i][j]);
				}
				printf("\n");
			}
			
			
		  if(matrix && n && start && end) write(sockfd, ack, sizeof(ack)); 
	}

	// close the socket
	close(sockfd);
}

int main(int argc, char *argv[]){
	int n = atoi(argv[1]), p = atoi(argv[2]), s = atoi(argv[3]), t, randnum, count = 0, bound = 0, start = 0, remainder = 0, end = 0;
	struct timeval stop;
	FILE *fp;
	char* line = NULL;
	char* ip_addr = NULL;
	size_t len = 0;
	ssize_t read;
	
	fp = fopen("cruz_lab04_config.in", "r");
	if(fp == NULL){
		exit(EXIT_FAILURE);
	}
	
	if(getline(&line, &len, fp) != -1) {
		ip_addr = (char*)malloc(strlen(line) + 1);
		
    if(ip_addr == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    
		strncpy(ip_addr, line, strlen(line) - 1);
    ip_addr[strlen(line) - 1] = '\0';
	}
	
	if(getline(&line, &len, fp) != -1) {
		t = atoi(line);
	}
	
	int ports[t]; // initialize ports array
	
	while ((read = getline(&line, &len, fp)) != -1) {
		if(!(read > 0 && line[0] == '\n')) ports[count++] = atoi(line);
	}

	fclose(fp);
	if (line) free(line);
	
	/*
	// asking for matrix size
	printf("enter matrix size: ");
	
	scanf("%d", &n);
	
	// asking for port number
	printf("enter port number: ");
	
	scanf("%d", &p);
	
	// asking for status
	printf("enter status: ");
	
	scanf("%d", &s);
	*/

	if(s == 0) {
		// initialization of arrays
		int** matrix = (int**) malloc (sizeof(int*) * n); // main matrix
		ARGS* params = (ARGS*) malloc (sizeof(ARGS) * t); // parameters for thread function

		if(matrix == NULL || params == NULL){
			printf("Memory allocation failed\n");
			return 0;
		}
		
		// 2d arrays
		for(int i=0; i<n; i++){
			matrix[i] = (int*) malloc (sizeof(int) * n);
			
			if(matrix[i] == NULL){
				printf("Memory allocation failed\n");
				exit(EXIT_FAILURE);
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
			
		// FOR CHECKING: prints original matrix	and vector y
		
		for(int i=0; i<n; i++){
		 	for(int j=0; j<n; j++){
		 		printf("%d\t", matrix[i][j]);
		 	}
		 	printf("\n");
		}
		
		printf("\n");
		
		/*
		for(int j=0; j<n; j++){
	 		printf("%d\t", y[j]);
		}

		printf("\n");
		*/
		
		// computing remainder
		remainder = n % t;
		
		count = 0, bound = 0, start = 0, remainder = 0, end = 0;
		
		// initialization of params
		for(int i=0; i<t; i++){
			if(i < remainder && remainder != 0){
				bound = n/t + 1;
			} else {
				bound = n/t;	
			}
			
			params[i].matrix = matrix;
			params[i].n = n;
			params[i].start = start; // starting column
			params[i].end = start+bound; // ending column
			params[i].tid = i;
			
			start += bound;
		}
				
	 	//gettimeofday(&begin, NULL);
	 	
	 	server(ip_addr, t, p, params, ports); // initialize server
	 	
	 	// memory deallocation
		for(int i=0; i<n; i++){
			free(matrix[i]);
		}
				
		free(matrix);
		free(params); 
		 	
	} else {		
		
		/*
		pthread_t* tid = (pthread_t*) malloc (sizeof(pthread_t) * t); // array for thread id
		PARAMS* clientarg = (PARAMS*) malloc (sizeof(PARAMS) * t);
		
		for(int i=0; i<t; i++){
			clientarg[i].ip = ip_addr;
			clientarg[i].count = t;
			clientarg[i].port = ports[i]; 
		}
		
		for(int i=0; i<t; i++){
			pthread_create(&tid[i], NULL, client, (void *)&clientarg[i]);
		}
		
		for(int i=0; i<t; i++){
			pthread_join(tid[i], NULL);
		}
		*/
		
		client(ip_addr, t, p);			
	}

	gettimeofday(&stop, NULL);
	
	printf("time elapsed: %f\n", (double)((stop.tv_sec - begin.tv_sec) * 1000000 + stop.tv_usec - begin.tv_usec)/1000000);

	return 0;
}