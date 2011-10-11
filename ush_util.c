#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<signal.h>

#include"ush.h"
#include"parse.h"

char *builtin_cmds[] = {"cd","echo","logout","nice","pwd","setenv","unsetenv","where"};
int old_stdin,old_stdout,old_stderr;
int pipefd[2][2];
char prompt[MAXLEN];
int rc_file, old_rc_stdin;
int rc_processing;
extern char **environ;
char *home;
char *path;

void saveStreams()
{
	if((old_stdin = dup(fileno(stdin))) == -1)
	{
		fprintf(stderr,"dup savestream stdin failed\n");
		exit(0);
	}
	if((old_stdout = dup(fileno(stdout))) == -1)
	{
		fprintf(stderr,"dup savestream stdout failed\n");
		exit(0);
	}
	if((old_stderr = dup(fileno(stderr))) == -1)
	{
		fprintf(stderr,"dup savestream stderr failed\n");
		exit(0);
	}

}

void resetStreams()
{
	if(dup2(old_stdin,fileno(stdin)) == -1)
	{
		fprintf(stderr,"dup reset stdin failed\n");
		exit(-1);
	}
	close(old_stdin);
	if(dup2(old_stdout,fileno(stdout)) == -1)
	{
		fprintf(stderr,"dup reset stdout failed\n");
		exit(-1);
	}
	close(old_stdout);
	if(dup2(old_stderr,fileno(stderr)) == -1)
	{
		fprintf(stderr,"dup reset stderr failed\n");
		exit(-1);
	}
	close(old_stderr);
}

void printPrompt()
{
	if(prompt_flag == 0)
	{
		printf("%s",prompt);
		fflush(stdout);
		prompt_flag = 1;
	}
	saveStreams();
}


void createPipe(int pipe_ref)
{
	if (pipe(pipefd[pipe_ref]) == -1)
	{
		printf("Pipe creation failed\n");
		exit(-1);
	}
#ifdef DEBUG				
	printf("pipe %d created \n",pipe_ref);
#endif
}

void setPipeRedirect(int pipe_ref,int stream)
{
	close(pipefd[pipe_ref][!stream]);
	if(dup2(pipefd[pipe_ref][stream],stream)==-1)
	{
		fprintf(stderr,"Pipe redirect failed\n");
		exit(-1);
	}
#ifdef DEBUG
	printf("The pipe ref is %d\n",pipe_ref);
#endif
}

int isBuiltinCmd(char *cmd)
{
	int i;
	for(i=0;i<NO_OF_CMDS;i++)
		if(!strcmp(cmd,builtin_cmds[i]))
			return i;
	return -1;
}

void handle_sigint()
{
	if(exit_flag)
		exit(0);
	printf("\n");
	prompt_flag = 0;
	resetStreams();
	printPrompt();
}

int initShell()
{
	char rcpath[MAXLEN];
	path = getenv("PATH");
	prompt_flag = 0;
#ifdef DEBUG
	printf("PATH is %s\n",path);
#endif
	home = getenv("HOME");
	strcpy(rcpath,home);
	strcat(rcpath,"/.ushrc");
	strcpy(prompt,"");
	signal(SIGQUIT,SIG_IGN);	//CTRL+\/
	signal(SIGTSTP,SIG_IGN);	//CTRL+Z
	signal(SIGINT,handle_sigint); //CTRL+C
	if((rc_file = open(rcpath,O_RDONLY)) != -1)
	{
		old_rc_stdin = dup(fileno(stdin));
		if(old_rc_stdin == -1)
		{
			fprintf(stderr,"dup rcfile failed");
			exit(-1);
		}
		if(dup2(rc_file,fileno(stdin))==-1)
		{
			fprintf(stderr,"dup rcfile failed");
			exit(-1);
		}
		close(rc_file);
		rc_processing = TRUE;	
	}
	else
	{
		char hostname[MAXLEN];
		int len;
		rc_processing = FALSE;
		gethostname(hostname,len);
		if(len)
			strcpy(prompt,hostname);
		strcat(prompt,"% ");
	}
}

int setupPrompt()
{
	char hostname[MAXLEN];
	int len;
#ifdef DEBUG
	printf("Inside Setup prompt\n");
#endif
	dup2(old_rc_stdin,fileno(stdin));
	close(old_rc_stdin);
	rc_processing = FALSE;
	gethostname(hostname,len);
	strcpy(prompt,hostname);
	strcat(prompt,"% ");
	prompt_flag = 0;
}

void exec_echo(Cmd c)
{
	int i=1;
	char buf[MAXLEN];
	while(c->args[i]!=NULL)
	{
		printf("%s ",c->args[i]);
		i++;	
	}
	if(i!=1)
		printf("\n");
}

void exec_cd(Cmd c)
{	
#ifdef DEBUG
	printf("Inside cd\n");
#endif
	char *cmd_path,*path1;
	if(c->args[1] == NULL)
	{
		c->args[1] = malloc(strlen(home));
		strcpy(c->args[1],home);
	#ifdef DEBUG
		printf("HOME is %s\n",c->args[1]);
	#endif
	}
	if(c->args[1][0]=='~')
	{
		char *temp,temp1[MAXLEN];
		int tlen = 0;
		temp = strtok(c->args[1],"~");
		if(temp)
			tlen = strlen(temp);
		c->args[1] = malloc(strlen(home)+tlen);
		strcpy(c->args[1],home);
		if(temp)
			strcat(c->args[1],temp);
	}
        if(isdir(c->args[1]))
	{
		if(c->args[1][0] == '/')
		{
			chdir("/");
		#ifdef DEBUG	
			printf("/");
		#endif	
		}
		cmd_path=strtok(c->args[1],"/");
		while(cmd_path != NULL)
		{
			if(cmd_path[0]!='~')
			{
				chdir(cmd_path);
			#ifdef DEBUG
				printf("%s/",cmd_path);
			#endif	
			}
			cmd_path = strtok(NULL,"/");
		}
	#ifdef DEBUG	
		printf("\n");
	#endif
	}	
	else
		fprintf(stderr,"Not a directory\n");
}

int isdir(char *cmd_path)
{
	int r;
#ifdef DEBUG
	printf("Inside isdir\n");
#endif
	struct stat *buf;
	buf = malloc(sizeof(struct stat));
	stat(cmd_path,buf);
	r = buf->st_mode & S_IFDIR;
#ifdef DEBUG
	printf("isdir %d\n",r);
#endif
	free(buf);
	return r;
}	

int iscmd(char *cmd_path)
{
	if(access(cmd_path,F_OK) !=0)	return 1;
	if(access(cmd_path,R_OK|X_OK) !=0 || isdir(cmd_path))	return 2;
	return 0;
}


void exec_nice(Cmd c)
{
	int which, who, prio;
	if(c->args[1]!=NULL)
	{
		prio = atoi(c->args[0]);
		if(prio<-19)
			prio = -19;
		else if(prio>20)
			prio = 20;
	}
	else	prio = 4;
	getpriority(which, who);
	setpriority(which, who, prio);
	if(c->args[2]!=NULL)
	{
	#ifdef DEBUG
		printf("Command present %s\n",c->args[0]);
	#endif
		Cmd temp;
		temp = malloc(sizeof(struct cmd_t));
		temp->args = malloc(sizeof(char*));
		temp->in = Tnil;
		temp->out = Tnil;
		temp->args[0] = malloc(strlen(c->args[2]));
		strcpy(temp->args[0],c->args[2]);
		execute_command(temp);
		free(temp->args[0]);
		free(temp->args);
		free(temp);
	}
}

void exec_pwd(Cmd c)
{
	char *dir;
	dir = (char*)get_current_dir_name();
	printf("%s\n",dir);

}

void exec_setenv(Cmd c)
{
	int i;
#ifdef DEBUG
	printf("In setenv\n");
#endif
	if(c->args[1] == NULL)
	{
		char **e = environ;
		while(*e)
		{
			printf("%s\n",*e);
			*e++;
		}
		return;
	}
	else if(c->args[2] == NULL)
		setenv(c->args[1],"",1);
	else
		setenv(c->args[1],c->args[2],1);
#ifdef DEBUG
	printf("%s\n",getenv(c->args[1]));	
#endif
}

void exec_unsetenv(Cmd c)
{
	if(c->args[1] == NULL)
	{
		fprintf(stderr,"unsetenv: too few arguments\n");
		return;
	}
	else
	{
		unsetenv(c->args[1]);
	#ifdef DEBUG
		printf("%s\n",getenv(c->args[1]));
	#endif
	}
}

void exec_where(Cmd c)
{
	char *needle;
	char path1[MAXLEN],cmd_path[MAXLEN]="";
	int i=isBuiltinCmd(c->args[1]);
	if(i!=-1)
		printf("%s\n",builtin_cmds[i]);
	strcpy(path1,path);
	needle = strtok(path1,":");
	while(needle != NULL)
	{
		strcpy(cmd_path,needle);
		strcat(cmd_path,"/");
		strcat(cmd_path,c->args[1]);
		if(!iscmd(cmd_path))
			printf("%s\n",cmd_path);
		needle = strtok(NULL,":");
	}

}

void execCmd(Cmd c)
{
#ifdef DEBUG
	printf("Command is %s\n",c->args[0]);
#endif
	if(!strcmp(c->args[0],"unsetenv"))	exec_unsetenv(c);
	else if(!strcmp(c->args[0],"cd"))	exec_cd(c);
	else if(!strcmp(c->args[0],"echo"))	exec_echo(c);
	else if(!strcmp(c->args[0],"nice"))	exec_nice(c);
	else if(!strcmp(c->args[0],"pwd"))	exec_pwd(c);
	else if(!strcmp(c->args[0],"setenv"))	exec_setenv(c);
	else if(!strcmp(c->args[0],"logout"))	exit(0);
	else if(!strcmp(c->args[0],"where"))	exec_where(c);	 
#ifdef DEBUG
	printf("Completed Execution\n");
#endif
}

setupRedirect(Cmd c)
{
	int temp;
	if(c->in==Tin)
		if((temp = open(c->infile,O_RDONLY)) != -1)
		{
			if(dup2(temp,fileno(stdin)) == -1)
			{
				fprintf(stderr,"Input redirect failed\n");
				exit(-1);
			}
			close(temp);
		}
		switch ( c->out )
		{
			case ToutErr:
				if((temp = open(c->outfile,O_CREAT|O_WRONLY|O_TRUNC)) != -1)
				{
					dup2(temp,fileno(stderr));
					close(temp);
				}	
			case Tout:
				if((temp = open(c->outfile,O_CREAT|O_WRONLY|O_TRUNC)) != -1)
				{
					dup2(temp,fileno(stdout));
					close(temp);
				}	
				break;
			case TappErr:
				if((temp = open(c->outfile,O_CREAT|O_WRONLY|O_APPEND)) != -1)
				{
					dup2(temp,fileno(stderr));
					close(temp);
				}
			case Tapp:
				if((temp = open(c->outfile,O_CREAT|O_WRONLY|O_APPEND)) != -1)
				{
					dup2(temp,fileno(stdout));
					close(temp);
				}
				break;

		}                         
}


/*Below code is from main file of the pasre provided by professor*/
static void prCmd(Cmd c)
{
  int i;

  if ( c ) {
    printf("%s%s ", c->exec == Tamp ? "BG " : "", c->args[0]);
    if ( c->in == Tin )
      printf("<(%s) ", c->infile);
    if ( c->out != Tnil )
      switch ( c->out ) {
      case Tout:
        printf(">(%s) ", c->outfile);
        break;
      case Tapp:
        printf(">>(%s) ", c->outfile);
        break;
      case ToutErr:
        printf(">&(%s) ", c->outfile);
        break;
      case TappErr:
        printf(">>&(%s) ", c->outfile);
        break;                  
      default:                  
        fprintf(stderr, "Shouldn't get here\n");
        exit(-1);               
      }                         
                                
    if ( c->nargs > 1 ) {
      printf("[");      
      for ( i = 1; c->args[i] != NULL; i++ )
        printf("%d:%s,", i, c->args[i]);
      printf("\b]");    
    }           
    putchar('\n');
    // this driver understands one command
    if ( !strcmp(c->args[0], "end") )
      exit(0);
  }
}

void prPipe(Pipe p)
{
  int i = 0;
  Cmd c;

  if ( p == NULL )
    return;

  printf("Begin pipe%s\n", p->type == Pout ? "" : " Error");
  for ( c = p->head; c != NULL; c = c->next ) {
    printf("  Cmd #%d: ", ++i);
    prCmd(c);
  }
  printf("End pipe\n");
  prPipe(p->next);
}
