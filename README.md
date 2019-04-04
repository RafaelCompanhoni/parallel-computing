REFERENCES

Parallel Programming in C with MPI and OpenMP
https://apps2.mdp.ac.id/perpustakaan/ebook/Karya%20Umum/Parallel_Programming_in_OpenMP.pdf
https://www.openmp.org/wp-content/uploads/OpenMP4.0.0.pdf


-------------

## INTRO

It is comprised of a set of compiler directives that describe the parallelism in the source code, along
with a supporting library of subroutines available to applications. Collectively, these directives and library routines are formally described by the application programming interface (API) now
known as OpenMP. 

Within the C/C++ languages, directives are referred to as “pragmas”)

The language extensions in OpenMP fall into one of three categories: 
* control structures for expressing parallelism
* data environment constructs for communicating between threads
* synchronization constructs for coordinating the execution of multiple threads. 

The omp keyword distinguishes the pragma as an OpenMP pragma, so that it is processed as such by OpenMP compilers and ignored by nonOpenMP compilers

```c

#pragma omp ...

```

## Parallel Control Structures

Control structures are constructs that alter the flow of control in a program. We call the basic execution model for OpenMP a fork/join model, and parallel control structures are those constructs that fork (i.e., start) new threads, or give execution control to one or another set of threads.

OpenMP provides two kinds of constructs for controlling parallelism. First, it provides a directive to create multiple threads of execution that execute concurrently with each other. The only instance of this is the **PARALLEL** directive: it encloses a block of code and creates a set of threads that
each execute this block of code concurrently. 

Second, OpenMP provides constructs to divide work among an existing set of parallel threads. An instance of this is the **DO** directive, used for exploiting loop-level parallelism. It divides the iterations of a loop among multiple concurrently executing threads. 

## Communication and Data Environment

An OpenMP program always begins with a single thread (master thread) of control that has associated with it an execution context or data environment. The execution context for a thread is the data address space containing all the variables specified in the program. 

A variable may
have one of three basic attributes: **shared**, **private**, or **reduction**. 

* A variable that has the shared scope clause on a parallel construct will have a single storage location in memory for the duration of that parallel construct. All parallel threads that reference the variable will always access the same memory location. 

* a variable that has private scope will have multiple storage locations, one within the execution context of each thread, for the duration of the parallel construct. All read/write operations on that variable by a thread will refer to the private copy of that variable within that thread

* reduction variables have both private and shared storage behavior. As the name implies, the reduction attribute is used on objects that are the target of an arithmetic reduction. Reduction operations are important to many applications, and the reduction attribute allows them to be implemented by the compiler efficiently. The most common example is the final summation of temporary local variables at the end of a parallel construct.


## Synchronization

Multiple OpenMP threads communicate with each other through ordinary reads and writes to shared variables. However, it is often necessary to coordinate the access to these shared variables across multiple threads. Such conflicting accesses can potentially lead to incorrect data values and must be avoided by explicit coordination between
multiple threads.

The two most common forms of synchronization are **mutual exclusion** and **event synchronization**.

* mutual exclusion: When multiple threads are modifying the same variable, acquiring exclusive access to the variable before modifying it ensures the integrity of that variable. OpenMP provides mutual exclusion through a **CRITICAL** directive.

* event synchronization: Typically used to signal the occurrence of an event across multiple threads. The simplest form of event synchronization is a barrier. A barrier directive in a parallel program defines a point where each thread waits for all other threads to arrive. Once all the threads arrive at that point, they can all continue execution past the barrier. Each thread is therefore guaranteed that all the code before the barrier has been completed across all other threads.


