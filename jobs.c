// jobs.c
#define _POSIX_C_SOURCE 200809L
#include "jobs.h"

job_t initJob(char* curr_cmd, jobStatus curr_status, pid_t curr_pid){
    job_t new_job = MALLOC_VALIDATED(struct job, sizeof(struct job));
    new_job->user_input = MALLOC_VALIDATED(char, strlen(curr_cmd) + 1);
    strcpy(new_job->user_input, curr_cmd);
    new_job->status = curr_status;
    new_job->creationTime = time(NULL);
    new_job->pid = curr_pid;
    return new_job;
}

void destroyJob(job_t curr_job){
    if (curr_job) {
        free(curr_job->user_input);
        free(curr_job);
    }
}

jobList_t initJobList(){
    jobList_t newJobList = malloc(sizeof(struct jobList));
    if (!newJobList) {
        perror("Failed to initialize job list");
        exit(EXIT_FAILURE);
    }
    newJobList->count = 0;
    newJobList->nextID = 0;
    for (int i = 0; i < JOBS_NUM_MAX; i++) {
        newJobList->jobs[i] = NULL;
    }
    return newJobList;
}

void destroyJobList() {
    jobList_t globJobList = globals->jobList;
    for (int i = 0; i < JOBS_NUM_MAX && globJobList->count ; i++) {
        if (globJobList->jobs[i]) {
            destroyJob(globJobList->jobs[i]);
            globJobList->count--;
        }
    }
    free(globJobList);
}

int addNewJob(char* name, jobStatus curr_status, pid_t curr_pid){
    removeFinishedJobs(); // remove finished jobs before adding a new one
    jobList_t globJobList = globals->jobList;
    if (globJobList->count == JOBS_NUM_MAX) {
        return -1;
    }
    unsigned int ID = globJobList->nextID;
    job_t new_job = initJob(name, curr_status, curr_pid);
    globJobList->jobs[ID] = new_job;
    if(++globJobList->count != JOBS_NUM_MAX) {
        while(globJobList->jobs[ID]){
            ID++;
        }
        globJobList->nextID = ID;
    }
    return 0;
}

int addExistingJob(job_t curr_job){
    removeFinishedJobs(); // remove finished jobs before adding a new one
    jobList_t globJobList = globals->jobList;
    if (globJobList->count == JOBS_NUM_MAX) {
        // fprintf(stderr, "Job list is full\n");
        return -1;
    }
    unsigned int ID = globJobList->nextID;
    globJobList->jobs[ID] = curr_job;
    if(++globJobList->count != JOBS_NUM_MAX) {
        while (globJobList->jobs[ID]){
            ID++;
        }
        globJobList->nextID = ID;
    }
    return 0;
}

void removeJob(unsigned int ID){
    jobList_t globJobList = globals->jobList;
    destroyJob(globJobList->jobs[ID]);
    globJobList->jobs[ID] = NULL;
    globJobList->count--;
    if(ID < globJobList->nextID) {
        globJobList->nextID = ID;
    }
}

void removeToFg(unsigned int ID){
    // run only in fg()! otherwise might run over fgjob
    jobList_t globJobList = globals->jobList;
    globals->fgJob = globJobList->jobs[ID];
    globJobList->jobs[ID] = NULL;
    globJobList->count--;
    if(ID < globJobList->nextID) {
        globJobList->nextID = ID;
    }
}

jobStatus getStatus(job_t curr_job){
    return curr_job->status;
}

void changeStatus(job_t curr_job, jobStatus new){
    curr_job->status = new;
}

void removeFinishedJobs(){
    jobList_t globJobList = globals->jobList;
    for (int i = 0; i < JOBS_NUM_MAX; i++) {
        if (globJobList->jobs[i]){
            int wait_status;
            pid_t result = waitpid(globJobList->jobs[i]->pid, &wait_status, WNOHANG); //WNOHANG flag to avoid blocking
            
            if (result == -1) {// error occurred
                // perror("waitpid failed");
            } else if (result > 0 && (WIFEXITED(wait_status)|| WIFSIGNALED(wait_status))) { // job finished
                removeJob(i);

            } // else job finished
        }
    }
}

void printJobList(){
    removeFinishedJobs(); // remove finished jobs before printing
    jobList_t globJobList = globals->jobList;
    for (int i = 0; i < JOBS_NUM_MAX; i++) {
        if (globJobList->jobs[i]) {
            job_t curr_job = globJobList->jobs[i];
            int time_elapsed_sec = (int)difftime(time(NULL), curr_job->creationTime);
            printf("[%d] %s: %d %d secs%s\n", i, curr_job->user_input, curr_job->pid, time_elapsed_sec,(curr_job->status == STOPPED)? " (stopped)" : ""); //ASSUMPTION - if job isn't stopped we dont print the space after "secs"
        }
    }
}

job_t jobLookup(unsigned int ID){
    jobList_t globJobList = globals->jobList;
    if (ID < JOBS_NUM_MAX) {
        return globJobList->jobs[ID];
    }
    return NULL;
}

int jobPIDLookup(unsigned int PID){
    jobList_t globJobList = globals->jobList;
    for (int i = 0; i < JOBS_NUM_MAX; i++) {
        if (globJobList->jobs[i] && globJobList->jobs[i]->pid == PID) {
            return i;
        }
    }
    return -1;
}

int maxAvailableJobID(){
    if (globals->jobList->count == 0) {
        return -1; 
    }
    for (int i = JOBS_NUM_MAX - 1; i >= 0; i--) {
        if (jobLookup(i)){
            return i;
        }
    }
    return -1;
}

int maxStoppedJobID(){
    if (globals->jobList->count == 0) {
        return -1; 
    }
    for (int i = JOBS_NUM_MAX - 1; i >= 0; i--) {
        if (jobLookup(i) && jobLookup(i)->status == STOPPED){
            return i;
        }
    }
    return -1;
}