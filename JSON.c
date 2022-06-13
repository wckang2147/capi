#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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

char * removeEndChar(char *strBuf, char ch)
{
	if (strBuf[strlen(strBuf)-1] == ch)
		strBuf[strlen(strBuf)-1] = '\0';

	return strBuf;
}
// name#url,url;url,url,url;

typedef struct {
	char str[10][100];
} t_cluster;
typedef struct {
	char index[100];
	// #
	t_cluster cluster[10];
	int nCluster;
} t_contents;

t_contents g_contents[100] = {0,};


int main(int argc, char *argv[]){


//	readFileContents("./test.txt");

    json_object *myobj = json_object_new_object();
    json_object_object_add(myobj, "name", json_object_new_string("KIM"));
    json_object_object_add(myobj, "phone", json_object_new_string("010000000000"));
    const char *result_json = json_object_to_json_string_ext(myobj, JSON_C_TO_STRING_PLAIN);
    printf("json: %s\n", result_json);

    json_object *root, *name, *phone;
    root = json_tokener_parse(result_json);
    json_object_object_get_ex(root, "name", &name);
    json_object_object_get_ex(root, "phone", &phone);
    printf("name: %s, phone: %s\n", json_object_get_string(name), json_object_get_string(phone));




	return EXIT_SUCCESS;
}

