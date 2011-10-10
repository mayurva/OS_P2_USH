#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

#include"ush.h"
#include"parse.h"

char **environ;

int main()
{
	Pipe cmd_line;
	int pipe_ref = 1;
	int exit_flag;
	int pid,subshell_pid;
	int status;
	initShell();
//	execrc();
	while(1)
	{
		printPrompt();
		cmd_line = parse();
		if(cmd_line == NULL) // no input
			continue;
		if(strcmp(cmd_line -> head -> args[0],"end")==0) // done with ushrc processing
		{
			setupPrompt();	
			continue;
		}
	
		Pipe pipe_line = cmd_line;
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
				printf("Command in pipe\n");
				e = c;
				while(e->next != d)
					e=e->next;
				d = e;
				if(c != d)
				{	
					pipe_ref = !pipe_ref;
					createPipe(pipe_ref);
					subshell_pid = fork();
					if(subshell_pid) //This is the parent
					{
						setPipeRedirect(pipe_ref,fileno(stdin));						
						waitpid(subshell_pid,&status,0);
						d = c;
						break;
					}
					else
					{
						setPipeRedirect(pipe_ref,fileno(stdout));
						exit_flag = 1;
					}
				}

			}
			setupRedirect(e);
			if(isBuiltinCmd(e->args[0]) != -1)
				execCmd(e);
			else
			{
				char path[MAXLEN];
				strcpy(path,e->args[0]);
				if(strstr(e->args[0],"/")==NULL) //this is the path case
				{
					strcpy(path,"/bin/");
					strcat(path,e->args[0]);	
					strcpy(e->args[0],path);
				}

				if(access(path,F_OK) !=0)
				{
					resetStreams();
					pipe_line = pipe_line -> next;
					printf("Command not found\n");
					continue;
				}				
							
				if(access(path,R_OK|X_OK) !=0 || isdir(path))
				{
					resetStreams();
					pipe_line = pipe_line -> next;
					printf("Permission denied\n");
					continue;
				}

				printf("Before fork()\n");
				pid = fork();
				if(!pid)
       		                {
					printf("Executing the command %s\n",e->args[0]);
					execv(e->args[0],e->args);
					printf("HERE EXECUTION COMPLETED\n");
					exit(0);
				}
				else
				{
					printf("pid is %d\n",pid);
					if(pid)
						waitpid(pid,&status,0);
				}	
			}

			printf("Exit flag is: %d\n",exit_flag);
			if(exit_flag)
				exit(0);
			resetStreams();
			printf("Streams Reset\n");
			pipe_line = pipe_line -> next;
		}
	}
}
