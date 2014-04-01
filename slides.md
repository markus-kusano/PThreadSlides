% An Intro to Threads
% Markus Kusano
% April 3 2014

## Motivation
Processors are not becoming faster

Processors are becoming more parallel

Thread's using shared memory is one effective way to write parallel programs

We will introduce some basic concepts of threading, and then go over an API (POSIX Threads).

![](../moarcoars.jpg)

## What is a Program?
Global variables

Local variables

Program counter (current position)

    int global;

    int main(int argc, char *argc) {
      global = 0;
      int local = global;

      return local;
    }

## What is a thread?
Each thread can be thought of as its own "program"

However: they all _share_ the same global variables

## Conceptual Model for Threads
Each thread has their own:

1. Local Variables
1. Program counter

The threads communicate through _shared_ (global) variables

![](../threads.jpg)

## Example
    int shared;

    void thread_1() {
      int local = shared;
      local--;
    }

    void thread_2() {
      int local = shared;
      local++;
    }

    int main(int argc, char *argv) {
      thread_create(thread_1);
      thread_create(thread_2);

      return 0;
    }

Local operations are "invisible" to other threads

## What is a Scheduler
The scheduler is the diety that decides when different threads run on the processor

Or, the scheduler decides when threads get to "make progress"

Conceptually, you should consider the scheduler to be non-deterministic

Or, the scheduler can run your threads in _any_ possible order

The scheduler is not guaranteed to run your threads in the same order everytime

When a different thread gets processor time (thus ejecting another thread) we
call this a _context switch_

![](../bonzibuddy.jpg)

## You've Just Won Three Dollars!
    BankAccount yourMoney;

    void thread_1(void) {
      int money1 = yourMoney.getBalance()
      money1 = money1 + 1;
      yourMoney.setBalance(money1);
    }

    void thread_2(void) {
      int money2 = yourMoney.getBalance()
      money2 = money2 + 2;
      yourMoney.setBalance(money2);
    }

    int main(int argc, char *argv) {
      thread_create(thread_1);
      thread_create(thread_2);

      return 0;
    }

Do you see the issue?

## You can potentially lose a dollar!
The scheduler _could_ cause a schedule to happen where you lose a dollar

    int money1 = yourMoney.getBalance(); // read of 0 dollars by thread 1
    int money2 = yourMoney.getBalance(); // read of 0 dollars by thread 1
    money1 = money1 + 1;                 // money1 == 1
    money2 = money2 + 2;                 // money2 == 2
    yourMoney.setBalance(money1);        // thread1 sets balance to 1
    yourMoney.setBalance(money2);        // thread2 sets balance to 2

Note: this is not guaraneteed to happen on every run!

![](../heisenbug.jpg)

## Data Races and Atomicity Violations
__Data Race:__ The value in memory changes depending on which thread updates it first

__Atomicity Violation:__ The programmer intended a region of code to be
executed without any interference from another thread

Our Bank Account has a data race _and_ atomicity violation!

![](../race_condition.bmp)

## The Mutex
__Mutex:__ A resource that only one thread can have at a time. If the resource
is not availible, then the thread will _wait_ until it is.

![](../shovel_01.jpg)
![](../shovel_02.jpg)
![](../shovel_03.jpg)

## More Cats
![](../mutex_01.jpg)
![](../mutex_02.jpg)
![](../mutex_03.jpg)
![](../mutex_04.jpg)
