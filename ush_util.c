#include<string.h>

#include"ush.h"

int initShell()
{
	char hostname[MAXLEN];
	int len;
	gethostname(hostname,len);
	strcpy(prompt,hostname);
	strcat(prompt,"% ");
}
