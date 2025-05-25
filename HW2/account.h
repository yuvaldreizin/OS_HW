#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "./utils.h"
#include "lock.h"
#include "linked_list.h"

/*=============================================================================
* Structures
* =============================================================================*/

typedef struct account{
    int id;
    int pass;
    int balance;
    rwlock_t lock;
} account;

// extern globals_t *globals;

/*=============================================================================
* Structures
* =============================================================================*/

account *account_init(int id, int pass, int balance);
void account_free(account *account);
int account_get_id(account *account);
int account_get_pass(account *account);
int account_get_balance(account *account);
void account_read_lock(account *account);
void account_read_unlock(account *account);
void account_write_lock(account *account);
void account_write_unlock(account *account);

int accounts_compare(void *acnt1, void *acnt2);
account *account_check_id_read(int id);
account *account_check_id_and_pass_read(int id, int pass, int atm_id);
account *account_check_id_write(int id);
account *account_check_id_and_pass_write(int id, int pass, int atm_id);

f_status_t account_o(int id, int pass, int initial_amount, int atm_id);
f_status_t account_d(int id, int pass, int amount, int atm_id);
f_status_t account_w(int id, int pass, int amount, int atm_id);
f_status_t account_b(int id, int pass, int atm_id);
f_status_t account_q(int id, int pass, int atm_id);
f_status_t account_t(int id, int pass, int amount, int to_id, int atm_id);
f_status_t account_print(account *acnt);
void lock_all_accounts();
void unlock_all_accounts();


#endif // ACCOUNT_H