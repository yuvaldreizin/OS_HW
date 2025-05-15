#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "lock.h"
#include "utils.h"

typedef struct{
    int id;
    int pass;
    int balance;
    rwlock_t lock;
} account_t;


account_t* account_init(int id, int pass, int balance);
void account_free(account_t* account);
int account_get_id(account_t* account);
int account_get_pass(account_t* account);
int account_get_balance(account_t* account);
f_status_t account_deposit(account_t* account, int pass, int amount);
f_status_t account_withdraw(account_t* account, int amount);
f_status_t account_transfer(account_t* from, account_t* to, int amount);
f_status_t account_print(account_t* account);
void account_read_lock(account_t* account);
void account_read_unlock(account_t* account);
void account_write_lock(account_t* account);
void account_write_unlock(account_t* account);



#endif // ACCOUNT_H