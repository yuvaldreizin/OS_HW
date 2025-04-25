#ifndef SIGNALS_H
#define SIGNALS_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

/*=============================================================================
* global functions
=============================================================================*/

/**
 * @brief Handler for SIGINT (Ctrl+C).
 * 
 * @param sig The signal number.
 */
void handleSIGINT(int sig);

/**
 * @brief Handler for SIGTSTP (Ctrl+Z).
 * 
 * @param sig The signal number.
 */
void handleSIGTSTP(int sig);

/**
 * @brief Sets up signal handlers for SIGINT and SIGTSTP.
 */
void setupSignalHandlers();

void sendSignal(int sig, unsigned int recieverJobID);
#endif //__SIGNALS_H__