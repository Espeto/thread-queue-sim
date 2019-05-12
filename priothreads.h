#ifndef _PRIOTHREADS_H_
#define _PRIOTHREADS_H_

/* Reinicializa o escalonador. Descarta todo o estado interno atual (limpa filas) e
   define o número de processadores virtuais entre 1 e 8. Default é 1 processador.
   Caso o usuário defina um número inválido de processadores, use o default.
*/
void pt_init(unsigned int processadores);


/* Cria uma nova thread com a prioridade entre 1 e 8. Default é 8. Valores menores
   definem maior prioridade. Caso o usuário defina uma prioridade inválida, use o default.
*/
void pt_spawn(unsigned int prioridade, void *(*funcao) (void *), void *parametros);

/* Faz a thread atual liberar o processador, voltar ao fim da fila de sua prioridade e
   esperar o próximo escalonamento
*/
void pt_yield();


/* Espera todas as threads terminarem */
void pt_barrier();

/* Libera todas as estruturas de dados do escalonador */
void pt_destroy();

/* Scheduler dos processadores
	é chamada na inicialização
*/
void * scheduler();

/*
	Função de execução da thread
*/
void * worker_thread(void *arg);

/* Checa se a thread atual é a com maior prioridade
*/
int is_high_priority_thread(int atual);


#endif /* _PRIOTHREADS_H */
