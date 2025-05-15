#include "account.h"

account_t* account_init(int id, int pass, int balance){
    account_t* account = MALLOC_VALIDATED(account_t, 1);
    account->id = id;
    account->pass = pass;
    account->balance = balance;
    rwlock_init(&(account->lock));
    return account;
}


void account_free(account_t* account){
    if(account == NULL) return;
    rwlock_destroy(&(account->lock));
    free(account);
}


int account_get_id(account_t* account){
    account_read_lock(account);
    int id =  account->id;
    account_read_unlock(account);
    return id;
}


int account_get_pass(account_t* account){
    account_read_lock(account);
    int pass =  account->pass;
    account_read_unlock(account);
    return pass;
}


int account_get_balance(account_t* account){
    account_read_lock(account);
    int balance = account->balance;
    account_read_unlock(account);
    return balance;
}


f_status_t account_deposit(account_t* account, int pass, int amount){
    // assume amount >= 0
    if (account->pass != pass) return FAILURE;
    account_write_lock(account);
    account->balance += amount;
    account_write_unlock(account);
    return SUCCESS;
}


f_status_t account_withdraw(account_t* account, int amount){

}


f_status_t account_transfer(account_t* from, account_t* to, int amount){

}


f_status_t account_print(account_t* account){

}


void account_read_lock(account_t* account){
    rwlock_acquire_read(&(account->lock));
}


void account_read_unlock(account_t* account){
    rwlock_release_read(&(account->lock));
}


void account_write_lock(account_t* account){
    rwlock_acquire_write(&(account->lock));
}


void account_write_unlock(account_t* account){
    rwlock_release_write(&(account->lock));
}

