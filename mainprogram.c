#include "declare.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>



//declare
pthread_mutex_t mutexTel, mutexRev, mutexMess, mutexCook, mutexOven, mutexDel;
pthread_cond_t condTel, condCook, condOven, condDel;

int av_tel = Ntel;
int av_cook = Ncook;
int av_oven = Noven;
int av_del = Ndeliverer;
int total_rev = 0;
int sales_marg = 0, sales_pepp = 0, sales_special = 0;
int success = 0;
int failed =0;
 
// output time 
long total_service_time =0 ;
long max_service_time =0 ;
long total_cooling_time = 0;
long max_cooling_time  = 0;

int pelates;
typedef struct {
	unsigned int seed;
	int id;
	struct timespec start_time;
} Thread_args;

void* manage_order(void* args) {
	pelates++;
	Thread_args* arg = (Thread_args*)args;
	int rc;
	
	int id = arg->id;
	int seed = arg->seed;
	struct timespec start_time = arg->start_time;
	struct timespec pack_time, end_del_time;
	clock_gettime(CLOCK_REALTIME, &start_time);;
	free(arg);
	
	int pitses = rand_r(&seed) % (Norderhigh - Norderlow + 1 ) + Norderlow;


	//telephone
	rc = pthread_mutex_lock(&mutexTel);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(args);
	}		


	while (av_tel == 0) {
		rc = pthread_cond_wait(&condTel, &mutexTel);
		if (rc != 0) {	
			printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
			pthread_exit(args);
		}
	}
	av_tel--;
	rc = pthread_mutex_unlock(&mutexTel);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(args);
	}
	
	//number of pizzas;
	int special =0 , marg = 0 , pep = 0;
	
	for (int i =0; i < pitses; i++) {
		int prob = rand_r(&seed) % 100;
		if (prob < Pm) marg++;
		else if (prob < Pm + Pp) pep++;
		else special++;
	}
		
	//payment
	int pay_time = rand_r(&seed) % (Tpaymenthigh - Tpaymentlow + 1) + Tpaymentlow;
	
	sleep(pay_time);
	
	int is_failure = rand_r(&seed) % 100;
	if (is_failure < Pfail) {
		
		rc = pthread_mutex_lock(&mutexMess);
		if (rc != 0) {	
			printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
			pthread_exit(args);
		}
		printf("Order number %d failed\n", id);
		rc = pthread_mutex_unlock(&mutexMess);
		if (rc != 0) {	
			printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
			pthread_exit(args);
		}

		
		rc = pthread_mutex_lock(&mutexTel);
		if (rc != 0) {	
			printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
			pthread_exit(args);
		}	
		av_tel++;
		rc = pthread_cond_signal(&condTel);
		if (rc != 0) {	
					printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
					pthread_exit(args);
				}

		rc = pthread_mutex_unlock(&mutexTel);
		if (rc != 0) {	
			printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
			pthread_exit(args);
		}
		
		rc = pthread_mutex_lock(&mutexRev);
		if (rc != 0) {	
			printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
			pthread_exit(args);
		}	
		failed++;
		rc = pthread_mutex_unlock(&mutexRev);
		if (rc != 0) {	
			printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
			pthread_exit(args);
		}
		
		return NULL;
	}
	
	//calculate revenue
	int order_revenue = marg*Cm + pep*Cp + special*Cs;
	rc = pthread_mutex_lock(&mutexRev);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(args);
	}	
	total_rev += order_revenue;
	sales_marg += marg;
	sales_special += special;
	sales_pepp += pep;
	success++;
	rc = pthread_mutex_unlock(&mutexRev);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(args);
	}
	
	rc = pthread_mutex_lock(&mutexMess);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(args);
	}	
	printf("Order No %d succeeeded \n", id);
	rc = pthread_mutex_unlock(&mutexMess);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(args);
	}
	
	rc = pthread_mutex_lock(&mutexTel);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(args);
	}	
	av_tel++;
	rc = pthread_cond_signal(&condTel);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
		pthread_exit(args);
	}
	rc = pthread_mutex_unlock(&mutexTel);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(args);
	}
	
	//cooking
	rc = pthread_mutex_lock(&mutexCook);	
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(args);
	}
	while (av_cook == 0) {
		pthread_cond_wait(&condCook, &mutexCook);
	}
	av_cook--;
	rc = pthread_mutex_unlock(&mutexCook);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(args);
	}
	
	sleep(pitses*Tprep);
	
	rc = pthread_mutex_lock(&mutexMess);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(args);
	}	
	rc = pthread_mutex_unlock(&mutexMess);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(args);
	}
	
	rc = pthread_mutex_lock(&mutexOven);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(args);
	}	
	while (av_oven < pitses) {
		rc = pthread_cond_wait(&condOven, &mutexOven);
		if (rc != 0) {	
			printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
			pthread_exit(args);
		}
	}
	av_oven -= pitses;
	rc = pthread_mutex_unlock(&mutexOven);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(args);
	}
	rc = pthread_mutex_lock(&mutexCook);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(args);
	}	
	av_cook++;	
	rc = pthread_cond_signal(&condCook);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
		pthread_exit(args);
	}
	rc = pthread_mutex_unlock(&mutexCook);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(args);
	}

	sleep(Tbake);
	
	rc = pthread_mutex_lock(&mutexMess);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(args);
	}	
	
	rc = pthread_mutex_unlock(&mutexMess);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(args);
	}

	
	
	//deliver
	rc = pthread_mutex_lock(&mutexDel);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(args);
	}	
	while(av_del==0) {
		rc = pthread_cond_wait(&condDel, &mutexDel);
		if (rc != 0) {	
			printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
			pthread_exit(args);
		}
	}
	av_del--;
	rc = pthread_mutex_unlock(&mutexDel);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(args);
	}
	
	rc = pthread_mutex_lock(&mutexOven);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(args);
	}	
	av_oven += pitses;
	rc = pthread_cond_signal(&condOven);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
		pthread_exit(args);
	}
	rc = pthread_mutex_unlock(&mutexOven);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(args);
	}
	
	sleep(pitses*Tpack);

	
    clock_gettime(CLOCK_REALTIME, &pack_time);
    long elapsed_pack = (pack_time.tv_sec - start_time.tv_sec) ;


	rc = pthread_mutex_lock(&mutexMess);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(args);
	}	
	printf("Order No %d prepared in %ld minutes\n", id, elapsed_pack);
	rc = pthread_mutex_unlock(&mutexMess);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(args);
	}
	
	
	int delivery_time = rand_r(&seed) % (Tdelhigh - Tdellow + 1) + Tdellow;
	sleep(delivery_time);
	

	struct timespec end_time;
	clock_gettime(CLOCK_REALTIME, &end_del_time);
    long elapsed_total = end_del_time.tv_sec - start_time.tv_sec;
    long cooling_time = end_del_time.tv_sec - pack_time.tv_sec;
	

	rc = pthread_mutex_lock(&mutexMess);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(args);
	}	
	printf("Order No %d delivered in %ld minutes\n", id, elapsed_total);
	rc = pthread_mutex_unlock(&mutexMess);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(args);
	}
	
	
	rc = pthread_mutex_lock(&mutexDel);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(args);
	}	
	av_del++;
	pthread_cond_broadcast(&condDel);
	rc = pthread_mutex_unlock(&mutexDel);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(args);
	}


	rc = pthread_mutex_lock(&mutexRev);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(args);
	}	
    total_service_time += elapsed_total;
    if (elapsed_total > max_service_time) {
        max_service_time = elapsed_total;
    }

    total_cooling_time += cooling_time;
    if (cooling_time > max_cooling_time) {
        max_cooling_time = cooling_time;
    }
    rc = pthread_mutex_unlock(&mutexRev);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(args);
	}
	
	
	pthread_exit(NULL);
}

int main(int argc, char* argv[]) {

	if (argc != 3) {
		printf("Usage: %s <number_of_customers> <seed>\n", argv[0]);
		return -1;
	}
	
	int Ncust = atoi(argv[1]);
	int seed = atoi(argv[2]);
	pthread_t threads[Ncust];
	int rc;
	
	rc = pthread_mutex_init(&mutexTel, NULL);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
		exit(-1);
	}
	rc = pthread_mutex_init(&mutexMess, NULL);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
		exit(-1);
	}
	rc = pthread_mutex_init(&mutexRev, NULL);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
		exit(-1);
	}
	rc = pthread_mutex_init(&mutexCook, NULL);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
		exit(-1);
	}
	rc = pthread_mutex_init(&mutexOven, NULL);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
		exit(-1);
	}
	rc = pthread_mutex_init(&mutexDel, NULL);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
		exit(-1);
	}

	
	rc = pthread_cond_init(&condTel, NULL);
	if (rc != 0) {
		printf("ERROR: return code from pthread_cond_init() is %d\n", rc);
		exit(-1);
	}
	rc = pthread_cond_init(&condCook, NULL);
	if (rc != 0) {
		printf("ERROR: return code from pthread_cond_init() is %d\n", rc);
		exit(-1);
	}
	rc = pthread_cond_init(&condOven, NULL);
	if (rc != 0) {
		printf("ERROR: return code from pthread_cond_init() is %d\n", rc);
		exit(-1);
	}
	rc = pthread_cond_init(&condDel, NULL);
	if (rc != 0) {
		printf("ERROR: return code from pthread_cond_init() is %d\n", rc);
		exit(-1);
	}
	///////////////////////
	struct timespec start_time;
	
	
	for (int i = 0; i < Ncust; i++) {
		clock_gettime(CLOCK_REALTIME, &start_time);
		Thread_args* args = (Thread_args*)malloc(sizeof(Thread_args));
		args->id = i+1;
		args->seed = seed +i;
		args->start_time = start_time;
		rc = pthread_create(&threads[i], NULL, manage_order, args);
		if (rc != 0) {
    		printf("ERROR: return code from pthread_create() is %d\n", rc);
       		exit(-1);
		}
		sleep(rand_r(&seed) % (Torderhigh + Torderlow));
	}
	
	for (int i = 0; i < Ncust; i++){
		rc = pthread_join(threads[i], NULL);
		if (rc != 0) {
			printf("ERROR: return code from pthread_join() is %d\n", rc);
			exit(-1);		
		}
		
	}
		
	printf("Total revenue: %d\n", total_rev);
    printf("Total margherita: %d\n", sales_marg);
    printf("Total pepperoni: %d\n", sales_pepp);
    printf("Total special: %d\n", sales_special);
    printf("Total successful orders: %d\n", success);
    printf("Total failed orders: %d\n", failed);

	 if (success > 0) {
        printf("Average service time: %ld minutes\n", total_service_time / success);
        printf("Max service time: %ld minutes\n", max_service_time);
        printf("Average cooling time: %ld minutes\n", (total_cooling_time) / success);
        printf("Max cooling time: %ld minutes\n", (max_cooling_time));
    }
	
	rc = pthread_mutex_destroy(&mutexTel);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}

	rc = pthread_mutex_destroy(&mutexMess);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}

	rc = pthread_mutex_destroy(&mutexRev);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}

	rc = pthread_mutex_destroy(&mutexCook);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}

	rc = pthread_mutex_destroy(&mutexOven);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}

	rc = pthread_mutex_destroy(&mutexDel);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}

		
	rc = pthread_cond_destroy(&condTel);
	if (rc != 0) {
		printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_cond_destroy(&condCook);
	if (rc != 0) {
		printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_cond_destroy(&condOven);
	if (rc != 0) {
		printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_cond_destroy(&condDel);
	if (rc != 0) {
		printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
		exit(-1);		
	}

	
	
	return 0;
}

	
	
	
	
	
	
	
