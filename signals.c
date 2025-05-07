// signals.c
#include "signals.h"
#define _GNU_SOURCE

void handleSIGINT(int sig) {
    // catch SIGINT (Ctrl+C) signal
    printf("smash: caught CTRL+C\n");
    if(getpid() == globals->smash_pid) {
        longjmp(env_buf, 1); // jump back to the point where setjmp was called
    } else {
        printf("smash: process %d was killed\n", globals->fgJob->pid); 
        destroyJob(globals->fgJob); // destroy the foreground job
        globals->fgJob = NULL; // clear the foreground job
        exit(0);
    }
}

void handleSIGTSTP(int sig) {
    // catch SIGTSTP (Ctrl+Z) signal
    if(getpid() == globals->smash_pid) {
        printf("smash: caught CTRL+Z\n");
        if (globals->fgJob) { // no command in FOREGROUND
            job_t curr_job = globals->fgJob;
            changeStatus(curr_job, STOPPED); // change the status of the job to STOPPED
            addExistingJob(curr_job);
            globals->fgJob = NULL; // clear the foreground job
            kill(curr_job->pid, SIGSTOP); // send SIGSTOP to the job
            printf("smash: process %d was stopped\n", curr_job->pid); 
        }
        longjmp(env_buf, 1); // jump back to the point where setjmp was called
    }
    else {
        printf("Error: ctrl+z signal received in child process\n");
    }
}

void setupSignalHandlers() {
    struct sigaction saSIGINT;
    struct sigaction saSIGTSTP;

    // prevent printing ^Z and ^C
    // struct termios term;
    // tcgetattr(STDIN_FILENO, &term);
    // term.c_lflag &= ~ECHOCTL;
    // tcsetattr(STDIN_FILENO, TCSANOW, &term);

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

int sendSignal(int sig, unsigned int receiverJobID) {
    job_t job = jobLookup(receiverJobID);
    if (!job) {
        printf("Something went wrong!!!!! signals::sendSignal\n");
        return 2;
    }
    if (kill(job->pid, sig) == -1) {
		printf("smash error: kill: invalid arguments\n");
        return 2;
    }
    printf("Signal %d sent to job with ID %d (PID %d)\n", sig, receiverJobID, job->pid);
    return 0;
}