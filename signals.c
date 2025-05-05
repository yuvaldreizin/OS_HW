// signals.c
#include "signals.h"

void handleSIGINT(int sig) {
    // catch SIGINT (Ctrl+C) signal
    if(getpid() == globals->smash_pid) {
        longjmp(env_buf, 1); // jump back to the point where setjmp was called
    } else {
        destroyJob(globals->fgJob); // destroy the foreground job
        globals->fgJob = NULL; // clear the foreground job
        exit(0);
    }
}

void handleSIGTSTP(int sig) {
    if(getpid() == globals->smash_pid) {
        job_t curr_job = globals->fgJob;
        changeStatus(curr_job, STOPPED); // change the status of the job to STOPPED
        addExistingJob(curr_job);
        globals->fgJob = NULL; // clear the foreground job
        kill(curr_job->pid, SIGSTOP); // send SIGSTOP to the job
        longjmp(env_buf, 1); // jump back to the point where setjmp was called
    }
    else {
        printf("ERORORROROROROROROROROROROORORORORORORROORROOROOROROOROROORORORORORROOROROROOROROROOROR");
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