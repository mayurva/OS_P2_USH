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
	int inpipe,outpipe;
	int temp;
	int exit_flag;
	int pid,subshell_pid;
	int status;
	char message[MAXLEN];
	strcpy(message,"");

	initShell();
	while(1)
	{
		printPrompt(message);
		dup2(fileno(stdin),temp);
//		cmd_line = NULL;
		fflush(stdout);
		cmd_line = parse();
		if(cmd_line == NULL)
			continue;
//    		prPipe(command);
		if(!strcmp(cmd_line->head->args[0],"logout"))
			exit(0);
		else
		{
			Pipe pipe_line = cmd_line;
			outpipe = 0;	
			while(pipe_line)
			{
				
				subshell_pid = pid = 0;
				exit_flag = 0;
				
				printf("Entered a pipe line\n");
				Cmd c = pipe_line -> head;
				Cmd d = NULL;
				Cmd e;
				while(c!=d)
				{
					inpipe = 0;
					printf("Command in pipe\n");
					e = c;
					while(e->next != d)
						e=e->next;
					d = e;
					if(c != d)
					{	
						pipe_ref = !pipe_ref;
						if (pipe(pipefd[pipe_ref]) == -1)
						{
							printf("Pipe creation failed\n");
							exit (-1);
						}				
						printf("pipe %d created \n",pipe_ref);
						subshell_pid = fork();
						if(subshell_pid) //This is the parent
						{
							inpipe = 1;
							waitpid(subshell_pid,&status,0);
							d = c;
							break;
						}
						else
						{
							outpipe = 1;
							exit_flag = 1;
						}
					}
					else
					{
						printf("HERE IN ELSE\n");
						break;
					}
				}
				pid = fork();
	                        if(!pid)
        	                {
					if(inpipe)
					{
						close(pipefd[pipe_ref][1]);
						dup2(pipefd[pipe_ref][0],fileno(stdin));
						close(pipefd[pipe_ref][0]);
						printf("Parent pipe ref is %d\n",pipe_ref);
					}
					
					printf("Outpipe is %d\n",outpipe);
					if(outpipe)
					{
						close(pipefd[pipe_ref][0]);
						dup2(pipefd[pipe_ref][1],fileno(stdout));
						close(pipefd[pipe_ref][1]);
						printf("Child pipe ref is %d\n",pipe_ref);
					}

					printf("Executing the command %s\n",e->args[0]);
					char run_cmd[MAXLEN] = "/bin/";
					strcat(run_cmd,e->args[0]);
					execv(run_cmd,e->args);
					printf("HERE EXECUTION COMPLETED\n");
					exit(0);
				}
				else
				{
					printf("pid is %d\n",pid);
					if(pid)
						waitpid(pid,&status,0);
					printf("Exit flag is: %d\n",exit_flag);
					if(exit_flag)
						exit(0);
					else
						dup2(temp,fileno(stdin));
				}

				pipe_line = pipe_line -> next;
				if(pipe_line == NULL)
				{
					printf("Here in if\n");
					break;
				}
			}
				
		}
	printf("Last line of while  1\n");
	}
}
