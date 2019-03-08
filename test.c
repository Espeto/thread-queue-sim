#include "simplegrade.h"
#include "priothreads.h"
#include <pthread.h>

/** DRAFT **/

struct contador{
	pthread_mutex_t * mutex;
	int contador;
};


void * incrementa(void *arg){
	struct contador *cp = (struct contador *)arg;
	pthread_mutex_lock(cp->mutex);
	cp->contador++;
	pthread_mutex_unlock(cp->mutex);
	return NULL;
}


void teste_foo_bar(){
	DESCRIBE("Exemplo simples de teste");

	struct contador c;
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	c.mutex = &mutex;
	c.contador = 0;
	WHEN("Duas threads incrementam um contador");
	IF("Contador iniciou com zero");
	THEN("Contador deve ter o valor dois");
	pt_init(1);
	pt_spawn(1, incrementa, (void *)&c);
	pt_spawn(2, incrementa, (void *)&c);
	pt_barrier();
	pt_destroy();
	pthread_mutex_destroy(&mutex);
	isEqual(c.contador,2,1);
}


int main(){

	teste_foo_bar();

	GRADEME();

	return 0;
}
