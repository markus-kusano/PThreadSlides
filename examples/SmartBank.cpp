#include <pthread.h>

class Account {
  public:
    Account() {
      pthread_mutex_init(&m, NULL);
    }
    void withdraw(int ammount) { balance -= ammount; }
    void deposit(int ammount) { balance += ammount; }
    void transfer(Account to, int ammount) {
      pthread_mutex_lock(&(this->m));
      pthread_mutex_lock(&(to.m));
      this->withdraw(ammount);
      to.deposit(ammount);
      pthread_mutex_unlock(&(to.m));
      pthread_mutex_unlock(&(this->m));
    }
  private:
    pthread_mutex_t m;
    int balance;
};

Account acc1;
Account acc2;

void *t1m(void *unused) {
  acc1.transfer(acc2, 10);
  return NULL;
}

void *t2m(void *unused) {
  acc2.transfer(acc1, 10);
  return NULL;
}

int main() {
  pthread_t t1, t2;

  pthread_create(&t1, NULL, t1m, NULL);
  pthread_create(&t2, NULL, t2m, NULL);

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  return 0;
}
