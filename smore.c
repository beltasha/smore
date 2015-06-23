#include "smore.h"
#include "util.h"

int k;                                 	 //k -- user-entered number
int c_l_d;                             	 //default count print line
int cur_line;                  		  	 //current line
int w_columns;
int w_rows;
int promt_size;		
int s_line;							     //skipped in the beginning
int p_line;							    //count print line in the beginning
char toPromt;
struct termio tio_s, tin;

void set_tty()
{
	struct winsize sz;
	
	if (signal(SIGINT, quit) == SIG_ERR)
		qsperror("Error init signal");
	if (isatty(STDIN_FILENO))
	{

		//Get current modes tty
		if (ioctl(STDIN_FILENO, TCGETA, &tio_s) < 0)
			qsperror("Error get tty");

		tin = tio_s;
		tin.c_lflag &= ~(ECHO | ICANON);      //turn off ECHO and ICANON
		
		//Emulate CBREAK mode.
		tin.c_cc[VMIN] = 1;
		tin.c_cc[VTIME] = 0;

		//Set the new modes
		if (ioctl(STDIN_FILENO, TCSETA, &tin) < 0)
			qsperror("Error set tty");


	}
	
	if (isatty(STDOUT_FILENO))
	{
		//Defined size window
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &sz);
		w_columns = sz.ws_col;
		w_rows = sz.ws_row;
	}
	else
	{
		//Defined size window
		w_columns = 200;
		w_rows = 80;
	}
}

void clearpromt()
{
	int s = promt_size;
	char *cl; 

	if(promt_size == 0)
			return;
	
	cl = (char*)malloc((s + 1)*sizeof(char));
	cl[0]='\r';
	cl[s + 1] = '\r';
	while(s > 0)
			cl[s--]=' ';
	if(write(STDOUT_FILENO,cl,promt_size+2) < 0)
			qsperror("Error write in clearpromt");
	promt_size=0;
}

void quit(int e)
{
	char *err = "Error exit";
	char *n = "\n";
	remove("temp_smore_file");
	clearpromt();

	if (ioctl(0, TCSETA, &tio_s) < 0)
	{
		WRITEOUT(err, "Error in recovery tty");
		exit(2);
	}
	exit(e);
}

int argscan(char* s)
{
	char f='0';
	int i=0;
	while(s[i])
	{
		switch(s[i])
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if(f=='-')
				{
					p_line=p_line*10+s[i]-'0';
				}
				else if (f == '+')
				{
					s_line = s_line * 10 + s[i] - '0';
				}
				break;
			case '-':
				if (_strlen(s) != 1) 
					p_line = 0;
				if(f=='-')
					return 3;				//end options
				f='-';
				break;
			case '+':
				s_line=0;
				f='+';
				break;
			case '\n':
			case '\0':
				if(f!='-' && f!='+')
						return 1;		//the arg is file
				break;
			default:
				break;
		}
		i++;
	}
	if (s[0] == '-' && _strlen(s) == 1)
		return 2;
	if(f=='-' || f=='+')
		return 0;		//the arg is file
	
	return 1;
}

int main(int argc, char *argv[])
{
	
	char *msg_next_file="\n::::NEXT FILE::::\n\n";
	int fid;
	int mystdin;
	int i,s;
	int rb;
	char nul=EOF;
	char str[1024];
	
	if(argc < 2)
	{
		WRITEOUT(usage,"Error write usage");
		exit(1);
	}
	if(!isatty(STDIN_FILENO) && (argc > 2 || (_strlen(argv[1])!=1 && argv[1][0]!='-')))
	{
		WRITEOUT(usage,"Error write usage");
		exit(1);
	}
	
	for(i=1; i < argc; i++)
	{
		switch(argscan(argv[i]))
		{
			//error
			case -1:
				WRITEOUT(usage,"Error write usage");
				quit(1);
			//the arg is option 
			case 0:
				break;
			//the arg is file
			case 1:
				goto end_arg_scan;
			//the arg is stdin
			case 2:
			read_stdin:
				if (!isatty(STDIN_FILENO))
				{

					if ((fid = open("temp_smore_file", O_CREAT | O_RDWR, S_IREAD | S_IWRITE))<0)
						qsperror("Error create temp file");
				
					while ((rb = read(STDIN_FILENO, str, 1023))>0)
					{
						
						if (write(fid, str, rb) < 0)
							qsperror("Error copy stdin to temp file");
					}
					if(lseek(fid, 0L, SEEK_SET) < 0)
						qsperror("Error lseek stdin");
					if (fideof(fid))
						qsperror("stdin is empty");
					if((mystdin = open("/dev/tty", O_RDONLY)) < 0)
						qsperror("Error open tty");
					if ((mystdin = dup(mystdin)) < 0)
						qsperror("Error dup");
					if(dup2(mystdin, 0)<0)
						qsperror("Error dup2");
					set_tty();
					more(fid);
					close(fid);
					quit(0);
				}
				else
					qsperror("stdin is empty");
			//end option
			case 3:
				i++;
				goto end_arg_scan;
		}
	}
	
	end_arg_scan:
	
	set_tty();

	while (i<argc)
	{
		
		if ((fid = open(argv[i], O_RDONLY)) < 0)
				sperror("Error open file");
		else
			more(fid);

		i++;
		if (argc - i>0)
		{
			WRITEOUT(msg_next_file, "Err write msg");
			prompt();
		}
	}

	//Reset the old tty modes.
	quit(0);
	return 0;
}

int getLine(int fid, int c, char* str)
{
	int r_s = 0;                            //read symbol in line
	int b;
	int i;
	int t=0;
	
	while(r_s < c)
	{
		if ((b=read(fid, str+r_s, c-r_s)) < 0)
			qsperror("Error read");
		//EOF
		if(b==0)
		    return 3;

		for(i=0; i < b; i++)
		{
			if(str[r_s+i]=='\n')
			{
				lseek(fid,i-b+1, SEEK_CUR);
				r_s+=i;
				goto end_loop_r_l;
			}
			else if(str[r_s+i]=='\t')
				t++;
			else if(str[r_s+i]==EOF)
				return 4;
			
 			if((r_s+i+t*(ST-1)) >= c)
			{
					str[r_s+i]='\n';
					str[r_s+i+1]='\0';
					lseek(fid,i-b,SEEK_CUR);
					return 0;
			}
		}
		r_s += b;
	}
	end_loop_r_l:
	str[r_s+1]='\0';
	return 0;
}

int displayPartFile(int fid, int c, int r)
{
	int r_l;                                 //read lines
	int r_s = 0;                            //read symbol in line
	int b;
	int i;
	char* str;

	r_l=1;
	while (r_l <= r)
	{
		str=(char*)calloc(1024,sizeof(char));
		i=getLine(fid,c,str);
		if(i==0)
		{
			WRITEOUT(str,"Error write line");
			r_l++;
			cur_line++;
		}
		if(fideof(fid))
			return 2; //end file;
		free(str);
	}

	return 1;
}

int more(int fid)
{
	int t;
	int i;
	char *msg_count_line="Number line: ";
	char *msg_cmd_nf="Command not found";
	char *msg_skip_line="...skiping line...\n";
	char *msg_back_page="...back page...\n";
	char* pr;
	char str[1024];
	w_rows = p_line == 0 ? w_rows-1 : p_line;
	k=w_rows;
	c_l_d=0;

	//skipped lines
	for (i = 0; i < s_line; i++)
	{
		getLine(fid, w_columns, str);
		if (fideof(fid))
			quit(0);
	}
	
	i = cur_line == 0 ? w_rows : 1;
	cur_line=0;
	//first screen - start display
	switch (displayPartFile(fid, w_columns, i))
	{
	case 1:
		i=0;
		while((t = prompt())>0)
		{
			switch(t)
			{
				//<enter>
				case 1:
					if(displayPartFile(fid,w_columns,1)==2)
						goto end_loop_m;
					break;
				//z and <space>
				case 2:
					if(displayPartFile(fid,w_columns,k)==2)
						goto end_loop_m;
					break;
				//=
				case 3:
					int2str(cur_line, str);
					WRITEOUT(msg_count_line,"Error write in count line");
					WRITEOUT(str,"Error write in count line");
					promt_size=_strlen(str)+_strlen(msg_count_line);
					break;
				//s
				case 4:	
					WRITEOUT(msg_skip_line,"Error write in count line");
					for (i = 0; i < k; i++)
					{
						getLine(fid, w_columns, str);
						if (fideof(fid))
							quit(0);
					}
					cur_line+=i;
					displayPartFile(fid,w_columns,w_rows-1);
					break;
				//b
				case 5:
					if(cur_line > 0)
					{
						WRITEOUT(msg_back_page, "Error write in count line");
						if(lseek(fid,0,SEEK_SET)<0)
							qsperror("Error back page");
						for(i=0; i <= cur_line-(k+1)*w_rows+k; i++)
							getLine(fid,w_columns,str);
						cur_line=i;
						displayPartFile(fid,w_columns,w_rows-1);
					}
					break;
				default:
					WRITEOUT(msg_cmd_nf,"Error");
					promt_size += _strlen(msg_cmd_nf);
					break;

			}
		}
		break;
		case 2:
			close(fid);
			return 0;
		default:
			close(fid);
			return 0;
	}
	end_loop_m:
	close(fid);
	return 0;
}

int prompt()
{
	char c;
	int r;
	char prom[] = "--More--";
	promt_size += _strlen(prom);
	WRITEOUT(prom,"Error write in prompt");
	r = read_number(&c);
	switch(c)
	{
		case '\n':
			clearpromt();
			return 1;
		case 'z':
			clearpromt();
			//defult print number line = screen_size
			if(r==-1)
					if(c_l_d==0)
							k = w_rows-1;
					else
							k=c_l_d;
			else
			{
					c_l_d = r;
					k=c_l_d;
			}
			return 2;
		case ' ':
			clearpromt();
			//defult print number line = screen_size
			if(r==-1)
					if(c_l_d==0)
						k = w_rows-1;
					else
						k=c_l_d;
			else
				k=c_l_d;
			return 2;
		case '=':
			clearpromt();
			return 3;
		case 's':
			clearpromt();
			k= r == -1 ? 1 : r;
			return 4;
		case 'b':
			clearpromt();
			k= r == -1 ? 1 : r;
			return 5;
		default:
			clearpromt();
			return 10;
	}
}
