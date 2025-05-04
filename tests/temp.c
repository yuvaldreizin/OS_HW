// #include <stdio.h>
// #include <stdlib.h>
// #include <signal.h>
// #include <setjmp.h>
// #include <unistd.h>

// #define N 100000000

// sigjmp_buf env;

// void handleSIGINT(int sig) {
//     printf("\nCaught SIGINT (Ctrl+C). Signal number: %d\n", sig);
//     siglongjmp(env, 1); // Jump back to the point where sigsetjmp was called
// }

// void handleSIGTSTP(int sig) {
//     printf("\nCaught SIGTSTP (Ctrl+Z). Signal number: %d\n", sig);
//     // Let the process stop, it will resume later and trigger SIGCONT
// }

// void handleSIGCONT(int sig) {
//     printf("\nCaught SIGCONT (process resumed). Signal number: %d\n", sig);
//     siglongjmp(env, 1);
// }

// void setupSignalHandlers() {
//     struct sigaction sa;

//     // SIGINT
//     sa.sa_handler = handleSIGINT;
//     sigemptyset(&sa.sa_mask);
//     sa.sa_flags = 0;
//     if (sigaction(SIGINT, &sa, NULL) == -1) {
//         perror("Error setting up SIGINT handler");
//         exit(EXIT_FAILURE);
//     }

//     // SIGTSTP
//     sa.sa_handler = handleSIGTSTP;
//     sigemptyset(&sa.sa_mask);
//     sa.sa_flags = 0;
//     if (sigaction(SIGTSTP, &sa, NULL) == -1) {
//         perror("Error setting up SIGTSTP handler");
//         exit(EXIT_FAILURE);
//     }

//     // SIGCONT
//     sa.sa_handler = handleSIGCONT;
//     sigemptyset(&sa.sa_mask);
//     sa.sa_flags = 0;
//     if (sigaction(SIGCONT, &sa, NULL) == -1) {
//         perror("Error setting up SIGCONT handler");
//         exit(EXIT_FAILURE);
//     }
// }

// int main() {
//     setupSignalHandlers();
//     printf("start here\n");

//     sigsetjmp(env, 1);
//     printf("jump here\n");

//     for (int i = -N; i < N; i++) {
//         for (int j = -N; j < N; j++) {
//             // Placeholder for computation
//             if (i * i + j * j == N) {
//                 // Do something
//             }
//         }
//     }

//     return 0;
// }
