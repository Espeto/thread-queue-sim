#include "priothreads.h"
#include <pthread.h>

/* inclua aqui as bibliotecas que precisar */
#include <stdlib.h>

/* adicione o que for preciso para uma thread ter prioridade e ficar bloqueada esperando sua vez de executar */
typedef struct {
	unsigned int prioridade;
	int estado;
	void *(*funcao) (void *);
	void *parametros;

} pt_thread_ctx;

/* Recomendo declarar tipos e estruturas internas aqui. */
typedef struct {
	void *(*funcao) (void *);

} Processor;

typedef struct {
	pt_thread_ctx *prox;
}queue_node;

typedef struct {
	queue_node *head;
	queue_node *tail;
}Queue;

Queue *threads_queue;

/* Reinicializa o escalonador. Descarta todo o estado interno atual (limpa filas) e
   define o número de processadores virtuais entre 1 e 8. Default é 1 processador.
   Caso o usuário defina um número inválido de processadores, use o default.
*/
void pt_init(unsigned int processadores){

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
