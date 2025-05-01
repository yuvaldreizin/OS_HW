- Yuval's commit 15.04
    Added jobs.c/h with job and job-list structs and basic functions.
    Added 2 important functions - removeFinishedJobs which checks if the processes finished (with its pid) and printJobList for the jobs command
    git grep AMIR for points I wanted you to notice
    git grep ASSUMPTION for things i wasnt sure about - we need to ask recitatior
    I used copilot to add doxygen docuemation - I think it's nice and help for our understanding of each other. i just told him "add doxygen documentaion in the .h file"

- Amir's commit 17.4
    added cmd struct and a few commands.
    important function -  parseCmd() gets cmd pointer and returns a filled cmd (command and args) with memory allocation
                          destory cmd()
                          run_cmd() calls for the relevant command function and return if the command is internal or external
    added 1 ASSUMPTION about max path length.
    getppid causes compilation error, not yet fixed.
    missing more commands and error handling - will do it in the weekend.


- Yuval's commit 25.04
    added support for your requests in jobs module.
    added signal module - 
        * handlers for Ctrl C/Z
        * sendSignal function
    reffered to the global variables - edited functions to fit.
    added utils.h to avoid include loop.

- Yuval's commit 30.04
    fixed compilation bugs.
    reorganized structs for compilation.
    added smash_status and fgJobt in globals for signal handling.
