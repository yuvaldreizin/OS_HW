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

account *account_check_id(int id){

}


f_status_t account_o(int id, int pass, int initial_amount, int atm_id){

}


f_status_t account_d(int id, int pass, int amount, int atm_id){

}


f_status_t account_w(int id, int pass, int amount, int atm_id){

}


f_status_t account_b(int id, int pass, int atm_id){

}


f_status_t account_q(int id, int pass, int atm_id){

}


f_status_t account_t(int id, int pass, int amount, int to_id, int atm_id){

}


f_status_t account_print(int id){

}

f_status_t account_print_all(){
    // lock all
    // print all
    // unlock all
}

