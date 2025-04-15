#ifndef JOBS_H
#define JOBS_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include "commands.h"
#define JOBS_NUM_MAX 100

/**
 * @enum jobStatus
 * @brief Represents the status of a job.
 */
typedef enum {
    FOREGROUND = 1, /**< Job is running in the foreground. */
    BACKGROUND,     /**< Job is running in the background. */
    STOPPED         /**< Job is stopped. */
} jobStatus;

/**
 * @struct job
 * @brief Represents a single job.
 */
struct job {
    unsigned int ID;         /**< Unique identifier for the job. */
    char* cmd;               /**< Command associated with the job. */
    jobStatus status;        /**< Current status of the job. */
    time_t creationTime;     /**< Time when the job was created. */
    unsigned int pid;        /**< Process ID of the job. */
};

typedef struct job* job_t;

/**
 * @struct jobList
 * @brief Represents a list of jobs.
 */
struct jobList {
    job_t jobs[JOBS_NUM_MAX]; /**< Array of job pointers. */
    unsigned int count;       /**< Number of jobs currently in the list. */
    unsigned int nextID;      /**< Next available job ID. */
};

typedef struct jobList* jobList_t;

/*=============================================================================
* global functions
=============================================================================*/

/**
 * @brief Initializes a new job.
 * 
 * @param ID The unique identifier for the job.
 * @param cmd The command associated with the job.
 * @param status The initial status of the job.
 * @return A pointer to the initialized job.
 */
job_t initJob(int ID, char* cmd, jobStatus status);

/**
 * @brief Destroys a job and frees its resources.
 * 
 * @param job The job to destroy.
 */
void destroyJob(job_t job);

/**
 * @brief Gets the status of a job.
 * 
 * @param job The job whose status is to be retrieved.
 * @return The status of the job.
 */
jobStatus getStatus(job_t job);

/**
 * @brief Changes the status of a job.
 * 
 * @param job The job whose status is to be changed.
 * @param new The new status to set.
 */
void chnageStatus(job_t job, jobStatus new);

/**
 * @brief Initializes a new job list.
 * 
 * @return A pointer to the initialized job list.
 */
jobList_t initJobList();

/**
 * @brief Destroys a job list and frees its resources.
 * 
 * @param jobList The job list to destroy.
 */
void destroyJobList(jobList_t jobList);

/**
 * @brief Adds a new job to the job list.
 * 
 * @param jobList The job list to which the job will be added.
 * @param name The name/command of the job.
 * @param status The initial status of the job.
 */
void addJob(jobList_t jobList, char* name, jobStatus status);

/**
 * @brief Removes a job from the job list by its ID.
 * 
 * @param jobList The job list from which the job will be removed.
 * @param ID The ID of the job to remove.
 */
void removeJob(jobList_t jobList, unsigned int ID);

/**
 * @brief Removes all finished jobs from the job list.
 * 
 * @param jobList The job list to clean up.
 */
void removeFinishedJobs(jobList_t jobList);

/**
 * @brief Prints the list of jobs.
 * 
 * @param jobList The job list to print.
 */
void printJobList(jobList_t jobList);

#endif //__JOBS_H__