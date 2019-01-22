#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <pwd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>


#define BLUE "\x1B[34;1m"
#define GREEN "\x1B[32;1m"
#define RESET "\x1B[0m"


void WelcomeScreen()
{
	printf("\n\t==================================================\n");
	printf("\t|                  microshell.c                  |\n");
	printf("\t--------------------------------------------------\n");
	printf("\t|              © Bartlomiej Gadzicki             |\n");
	printf("\t==================================================\n");
	printf("\n");
}

void Prompt(char cwd[])
{
	register struct passwd *pw;
	register uid_t uid;

	uid = geteuid ();
	pw = getpwuid (uid);

	printf(BLUE "%s" RESET ":" GREEN "%s" RESET "$ ", pw->pw_name, cwd);

	fflush(stdout);
}

void ParseCommand(char input[], char *cmd[])
{
	char **cmd_pointer;

	cmd_pointer = cmd;
	*cmd_pointer++ = strtok(input," \n\t");
	while((*cmd_pointer++ = strtok(NULL," \n\t")));
}

void help()
{
	printf("\nmicroshell.c © Bartlomiej Gadzicki\n\n\
Zaimplementowane polecenia:\n\
  exit - wyjscie z powloki\n\
  help - wyswietla pomoc\n\
  pwd - wyswietla aktualny katalog roboczy \n\
  ls [path] - wyswietla pliki w katalogu.\n\
  cd [path] - zmiana aktualnego katalogu roboczego\n\
  cp [src] [dest] - kopiuje plik\n\
  seq [start] [step] [stop] - wyswietla sekwencje liczb\n\
  clear - czyszczenie konsoli\n\n");
}

void pwd(char cwd[])
{
	printf("%s\n", cwd);
}

void cd(char *path, char cwd[], char prvwd[])
{
	if(path == NULL || strcmp(path, ".") == 0 || strcmp(path, "./") == 0)
	{
		chdir(cwd);
		strcpy(prvwd, cwd);
	}
	else if(strcmp(path, "-") == 0)
	{
		if(chdir(prvwd) != 0)
		{
			perror(prvwd);
		}
		else
		{
			printf("%s\n", prvwd);
			strcpy(prvwd, cwd);
		}
	}
	else
	{
		if(chdir(path) != 0)
		{
			perror(path);
		}
		else
		{
			strcpy(prvwd, cwd);
		}
	}
}

void ls(char *path)
{
	if(path == NULL)
	{
		path = ".";
	}
	
	DIR *dir;
	struct dirent *file;

	dir = opendir(path);
	if(dir != NULL)
	{
		while((file = readdir(dir)) != NULL)
		{
			if(strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0)
			{
				continue;
			}
			printf(" %s\n", file->d_name);
		}
	}
	else
	{
		printf("ls: cannot open directory '%s'\n", path);
	}
	closedir(dir);
}

void cp(char * src, char * dest)
{
	if(src == NULL || dest == NULL)
	{
		printf("cd: argument(s) missing. type 'help'.\n");
	}
	else
	{
		char b[1024];
		int files[2];
		ssize_t c;

		files[0] = open(src, O_RDONLY);
		if (files[0] == -1)
		{
			printf("cp: cannot open %s.\n", src);
		}
		else
		{
			files[1] = open(dest, O_WRONLY | O_CREAT, 0777);
			if(files[1] == -1)
			{
				close(files[0]);
				printf("cp: cannot open / create %s.\n", dest);
			}
			else
			{			
				while((c = read(files[0], b, sizeof(b))) != 0)
				{
					write(files[1], b, c);
				}
				
				struct stat p;
				stat(src, &p);
				chmod(dest, p.st_mode);

				close(files[1]);
			}
		}

	}
}

void seq(char * cmd[])
{
	if(cmd[1] == NULL)
	{
		printf("seq: missing argument(s).\n");
	}
	else if(cmd[2] == NULL)
	{
		long number;
		char * r;
		number = strtol(cmd[1], &r, 10);
		
		if(r == cmd[1] || *r != '\0')
		{
			printf("seq: invalid argument: '%s'\n", cmd[1]);
		}
		else
		{
			int i = 1;
			while(i <= number)
			{
				printf("%d\n", i);
				i++;
			}
		}
	}
	else if(cmd[3] == NULL)
	{
		long start, stop;
		char *r, *r2;
		start = strtol(cmd[1], &r, 10);
		stop = strtol(cmd[2], &r2, 10);
		
		if(r == cmd[1] || *r != '\0')
		{
			printf("seq: invalid argument: '%s'\n", cmd[1]);
		}
		else if(r2 == cmd[2] || *r2 != '\0')
		{
			printf("seq: invalid argument: '%s'\n", cmd[2]);
		}
		else
		{
			while(start <= stop)
			{
				printf("%ld\n", start);
				start++;
			}
		}
	}
	else
	{
		long start, step, stop;
		char *r, *r2, *r3;
		start = strtol(cmd[1], &r, 10);
		step = strtol(cmd[2], &r2, 10);
		stop = strtol(cmd[3], &r3, 10);

		if(r == cmd[1] || *r != '\0')
		{
			printf("seq: invalid argument: '%s'\n", cmd[1]);
		}
		else if(r2 == cmd[2] || *r2 != '\0')
		{
			printf("seq: invalid argument: '%s'\n", cmd[2]);
		}
		else if(r3 == cmd[3] || *r3 != '\0')
		{
			printf("seq: invalid argument: '%s'\n", cmd[3]);
		}
		else
		{
			while(start <= stop)
			{
				printf("%ld\n", start);
				start = start + step;
			}
		}
	}
}

void Clear()
{
	system("@cls||clear");
}


int main()
{
	Clear();
	WelcomeScreen();

	char cwd[256];
	char input[256];
	char prvwd[256];
	char *cmd[256];

	regex_t regex;
	int r;

	r = regcomp(&regex, "^[ \n\t]*$", 0);
	if(r != 0)
	{
		printf("Could not compile regex\n");
		exit(1);
	}

	while(1)
	{
		getcwd(cwd, 256);
		Prompt(cwd);

		fgets(input, 256, stdin);
		r = regexec(&regex, input, 0, NULL, 0);
		if(r == 0)
		{
			continue;
		}

		ParseCommand(input, cmd);

		if(strcmp(cmd[0], "exit") == 0)
		{
			exit(1);
		}
		else if(strcmp(cmd[0], "help") == 0)
		{
			help();
		}
		else if(strcmp(cmd[0], "pwd") == 0)
		{
			pwd(cwd);
		}
		else if(strcmp(cmd[0], "cd") == 0)
		{
			cd(cmd[1], cwd, prvwd);
		}
		else if(strcmp(cmd[0], "cp") == 0)
		{
			cp(cmd[1], cmd[2]);
		}
		else if(strcmp(cmd[0], "ls") == 0)
		{
			ls(cmd[1]);
		}
		else if(strcmp(cmd[0], "seq") == 0)
		{
			seq(cmd);
		}
		else if(strcmp(cmd[0], "clear") == 0)
		{
			Clear();
		}
		else
		{
			printf("komenda inwalidzka. typnij 'help'\n");
		}
	}
	return 0;
}
