//int dir_list_print(char *dirpath, char *name, char **find_path)
// dirpath : 찾고 싶은 폴더의 시작 위치
// name : 찾고자 하는 파일 (확장자 포함)
// *find_path : 경로를 포함한 파일명 (return)

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

int dir_list_print(char *dirpath, char *name, char **find_path)
{
	DIR * dir_info = NULL;
	struct dirent *dir_entry = NULL;
	struct stat buf;
	int dir_mode_err;


	char filepath[100];
	dir_info = opendir(dirpath);
	if (NULL == dir_info)
	{
		printf("Directory Open Error\n");
		return -1;
	}
	while ( (dir_entry = readdir (dir_info)) != NULL)
	{
		//printf("%s\n", dir_entry->d_name);
		if (strncmp(dir_entry->d_name, "..",2)==0 ||strncmp(dir_entry->d_name, ".",1)==0 )
		{
			continue;
		}
		sprintf(filepath, "%s%s", dirpath, dir_entry->d_name);

		dir_mode_err = lstat(filepath, &buf);

		if (dir_mode_err == -1)
		{
			puts("Dir mode error");
			break;
		}
		if (S_ISDIR(buf.st_mode))
		{
			strcat (filepath, "/");
			if (dir_list_print(filepath, name, find_path) == 1)
			{
				closedir (dir_info);
				return 1;
			}
		}
		else
		{
			if (S_ISREG(buf.st_mode))
			{
				if (strcmp(dir_entry->d_name, name) == 0)
				{
					//printf("Found path = %s, dirpath = %s, name = %s\n", *find_path, dirpath, dir_entry->d_name);
					strcat ( *find_path, dirpath);
					strcat ( *find_path, "/");
					strcat ( *find_path, dir_entry->d_name);

					return 1;
				}
				//printf("%s\n", dir_entry->d_name);
			}
		}
	}
	closedir(dir_info);
	return 0;
}


static char g_foundFilename[1024][1024] = {0,};
static int g_fileidx = 0;

int readDir(char *dirpath)
{
	DIR * dir_info = NULL;
	struct dirent *dir_entry = NULL;
	struct stat buf;
	int dir_mode_err;

	printf("Current Directory = %s\n", dirpath);

	char filepath[100];
	dir_info = opendir(dirpath);
	if (NULL == dir_info)
	{
		printf("Directory Open Error\n");
		return -1;
	}

	while ( (dir_entry = readdir (dir_info)) != NULL)
	{
		if (strncmp(dir_entry->d_name, "..",2)==0 ||strncmp(dir_entry->d_name, ".",1)==0 )
		{
			//printf("skip %s\n", dir_entry->d_name);
			continue;
		}

		sprintf(filepath, "%s%s", dirpath, dir_entry->d_name);
		dir_mode_err = lstat(filepath, &buf);

		if (dir_mode_err == -1)
		{
			perror("Dir mode error");
			break;
		}

		if (S_ISDIR(buf.st_mode))
		{

			char path[200] = {0,};

			strcat (filepath, "/");
			readDir(filepath) ;

		}
		else
		{
			if (S_ISREG(buf.st_mode))
			{
				// filename : dir_entry->d_name
				printf("full name = %s\n", filepath);
				sprintf(g_foundFilename[g_fileidx++], "%s", filepath);
			}
		}
	}
	closedir(dir_info);
	return 0;
}
void main()
{
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	readDir("./");

	for (int i=0; i< g_fileidx ; i++)
		printf("%s\n", g_foundFilename[i]);
}
