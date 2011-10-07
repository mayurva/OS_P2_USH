#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

#include"ush.h"
#include"parse.h"

int main()
{
	Pipe cmd_line;
	int pipefd[2][2];
	int pipe_ref = 1;
	int exit_flag = 0;
	int pid=0;
	int status;
	int i;
	char message[MAXLEN];

	initShell();
	while(1)
	{
		printPrompt(message);
		cmd_line = parse();
//    		prPipe(command);
		if(!strcmp(cmd_line->head->args[0],"logout"))
			exit(0);
		else
		{
			Pipe pipe_line = cmd_line;
			
			while(pipe_line)
			{
				printf("Entered a pipe line\n");
				Cmd c = pipe_line -> head;
				Cmd d = NULL;
				Cmd e;
				while(c!=d)
				{
					printf("Command in pipe\n");
					e = c;
					while(e->next != d)
						e=e->next;
					d = e;
					if(c != d)
					{	
						pipe_ref = !pipe_ref;
						pipe(pipefd[pipe_ref]);				
						printf("pipe %d created \n",pipe_ref);
						pid = fork();
						if(pid) //This is the parent
						{
							close(pipefd[pipe_ref][1]);
							dup2(0,pipefd[pipe_ref][0]);
							waitpid(pid,&status,0);
						}
						else
						{
							close(pipefd[pipe_ref][0]);
							dup2(1,pipefd[pipe_ref][1]);
							exit_flag = 1;
						}
					}
				}
	
				printf("Executing the command\n");
				char run_cmd[MAXLEN] = "/bin/";
				strcat(run_cmd,e->args[0]);
				execv(run_cmd,e->args);

				printf("Execution completed\n");
				printf("pid is %d\n",pid);
				if(pid)
					waitpid(pid,&status,0);
				printf("Exit flag is: %d\n",exit_flag);
				if(exit_flag)
					exit(0);
				
				pipe_line = pipe_line -> next;
			}
				
		}
	printf("Last line of while  1\n");
	}
}
