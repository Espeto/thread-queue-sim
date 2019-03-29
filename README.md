# Trabalho 1 - Sistemas Operacionais 2019/1
Prof. Maurício Lima Pilla - http://lups.inf.ufpel.edu.br/~pilla

## Dados do/a aluno/a

Declaro que o presente trabalho contém código desenvolvido exclusivamente por mim e que não foi compartilhado de nenhuma forma com terceiros a não ser o professor da disciplina. Compreendo que qualquer violação destes princípios será punida rigorosamente de acordo com o Regimento da UFPEL.

(Preencha com seus dados)

- Nome completo: Gilberto Kreisler Franco Neto
- Username do Bitbucket: gkreisler
- Email @inf: gkfneto@inf.ufpel.edu.br

## Descrição

Neste trabalho será desenvolvido uma biblioteca de threads de nível de usuário com prioridades. O usuário poderá criar threads com diferentes prioridades (onde 1 é a maior prioridade). O escalonamento é por prioridades com filas FCFS para cada prioridade.

O usuário define o número de processadores virtuais (threads de sistema) que serão usados.

Os eventos que podem gerar o escalonamento de uma thread são:

1. Criação de nova thread com _pt_spawn()_;
2. Término de uma thread; e
3. Chamada de _pt_yield()_.

Para criar uma thread, o usuário deve passar sua prioridade, a função a ser executada e um ponteiro para os argumentos (como em _pthread_create()_).

A implementação deverá usar a biblioteca PThreads.


## Produtos

* Implementação (*prioritythreads.c*)
* Arquivos acessórios (casos de teste do/a aluno/a, etc.)
	* Não serão pontuados, mas são sugeridos fortemente como uma etapa parcial do trabalho
	* O arquivo de testes do professor será chamado _test.c_

## Cronograma

* Entrega final: 24/05


## Correção

Quando o/a aluno/a fizer o _fork_ deste repositório, deverá incluir o professor como _admin_ (veja o tutorial para ver como fazer).

Para realizar a correção do trabalho, uma _issue_ deverá ser criada e o professor (_mlpilla_) como _assignee_ até o prazo final. O trabalho deve estar executando os testes que serão fornecidos pelo professor no _pipeline_ do Bitbucket (conforme tutorial).

Trabalhos sem histórico de _commits_ adequado terão descontos na nota, podendo inclusive chegar à nota _zero_.


## Referências


* [1] [Tutorial sobre POSIX Threads](https://computing.llnl.gov/tutorials/pthreads/)
