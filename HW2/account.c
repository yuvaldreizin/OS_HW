#include "account.h"

account* account_init(int id, int pass, int balance){
    account* account = MALLOC_VALIDATED(account, 1);
    account->id = id;
    account->pass = pass;
    account->balance = balance;
    rwlock_init(&(account->lock));
    return account;
}


void account_free(account* account){
    if(account == NULL) return;
    rwlock_destroy(&(account->lock));
    free(account);
}


int account_get_id(account* account){
    account_read_lock(account);
    int id =  account->id;
    account_read_unlock(account);
    return id;
}


int account_get_pass(account* account){
    account_read_lock(account);
    int pass =  account->pass;
    account_read_unlock(account);
    return pass;
}


int account_get_balance(account* account){
    account_read_lock(account);
    int balance = account->balance;
    account_read_unlock(account);
    return balance;
}


void account_read_lock(account* account){
    rwlock_acquire_read(&(account->lock));
}


void account_read_unlock(account* account){
    rwlock_release_read(&(account->lock));
}


void account_write_lock(account* account){
    rwlock_acquire_write(&(account->lock));
}


void account_write_unlock(account* account){
    rwlock_release_write(&(account->lock));
}

gint (*account_compare_ids)(gconstpointer id1, gconstpointer id2){
    if ((account *)id1->id < (account *)id2->id) return -1;
    else if ((account *)id1->id > (account *)id2->id) return 1;
    else return 0;
}

account *account_check_id(int id){
    rwlock_acquire_read(&(globals->account_lock));
    for (int i = 0; i < globals->num_accounts; i++){
        account *acnt = (account*)g_list_nth_data(globals->accounts, i);
        if(account_get_id(account) == id){
            rwlock_release_read(&(globals->account_lock));
            return acnt;
        }
    }
    rwlock_release_read(&(globals->account_lock));
    return NULL;
}

account *account_check_id_and_pass(int id, int pass, int atm_id){
    account *acnt = account_check_id(id);
    // check id
    if (acnt){
        log_lock();
        fprintf(globals->log_file, "Error %d: Your transaction failed - account id %d does not exist\n", atm_id, id);
        log_unlock();
        return NULL;
    }
    // check pass
    if (account_get_pass(acnt) != pass){
        log_lock();
        fprintf(globals->log_file, "Error %s: Your transaction failed - password for account id %d is incorrect\n", atm_id, id);
        log_unlock();
        return NULL;
    }
    return acnt;
}


f_status_t account_o(int id, int pass, int initial_amount, int atm_id){
    // check if id exists
    account *check_acnt = account_check_id(id);
    if (check_acnt){
        log_lock();
        fprintf(globals->log_file, "Error %d: Your transaction failed - account with the same id exists\n", atm_id);
        log_unlock();
        return INVALID_ID;
    }
    // add account
    rwlock_acquire_write(&(globals->account_lock));
    account *acnt = account_init(id, pass, initial_amount);
    g_list_insert_sorted(globals->accounts, acnt, account_compare_ids);
    globals->num_accounts++;
    rwlock_release_write(&(globals->account_lock));
    // write to log
    log_lock();
    fprintf(globals->log_file, "%d: New account id is %d with password %d and initial balance %d\n", atm_id, id, pass, initial_amount);
    log_unlock();
    return SUCCESS;
}


f_status_t account_d(int id, int pass, int amount, int atm_id){
    account *acnt = account_check_id_and_pass(id, pass, atm_id);
    // add amount
    // TODO - check if amount > 0 ?
    account_write_lock(acnt);
    acnt->balance += amount;
    account_write_unlock(acnt);
    int balance = account_get_balance(acnt);
    // write to log
    log_lock();
    fprintf(globals->log_file, "%s: Account %d new balance is %d after %d $ was deposited\n", atm_id, id, balance, amount);
    log_unlock();
    return SUCCESS;
}


f_status_t account_w(int id, int pass, int amount, int atm_id){
    account *acnt = account_check_id_and_pass(id, pass, atm_id);
    // check balance
    int balance = account_get_balance(acnt);
    if (balance < amount){
        log_lock();
        fprintf(globals->log_file, "Error %d: Your transaction failed - account id %d balance is lower than %d\n", atm_id, id, amount);
        log_unlock();
        return FAILURE;
    }
    account_write_lock(acnt);
    acnt->balance -= amount;
    account_write_unlock(acnt);
    balance -= amount;
    // write to log
    log_lock();
    fprintf(globals->log_file, "%s: Account %d new balance is %d after %d $ was withdrawn\n", atm_id, id, balance, amount);
    log_unlock();
    return SUCCESS;
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

