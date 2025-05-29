#define _POSIX_C_SOURCE 200809L
#include "account.h"

account *account_init(int id, int pass, int balance){
    account *acnt = MALLOC_VALIDATED(account, sizeof(account));
    acnt->id = id;
    acnt->pass = pass;
    acnt->balance = balance;
    acnt->lock = rwlock_init();
    return acnt;
}


void account_free(void* acc){
    account * acnt = (account *)acc;
    if (acnt == NULL) return;
    rwlock_destroy((acnt->lock));
    free(acnt);
}

// Better delete, messes lock handling
// int account_get_id(account *account){
//     account_read_lock(account);
//     int id =  account->id;
//     account_read_unlock(account);
//     return id;
// }


// int account_get_pass(account *account){
//     account_read_lock(account);
//     int pass =  account->pass;
//     account_read_unlock(account);
//     return pass;
// }


// int account_get_balance(account *account){
//     account_read_lock(account);
//     int balance = account->balance;
//     account_read_unlock(account);
//     return balance;
// }


void account_read_lock(account *account){
    rwlock_acquire_read((account->lock));
}


void account_read_unlock(account *account){
    rwlock_release_read((account->lock));
}


void account_write_lock(account *account){
    rwlock_acquire_write((account->lock));
}


void account_write_unlock(account *account){
    rwlock_release_write((account->lock));
}

int accounts_compare(void *acnt1, void *acnt2){
    // no locks because id doesn't change unless deleted + global account list is locked
    account *acnt1_casted = (account *)acnt1;
    account *acnt2_casted = (account *)acnt2;
    if (acnt1_casted->id < acnt2_casted->id) return -1;
    else if (acnt1_casted->id > acnt2_casted->id) return 1;
    else return 0;
}

account *account_check_id_read(int id){ 
    rwlock_acquire_read((globals->account_lock));
    Node *l;
    for (l = globals->accounts->head; l != NULL; l=l->next){
        account *acnt = (account*)l->data;
        if(acnt->id == id){
            // lock account for function and release global list
            account_read_lock(acnt);
            rwlock_release_read((globals->account_lock));
            return acnt;
        }
    }
    rwlock_release_read((globals->account_lock));
    return NULL;
}

account *account_check_id_and_pass_read(int id, int pass, int atm_id){
    account *acnt = account_check_id_read(id);
    // check id
    if (!acnt){
        log_lock();
        fprintf(globals->log_file, "Error %d: Your transaction failed - account id %d does not exist\n", atm_id, id);
        fflush(globals->log_file);
        log_unlock();
        return NULL;
    }
    // check pass
    if (acnt->pass != pass){
        log_lock();
        fprintf(globals->log_file, "Error %d: Your transaction failed - password for account id %d is incorrect\n", atm_id, id);
        fflush(globals->log_file);
        log_unlock();
        account_read_unlock(acnt);
        return NULL;
    }
    return acnt;
}


account *account_check_id_write(int id){ 
    rwlock_acquire_read((globals->account_lock));
    Node *l;
    for (l = globals->accounts->head; l != NULL; l=l->next){
        account *acnt = (account*)l->data;
        if(acnt->id == id){
            // lock account for function and release global list
            fprintf(stderr, "reached here\n");
            account_write_lock(acnt);
            fprintf(stderr, "reached here too\n");
            rwlock_release_read((globals->account_lock));
            fprintf(stderr, "account id is %d\n", acnt->id);
            return acnt;
        }
    }
    rwlock_release_read((globals->account_lock));
    return NULL;
}

account *account_check_id_and_pass_write(int id, int pass, int atm_id){
    account *acnt = account_check_id_write(id);
    // check id
    if (!acnt){
        log_lock();
        fprintf(globals->log_file, "Error %d: Your transaction failed - account id %d does not exist\n", atm_id, id);
        fflush(globals->log_file);
        log_unlock();
        return NULL;
    }
    fprintf(stderr, "account id is %d\n", acnt->id);
    // check pass
    if (acnt->pass != pass){
        log_lock();
        fprintf(globals->log_file, "Error %d: Your transaction failed - password for account id %d is incorrect\n", atm_id, id);
        fflush(globals->log_file);
        log_unlock();
        account_write_unlock(acnt);
        return NULL;
    }
    return acnt;
}


f_status_t account_o(int id, int pass, int initial_amount, int atm_id){
    // check if id exists
    account *check_acnt = account_check_id_read(id);
    if (check_acnt){
        log_lock();
        fprintf(globals->log_file, "Error %d: Your transaction failed - account with the same id exists\n", atm_id);
        fflush(globals->log_file); 
        log_unlock();
        return INVALID_ID;
    }
    // add account
    rwlock_acquire_write((globals->account_lock));
    account *acnt = account_init(id, pass, initial_amount);
    linked_list_sorted_insert(globals->accounts, acnt, accounts_compare);
    rwlock_release_write((globals->account_lock));
    // write to log
    log_lock();
    fprintf(globals->log_file, "%d: New account id is %d with password %d and initial balance %d\n", atm_id, id, pass, initial_amount);
    fflush(globals->log_file);
    log_unlock();
    return SUCCESS;
}


f_status_t account_d(int id, int pass, int amount, int atm_id){
    account *acnt = account_check_id_and_pass_write(id, pass, atm_id);
    if (!acnt) return FAILURE;
    fprintf(stderr, "acnt is %p\n", (void*)acnt);
    // add amount
    // TODO - check if amount > 0 ?
    fprintf(stderr, "account_d called with id: %d, pass: %d, amount: %d, atm_id: %d\n", id, pass, amount, atm_id);
    acnt->balance += amount;
    int balance = acnt->balance;
    account_write_unlock(acnt);
    // write to log
    log_lock();
    fprintf(globals->log_file, "%d: Account %d new balance is %d after %d $ was deposited\n", atm_id, id, balance, amount);
    fflush(globals->log_file);
    log_unlock();
    return SUCCESS;
}


f_status_t account_w(int id, int pass, int amount, int atm_id){
    account *acnt = account_check_id_and_pass_write(id, pass, atm_id);
    if (!acnt) return FAILURE;
    // check balance
    int balance = acnt->balance;
    if (balance < amount){
        log_lock();
        fprintf(globals->log_file, "Error %d: Your transaction failed - account id %d balance is lower than %d\n", atm_id, id, amount);
        fflush(globals->log_file);
        log_unlock();
        account_write_unlock(acnt);
        return FAILURE;
    }
    acnt->balance -= amount;
    account_write_unlock(acnt);
    balance -= amount;
    // write to log
    log_lock();
    fprintf(globals->log_file, "%d: Account %d new balance is %d after %d $ was withdrawn\n", atm_id, id, balance, amount);
    fflush(globals->log_file);
    log_unlock();
    return SUCCESS;
}


f_status_t account_b(int id, int pass, int atm_id){
    account *acnt = account_check_id_and_pass_read(id, pass, atm_id);
    if (!acnt) return FAILURE;
    // check balance
    int balance = acnt->balance;
    account_read_unlock(acnt);
    // write to log
    log_lock();
    fprintf(globals->log_file, "%d: Account %d balance is %d\n", atm_id, id, balance);
    fflush(globals->log_file);
    log_unlock();
    return SUCCESS;
}


f_status_t account_q(int id, int pass, int atm_id){
    account *acnt = account_check_id_and_pass_write(id, pass, atm_id);
    if (!acnt) return FAILURE;
    // delete account
    int balance = acnt->balance;
    rwlock_acquire_write((globals->account_lock));
    linked_list_remove(globals->accounts, acnt);
    rwlock_release_write((globals->account_lock));
    account_write_unlock(acnt);

    account_free(acnt);
    // write to log
    log_lock();
    fprintf(globals->log_file, "%d: Account %d is now closed. Balance was %d\n", atm_id, id, balance);
    fflush(globals->log_file);
    log_unlock();
    return SUCCESS;
}


f_status_t account_t(int id, int pass, int amount, int to_id, int atm_id){
    account *acnt = account_check_id_and_pass_write(id, pass, atm_id);
    if (!acnt) return FAILURE;
    // check balance
    int balance = acnt->balance;
    if (balance < amount){
        log_lock();
        fprintf(globals->log_file, "Error %d: Your transaction failed - account id %d balance is lower than %d\n", atm_id, id, amount);
        fflush(globals->log_file);
        log_unlock();
        account_write_unlock(acnt);
        return FAILURE;
    }
    account *to_acnt = account_check_id_write(atm_id);
    if (!to_acnt){
        account_write_unlock(acnt);
        return FAILURE;
    }
    acnt->balance -= amount;
    balance -= amount;
    to_acnt->balance += amount;
    int to_balance = to_acnt->balance;
    account_write_unlock(to_acnt);
    account_write_unlock(acnt);
    // write to log
    log_lock();
    fprintf(globals->log_file, "%d: Transfer %d from account %d to account %d new account balance is %d new target account balance is %d\n",
        atm_id, amount, id, to_id, balance, to_balance);
    fflush(globals->log_file);
    log_unlock();
    return SUCCESS;
}


f_status_t account_print(account *acnt){ 
    // ASSUME ACCOUNT IS LOCKED
    // print account
    int id = acnt->id;
    int pass = acnt->pass;
    int balance = acnt->balance;
    printf("Account %d: Balance - %d $, Account Password - %d\n", id, balance, pass);
    return SUCCESS;
}

void lock_all_accounts(){
    rwlock_acquire_write((globals->account_lock));
    Node *l;
    for (l = globals->accounts->head; l != NULL; l=l->next){
        account *acnt = (account*)l->data;
        account_write_lock(acnt);
    }
}

void unlock_all_accounts(){
    Node *l;
    for (l = globals->accounts->head; l != NULL; l=l->next){
        account *acnt = (account*)l->data;
        account_write_unlock(acnt);
    }
    rwlock_release_write((globals->account_lock));
}