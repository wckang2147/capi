#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>

void __attribute__((constructor)) console_setting_for_eclipse_debugging( void ){
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
}
#define DEBUG
//#undef DEBUG
#ifdef DEBUG
#define d_printf(fmt, args...) fprintf(stderr, "[%s:%d:%s()]: " fmt, \
__FILE__, __LINE__, __func__, ##args)
#else
#define d_printf(fmt, args...)
#endif
void removeLineFeed(char *strBuf)
{
	char *pos = NULL;
	if ((pos = strchr(strBuf, '\n')) != NULL)
		*pos = '\0';
	if ((pos = strchr(strBuf, '\r')) != NULL)
		*pos = '\0';
}

typedef struct {
	char name[1024];
	char value[1024];
} t_input;

t_input condition[1024] = { 0, };
int index_cond = 0;
void parseInput(char *input, char *delim)
{
	char *buf ;

	buf = strtok((char*)input, delim);
	strcpy( condition[index_cond].name , buf);
	buf = strtok(NULL, delim);
	strcpy( condition[index_cond].value , buf);

	index_cond++;

}

typedef struct {
	char date[1024];
	char time[1024];
	char name[1024];
	char category[1024];
	char comments[1024];
} t_log;

t_log g_log[1024] = {0,};
int g_idx=0;
void parseInput2(FILE *fp)
{
	int i=0;
	char buf[2048];
	while ( fgets(buf, 1024, fp) != NULL )
	{
		removeLineFeed(buf);
		d_printf("Read Line\n");

		sscanf(buf, "%[^#]#%[^#]#%[^#]#%[^#]#%[^#]", g_log[i].date , g_log[i].time, g_log[i].name, g_log[i].category, g_log[i].comments);
		//d_printf("%s %s %s %s Comments = %s\n", g_log[i].date , g_log[i].time, g_log[i].name, g_log[i].category, g_log[i].comments);

		i++;
	}
	g_idx = i;

}

int main(int argc, char *argv[])
{
	FILE *fp = fopen("./data.txt", "r+");
	parseInput2 (fp);

	for (int i=0; i<g_idx; i++)
		printf("%s %s %s %s %s\n", g_log[i].date , g_log[i].time, g_log[i].name, g_log[i].category, g_log[i].comments);

	fclose(fp);

}

