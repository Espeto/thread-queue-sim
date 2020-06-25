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
typedef struct all_threads{
	pthread_t thread_id;
	struct all_threads *next;
} all_threads;

all_threads *all_threads_queue;

pthread_mutex_t barrier_called_lock;
int barrier_called;

pthread_mutex_t on_processor_lock;
pt_thread_ctx **on_processor;

pthread_cond_t queue_cond[MAX_SIZE];
pthread_mutex_t threads_per_queue_lock;
int threads_per_queue[MAX_SIZE];
int can_go[MAX_SIZE];

unsigned int total_processors;

pthread_mutex_t finished_threads_lock;
unsigned int finished_threads;

pthread_cond_t all_processors_inuse;
pthread_mutex_t inuse_processors_lock;
unsigned int inuse_processors_count;

pthread_mutex_t total_threads_count_lock;
unsigned int total_threads_count;

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

	all_threads_queue = NULL;

	on_processor = (pt_thread_ctx**) malloc(total_processors * sizeof(pt_thread_ctx*));
	for(int i = 0; i < (int) total_processors; i++) {
		on_processor[i] = NULL;
	}
	pthread_mutex_init(&on_processor_lock, NULL);

	pthread_mutex_init(&barrier_called_lock, NULL);
	barrier_called = 0;

	inuse_processors_count = 0;
	pthread_mutex_init(&inuse_processors_lock, NULL);
	pthread_cond_init(&all_processors_inuse, NULL);

	finished_threads = 0;
	pthread_mutex_init(&finished_threads_lock, NULL);

	total_threads_count = 0;
	pthread_mutex_init(&total_threads_count_lock, NULL);

	for (int i = 0; i < MAX_SIZE; i++) {
		pthread_cond_init(&queue_cond[i], NULL);
		threads_per_queue[i] = 0;
		can_go[i] = 0;
	}
	pthread_mutex_init(&threads_per_queue_lock, NULL);

	pthread_create(&scheduler_thread, NULL, scheduler, NULL);
}

/* Scheduler dos processadores
	é chamada na inicialização
*/
void * scheduler() {
	while(1) {

		pthread_mutex_lock(&total_threads_count_lock);
		if(total_threads_count == 0) {
			pthread_mutex_unlock(&total_threads_count_lock);
			continue;
		}
		pthread_mutex_unlock(&total_threads_count_lock);

		for (int i = 0; i < MAX_SIZE; i++) {

			//wait na condição em que todos os processadores estão em uso
			pthread_mutex_lock(&inuse_processors_lock);
			while(inuse_processors_count == total_processors) {
				pthread_cond_wait(&all_processors_inuse, &inuse_processors_lock);
			}
			pthread_mutex_unlock(&inuse_processors_lock);

			pthread_mutex_lock(&threads_per_queue_lock);
			if(threads_per_queue[i] == 0) {
					pthread_mutex_unlock(&threads_per_queue_lock);
					continue;
			}
			pthread_mutex_unlock(&threads_per_queue_lock);

			if(is_high_priority_thread(i)) {

				pthread_mutex_lock(&inuse_processors_lock);
				can_go[i] = 1;
				pthread_cond_signal(&queue_cond[i]);
				inuse_processors_count++;
				pthread_mutex_unlock(&inuse_processors_lock);
				//Acorda a thread
			}
			else {
				break;
			}

		}

		//Checa se vai terminar
		pthread_mutex_lock(&finished_threads_lock);
		pthread_mutex_lock(&barrier_called_lock);
		if(barrier_called == 1 && finished_threads == total_threads_count) {

			pthread_mutex_unlock(&barrier_called_lock);
			pthread_mutex_unlock(&finished_threads_lock);

			pthread_exit(NULL);
		}
		pthread_mutex_unlock(&barrier_called_lock);
		pthread_mutex_unlock(&finished_threads_lock);
	}
}

/* Checa se a thread atual é a com maior prioridade
*/
int is_high_priority_thread(int atual) {
	pthread_mutex_lock(&threads_per_queue_lock);
	for (int i = 0; i < atual; i++) {
		if(threads_per_queue[i] > 0) {
			pthread_mutex_unlock(&threads_per_queue_lock);
			return 0;
		}
	}
	pthread_mutex_unlock(&threads_per_queue_lock);

	return 1;
}

/* Cria uma nova thread com a prioridade entre 1 e 8. Default é 8. Valores menores
   definem maior prioridade. Caso o usuário defina uma prioridade inválida, use o default.
*/
void pt_spawn(unsigned int prioridade, void *(*funcao) (void *), void *parametros) {
	pt_thread_ctx *thread_ctx;
	all_threads *Node, *aux;
	/* crie a thread e coloque ela na fila correta */
	Node = (all_threads*) malloc(sizeof(all_threads));
	if(all_threads_queue == NULL) {
		all_threads_queue = Node;
		Node->next = NULL;
	}
	else {
		aux = all_threads_queue;
		all_threads_queue = Node;
		Node->next = aux;
	}

	thread_ctx = (pt_thread_ctx*) malloc(sizeof(pt_thread_ctx));
	thread_ctx->funcao = funcao;
	thread_ctx->parametros = parametros;

	if(prioridade >= 1 && prioridade <= 8) {
		thread_ctx->prioridade = prioridade;
	}
	else thread_ctx->prioridade = 8;

	pthread_create(&thread_ctx->thread_id, NULL, worker_thread, (void *) thread_ctx);

	Node->thread_id = thread_ctx->thread_id;
}

/*
	Função de execução da thread
*/
void * worker_thread(void *arg) {
	pt_thread_ctx *thread_ctx = (pt_thread_ctx*) arg;
	pthread_mutex_lock(&total_threads_count_lock);
	total_threads_count++;
	pthread_mutex_unlock(&total_threads_count_lock);

	//bloqueia na fila de sua prioridade
	pthread_mutex_lock(&inuse_processors_lock);

	pthread_mutex_lock(&threads_per_queue_lock);
	threads_per_queue[(int) thread_ctx->prioridade - 1]++;
	pthread_mutex_unlock(&threads_per_queue_lock);

	while(!can_go[(int)thread_ctx->prioridade - 1]) {
		pthread_cond_wait(&queue_cond[(int)thread_ctx->prioridade - 1], &inuse_processors_lock);
	}//Thread foi acordada - está com o processador
	can_go[thread_ctx->prioridade - 1] = 0;

	//Contabiliza remoção da fila
	pthread_mutex_lock(&threads_per_queue_lock);
	threads_per_queue[(int) thread_ctx->prioridade - 1]--;
	pthread_mutex_unlock(&threads_per_queue_lock);

	pthread_mutex_unlock(&inuse_processors_lock);
	//Thread vai executar

	//Se coloca na estrutura para possível yeld
	pthread_mutex_lock(&on_processor_lock);
	for (int i = 0; i < (int)total_processors; i++) {
		if (on_processor[i] == NULL) {
			on_processor[i] = thread_ctx;
			break;
		}
	}
	pthread_mutex_unlock(&on_processor_lock);

	//Thread vai executar
	(*(thread_ctx->funcao)) (thread_ctx->parametros);

	//Thread  se remove do processador
	 pthread_mutex_lock(&on_processor_lock);
	for(int i=0; i < (int) total_processors; i++) {
		if(on_processor[i] != NULL && pthread_equal(thread_ctx->thread_id, on_processor[i]->thread_id) != 0) {
			on_processor[i] = NULL;
			break;
		}
	}
	pthread_mutex_unlock(&on_processor_lock);

	//Thread terminou execução, vai liberar o processador
	pthread_mutex_lock(&inuse_processors_lock);
	if(inuse_processors_count == total_processors) {
		pthread_cond_signal(&all_processors_inuse);
	}
	inuse_processors_count--;

	//Contabiliza que terminou
	pthread_mutex_lock(&finished_threads_lock);
	finished_threads++;
	pthread_mutex_unlock(&finished_threads_lock);

	pthread_mutex_unlock(&inuse_processors_lock);

	free(thread_ctx);

	pthread_exit(NULL);
}

/* Faz a thread atual liberar o processador, voltar ao fim da fila de sua prioridade e
   esperar o próximo escalonamento
*/
void pt_yield() {
	pthread_t self_id = pthread_self();
	pt_thread_ctx *thread;
	//Thread  se remove do processador
	pthread_mutex_lock(&on_processor_lock);
	for(int i=0; i < (int) total_processors; i++) {
		if(on_processor[i] != NULL && pthread_equal(self_id, on_processor[i]->thread_id) != 0) {
			thread = on_processor[i];
			on_processor[i] = NULL;
			break;
		}
	}
	pthread_mutex_unlock(&on_processor_lock);

	//incrementando a posição dentro da mudança de estado do processador
	//pra evitar que incremente a fila, mas a thread n esteja de fato esperando
	pthread_mutex_lock(&inuse_processors_lock);
	//Incrementa a quantidade de Threads na fila de sua prioridade
	pthread_mutex_lock(&threads_per_queue_lock);
	threads_per_queue[(int)thread->prioridade - 1]++;
	pthread_mutex_unlock(&threads_per_queue_lock);

	if(inuse_processors_count == total_processors) {
		pthread_cond_signal(&all_processors_inuse);
	}
	inuse_processors_count--;
	//Thread se bloqueia em sua fila de prioridade
	while(!can_go[(int)thread->prioridade - 1]) {
		pthread_cond_wait(&queue_cond[(int)thread->prioridade - 1], &inuse_processors_lock);
	}//Thread acordada

	pthread_mutex_lock(&threads_per_queue_lock);
	threads_per_queue[(int)thread->prioridade - 1]--;
	pthread_mutex_unlock(&threads_per_queue_lock);

	can_go[(int)thread->prioridade - 1] = 0;
	pthread_mutex_unlock(&inuse_processors_lock);
	//Thread vai executar

	//Thread se coloca no vetor de processador
	pthread_mutex_lock(&on_processor_lock);
	for(int i=0; i < (int) total_processors; i++) {
		if(on_processor[i] == NULL ) {
			on_processor[i] = thread;
			break;
		}
	}
	pthread_mutex_unlock(&on_processor_lock);
}

/* Espera todas as threads terminarem */
void pt_barrier() {
	all_threads *aux;
	aux = all_threads_queue;
	pthread_mutex_lock(&barrier_called_lock);
	barrier_called = 1;
	pthread_mutex_unlock(&barrier_called_lock);

	while(aux != NULL) {
		pthread_join(aux->thread_id, NULL);
		aux = aux->next;
	}

	pthread_join(scheduler_thread, NULL);
}

/* Libera todas as estruturas de dados do escalonador */
void pt_destroy(){
	all_threads *aux;
	/* destrua as threads que estão esperando nas filas... */
	pthread_mutex_destroy(&total_threads_count_lock);
	pthread_mutex_destroy(&inuse_processors_lock);
	pthread_mutex_destroy(&finished_threads_lock);
	pthread_mutex_destroy(&barrier_called_lock);

	pthread_mutex_destroy(&on_processor_lock);

	pthread_cond_destroy(&all_processors_inuse);

	for (int i = 0; i < MAX_SIZE; i++) {
		pthread_cond_destroy(&queue_cond[i]);
	}

	pthread_mutex_destroy(&threads_per_queue_lock);

	/* libere memória da heap */
	free(on_processor);

	while(all_threads_queue != NULL) {
		aux = all_threads_queue;
		all_threads_queue = aux->next;
		free(aux);
	}
}
