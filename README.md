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
