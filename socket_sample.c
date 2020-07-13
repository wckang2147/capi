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

/*
* tcpip_lib.c와 tcpip_lib.h는 최상단의 링크에서 다운받으세요.
*/
#include "tcpip_lib.h"

/* 서버 port번호를 정의합니다. 업무에 따라서 정의합니다.*/
#define SERVICE_PORT    6905

/* client와 접속된 socket은 전역변수로 설정하는 것이 좋습니다. */
int  acpt_sock;


#include "tcpip_lib.h"
#define SERVER_NAME "localhost" // 서버에 대한 domain이나 IP address 정의
#define SERVER_PORT 7654 // 서버에 대한 port번호 설정

int communicatewithServer(char *msg)
{
	int sock;
	int recv_len;
	char recv_buf[4096];
	char send_buf[4096];
	FILE *fp = NULL;
	if((sock = TCPIPconnect(SERVER_NAME, SERVER_PORT)) == -1)
	{
		fprintf(stderr, "tcp_connect error: %s\n", strerror(errno));
		return -1;
	}

	if((fp = fdopen(sock, "r+")) != NULL)
	{
		return 1;
	}

	setvbuf(fp, NULL, _IOLBF, 0);
	if(fgets(recv_buf, 4096, fp) != NULL)
	{
		printf("%s\n", recv_buf);
		fprintf(fp, "hw\n");
	}
	fclose(fp);
	return 0;
}

int sendToServer2(char *msg)
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

	strcpy(send_buf, msg);

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

int main(int argc, char *argv[])
{
    int     idx;
    int     fd;
    int     sockfd;
    struct  sockaddr_in client;
    struct  sigaction   act_ignore;

    int recv_len;
    char recv_buf[4096];
    char send_buf[4096];

    if((sockfd = TCPIPserver(SERVICE_PORT)) == -1) {
        /* TO-DO : 오류 로그 생성 */
    	printf("Cannot Create TCPIP Server\n");
        exit(1);
    }

    while(1) {
        if((acpt_sock = TCPIPaccept(sockfd, &client)) == -1) {
            /* TO-DO : 오류 로그 생성 */
        	printf("Error TCPIPaccept\n");
            continue;
        }

        while(1) {
			if((recv_len = TCPIPrecv(acpt_sock, recv_buf, 2048, 0)) == -1)
			{
				/* TO-DO : 오류 로그 생성 */
				exit(1);
			}

			//char msg[1024]={0,};
			/* TO-DO : 입력한 데이터로 처리하는 로직  */

			//communicatewithServer(msg);

			// ......
			if((TCPIPsend(acpt_sock, send_buf, 2048, 0)) == -1)
			{
				/* TO-DO : 오류 로그 생성 */
				exit(1);
			}
        }
    }
    return 0;
}

