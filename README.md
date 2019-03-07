# Trabalho 1 - Sistemas Operacionais 2018/2
Prof. Maurício Lima Pilla - http://lups.inf.ufpel.edu.br/~pilla

## Dados do/a aluno/a

Declaro que o presente trabalho contém código desenvolvido exclusivamente por mim e que não foi compartilhado de nenhuma forma com terceiros a não ser o professor da disciplina. Compreendo que qualquer violação destes princípios será punida rigorosamente de acordo com o Regimento da UFPEL.

(Preencha com seus dados)

- Nome completo: 
- Username do Bitbucket: 
- Email @inf: 

## Descrição

Neste trabalho, os/as alunos/as irão desenvolver um programa em C usando POSIX Threads para calcular o posicionamento de rainhas em tabuleiros de xadrez de dimensões quadradas variáveis, de forma que nenhuma rainha ataque outra.

Os parâmetros de entrada da função são:

* Número de rainhas
* Dimensões do tabuleiro (sempre tabuleiros quadrados)

A função deverá retornar a soma das posições das rainhas em cada uma das soluções. Cada casa do tabuleiro vale uma potência de 2, começando da primeira casa como 2^0 e incrementando a potência de 1 em 1 da esquerda para direita, de cima para baixo. Por exemplo, para um tabuleiro 2x2 e 1 rainha, as soluções são:

		X 0
		0 0  -->  2^0

		0 X
		0 0  -->  2^1

		0 0
		X 0  -->  2^2

		0 0
		0 X  -->  2^3

Somando, temos 1 + 2 + 4 + 8 = *15*.

Caso haja mais de uma rainha em uma solução, os valores das potências deverão ser somados:

		X 0 0
		0 0 X
		0 0 0  --> 2^0 + 2^5 = 33 

A implementação deverá explorar concorrência criando *threads* para explorar diferentes posicionamentos. A solução deverá usar alguma das técnicas de sincronização estudadas (*mutexes*, *semáforos* ou *monitores*) para garantir que não haja condição de corrida.


## Produtos

* Implementação (*nqueenst.c*)
* Arquivos acessórios (casos de teste do/a aluno/a, versão sequencial, etc.)
	* Não serão pontuados, mas são sugeridos fortemente como uma etapa parcial do trabalho
	* O arquivo de testes do professor será chamado _test.c_

## Cronograma

* Envio da versão final paralela: _05/10/2018_


## Correção

Quando o/a aluno/a fizer o _fork_ deste repositório, deverá incluir o professor como _admin_ (veja o tutorial para ver como fazer).

Para realizar a correção do trabalho, uma _issue_ deverá ser criada e o professor (_mlpilla_) como _assignee_ até o prazo final. O trabalho deve estar executando os testes que serão fornecidos pelo professor no _pipeline_ do Bitbucket (conforme tutorial).

Trabalhos sem histórico de _commits_ adequado terão descontos na nota, podendo inclusive chegar à nota _zero_.

Não serão aceitos entregues com atraso sem o uso de *tokens* de acordo com o documento no Google Classroom.


## Referências


* [1] [Tutorial sobre POSIX Threads](https://computing.llnl.gov/tutorials/pthreads/)
