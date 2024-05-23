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


typedef struct {
	unsigned int seed;
	int id;
	struct timespec start_time;
} Thread_args;

void* manage_order(void* args) {
	
	Thread_args* arg = (Thread_args*)args;
	
	int id = arg->id;
	int seed = arg->seed;
	struct timespec start_time;
	clock_gettime(CLOCK_REALTIME, &start_time);;
	free(arg);
	
	int pitses = rand_r(&seed) % (Norderhigh - Norderlow + 1 ) + Norderlow;


	//telephone
	pthread_mutex_lock(&mutexTel);
	while (av_tel == 0) {
		pthread_cond_wait(&condTel, &mutexTel);
	}
	av_tel--;
	//printf("tilefona\n");
	pthread_mutex_unlock(&mutexTel);
	
	//number of pizzas;
	int special =0;
	int marg = 0;
	int pep = 0;
	
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
		
		pthread_mutex_lock(&mutexMess);
		printf("Order number %d failed\n", id);
		pthread_mutex_unlock(&mutexMess);
		
		pthread_mutex_lock(&mutexTel);
		av_tel++;
		pthread_cond_signal(&condTel);
		pthread_mutex_unlock(&mutexTel);
		
		pthread_mutex_lock(&mutexRev);
		failed++;
		pthread_mutex_unlock(&mutexRev);
		
		
		return NULL;
	}
	
	//calculate revenue
	int order_revenue = marg*Cm + pep*Cp + special*Cs;
	pthread_mutex_lock(&mutexRev);
	total_rev += order_revenue;
	sales_marg += marg;
	sales_special += special;
	sales_pepp += pep;
	success++;
	pthread_mutex_unlock(&mutexRev);
	
	pthread_mutex_lock(&mutexMess);
	//printf("Order with number %d succeeeded: %d margherita, %d pepperoni, %d special\n", id, marg, pep, special);
	pthread_mutex_unlock(&mutexMess);
	
	pthread_mutex_lock(&mutexTel);
	av_tel++;
	//printf("mutexTEL\n");
	pthread_cond_signal(&condTel);
	pthread_mutex_unlock(&mutexTel);
	
	//cooking
	
	pthread_mutex_lock(&mutexCook);	
	while (av_cook == 0) {
		pthread_cond_wait(&condCook, &mutexCook);
	}
	av_cook--;
	pthread_mutex_unlock(&mutexCook);
	
	
	sleep(pitses*Tprep);
	
	pthread_mutex_lock(&mutexMess);
	//printf("Order with number %d prepared\n", id);
	pthread_mutex_unlock(&mutexMess);
	
	pthread_mutex_lock(&mutexOven);
	
	while (av_oven < pitses) {
		pthread_cond_wait(&condOven, &mutexOven);
	}
	av_oven -= pitses;
	pthread_mutex_unlock(&mutexOven);
	
	pthread_mutex_lock(&mutexCook);
	av_cook++;	
	pthread_cond_signal(&condCook);
	pthread_mutex_unlock(&mutexCook);
	
	sleep(Tbake);
	
	pthread_mutex_lock(&mutexMess);
	//printf("Order with number %d baked\n", id);
	pthread_mutex_unlock(&mutexMess);
	//printf("meta to sleep 1\n");
	
	
	
	//deliver
	
	pthread_mutex_lock(&mutexDel);
	while(av_del==0) {
		pthread_cond_wait(&condDel, &mutexDel);
	}
	av_del--;
	pthread_mutex_unlock(&mutexDel);
	
	pthread_mutex_lock(&mutexOven);
	av_oven += pitses;
	//printf("mesa sto mutex oven\n");
	pthread_cond_signal(&condOven);
	pthread_mutex_unlock(&mutexOven);
	
	sleep(pitses*Tpack);
	
	pthread_mutex_lock(&mutexMess);
	//printf("Order with number %d packed\n", id);
	pthread_mutex_unlock(&mutexMess);
	
	
	struct timespec pack_time;
	clock_gettime(CLOCK_REALTIME, &pack_time);
	long elapsed_pack = pack_time.tv_sec - start_time.tv_sec;
	//printf("meta tin proti ora\n");
	
	int delivery_time = rand_r(&seed) % (Tdelhigh - Tdellow + 1) + Tdellow;
	
	sleep(delivery_time);
	
	pthread_mutex_lock(&mutexMess);
	//printf("Order with number %d delivered\n", id);
	pthread_mutex_unlock(&mutexMess);
	
	
	pthread_mutex_lock(&mutexDel);
	av_del++;
	pthread_cond_broadcast(&condDel);
	pthread_mutex_unlock(&mutexDel);
	
	struct timespec end_time;
	clock_gettime(CLOCK_REALTIME, &end_time);
	long elapsed_total = end_time.tv_sec - start_time.tv_sec;
	
	pthread_mutex_lock(&mutexMess);
	printf("order No %d prepared in %ld minutes.\n", id, elapsed_pack);
	printf("order No %d delivered in %ld minutes.\n", id, elapsed_total);
	pthread_mutex_unlock(&mutexMess);
	
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
	
	pthread_mutex_init(&mutexTel, NULL);
	pthread_mutex_init(&mutexMess, NULL);
	pthread_mutex_init(&mutexRev, NULL);
	pthread_mutex_init(&mutexCook, NULL);
	pthread_mutex_init(&mutexOven, NULL);
	pthread_mutex_init(&mutexDel, NULL);
	
	pthread_cond_init(&condTel, NULL);
	pthread_cond_init(&condCook, NULL);
	pthread_cond_init(&condOven, NULL);
	pthread_cond_init(&condDel, NULL);
	///////////////////////
	struct timespec start_time;
	clock_gettime(CLOCK_REALTIME, &start_time);
	
	for (int i = 0; i < Ncust; i++) {
		Thread_args* args = (Thread_args*)malloc(sizeof(Thread_args));
		args->id = i+1;
		args->seed = seed +i;
		args->start_time = start_time;
		pthread_create(&threads[i], NULL, manage_order, args);
		sleep(rand_r(&seed) % (Torderhigh + Torderlow));
	}
	
	for (int i = 0; i < Ncust; i++){
		pthread_join(threads[i], NULL);
		
	}
		
	printf("Total revenue is %d\n", total_rev);
	printf("Total sales - M: %d, P: %d, S: %d\n", sales_marg, sales_pepp, sales_special);
	printf("succes: %d, failed: %d\n", success, failed);
	
	
	pthread_mutex_destroy(&mutexTel);
	pthread_mutex_destroy(&mutexMess);
	pthread_mutex_destroy(&mutexRev);
	pthread_mutex_destroy(&mutexCook);
	pthread_mutex_destroy(&mutexOven);
	pthread_mutex_destroy(&mutexDel);
		
	pthread_cond_destroy(&condTel);
	pthread_cond_destroy(&condCook);
	pthread_cond_destroy(&condOven);
	pthread_cond_destroy(&condDel);
	
	return 0;
	
}

	
	
	
	
	
	
	
