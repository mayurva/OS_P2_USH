#ifndef USH
#define USH

#define TRUE 1
#define FALSE 0

#define MAXLEN 500
#define NO_OF_CMDS 8

extern int old_stdin;
extern int old_stdout;
extern int old_stderr;
extern char message[MAXLEN];
extern int pipefd[2][2];
extern int rc_processing;
extern char *home;
extern char *path;
#endif

