#include "priothreads.h"
#include <pthread.h>

/* inclua aqui as bibliotecas que precisar */
#include <stdlib.h>
#include <stdio.h>

/* adicione o que for preciso para uma thread ter prioridade e ficar bloqueada esperando sua vez de executar */
typedef struct {
	long thread_id;
	unsigned int prioridade;
	int yeld;
	void *(*funcao) (void *);
	void *parametros;

} pt_thread_ctx;

/* Recomendo declarar tipos e estruturas internas aqui. */
#define MAX_POS 8

typedef struct {
} pt_thread_processor;

pthread_cond_t queue_cond[MAX_POS];
int is_empty[MAX_POS];
pthread_mutex_t threads_per_queue_mutex[MAX_POS];
int threads_per_queue[MAX_POS];

unsigned int total_processors;

pthread_mutex_t inuse_processors_mutex;
pthread_cond_t inuse_processors_cond;
unsigned int inuse_processors_count;

pthread_cond_t zero_threads;
pthread_mutex_t total_threads_count_mutex;
unsigned int total_threads_count;

pthread_mutex_t is_scheduler_sleeping_mutex;
int is_scheduler_sleeping;

pthread_t scheduler_thread;

/* Reinicializa o escalonador. Descarta todo o estado interno atual (limpa filas) e
   define o número de processadores virtuais entre 1 e 8. Default é 1 processador.
   Caso o usuário defina um número inválido de processadores, use o default.
*/
void pt_init(unsigned int processadores){

	pthread_mutex_destroy(&total_threads_count_mutex);
	pthread_mutex_destroy(&inuse_processors_mutex);
	pthread_cond_destroy(&inuse_processors_cond);

	for (size_t i = 0; i < MAX_POS; i++) {
		pthread_mutex_destroy(threads_per_queue_mutex[i]);
		pthread_cond_destroy(queue_cond[i]);
	}



	if(processadores > 1 && processadores <= 8) {
		total_processors = processadores;
	}
	else {
		total_processors = 1;
	}

	inuse_processors_count = 0;
	pthread_mutex_init(&inuse_processors_mutex, NULL);
	pthread_cond_init(&inuse_processors_cond, NULL);

	total_threads_count = 0;
	pthread_mutex_init(&total_threads_count_mutex, NULL);
	pthread_cond_init(&zero_threads, NULL);

	for (size_t i = 0; i < MAX_POS; i++) {
		pthread_cond_init(queue_cond[i], NULL);
		is_empty[i] = 1;
		pthread_mutex_init(threads_per_queue_mutex[i], NULL);
		threads_per_queue[i] = 0;
	}

	pthread_create(&scheduler_thread, NULL, scheduler, NULL);

}

/*
	Scheduler dos processadores
	é chamada na inicialização
*/
void *scheduler() {
	int status;

	while(true) {

		if (pt_barrier) {
			/* espera as threads terminarem */
		}

		if (pt_destroy) {
			//limpa tudo
			return;
		}

		pthread_mutex_lock(&total_threads_count)
		while(total_threads_count == 0) {
			status = pthrad_cond_wait(&zero_threads, &total_threads_count);
			if(status != 0) {
				break;
			}
		}




	}
}

int exists_high_prority() {

}

/* Cria uma nova thread com a prioridade entre 1 e 8. Default é 8. Valores menores
   definem maior prioridade. Caso o usuário defina uma prioridade inválida, use o default.
*/
void pt_spawn(unsigned int prioridade, void *(*funcao) (void *), void *parametros){
	pthread_t thread;
	pt_thread_ctx *thread_ctx;

	/* crie a thread e coloque ela na fila correta */
	thread_ctx = (pt_thread_ctx*) malloc(sizeof(pt_thread_ctx));
	thread_ctx->funcao = funcao;
	thread_ctx->parametros = parametros;

	if(prioridade >= 1 && prioridade <= 8) {
		thread_ctx->prioridade = prioridade;
	}
	else thread_ctx->prioridade = 8;

	pthread_create(&thread, NULL, worker_thread, (void *) thread_ctx);
}

/*
	Função de execução da thread
*/
void *worker_thread(void *arg) {

}

/* Faz a thread atual liberar o processador, voltar ao fim da fila de sua prioridade e
   esperar o próximo escalonamento
*/
void pt_yield(){

}

/* Espera todas as threads terminarem */
void pt_barrier(){

}

/* Libera todas as estruturas de dados do escalonador */
void pt_destroy(){
	/* destrua as threads que estão esperando nas filas... */


	/* libere memória da heap */

}
