#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#include <glib.h>
#include <microhttpd.h>
#include <json-c/json.h>
#include <curl/curl.h>


#define DEBUG
//#undef DEBUG
#ifdef DEBUG
#define d_printf(fmt, args...) fprintf(stderr, "[%s:%d:%s()]: " fmt, \
__FILE__, __LINE__, __func__, ##args)
#else
#define d_printf(fmt, args...)
#endif

void removeLineFeed(char *strBuf) {
	char *pos = NULL;
	if ((pos = strchr(strBuf, '\n')) != NULL)
		*pos = '\0';
	if ((pos = strchr(strBuf, '\r')) != NULL)
		*pos = '\0';
}

char* removeEndChar(char *strBuf, char ch) {
	if (strBuf[strlen(strBuf) - 1] == ch)
		strBuf[strlen(strBuf) - 1] = '\0';

	return strBuf;
}

/*
 * constructor 함수 : main 이전에 실행됨, GCC only
 *
 * eclipse에서 cygwin gdb 디버깅할 경우
 * gdb를 통한 프로그램의 콘솔 출력 버퍼가 정상동작 하지 않고
 * 실행 종료 후 한꺼번에 출력되는 경우가 있음.
 * 이를 피하기 위해 stdout, stderr의 buffering을 끄는 함수.
 * 만일 buffer가 꼭 필요하다면 아래 함수를 주석처리 할 것.
 */
void __attribute__((constructor)) console_setting_for_eclipse_debugging( void ){
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
}

// name#url,url;url,url,url;
typedef struct {
	char urlStr[50];
	int nMaxSession;
	int nCurSession;
} t_url;

typedef struct {
	char indexStr[50];
	int indexInt;

	t_url listURL[10];
	int nCountURL;
} t_contents;


int g_cntInfo=0;
#define MAX_SERVICE 50
t_contents g_readinfo[MAX_SERVICE] = { 0, };

// name#url,url;url,url,url;
void readFileContents(char *filename) {

	char str[1024] = { 0, };
	char delim1[2] = "#", delim2[2] = ":", delim3[2] = ",";

	char *first;
	char *second;
	char *third;
	char buf1[200], buf2[200], buf3[200];
	FILE *fp = fopen(filename, "r+");

	while (fgets(str, 1000, fp) != NULL) {
		removeLineFeed(str);

		d_printf("Line: %s\n", str);

		for (first = strtok(str, delim1); first != NULL;
				first = strtok(first + strlen(first) + 1, delim1)) {
			strncpy(buf1, first, sizeof(buf1));

			// left 항목 저장
			d_printf("left: %s\n", buf1);
			strcpy((char*) g_readinfo[g_cntInfo].indexStr, first);
			// g_readinfo[g_cntInfo].indexInt = atoi (first); // 숫자 변환 필요 시

			// Right 항목 추가 분류를 위해 다음 토큰 호출
			first = strtok(first + strlen(first) + 1, delim1);
			strncpy(buf1, first, sizeof(buf1));
			d_printf("right str: %s, len = %d\n", buf1, strlen(buf1));

			// Right 항목 추가 분류 시작
			int nCountURL = 0;
			for (second = strtok(buf1, delim2); second != NULL;
					second = strtok(second + strlen(second) + 1, delim2)) {
				strncpy(buf2, second, sizeof(buf2));
				d_printf("\tsecond: %s\n", buf2);

				// 추가 분류 필요 시 반복 호출

				// third의 첫번째가 URL인 경우
				third = strtok(buf2, delim3);
				strcpy((char*) g_readinfo[g_cntInfo].listURL[nCountURL].urlStr,
						third);
				d_printf("\t  third: %s\n", third);
				// third의 두번째가 숫자인 경우
				third = strtok(NULL, delim3);
				if (third != NULL)
				{
					d_printf("\t  third: %s\n", third);
					g_readinfo[g_cntInfo].listURL[nCountURL].nMaxSession = atoi(
							third);
				}
				else
					g_readinfo[g_cntInfo].listURL[nCountURL].nMaxSession = 0;

				nCountURL++;

			}
			g_readinfo[g_cntInfo].nCountURL = nCountURL;
		}
		g_cntInfo++;
	}
	fclose(fp);
}

struct {
	char left[100];
	char right[100];
} g_data[1024];
//char g_data[10][100];
int g_nIndex=0;

void readSimpleFile(char *filename) {

	char str[1024] = { 0, };
	char delim1[2] = "#";

	char *ptr;
	FILE *fp = fopen(filename, "r+");

	while (fgets(str, 1000, fp) != NULL) {
		removeLineFeed(str);
		d_printf("Line: %s\n", str);

		ptr = strtok (str, delim1);
		d_printf("left: %s\n", ptr);
		strcpy (g_data[g_nIndex].left, ptr);
		ptr = strtok (NULL, delim1);
		strcpy (g_data[g_nIndex++].right, ptr);
		d_printf("right: %s\n", ptr);
	}
	fclose(fp);
}


typedef struct {
	char name[1024];
	char value[1024];
} t_input;

t_input condition[1024] = { 0, };
int index_cond = 0;
void splitString(char *input, char *delim)
{
	char *buf ;

	buf = strtok((char*)input, delim);
	strcpy( condition[index_cond].name , buf);
	buf = strtok(NULL, delim);
	strcpy( condition[index_cond].value , buf);

	index_cond++;

}

void findService (char *filename, char *cmd)
{

	char str[1024] = { 0, };
	char delim1[2] = "#";

	char *ptr;
	char buf1[200];
	FILE *fp = fopen(filename, "r+");

	if (fp != NULL)
	{
		while (fgets(str, 1000, fp) != NULL) {
			removeLineFeed(str);
			d_printf("Line: %s\n", str);

			ptr = strtok (str, delim1);
			strcpy (buf1, ptr);
			if (strcmp (buf1, cmd) == 0)
			{
				ptr = strtok (NULL, delim1);
				printf("%s\n", ptr);
				fclose(fp);
				return;
			}
		}
		printf("EMPTY\n");
		fclose(fp);
	}
	else
	{
		printf("ERROR\n");
	}
}

int main(int argc, char *argv[]){
	readSimpleFile ("SERVICE.TXT");

	char cmd[100], svc[100];
	scanf ("%s %s", svc, cmd );
	d_printf("svc = %s , cmd = %s\n", svc, cmd);

	char filepath[100];
	sprintf(filepath, "./SERVICE/");

	int i=0;
	for (i=0;i<g_nIndex;i++)
	{
		printf("data = %s, svc = %s\n", g_data[i].left, svc);
		if (strcmp ( g_data[i].left , svc) == 0)
		{
			strcat (filepath, g_data[i].right);
			break;
		}
	}
	if (i==g_nIndex)
	{
		printf("ERROR\n");
	}
	else
	{
		d_printf("svc file = %s\n", filepath);
		findService (filepath, cmd);
	}

	return 0;
}

