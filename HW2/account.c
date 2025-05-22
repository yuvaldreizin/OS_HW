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
    // no locks because id doesn't change unless deleted + global account list is locked
    if ((account *)id1->id < (account *)id2->id) return -1;
    else if ((account *)id1->id > (account *)id2->id) return 1;
    else return 0;
}

account *account_check_id_read(int id){ 
    rwlock_acquire_read(&(globals->account_lock));
    for (int i = 0; i < globals->num_accounts; i++){
        account *acnt = (account*)g_list_nth_data(globals->accounts, i);
        if(account_get_id(account) == id){
            // lock account for function and release global list
            account_read_lock(acnt);
            rwlock_release_read(&(globals->account_lock));
            return acnt;
        }
    }
    rwlock_release_read(&(globals->account_lock));
    return NULL;
}

account *account_check_id_and_pass_read(int id, int pass, int atm_id){
    account *acnt = account_check_id_read(id);
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
        account_read_unlock(acnt);
        return NULL;
    }
    return acnt;
}


account *account_check_id_write(int id){ 
    rwlock_acquire_read(&(globals->account_lock));
    for (int i = 0; i < globals->num_accounts; i++){
        account *acnt = (account*)g_list_nth_data(globals->accounts, i);
        if(account_get_id(account) == id){
            // lock account for function and release global list
            account_write_lock(acnt);
            rwlock_release_read(&(globals->account_lock));
            return acnt;
        }
    }
    rwlock_release_read(&(globals->account_lock));
    return NULL;
}

account *account_check_id_and_pass_write(int id, int pass, int atm_id){
    account *acnt = account_check_id_write(id);
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
        log_unlock();
        account_read_unlock(check_acnt);
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
    account *acnt = account_check_id_and_pass_write(id, pass, atm_id);
    if (!acnt) return FAILURE;
    // add amount
    // TODO - check if amount > 0 ?
    acnt->balance += amount;
    int balance = avnt->balance;
    account_write_unlock(acnt);
    // write to log
    log_lock();
    fprintf(globals->log_file, "%s: Account %d new balance is %d after %d $ was deposited\n", atm_id, id, balance, amount);
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
        log_unlock();
        account_write_unlock(acnt);
        return FAILURE;
    }
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
    account *acnt = account_check_id_and_pass_read(id, pass, atm_id);
    if (!acnt) return FAILURE;
    // check balance
    int balance = acnt->balance;
    account_read_unlock(acnt);
    // write to log
    log_lock();
    fprintf(globals->log_file, "%d: Account %d balance is %d\n", atm_id, id, balance);
    log_unlock();
    return SUCCESS;
}


f_status_t account_q(int id, int pass, int atm_id){
    account *acnt = account_check_id_and_pass_write(id, pass, atm_id);
    if (!acnt) return FAILURE;
    // delete account
    int balance = account_get_balance(acnt);
    rwlock_acquire_write(&(globals->account_lock));
    g_list_remove(globals->accounts, acnt);
    globals->num_accounts--;
    rwlock_release_write(&(globals->account_lock));
    account_write_unlock(acnt);
    account_free(acnt);
    // write to log
    log_lock();
    fprintf(globals->log_file, "%d: Account %d is now closed. Balance was %d\n", atm_id, id, balance);
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
    int balance = acnt->balance;
    to_acnt->balance += amount;
    int to_balance = to_acnt->balance;
    account_write_unlock(to_acnt);
    account_write_unlock(acnt);
    // write to log
    log_lock();
    fprintf(globals->log_file, "%d: Transfer <amount> from account <source account> to account <target account> new account balance is <source account balance> new target account balance is <target account balance>\n",
        atm_id, amount, id, to_id, balance, to_balance);
    log_unlock();
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

// void lock_read_all_accounts(){
//     rwlock_acquire_read(&(globals->account_lock));
//     for (int i = 0; i < globals->num_accounts; i++){
//         account *acnt = (account*)g_list_nth_data(globals->accounts, i);
//         account_read_lock(acnt);
//     }
// }

// void unlock_read_all_accounts(){
//     for (int i = 0; i < globals->num_accounts; i++){
//         account *acnt = (account*)g_list_nth_data(globals->accounts, i);
//         account_read_unlock(acnt);
//     }
//     rwlock_release_read(&(globals->account_lock));
// }

// void lock_write_all_accounts(){
//     rwlock_acquire_write(&(globals->account_lock));
//     for (int i = 0; i < globals->num_accounts; i++){
//         account *acnt = (account*)g_list_nth_data(globals->accounts, i);
//         account_write_lock(acnt);
//     }
// }

// void unlock_write_all_accounts(){
//     for (int i = 0; i < globals->num_accounts; i++){
//         account *acnt = (account*)g_list_nth_data(globals->accounts, i);
//         account_write_unlock(acnt);
//     }
//     rwlock_release_write(&(globals->account_lock));
// }


void lock_all_accounts(){
    rwlock_acquire_write(&(globals->account_lock));
    for (int i = 0; i < globals->num_accounts; i++){
        account *acnt = (account*)g_list_nth_data(globals->accounts, i);
        account_write_lock(acnt);
    }
}

void unlock_all_accounts(){
    for (int i = 0; i < globals->num_accounts; i++){
        account *acnt = (account*)g_list_nth_data(globals->accounts, i);
        account_write_unlock(acnt);
    }
    rwlock_release_write(&(globals->account_lock));
}