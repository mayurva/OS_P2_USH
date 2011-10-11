#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

#include"ush.h"
#include"parse.h"
int exit_flag;
int pipe_ref;
char *invalid_cmd[] = {"","Command not found","Permission denied"};
int pid,status;
int prompt_flag;

void execute_command(Cmd c)
{
	setupRedirect(c);
	if(isBuiltinCmd(c->args[0]) != -1)
		execCmd(c);
	else
	{
		char cmd_path[MAXLEN],path1[MAXLEN];
		strcpy(cmd_path,c->args[0]);
		if(strstr(c->args[0],"/")==NULL) //this is the path case
		{
			char *needle;
			strcpy(path1,path);
			needle = strtok(path1,":");
			while(needle != NULL)
			{
				strcpy(cmd_path,needle);
				strcat(cmd_path,"/");
				strcat(cmd_path,c->args[0]);
				if(!iscmd(cmd_path))
					break;
				needle = strtok(NULL,":");
			}
		#ifdef DEBUG
			printf("cmd_path is %s\n",cmd_path);
		#endif
			strcpy(c->args[0],cmd_path);
		}
	#ifdef DEBUG
		printf("command is %s\n",c->args[0]);
	#endif
		int r = iscmd(c->args[0]);				
		if(r)
		{
			fprintf(stderr,"%s\n",invalid_cmd[r]);
			return;
		}
		pid = fork();
		if(!pid)
                {
		#ifdef DEBUG
			printf("Executing the command %s\n",c->args[0]);
		#endif
			execv(c->args[0],c->args);
		#ifdef DEBUG
			printf("HERE EXECUTION COMPLETED\n");
		#endif
			exit(0);
		}
		else
		{
		#ifdef DEBUG
			printf("pid is %d\n",pid);
		#endif
			if(pid)
				waitpid(pid,&status,0);
		}	
	}

}

void runShell()
{
	Pipe cmd_line;
	int subshell_pid;

	while(1)
	{
//		printf("prompt_flag %d\n",prompt_flag);
		printPrompt();
		cmd_line = parse();
		if(cmd_line == NULL) // no input
		{
			prompt_flag = 0;
			continue;
		}
		if(strcmp(cmd_line -> head -> args[0],"end")==0) // done with ushrc processing
		{
			if(rc_processing)
			{
				setupPrompt();	
				continue;
			}
			break;
		}
		prompt_flag = 0;
		Pipe pipe_line = cmd_line;
		while(pipe_line)
		{
			
			subshell_pid = pid = 0;
			exit_flag = 0;
			
		#ifdef DEBUG
			printf("Entered a pipe line\n");
		#endif
			Cmd c = pipe_line -> head;
			Cmd d = NULL;
			Cmd e;
			
			while(c!=d)
			{
			#ifdef DEBUG		
				printf("Command in pipe\n");
			#endif
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
			execute_command(e);
			waitpid(subshell_pid,&status,0);
		#ifdef DEBUG
			printf("Exit flag is: %d\n",exit_flag);
		#endif		
			if(exit_flag)
				exit(0);
		#ifdef DEBUG
			printf("Streams Reset\n");
		#endif
			pipe_line = pipe_line -> next;
		}

		resetStreams();
	}
}

int main()
{
	initShell();
	pipe_ref = 1;
	runShell();
}
