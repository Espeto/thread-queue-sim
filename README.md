<h2 align="center"> Priority Queue Thread Simulator</h2>
<h3 align="center"> Implemented for the class of Operational Systems</h3> 

## Description

In this work, a library of user-level threads with priorities will be developed. The user can create threads with different priorities (where 1 is the highest priority). Scaling is by priority with FCFS queues for wach priority.

The user defines the number of virtual processors (system thread) that will be used.

The events that can generate a thread scalation are:

1. Creation of a new thread with _pt_spawn()_;
2. Termination of a thread; and
3. Call _pt_yield()_

To create a thread the user must pass the thread's priority, the function to be executed and a pointer to the arguments (as in _pthread_create()_)

## Referencies

* [1] [POSIX Threads Tutorial](https://computing.llnl.gov/tutorials/pthreads/)

## :memo: Licença

This project is under the MIT license. See the file [LICENSE](LICENSE) for more details.
