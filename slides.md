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
The scheduler decides when threads get to "make progress". It is,

- non-deterministic

- Threads can run in _any_ possible order

- Not guaranteed to run your threads in the same order everytime

When a different thread gets scheduled we call this a _context switch_

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

__Atomic Operation:__ An operation that cannot be interrupted by a thread

__Atomicity Violation:__ The programmer intended an operation to be atomic

Our Bank Account has a data race _and_ atomicity violation!

![](../race_condition.bmp)

## Bank Account Bugs
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

The operation of `getBalance()` and `setBalance()` was intended to be atomic!

There is a data race in setting the actual balance...

## Read-Modify-Write
A single line of C/C++ code is not guaranteed to be atomic

__C Code__

    int shared;

    void thread_1(void) {
      shared = shared + 1;
    }

    void thread_2(void) {
      shared = shared + 2;
    }

__"Assembly"__

    int shared;

    void thread_1(void) {
      R1 = shared;      // read
      R1 = R1 + 1;      // modify
      store R1 shared;  // write
    }

    void thread_2(void) {
      R2 = shared;      // read
      R2 = R2 + 2;      // modify
      store R2 shared;  // write
    }

Do you see the bug?

## Bugaboo
      R1 = shared;      // read of 0 by thread1
      R2 = shared;      // read of 0 by thread2
      R1 = R1 + 1;      // R1 = 0 + 1 == 1
      R2 = R2 + 2;      // R2 = 0 + 2 == 2
      store R1 shared;  // store by thread1: shared == 1
      store R2 shared;  // store by thread2: shared == 2

Remember: when you run the program there is only a _chance_ this schedule will happen.

Most of the time 2 + 1 = 3

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

## Mutex Operations
A thread can acquire (lock) a mutex

A thread can release (unlock) a mutex

## Data Races: A Fix
__Racey Code__

    int shared;

    void thread_1(void) {
      shared = shared + 1;
    }

    void thread_2(void) {
      shared = shared + 2;
    }

__Race Freedom__

    mutex m;
    int shared;

    void thread_1(void) {
      lock(m);
      shared = shared + 1;
      unlock(m);
    }

    void thread_2(void) {
      lock(m);
      shared = shared + 2;
      unlock(m);
    }

## Possible Schedules
The mutex ensures that there are only two possible schedules

__Schedule 1__

      lock(m);              /* lock by thread1 */
      shared = shared + 1;  /* increment by thread1 */
      unlock(m);            /* unlock by thread1 */

      lock(m);              /* lock by thread2 */
      shared = shared + 2;  /* increment by thread2 */
      unlock(m);            /* unlock by thread2 */


__Schedule 2__

      lock(m);              /* lock by thread2 */
      shared = shared + 2;  /* increment by thread2 */
      unlock(m);            /* unlock by thread2 */

      lock(m);              /* lock by thread1 */
      shared = shared + 1;  /* increment by thread1 */
      unlock(m);            /* unlock by thread1 */

1 + 2 always equals 3.

This is a proof by exhaustion that our program is correct. QED.

## A Smarter Bank

__Header__

    class Account {
      public:
        void withdraw(int ammount);
        void deposit(int ammount);
        void transfer(Account to, int ammount);
      private:
        mutex m;
        int balance;
    };


__Implementation__

    void Account::transfer(Account to, int ammount) {
      lock(this->m);
      lock(to.m);
      this->withdraw(ammount);
      to.deposit(ammount);
      unlock(to.m);
      unlock(this->m);

    }

## Transferring Between Two Accounts

    void thread_1() {
      acc1.transfer(acc2, 10);
      return NULL;
    }

    void thread_2() {
      acc2.transfer(acc1, 10);
      return NULL;
    }

    int main() {
      thread_create(&t1, NULL, t1m, NULL);
      thread_create(&t2, NULL, t2m, NULL);
      return 0;
    }

Do you see the bug?

    void Account::transfer(Account to, int ammount) {
      lock(this->m);
      lock(to.m);
      this->withdraw(ammount);
      to.deposit(ammount);
      unlock(to.m);
      unlock(this->m);

    }

## Deadlock

    lock(acc1.m);   /* Lock by thread1 */
    lock(acc2.m);   /* Lock by thread2 */
    ...


Thread1 wants to `lockacc2.m)` but thread2 holds the lock

Thread2 wants to `lock(acc1.m)` but thread1 holds the lock

## Condition Variables
Event based programming: `signal` and `wait`

![](../cond_01.jpg)
![](../cond_02.jpg)
![](../cond_03.jpg)
![](../cond_04.jpg)

## An Interrupt Handler is a Thread!

  Cond c;

  void handler() {
    wait(c);
  }

  void external() {
    signal(c);
  }

`wait(c)` puts the thread to sleep until a signal on `c` is received


## PThreads
POSIX Threads

POSIX: Portable Operating Systems

Header File: `pthread.h`

Compile with: `-pthread`

## PThread API: Mutex
Locking a mutex:

    pthread_mutex_t mut;

    pthread_mutex_lock(&mut);

Mutexes need to be initialized

    pthread_mutex_init(&mut, NULL)

The second argument are the options; use NULL for defaults.

## PThread API: Creating a Thread

    pthread_t t1;
    pthread_create(t1, NULL, t1_main, NULL);

`pthread_t` is an ID for a thread

Argument 2: Thread options (NULL is the default);

Argument 4: Arguments to pass to thread

Thread functions _must_ take a `void *` parameter and return a `void *`.

    void *t1_main(void *args);

`void *` is a C "generic"; it can be a pointer to _anything_.


## Thread Creation
Passing an `int` to a thread function: use a cast

    void *t1_main(void *arg) {
      int a = (int) arg;
    }

    int main() {
      pthread_t t1;
      int a = 5;
      pthread_create(&t1, NULL, t1_main, (void *) a);
    }

Want to pass more arguments? Use a `struct`!

## Waiting for thread to exit

    void *t1_main(void *arg) {
      int a = (int) arg;
    }

    int main() {
      pthread_t t1;
      int a = 5;
      pthread_create(&t1, NULL, t1_main, (void *) a);
      pthread_join(t1, (void **)&a);
      return a;
    }

Pthread join:

- Wait for the thread to exit
- Store the value return by the thread in the address pointed to by `a`


## Conclusion
Data races, atomicity violations and deadlocks are plague shared memory concurrent programs

Banks should be single threaded
