#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include "rtclock.h"
#include "mmm.h"

int SIZE = 0;
double p_stopwatch;
double s_stopwatch;


int main(int argc, char *argv[]) {
	double clockstart, clockend;
	clockstart = 0; 
	clockend = 0;
	
	// Sequential run...
	if(argc == 1 || strcmp(argv[1], "S") == 0){
		if(argc == 3){
			SIZE = atoi(argv[2]);
			printf("========\nmode: Sequential\nThread Count: 1\nSize: %d\n========", SIZE);

			mmm_init();
			mmm_seq();
			mmm_reset(matrix_A);
			mmm_reset(matrix_B);
			mmm_reset(output);
			mmm_reset(output_two);

			for(int i=0; i<3; i++){
				clockstart += rtclock();
				mmm_init();
				mmm_seq();
				mmm_reset(matrix_A);
				mmm_reset(matrix_B);
				mmm_reset(output);
				clockend += rtclock();
				mmm_reset(output_two);
			}
			
		}else{
			printf("Usage: ./mmm <mode> <size>\n\n\n");
			exit(1);
		}
		printf("\nSequential Time (avg of 3 runs): %.6f sec\n", ((clockend - clockstart)/3));
	}
	
	// Parallel run...
	int thread_num = 0;


	if(strcmp(argv[1], "P") == 0){
		if(argc == 4){
			thread_num = atoi(argv[2]);
			SIZE = atoi(argv[3]);
		

			thread_args *params = (thread_args*) malloc(sizeof(thread_args) * thread_num);
			

			for(int i=0; i<thread_num; i++){
				params[i].tid = i;
				params[i].begin = i * SIZE/thread_num + 1;
				params[i].end = (i + 1) * SIZE / thread_num;
			}
			pthread_t *threads = (pthread_t*) malloc(sizeof(pthread_t) * thread_num);
			mmm_init();
			
			
			for (int i = 0; i < thread_num; i++) {
    			pthread_create(&threads[i], NULL, mmm_par, &params[i]);
  			}
			for(int i = 0; i < thread_num; i++){
				pthread_join(threads[i], NULL);
			}

			mmm_seq();

			mmm_reset(matrix_A);
			mmm_reset(matrix_B);
			mmm_reset(output);
			mmm_reset(output_two);
			
			clockstart = 0;
			clockend = 0;

			double diff = 0;
			
			for(int i=0; i<3; i++){
				
				clockstart += rtclock();
				for (int i = 0; i < thread_num; i++) {
    			pthread_create(&threads[i], NULL, mmm_par, &params[i]);
  			}
			for(int i = 0; i < thread_num; i++){
				pthread_join(threads[i], NULL);
			}
			clockend += rtclock();
			p_stopwatch += (clockend - clockstart);

			clockstart = 0;
			clockend = 0;

			clockstart += rtclock();
			mmm_seq();
			clockend += rtclock();
			s_stopwatch += (clockend - clockstart);

			diff += mmm_verify();
			
			mmm_reset(matrix_A);
			mmm_reset(matrix_B);
			mmm_reset(output);
			mmm_reset(output_two);

			clockstart = 0;
			clockend = 0;
			}

			free(params);
			free(threads);
			params = NULL;
			threads = NULL;

			printf("========\nmode: Parallel\nThread Count: %d\nSize: %d\n========", thread_num, SIZE);
			printf("\nSequential Time (avg of 3 runs): %.6f sec\n", ((s_stopwatch)/3));
			printf("Parallel Time (avg of 3 runs): %.6f sec\n", ((p_stopwatch)/3));
			printf("Speedup: %.6f sec\n", ((s_stopwatch/3)/(p_stopwatch/3)));
			printf("Verifying... largest error between Parallel and Sequential matrix is: %.6f\n", diff);


		} else{
			printf("Usage: ./mmm <mode> [num threads] <size>\n\n\n");
			exit(1);
		}
	}

	mmm_freeup();

	return 0;
}
