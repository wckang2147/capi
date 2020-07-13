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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>

/* * tcpip_lib.c와 tcpip_lib.h는 최상단의 링크에서 다운받으세요. */
#include "tcpip_lib.h"
#define SERVER_NAME "localhost" // 서버에 대한 domain이나 IP address 정의
#define SERVER_PORT 6905 // 서버에 대한 port번호 설정
int main(int argc, char *argv[])
{
	/* * TO-DO: 변수는 업무에 맞게 데이터 송수신 구조체를 정의하여 사용합니다. * 필요시 전역변수를 선언하여 사용합니다. */
	int sock;
	int recv_len;
	char recv_buf[4096];
	char send_buf[4096];
	if((sock = TCPIPconnect(SERVER_NAME, SERVER_PORT)) == -1)
	{
		fprintf(stderr, "tcp_connect error: %s\n", strerror(errno));
		return -1;
	}
	/* TO-DO: 이 함수는 실제 업무 로직을 구현합니다. */

	while(1)
	{
		if((TCPIPsend(sock, send_buf, 2048, 0)) == -1)
		{
			/* TO-DO : 오류 로그 생성 */
			exit(1);
		}

		/* TO-DO : 입력한 데이터로 처리하는 로직 */
		// ......
		if((recv_len = TCPIPrecv(sock, recv_buf, 2048, 0)) == -1)
		{
			/* TO-DO : 오류 로그 생성 */
			exit(1);
		}
	}
	close(sock);
	return 0;
}
