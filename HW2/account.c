#define _POSIX_C_SOURCE 200809L
#include "account.h"

account_with_id *account_init(int id, int pass, int balance){
    account_with_id *acnt_with_id = MALLOC_VALIDATED(account_with_id, sizeof(account_with_id));
    acnt_with_id->id = id;
    acnt_with_id->acc = MALLOC_VALIDATED(account, sizeof(account));
    account *acnt = acnt_with_id->acc; 
    acnt->id = id;
    acnt->pass = pass;
    acnt->balance = balance;
    acnt->lock = rwlock_init();
    return acnt_with_id;
}


void account_free(void* acc){
    if (acc == NULL) return;
    account_with_id * acnt_with_id = (account_with_id *)acc;
    rwlock_destroy((acnt_with_id->acc->lock));
    free(acnt_with_id->acc);
    free(acnt_with_id);
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
    account_with_id *acnt1_casted = (account_with_id *)acnt1;
    account_with_id *acnt2_casted = (account_with_id *)acnt2;
    if (acnt1_casted->id < acnt2_casted->id) return -1;
    else if (acnt1_casted->id > acnt2_casted->id) return 1;
    else return 0;
}

account *account_check_id_read(int id){ 
    rwlock_acquire_read((globals->account_lock));
    Node *l;
    for (l = globals->accounts->head; l != NULL; l=l->next){
        account_with_id *acnt = (account_with_id*)l->data;
        if(acnt->id == id){
            // lock account for function and release global list
            account_read_lock(acnt->acc);
            rwlock_release_read((globals->account_lock));
            return acnt->acc;
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


account* account_check_id_write(int id){ 
    rwlock_acquire_read((globals->account_lock));
    Node *l;
    for (l = globals->accounts->head; l != NULL; l=l->next){
        account_with_id *acnt = (account_with_id*)l->data;
        if(acnt->id == id){
            // lock account for function and release global list
            account_write_lock(acnt->acc);
            rwlock_release_read((globals->account_lock));
            return acnt->acc;
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
        account_read_unlock(check_acnt);
        log_lock();
        fprintf(globals->log_file, "Error %d: Your transaction failed - account with the same id exists\n", atm_id);
        fflush(globals->log_file); 
        log_unlock();
        return INVALID_ID;
    }
    // add account
    rwlock_acquire_write((globals->account_lock));
    account_with_id *acnt = account_init(id, pass, initial_amount);
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
    // add amount
    // TODO - check if amount > 0 ?
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
    // check if account exists
    rwlock_acquire_write((globals->account_lock));
    account_with_id *acnt;
    Node *l;
    int found = 0;
    for (l = globals->accounts->head; l != NULL; l=l->next){
        acnt = (account_with_id*)l->data;
        if(acnt->id == id){
            account_write_lock(acnt->acc);
            found = 1;
            break;
        }
    }
    if (!found){
        rwlock_release_write((globals->account_lock));
        log_lock();
        fprintf(globals->log_file, "Error %d: Your transaction failed - account id %d does not exist\n", atm_id, id);
        fflush(globals->log_file);
        log_unlock();
        return FAILURE;
    }
    // check for password
    if (acnt->acc->pass != pass){
        rwlock_release_write((globals->account_lock));
        log_lock();
        fprintf(globals->log_file, "Error %d: Your transaction failed - password for account id %d is incorrect\n", atm_id, id);
        fflush(globals->log_file);
        log_unlock();
        account_write_unlock(acnt->acc);
        return FAILURE;
    }
    // delete account
    int balance = acnt->acc->balance;
    linked_list_remove(globals->accounts, acnt);
    rwlock_release_write((globals->account_lock));
    account_write_unlock(acnt->acc);
    account_free(acnt);
    // write to log
    log_lock();
    fprintf(globals->log_file, "%d: Account %d is now closed. Balance was %d\n", atm_id, id, balance);
    fflush(globals->log_file);
    log_unlock();
    return SUCCESS;
}


f_status_t account_t(int id, int pass, int amount, int to_id, int atm_id){
    account *from_acnt = NULL;
    account *to_acnt = NULL;
    rwlock_acquire_read((globals->account_lock));
    Node *l;
    for (l = globals->accounts->head; l != NULL; l=l->next){
        account_with_id *acnt = (account_with_id*)l->data;
        if(acnt->id == id){
            // lock account for function and release global list
            account_write_lock(acnt->acc);
            from_acnt = acnt->acc;
        }
        if(acnt->id == to_id){
            // lock account for function and release global list
            account_write_lock(acnt->acc);
            to_acnt = acnt->acc;
        }
    }
    rwlock_release_read((globals->account_lock));  
    
    // check id
    if (!from_acnt){
        log_lock();
        fprintf(globals->log_file, "Error %d: Your transaction failed - account id %d does not exist\n", atm_id, id);
        fflush(globals->log_file);
        log_unlock();
        if (to_acnt) account_write_unlock(to_acnt);
        return FAILURE;
    }
    if (!to_acnt){
        log_lock();
        fprintf(globals->log_file, "Error %d: Your transaction failed - account id %d does not exist\n", atm_id, to_id);
        fflush(globals->log_file);
        log_unlock();
        if (from_acnt) account_write_unlock(from_acnt);
        return FAILURE;
    }
    // check pass
    if (from_acnt->pass != pass){
        log_lock();
        fprintf(globals->log_file, "Error %d: Your transaction failed - password for account id %d is incorrect\n", atm_id, id);
        fflush(globals->log_file);
        log_unlock();
        account_write_unlock(from_acnt);
        if (to_acnt) account_write_unlock(to_acnt);
        return FAILURE;
    }
    // check balance
    int balance = from_acnt->balance;
    if (balance < amount){
        log_lock();
        fprintf(globals->log_file, "Error %d: Your transaction failed - account id %d balance is lower than %d\n", atm_id, id, amount);
        fflush(globals->log_file);
        log_unlock();
        account_write_unlock(from_acnt);
        if (to_acnt) account_write_unlock(to_acnt);
        return FAILURE;
    }
    
    from_acnt->balance -= amount;
    balance -= amount;
    to_acnt->balance += amount;
    int to_balance = to_acnt->balance;
    account_write_unlock(to_acnt);
    account_write_unlock(from_acnt);
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
        account *acnt = ((account_with_id*)l->data)->acc;
        account_write_lock(acnt);
    }
}

void unlock_all_accounts(){
    Node *l;
    for (l = globals->accounts->head; l != NULL; l=l->next){
        account *acnt = ((account_with_id*)l->data)->acc;
        account_write_unlock(acnt);
    }
    rwlock_release_write((globals->account_lock));
}