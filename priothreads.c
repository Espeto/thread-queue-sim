#include "priothreads.h"
#include <pthread.h>

#define MAX_SIZE 8

/* inclua aqui as bibliotecas que precisar */
#include <stdlib.h>
#include <stdio.h>

/* adicione o que for preciso para uma thread ter prioridade e ficar bloqueada esperando sua vez de executar */
typedef struct{
	pthread_t thread_id;
	unsigned int prioridade;
	void *(*funcao) (void *);
	void *parametros;
} pt_thread_ctx;

/* Recomendo declarar tipos e estruturas internas aqui. */

pthread_mutex_t barrier_called_mutex;
int barrier_called;

pt_thread_ctx **on_processor;

pthread_cond_t queue_cond[MAX_SIZE];
pthread_mutex_t threads_per_queue_mutex[MAX_SIZE];
int is_empty[MAX_SIZE];
int threads_per_queue[MAX_SIZE];

unsigned int total_processors;

pthread_mutex_t finished_threads_mutex;
unsigned int finished_threads;

pthread_mutex_t waiting_call_lock;

pthread_cond_t all_processors_inuse;
pthread_mutex_t inuse_processors_mutex;
unsigned int inuse_processors_count;

pthread_mutex_t total_threads_count_mutex;
unsigned int total_threads_count;

//Contabilizar o total de threads finalizadas

pthread_cond_t zero_threads;
pthread_mutex_t is_scheduler_sleeping_mutex;
int is_scheduler_sleeping;

pthread_t scheduler_thread;

/* Reinicializa o escalonador. Descarta todo o estado interno atual (limpa filas) e
   define o número de processadores virtuais entre 1 e 8. Default é 1 processador.
   Caso o usuário defina um número inválido de processadores, use o default.
*/
void pt_init(unsigned int processadores){

	if(processadores > 1 && processadores <= 8) {
		total_processors = processadores;
	}
	else {
		total_processors = 1;
	}

	on_processor = (pt_thread_ctx**) malloc(total_processors * sizeof(pt_thread_ctx*));

	for(int i=0; i<(int)total_processors; i++) {
		on_processor[i] = NULL;
	}

	inuse_processors_count = 0;
	pthread_mutex_init(&inuse_processors_mutex, NULL);
	pthread_cond_init(&all_processors_inuse, NULL);

	finished_threads = 0;
	pthread_mutex_init(&finished_threads_mutex, NULL);

	barrier_called = 0;
	pthread_mutex_init(&barrier_called_mutex, NULL);

	total_threads_count = 0;
	pthread_mutex_init(&total_threads_count_mutex, NULL);

	pthread_cond_init(&zero_threads, NULL);
	pthread_mutex_init(&is_scheduler_sleeping_mutex, NULL);

	pthread_mutex_init(&waiting_call_lock, NULL);

	for (int i = 0; i < MAX_SIZE; i++) {
		pthread_cond_init(&queue_cond[i], NULL);
		is_empty[i] = 1;
		pthread_mutex_init(&threads_per_queue_mutex[i], NULL);
		threads_per_queue[i] = 0;
	}

	pthread_create(&scheduler_thread, NULL, scheduler, NULL);

}

/* Scheduler dos processadores
	é chamada na inicialização
*/
void * scheduler() {
	int status;

	while(1) {

		pthread_mutex_lock(&barrier_called_mutex);
		pthread_mutex_lock(&finished_threads_mutex);
		if(barrier_called && finished_threads == total_threads_count) {

			pthread_mutex_unlock(&finished_threads_mutex);
			pthread_mutex_unlock(&barrier_called_mutex);

			pthread_exit(NULL);
		}

		//se não houver threads em nenhuma fila, chama wait aguardando por uma
		pthread_mutex_lock(&total_threads_count_mutex);
		//evitar spurious wake-up
		while(total_threads_count == 0) {
			pthread_mutex_lock(&is_scheduler_sleeping_mutex);
			is_scheduler_sleeping = 1;
			pthread_mutex_unlock(&is_scheduler_sleeping_mutex);
			status = pthread_cond_wait(&zero_threads, &total_threads_count_mutex);
			//Handle possível erro com a condição
			if(status != 0) {
				printf("Erro na variável de condição\n");
				exit(-1);
			}
		}
		pthread_mutex_unlock(&total_threads_count_mutex);

		//wait na condição em que todos os processadores estão em uso
		pthread_mutex_lock(&inuse_processors_mutex);
		while(inuse_processors_count == total_processors) {
			status = pthread_cond_wait(&all_processors_inuse, &inuse_processors_mutex);

			if(status != 0) {
				printf("Erro na variável de condição\n");
				exit(-1);
			}
		}
		pthread_mutex_unlock(&inuse_processors_mutex);

		for (int i = 0; i < MAX_SIZE; i++) {
			pthread_mutex_lock(&threads_per_queue_mutex[i]);
			if(is_empty[i]) {
					pthread_mutex_unlock(&threads_per_queue_mutex[i]);
					continue;
			}
			//Tem coisa pra ajeitar na lógica desse if
			if(is_high_priority_thread(i)) {
				pthread_cond_signal(&queue_cond[i]);
				threads_per_queue[i]--;
				if(threads_per_queue[i] == 0) is_empty[i] = 1;
				pthread_mutex_unlock(&threads_per_queue_mutex[i]);
				break;
			}
			pthread_mutex_unlock(&threads_per_queue_mutex[i]);
		}
	}
}

/* Checa se a thread atual é a com maior prioridade
*/
int is_high_priority_thread(int atual) {
	for (int i = 0; i < atual; i++) {
		pthread_mutex_lock(&threads_per_queue_mutex[i]);
		if(!is_empty[i]) {
			pthread_mutex_unlock(&threads_per_queue_mutex[i]);
			return 0;
		}
		pthread_mutex_unlock(&threads_per_queue_mutex[i]);
	}

	return 1;
}

/* Cria uma nova thread com a prioridade entre 1 e 8. Default é 8. Valores menores
   definem maior prioridade. Caso o usuário defina uma prioridade inválida, use o default.
*/
void pt_spawn(unsigned int prioridade, void *(*funcao) (void *), void *parametros){
	pt_thread_ctx *thread_ctx;

	/* crie a thread e coloque ela na fila correta */
	thread_ctx = (pt_thread_ctx*) malloc(sizeof(pt_thread_ctx));
	thread_ctx->funcao = funcao;
	thread_ctx->parametros = parametros;

	if(prioridade >= 1 && prioridade <= 8) {
		thread_ctx->prioridade = prioridade;
	}
	else thread_ctx->prioridade = 8;

	pthread_create(&thread_ctx->thread_id, NULL, worker_thread, (void *) thread_ctx);
}

/*
	Função de execução da thread
*/
void * worker_thread(void *arg) {
	int i;
	pt_thread_ctx *thread_ctx = (pt_thread_ctx*) arg;

	pthread_mutex_lock(&total_threads_count_mutex);
	total_threads_count++;
	pthread_mutex_lock(&is_scheduler_sleeping_mutex);
	if(is_scheduler_sleeping) {
		pthread_cond_signal(&zero_threads);
		is_scheduler_sleeping = 0;
	}
	pthread_mutex_unlock(&is_scheduler_sleeping_mutex);
	pthread_mutex_unlock(&total_threads_count_mutex);

	pthread_mutex_lock(&threads_per_queue_mutex[thread_ctx->prioridade]);
	threads_per_queue[(int)thread_ctx->prioridade]++;
	if(is_empty[(int)thread_ctx->prioridade]) is_empty[(int)thread_ctx->prioridade] = 0;
	pthread_mutex_unlock(&threads_per_queue_mutex[(int)thread_ctx->prioridade]);

	pthread_mutex_lock(&waiting_call_lock);
	while(1) {
		pthread_cond_wait(&queue_cond[(int)thread_ctx->prioridade], &waiting_call_lock);
		break;
	}
	pthread_mutex_unlock(&waiting_call_lock);

	pthread_mutex_lock(&threads_per_queue_mutex[thread_ctx->prioridade]);
	threads_per_queue[(int)thread_ctx->prioridade]--;
	if(threads_per_queue[(int)thread_ctx->prioridade] == 0) is_empty[(int)thread_ctx->prioridade] = 1;

	pthread_mutex_lock(&inuse_processors_mutex);
	pthread_mutex_unlock(&threads_per_queue_mutex[(int)thread_ctx->prioridade]);

	inuse_processors_count++;

	for (i = 0; i < (int)total_processors; i++) {
		if (on_processor[i] == NULL) {
			on_processor[i] = thread_ctx;
			break;
		}
	}
	pthread_mutex_unlock(&inuse_processors_mutex);

	(*(thread_ctx->funcao)) (thread_ctx->parametros);

	pthread_mutex_lock(&inuse_processors_mutex);
	on_processor[i] = NULL;
	if(inuse_processors_count == total_processors) {
		pthread_cond_signal(&all_processors_inuse);
	}
	inuse_processors_count--;

	pthread_mutex_unlock(&inuse_processors_mutex);

	pthread_mutex_lock(&finished_threads_mutex);
	finished_threads++;
	pthread_mutex_unlock(&finished_threads_mutex);

	free(thread_ctx);

	pthread_exit(NULL);
}

/* Faz a thread atual liberar o processador, voltar ao fim da fila de sua prioridade e
   esperar o próximo escalonamento
*/
void pt_yield() {
	pthread_t self_id = pthread_self();
	int i = 0;
	pt_thread_ctx *thread;

	pthread_mutex_lock(&inuse_processors_mutex);

	for(int i=0; i < (int) total_processors; i++) {
		if(on_processor[i] != NULL && pthread_equal(self_id, on_processor[i]->thread_id) != 0) {
			thread = on_processor[i];
			on_processor[i] = NULL;
			break;
		}
	}

	if(inuse_processors_count == total_processors) {
		pthread_cond_signal(&all_processors_inuse);
	}
	inuse_processors_count--;

	pthread_mutex_lock(&threads_per_queue_mutex[(int)thread->prioridade]);
	pthread_mutex_unlock(&inuse_processors_mutex);

	threads_per_queue[(int)thread->prioridade]++;
	if(is_empty[(int)thread->prioridade]) is_empty[(int)thread->prioridade] = 0;
	pthread_mutex_unlock(&threads_per_queue_mutex[(int)thread->prioridade]);

	pthread_mutex_lock(&waiting_call_lock);
	while(1) {
		pthread_cond_wait(&queue_cond[(int)thread->prioridade], &waiting_call_lock);
		break;
	}
	pthread_mutex_unlock(&waiting_call_lock);

	pthread_mutex_lock(&threads_per_queue_mutex[(int)thread->prioridade]);
	threads_per_queue[(int)thread->prioridade]--;
	if(threads_per_queue[(int)thread->prioridade] == 0) {
		is_empty[(int)thread->prioridade] = 1;
	}
	pthread_mutex_lock(&inuse_processors_mutex);
	pthread_mutex_unlock(&threads_per_queue_mutex[(int)thread->prioridade]);

	for ( i = 0; i < (int)total_processors; i++) {
		if(on_processor[i] == NULL) {
			on_processor[i] = thread;
			break;
		}
	}

	inuse_processors_count++;
	pthread_mutex_unlock(&inuse_processors_mutex);
}

/* Espera todas as threads terminarem */
void pt_barrier() {
	pthread_mutex_lock(&barrier_called_mutex);
	barrier_called = 1;
	pthread_mutex_unlock(&barrier_called_mutex);

	pthread_join(scheduler_thread, NULL);

}

/* Libera todas as estruturas de dados do escalonador */
void pt_destroy(){
	/* destrua as threads que estão esperando nas filas... */
	pthread_mutex_destroy(&total_threads_count_mutex);
	pthread_mutex_destroy(&inuse_processors_mutex);
	pthread_mutex_destroy(&waiting_call_lock);
	pthread_mutex_destroy(&is_scheduler_sleeping_mutex);
	pthread_mutex_destroy(&finished_threads_mutex);
	pthread_mutex_destroy(&barrier_called_mutex);

	pthread_cond_destroy(&all_processors_inuse);
	pthread_cond_destroy(&zero_threads);

	for (int i = 0; i < MAX_SIZE; i++) {
		pthread_mutex_destroy(&threads_per_queue_mutex[i]);
		pthread_cond_destroy(&queue_cond[i]);
	}

	/* libere memória da heap */
	free(on_processor);
}
