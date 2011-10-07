#include<string.h>
#include<stdio.h>
#include<stdlib.h>

#include"ush.h"
#include"parse.h"

char prompt[MAXLEN];

void printPrompt(char *message)
{
	if(strcmp(message,"")!=0)
		printf("%s\n",message);
	printf("%s",prompt);
}

int initShell()
{
	char hostname[MAXLEN];
	int len;
	gethostname(hostname,len);
	strcpy(prompt,hostname);
	strcat(prompt,"% ");
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
