#include "priothreads.h"
#include <pthread.h>

/* inclua aqui as bibliotecas que precisar */
#include <stdlib.h>
#include <stdio.h>

/* adicione o que for preciso para uma thread ter prioridade e ficar bloqueada esperando sua vez de executar */
typedef struct {
	unsigned int prioridade;
	int estado;
	void *(*funcao) (void *);
	void *parametros;

} pt_thread_ctx;

/* Recomendo declarar tipos e estruturas internas aqui. */
#define NUM_FILA 8

typedef struct {
	void *(*funcao) (void *);

} Processor;

typedef struct {
	pt_thread_ctx thread_pcb;
	pt_thread_ctx *next;
} Node;

typedef struct {
	Node *head;
	Node *tail;
} Queue;

Queue **queues;
Processor *processors;

/* Reinicializa o escalonador. Descarta todo o estado interno atual (limpa filas) e
   define o número de processadores virtuais entre 1 e 8. Default é 1 processador.
   Caso o usuário defina um número inválido de processadores, use o default.
*/
void pt_init(unsigned int processadores){

	if(processadores > 1 && processadores <= 8) {
		processors = (Processor*) malloc(processadores * sizeof(Processor));
	}
	else {
		processors = (Processor*) malloc(processadores * sizeof(Processor));
	}

	if(processors == NULL) {
		printf("Erro de alocação de memória\n");
		exit(1);
	}
}

/*
	Inicializa/Reinicializa as filas
*/
void init_queue() {
	Node *aux;
	Queue *single_queue;

	if (queues == NULL) {
		queues = (Queue**) malloc(NUM_FILA * sizeof(Queue*));
		if(queues == NULL) {
			printf("Erro de alocação de memória\n");
			exit(1);
		}
		for (int i = 0; i < NUM_FILA; i++) {
			queues[i]->head = NULL;
			queues[i]->tail = NULL;
		}
	}

	else {
		single_queue = queues[i];
		aux = single_queue->head;

		do {
			queues->head = aux->next;
		}while(aux != NULL);
	}


}

/*
	Scheduler dos processadores
*/
void scheduler() {

}

/* Cria uma nova thread com a prioridade entre 1 e 8. Default é 8. Valores menores
   definem maior prioridade. Caso o usuário defina uma prioridade inválida, use o default.
*/
void pt_spawn(unsigned int prioridade, void *(*funcao) (void *), void *parametros){
	pt_thread_ctx * thread;

	/* crie a thread e coloque ela na fila correta */


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
