#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/stat.h>

void WelcomeScreen()
{
	printf("\n\t============================================\n");
	printf("\t|               microshell.c               |\n");
	printf("\t--------------------------------------------\n");
	printf("\t|           © Bartlomiej Gadzicki          |\n");
	printf("\t============================================\n");
	printf("\n");
}

void Prompt(char cwd[])
{
	register struct passwd *pw;
	register uid_t uid;

	uid = geteuid ();
	pw = getpwuid (uid);

	printf("%c[%dm%s", 0x1B, 32, pw->pw_name);
	printf("%c[%dm", 0x1B, 0);
	printf(":");
	printf("%c[%dm%s", 0x1B, 34, cwd);
	printf("%c[%dm", 0x1B, 0);
	printf("$ ");

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
	printf("\nmicroshell.c © Bartlomiej Gadzicki\n");
	printf("Zaimplementowane polecenia:\n\
  exit - wyjscie z powloki\n\
  help - wyswietla pomoc\n\
  pwd - wyswietla aktualny katalog roboczy \n\
  ls [sciezka do katalogu] - wyswietla pliki w katalogu.\n\
  cd [sciezka do katalogu] - zmiana aktualnego katalogu roboczego\n\
  seq [start] [krok] [stop] - wyswietla sekwencje liczb\n\
  clear - czyszczenie konsoli\n\n\
powloka pozwala wywolac pogram z argumentami:\n\
syntax: [sciezka do pliku] [argumenty]\n\n");
}

void pwd(char cwd[])
{
	printf("%s\n", cwd);
}

void cd(char *cmd[], char cwd[], char prvwd[])
{
	if(cmd[1] == NULL)
	{
		chdir(cwd);
	}
	else if(strcmp(cmd[1], "-") == 0)
	{
		chdir(prvwd);
	}
	else
	{
		if(chdir(cmd[1]) != 0)
		{
			perror(cmd[1]);
		}
		else
		{
			strcpy(prvwd, cwd);
		}
	}
}

void ls(char *path)
{
	DIR *dir;
	struct dirent *myfile;

	dir = opendir(path);
	while((myfile = readdir(dir)) != NULL)
	{
		if(strcmp(myfile->d_name, ".") == 0 || strcmp(myfile->d_name, "..") == 0)
		{
			continue;
		}
		printf(" %s\n", myfile->d_name);
}
	closedir(dir);
}

void seq(char * cmd[])
{
	if(cmd[1] == NULL)
	{
		printf("seq: missing operand\n");
	}
	else if(cmd[2] == NULL)
	{
		int i = 1;
		while(i <= atoi(cmd[1]))
		{
			printf("%d\n", i);
			i++;
		}
	}
	else if(cmd[3] == NULL)
	{
		int start = atoi(cmd[1]);
		int stop = atoi(cmd[2]);

		while(start <= stop)
		{
			printf("%d\n", start);
			start++;
		}

	}
	else
	{
		int start = atoi(cmd[1]);
		int step = atoi(cmd[2]);
		int stop = atoi(cmd[3]);

		while(start <= stop)
		{
			printf("%d\n", start);
			start = start + step;
		}
	}
}


int main()
{
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
			cd(cmd, cwd, prvwd);
		}
		else if(strcmp(cmd[0], "ls") == 0)
		{
			if(cmd[1] == NULL)
			{
				ls(".");
			}
			else
			{
				ls(cmd[1]);
			}
		}
		else if(strcmp(cmd[0], "seq") == 0)
		{
			seq(cmd);
		}
		else if(strcmp(cmd[0], "clear") == 0)
		{
			system("@cls||clear");
		}
		else
		{
			pid_t child;
			child = fork();

			if(child == 0)
			{
				execv(cmd[0], cmd);
				printf("komenda inwalidzka. typnij 'help'\n");
				break;
			}
			else
			{
				waitpid(child, NULL, 0);
			}
		}
	}
	return 0;
}
