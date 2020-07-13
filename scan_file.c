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

/*
 *
 * constructor 함수 : main 이전에 실행됨, GCC only
 *
 * eclipse에서 cygwin gdb 디버깅할 경우 gdb를 통한 프로그램의 콘솔 출력 버퍼가 정상동작 하지 않고 실행 종료 후 한꺼번에 출력됨.
 * 이를 피하기 위해 stdout, stderr의 buffering을 끄는 함수.
 * 만일 buffer가 꼭 필요하다면 아래 함수를 주석처리 할 것.
 */
void __attribute__((constructor)) console_setting_for_eclipse_debugging( void ){
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
}

#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include "tcpip_lib.h"


int backup_file_only(const struct dirent *info)
{
	char *ext;
	ext = strrchr(info->d_name, '.');
	if(ext == NULL)
	{
		return 0; /* 확장자가 없으면 skip함 */
	}
	if(strcmp(ext, ".bak") == 0)
	{
		return 1; /* 목록에 포함 시킴 */
	}
	else
	{
		return 0; /* 목록에 포함 시키지 않음 */
	}
}

int main(void)
{
	struct dirent **namelist;
	int count;
	int idx;
	char file_path[1024];
	const char *path = "./";
	if((count = scandir(path, &namelist, backup_file_only, alphasort)) == -1)
	{
		printf("%s 디렉토리 조회 오류: %s\n", path, strerror(errno));
		return 1;
	}

	for(idx = 0; idx < count; idx++)
	{
		snprintf(file_path, 1024, "%s/%s", path, namelist[idx]->d_name);

		if(unlink(file_path) == 0)
		{
			printf("%s file이 삭제되었습니다.\n", namelist[idx]->d_name);
		}
		else
		{
			printf( "%s file이 삭제 오류: %s.\n", namelist[idx]->d_name, strerror(errno));
		}
		printf("Test\n");
	}
	// 건별 데이터 메모리 해제
	for(idx = 0; idx < count; idx++)
	{
		free(namelist[idx]);
	}
	// namelist에 대한 메모리 해제
	free(namelist);
	return 0;
}


