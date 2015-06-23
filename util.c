#include "util.h"

void sperror(char *s)
{
	char t[200];
	strerror_r(errno,t,200);
	if(write(STDERR_FILENO,s,_strlen(s)*sizeof(char))<0)
		quit(1);
	if(write(STDERR_FILENO,": ",2*sizeof(char))<0)
		quit(1);
	if(write(STDERR_FILENO,t,_strlen(t))<0)
		quit(1);
}

void qsperror(char *s)
{
	sperror(s);
	quit(1);	//exit with error
}

void int2str(int i, char* str)
{	
	int t=i;
	int k=10;
	int j = 0;

	while((t%k) != i)
	{
		k*=10;
		j++;
	}

	str[j+1]='\0';

	while(j>=0)
	{
		t = i % 10;
		str[j--]='0'+t;
		i-=t;
		i/=10;
	}
}

int _strlen(char *str)
{
	int i;
	for(i=0;*str++;i++);
	return i;
}

int read_number(char *cmd)
{
	int r=0;
	int b=0;
	char ch;

	for (;;) {
			if((b=read(STDIN_FILENO, &ch,1))<0)
					sperror("Error read cmd");
			if (isdigit(ch))
					r = r * 10 + ch - '0';
			else {
					*cmd = ch;
					r = r == 0 ? -1 : r;
					break;
			}
	}
	return r;
}

int fideof(int fid)
{
	int cur; 
	int end;
	if ((cur=lseek(fid, 0, SEEK_CUR)) < 0)
		qsperror("Error check end file");
	if ((end=lseek(fid, 0, SEEK_END)) < 0)
		qsperror("Error check end file");
	if ((lseek(fid, cur, SEEK_SET)) < 0)
		qsperror("Error check end file");

	if ((end==cur))
		return 1;
	else
		return 0;
}