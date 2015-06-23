#include <termio.h>
#include <curses.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <term.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>

#define WRITEOUT(s,e) if(write(STDOUT_FILENO,s,_strlen(s))<0) \
					  { sperror(e); quit(); }
#define NL "\n"
#define CR "\r"
#define ST 8			//count spice in \t


char *usage="Usage: more [-num +num] file OR - [file...]\n";


void clearpromt();
void quit();
int argscan(char* s);
int getLine(int fid, int c, char* str);
int displayPartFile(int fid, int c, int r);
int more(int fid);
int prompt();
