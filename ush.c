#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include"ush.h"

char prompt[MAXLEN];
char command[MAXLEN];

int main()
{
	int pid;
	int status;
	initShell();
//	strcpy(prompt,"% ");
	while(1)
	{
		printf("%s",prompt);
		gets(command);
//		printf("%s\n",command);
		if(!strcmp(command,"logout"))
			exit(0);
		else
		{
			pid = fork();
			if(!pid)
			{
				char run_cmd[MAXLEN] = "/bin/";
				char *cmd_param[2];
				cmd_param[0] = malloc(MAXLEN*sizeof(char));
				strcpy(cmd_param[0],command);	
				cmd_param[1] = NULL;
				strcat(run_cmd,command);
			//	printf("Before Execv\n");
				execv(run_cmd,cmd_param);
			//	printf("Execution completed\n");
				exit(0);
			}
			else
			{
				waitpid(pid,&status,0);
			}
		}
	}
}
