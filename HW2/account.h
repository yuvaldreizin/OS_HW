#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "lock.h"
#include "utils.h"

typedef struct{
    int id;
    int pass;
    int balance;
    rwlock_t lock;
} account;


account *account_init(int id, int pass, int balance);
void account_free(account *account);
int account_get_id(account *account);
int account_get_pass(account *account);
int account_get_balance(account *account);
void account_read_lock(account *account);
void account_read_unlock(account *account);
void account_write_lock(account *account);
void account_write_unlock(account *account);

account *account_check_id(int id);

f_status_t account_o(int id, int pass, int initial_amount, int atm_id);
f_status_t account_d(int id, int pass, int amount, int atm_id);
f_status_t account_w(int id, int pass, int amount, int atm_id);
f_status_t account_b(int id, int pass, int atm_id);
f_status_t account_q(int id, int pass, int atm_id);
f_status_t account_t(int id, int pass, int amount, int to_id, int atm_id);
f_status_t account_print(int id);
f_status_t account_print_all();


#endif // ACCOUNT_H