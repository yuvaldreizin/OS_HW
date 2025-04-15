// jobs.c
#include "jobs.h"

job_t initJob(int ID, char* cmd, jobStatus status){
    job_t job = MALLOC_VALIDATED(struct job, sizeof(struct job));
    job->cmd = MALLOC_VALIDATED(char, strlen(cmd) + 1);
    strcpy(job->cmd, cmd);
    job->ID = ID;
    job->status = status;
    job->creationTime = time(NULL);
    return job;
    //AMIR need to add job's PID for a check if the job is finished or not. I think that the proper place is smash.c (where the fork happens)
    //maybe gedpid()?
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

void destroyJobList(jobList_t jobList) {
    for (int i = 0; i < JOBS_NUM_MAX && jobList->count ; i++) {
        if (jobList->jobs[i]) {
            destroyJob(jobList->jobs[i]);
            jobList->count--;
        }
    }
    free(jobList);
}

void addJob(jobList_t jobList, char* name, jobStatus status){
    if (jobList->count == JOBS_NUM_MAX) {
        fprintf(stderr, "Job list is full\n");
        return;
    }
    unsigned int ID = jobList->nextID;
    job_t new_job = initJob(ID, name, status);
    jobList->jobs[ID] = new_job;
    if(++jobList->count != JOBS_NUM_MAX) {
        while (jobList->jobs[ID])
            ID++;
        jobList->nextID = ID;
    }
}

void removeJob(jobList_t jobList, unsigned int ID){
    destroyJob(jobList->jobs[ID]);
    jobList->jobs[ID] = NULL;
    jobList->count--;
    if(ID < jobList->nextID) {
        jobList->nextID = ID;
    }
}

void removeFinishedJobs(jobList_t jobList){ //AMIR a fo=unction that removes finished jobs from the job list. need to activate it before any commant 
    for (int i = 0; i < JOBS_NUM_MAX; i++) {
        if (jobList->jobs[i] && jobList->jobs[i]->status == BACKGROUND){
            int status;
            pid_t result = waitpid(jobList->jobs[i]->pid, &status, WNOHANG); //WHOHANG flag to avoid blocking
            if (result == -1) // error occurred
                perror("waitpid failed");
            if (result > 0) // job finished
                removeJob(jobList, i);
        }
    }
}

void printJobList(jobList_t jobList){
    for (int i = 0; i < JOBS_NUM_MAX; i++) {
        if (jobList->jobs[i]) {
            job_t curr_job = jobList->jobs[i];
            int time_elapsed_sec = (int)difftime(time(NULL), curr_job->creationTime);
            printf("[%d] %s: %d %d secs%s\n", curr_job->ID, curr_job->cmd, curr_job->pid, time_elapsed_sec,(curr_job->status == STOPPED)? " stopped" : ""); //ASSUMPTION - if job isn't stopped we dont print the space after "secs"
        }
    }
}