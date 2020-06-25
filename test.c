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
	printf("incrementa iniciou\n");
	pthread_mutex_lock(cp->mutex);
	cp->contador++;
	pthread_mutex_unlock(cp->mutex);
	printf("incrementa terminou\n");
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

/* O parâmetro desabilitar desliga o teste (mas perde os pontos) */
void teste_contador_uma_thread(int desabilitar){
	DESCRIBE("Teste com uma thread e um contador");

	struct contador c;
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	c.mutex = &mutex;
	c.contador = 0;
	WHEN("Uma thread incrementa um contador");
	IF("Contador iniciou com zero");
	THEN("Contador deve ter o valor um");
	if (!desabilitar){
		pt_init(1);
		pt_spawn(1, incrementa, (void *)&c);
		pt_barrier();
		pt_destroy();
		pthread_mutex_destroy(&mutex);
	}
	isEqual(c.contador,1,1);
}


/* O parâmetro desabilitar desliga o teste (mas perde os pontos) */
void teste_contador_duas_threads_dois_cores(int desabilitar){
	DESCRIBE("Teste com duas threads e dois processadores virtuais");

	struct contador c;
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	c.mutex = &mutex;
	c.contador = 0;
	WHEN("Duas threads incrementam um contador");
	IF("Contador iniciou com zero");
	THEN("Contador deve ter o valor dois");
	if (!desabilitar){
		pt_init(2);
		pt_spawn(1, incrementa, (void *)&c);
		pt_spawn(2, incrementa, (void *)&c);
		pt_barrier();
		pt_destroy();
		pthread_mutex_destroy(&mutex);
	}
	isEqual(c.contador,2,2);
}


void * incrementa_100_vezes(void *arg){
	struct contador *cp = (struct contador *)arg;
	for(int i=0; i<100; i++){
		pthread_mutex_lock(cp->mutex);
		cp->contador++;
		pthread_mutex_unlock(cp->mutex);
		pt_yield();
	}
	return NULL;
}

/* O parâmetro desabilitar desliga o teste (mas perde os pontos) */
void teste_contador_duas_threads_dois_cores_laco(int desabilitar){
	DESCRIBE("Teste com duas threads e dois processadores virtuais, laco 100x");

	struct contador c;
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	c.mutex = &mutex;
	c.contador = 0;
	WHEN("Duas threads incrementam um contador 100x cada");
	IF("Contador iniciou com zero");
	THEN("Contador deve ter o valor 200");
	if (!desabilitar){
		pt_init(2);
		pt_spawn(1, incrementa_100_vezes, (void *)&c);
		pt_spawn(2, incrementa_100_vezes, (void *)&c);
		pt_barrier();
		pt_destroy();
		pthread_mutex_destroy(&mutex);
	}
	isEqual(c.contador,200,2);
}


/* O parâmetro desabilitar desliga o teste (mas perde os pontos) */
void teste_contador_dez_threads_quatro_cores_laco(int desabilitar){
	DESCRIBE("Teste com dez threads e quatro processadores virtuais, laco 100x");

	struct contador c;
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	c.mutex = &mutex;
	c.contador = 0;
	WHEN("Dez threads incrementam um contador 100x cada");
	IF("Contador iniciou com zero");
	THEN("Contador deve ter o valor 1000");
	if (!desabilitar){
		pt_init(4);
		for(int i=0; i<10; i++){
			pt_spawn(1, incrementa_100_vezes, (void *)&c);
		}
		pt_barrier();
		pt_destroy();
		pthread_mutex_destroy(&mutex);
	}
	isEqual(c.contador,1000,2);
}


struct potencia{
	pthread_mutex_t * mutex;
	float pot;
	float *var;
};



void * potencia_yield(void *arg){
	struct potencia *cp = (struct potencia *)arg;
	printf("potencia yield iniciou\n");
	pt_yield();
	pthread_mutex_lock(cp->mutex);
	*(cp->var) = pow(*(cp->var), cp->pot);
	pthread_mutex_unlock(cp->mutex);
	printf("potencia yield terminou\n");
	return NULL;
}

void * potencia(void *arg){
	struct potencia *cp = (struct potencia *)arg;
	printf("potencia iniciou\n");

	pthread_mutex_lock(cp->mutex);
	*(cp->var) = pow(*(cp->var), cp->pot);
	pthread_mutex_unlock(cp->mutex);
	printf("potencia terminou\n");
	return NULL;
}




void teste_duas_threads_um_core_yield(int desabilitar){
	DESCRIBE("Teste do yield");
	WHEN("Duas threads fazem operações de potencia");
	IF("Variavel iniciou com 2");
	THEN("Se executadas na ordem esperada, resultado deve ser 64");
	if(!desabilitar){
		struct potencia p1, p2;
		pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
		float var = 2;
		p1.mutex = &mutex;
		p2.mutex = &mutex;
		p1.var = &var;
		p2.var = &var;
		p1.pot = 2;
		p2.pot = 3;
		pt_init(1);
		pt_spawn(3, potencia_yield, (void *)&p1); // 8^2 = 64
		pt_spawn(4, potencia, (void *)&p2); // 2^3 = 8
		pt_barrier();
		pt_destroy();
		pthread_mutex_destroy(&mutex);
		isEqual(var,64,2);
	}
	else isEqual(0,1,2);
}



int main(){

	teste_foo_bar();
	// para desabilitar um teste que não esteja passando, troque o 0 por 1
	teste_contador_uma_thread(0);
	teste_contador_duas_threads_dois_cores(0);
	teste_contador_duas_threads_dois_cores_laco(0);
	teste_contador_dez_threads_quatro_cores_laco(0);
	teste_duas_threads_um_core_yield(0);

	GRADEME();

	return 0;
}
