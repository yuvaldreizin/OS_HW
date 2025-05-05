// signals.c
#include "signals.h"

void handleSIGINT(int sig) {
    // catch SIGINT (Ctrl+C) signal
    printf("\nCaught SIGINT (Ctrl+C). Signal number: %d\n", sig);
    if (globals->fgJob) {
        kill(globals->fgJob->pid, SIGKILL);
    }
    longjmp(env_buf, 1); // jump back to the point where setjmp was called
}

void handleSIGTSTP(int sig) {
    printf("\nCaught SIGTSTP (Ctrl+Z). Signal number: %d\n", sig);
    if (globals->fgJob){
        kill(globals->fgJob->pid, SIGSTOP);
        changeStatus(globals->fgJob, STOPPED); // change the status of the job to STOPPED
        globals->fgJob->pid = getpid(); // update the pid of the job
        addExistingJob(globals->fgJob);
        globals->fgJob = NULL; // clear the foreground job
        longjmp(env_buf, 1);
    }
}

void setupSignalHandlers() {
    struct sigaction saSIGINT;
    struct sigaction saSIGTSTP;

    // Setup SIGINT handler
    saSIGINT.sa_handler = handleSIGINT;
    sigemptyset(&saSIGINT.sa_mask);
    saSIGINT.sa_flags = 0;
    if (sigaction(SIGINT, &saSIGINT, NULL) == -1) {
        perror("Error setting up SIGINT handler");
        exit(EXIT_FAILURE);
    }

    // Setup SIGTSTP handler
    saSIGTSTP.sa_handler = handleSIGTSTP;
    sigemptyset(&saSIGTSTP.sa_mask);
    saSIGTSTP.sa_flags = 0;
    if (sigaction(SIGTSTP, &saSIGTSTP, NULL) == -1) {
        perror("Error setting up SIGTSTP handler");
        exit(EXIT_FAILURE);
    }
}

void sendSignal(int sig, unsigned int receiverJobID) {
    job_t job = jobLookup(receiverJobID);
    if (job == NULL) {
        fprintf(stderr, "Job with ID %d not found\n", receiverJobID);
        return;
    }
    if (kill(job->pid, sig) == -1) {
        perror("Failed to send signal");
    } else {
        printf("Signal %d sent to job with ID %d (PID %d)\n", sig, receiverJobID, job->pid);
    }
}