# CSE-320
CSE 320: Systems Fundamentals II

# Homework 1
## Introduction
In this assignment, you will write a command line utility to perform decompression of data that has been compressed into a set of rules using a data compression algorithm called Sequitur, and you will also implement some data structures to support the core Sequitur algorithm (for which I will provide an implementation), so that in the end you will have constructed a utility that can perform both compression and decompression. The goal of this homework is to familiarize yourself with C programming, with a focus on input/output, bitwise manipulations, and the use of pointers.
For all assignments in this course, youMUST NOT put any of the functions that you write into themain.c file. The file main.c MUST ONLY contain #include s, local #define s and the main function (you may of course modify the main function body). The reason for this restriction has to do with our use of the Criterion library to test your code. Beyond this, you may have as many or as few additional .c files in the src directory as you wish. Also, you may declare as many or as few headers as you wish. Note, however, that header and .c files distributed with the assignment base code often contain a comment at the beginning which states that they are not to be modified. PLEASE take note of these comments and do not modify any such files, as they will be replaced by the original versions during grading.
:scream: Array indexing ('A[]') is not allowed in this assignment. You MUST USE pointer arithmetic instead. All necessary arrays are declared in the const.h header file. You MUST USE these arrays. DO NOT create your own arrays. We WILL check for this.
:nerd: Reference for pointers: https://beej.us/guide/bgc/html/multi/pointers.html.

# Homework 2
## Introduction
In this assignment you are tasked with updating an old piece of software, making sure it compiles, and that it works properly in your VM environment.
Maintaining old code is a chore and an often hated part of software engineering. It is definitely one of the aspects which are seldom discussed or thought about by aspiring computer science students. However, it is prevalent throughout industry and a worthwhile skill to learn. Of course, this homework will not give you a remotely realistic experience in maintaining legacy code or code left behind by previous engineers but it still provides a small taste of what the experience may be like. You are to take on the role of an engineer whose supervisor has asked you to correct all the errors in the program, plus add additional functionality.
By completing this homework you should become more familiar with the C programming language and develop an understanding of:
How to use tools such as gdb and valgrind for debugging C code. Modifying existing C code.
C memory management and pointers.

# Homework 3
## Introduction
You must read Chapter 9.9 Dynamic Memory Allocation Page 839before starting this assignment. This chapter contains all the theoretical information needed to complete this assignment. Since the textbook has sufficient information about the different design strategies and implementation details of an allocator, this document will not cover this information. Instead, it will refer you to the necessary sections and pages in the textbook.
## Takeaways
After completing this assignment, you will have a better understanding of: * The inner workings of a dynamic memory allocator * Memory padding and alignment * Structs and linked lists in C * errno numbers in C * Unit testing in C

# Homework 4
## Introduction
The goal of this assignment is to become familiar with low-level Unix/POSIX system calls related to processes, signal handling, files, and I/O redirection. You will implement a "problem solver" program, called polya , which manages a collection of "worker" processes that concurrently engage in solving computationally intensive problems.
## Takeaways
After completing this assignment, you should:
Understand process execution: forking, executing, and reaping. Understand signal handling.
Understand the use of "dup" to perform I/O redirection.
Have gained experience with C libraries and system calls. Have enhanced your C programming abilities.

# Homework 5
## Introduction
The goal of this assignment is to become familiar with low-level POSIX threads, multi-threading safety, concurrency guarantees, and networking. The overall objective is to implement a server that simulates the behavior of a Private Branch Exchange (PBX) telephone system. As you will probably find this somewhat difficult, to grease the way I have provided you with a design for the server, as well as binary object files for almost all the modules. This means that you can build a functioning server without initially facing too much complexity. In each step of the assignment, you will replace one of my binary modules with one built from your own source code. If you succeed in replacing all of my modules, you will have completed your own version of the server.
For this assignment, there are three modules to work on: * Server initialization ( main.c ) * Server module ( server.c ) * PBX module ( pbx.c )
It is probably best if you work on the modules in the order listed. You should turn in whatever modules you have worked on. Though the exact details are not set at this time, I expect that your code will be compiled and tested in the following four configurations: 1. Your with my server.o and . 2. Your main.c and server.c with my pbx.o . 3. Your
and     . 4. Unit tests on your     in isolation.
If your code fails to compile in one or more of these configurations, you will receive zero points for that configuration. If your code compiles in a particular configuration, then you will receive a point for your code having compiled and you will receive points for whatever test cases for that configuration your program passes.
## Takeaways
After completing this homework, you should:
Have a basic understanding of socket programming Understand thread execution, mutexes, and semaphores
Have an understanding of POSIX threads
Have some insight into the design of concurrent data structures Have enhanced your C programming abilities
