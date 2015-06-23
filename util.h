#include <stdio.h>
#include <unistd.h>
#include <errno.h>

void sperror(char *s);
void qsperror(char *s);
void int2str(int i, char* str);
int _strlen(char *str);
int read_number(char *cmd);
int fideof(int fid);