#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

#include"ush.h"
#include"parse.h"


int main()
{
	Pipe cmd_line;
	char *invalid_cmd[] = {"","Command not found","Permission denied"};
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
				char cmd_path[MAXLEN],path1[MAXLEN];
				strcpy(cmd_path,e->args[0]);
				if(strstr(e->args[0],"/")==NULL) //this is the path case
				{
					char *needle;
					strcpy(path1,path);
					needle = strtok(path1,":");
					while(needle != NULL)
					{
						strcpy(cmd_path,needle);
						strcat(cmd_path,"/");
						strcat(cmd_path,e->args[0]);
						if(!iscmd(cmd_path))
							break;
						needle = strtok(NULL,":");
					}
					printf("cmd_path is %s\n",cmd_path);
					strcpy(e->args[0],cmd_path);
				}

				printf("command is %s\n",e->args[0]);
				int r = iscmd(e->args[0]);				
				if(r)
				{
					resetStreams();
					pipe_line = pipe_line -> next;
					printf("%s\n",invalid_cmd[r]);
					continue;
				}
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
