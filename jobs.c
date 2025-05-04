// jobs.c
#include "jobs.h"

job_t initJob(char* cmd, jobStatus status, pid_t pid){
    job_t job = MALLOC_VALIDATED(struct job, sizeof(struct job));
    job->cmd = MALLOC_VALIDATED(char, strlen(cmd) + 1);
    strcpy(job->cmd, cmd);
    job->status = status;
    job->creationTime = time(NULL);
    job->pid = pid;
    return job;
}

void destroyJob(job_t job){
    if (job) {
        free(job->cmd);
        free(job);
    }
}

jobList_t initJobList(){
    jobList_t jobList = malloc(sizeof(struct jobList));
    if (!jobList) {
        perror("Failed to initialize job list");
        exit(EXIT_FAILURE);
    }
    jobList->count = 0;
    jobList->nextID = 0;
    for (int i = 0; i < JOBS_NUM_MAX; i++) {
        jobList->jobs[i] = NULL;
    }
    return jobList;
}

void destroyJobList() {
    jobList_t jobList = globals->jobList;
    for (int i = 0; i < JOBS_NUM_MAX && jobList->count ; i++) {
        if (jobList->jobs[i]) {
            destroyJob(jobList->jobs[i]);
            jobList->count--;
        }
    }
    free(jobList);
}

int addNewJob(char* name, jobStatus status, pid_t pid){
    jobList_t jobList = globals->jobList;
    if (jobList->count == JOBS_NUM_MAX) {
        fprintf(stderr, "Job list is full\n");
        return -1;
    }
    unsigned int ID = jobList->nextID;
    job_t new_job = initJob(name, status, pid);
    jobList->jobs[ID] = new_job;
    if(++jobList->count != JOBS_NUM_MAX) {
        while (jobList->jobs[ID])
            ID++;
        jobList->nextID = ID;
    }
    return 0;
}

int addExistingJob(job_t job){
    jobList_t jobList = globals->jobList;
    if (jobList->count == JOBS_NUM_MAX) {
        fprintf(stderr, "Job list is full\n");
        return -1;
    }
    unsigned int ID = jobList->nextID;
    jobList->jobs[ID] = job;
    if(++jobList->count != JOBS_NUM_MAX) {
        while (jobList->jobs[ID])
            ID++;
        jobList->nextID = ID;
    }
    return 0;
}

void removeJob(unsigned int ID){
    jobList_t jobList = globals->jobList;
    destroyJob(jobList->jobs[ID]);
    jobList->jobs[ID] = NULL;
    jobList->count--;
    if(ID < jobList->nextID) {
        jobList->nextID = ID;
    }
}

jobStatus getStatus(job_t job){
    return job->status;
}

void changeStatus(job_t job, jobStatus new){
    job->status = new;
}

void removeFinishedJobs(){ //AMIR a function that removes finished jobs from the job list. need to activate it before any commant 
    jobList_t jobList = globals->jobList;
    for (int i = 0; i < JOBS_NUM_MAX; i++) {
        if (jobList->jobs[i] && jobList->jobs[i]->status == BACKGROUND){
            int status;
            pid_t result = waitpid(jobList->jobs[i]->pid, &status, WNOHANG); //WHOHANG flag to avoid blocking
            if (result == -1) // error occurred
                perror("waitpid failed");
            else if (result > 0) // job finished
                removeJob(i);
            //else - job still running, do nothing
        }
    }
}

void printJobList(){
    jobList_t jobList = globals->jobList;
    for (int i = 0; i < JOBS_NUM_MAX; i++) {
        if (jobList->jobs[i]) {
            job_t curr_job = jobList->jobs[i];
            int time_elapsed_sec = (int)difftime(time(NULL), curr_job->creationTime);
            printf("[%d] %s: %d %d secs%s\n", i, curr_job->cmd, curr_job->pid, time_elapsed_sec,(curr_job->status == STOPPED)? " stopped" : ""); //ASSUMPTION - if job isn't stopped we dont print the space after "secs"
        }
    }
}

job_t jobLookup(unsigned int ID){
    jobList_t jobList = globals->jobList;
    if (ID < JOBS_NUM_MAX) {
        return jobList->jobs[ID];
    }
    return NULL;
}

int jobPIDLookup(unsigned int PID){
    jobList_t jobList = globals->jobList;
    for (int i = 0; i < JOBS_NUM_MAX; i++) {
        if (jobList->jobs[i] && jobList->jobs[i]->pid == PID) {
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