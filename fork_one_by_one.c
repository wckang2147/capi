/*
* client와 접속시 fork()하는 서버 model
* sample program...
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
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

void child_main(void);


int main(int argc, char *argv[])
{
    int     idx;
    int     fd;
    int     sockfd;
    struct  sockaddr_in client;
    struct  sigaction   act_ignore;

    /* 
    * background program으로 실행과 terminal에 의한 signal 발생을 배제함. (Control-C 등)
    * 최초실행 프로그램은 종료하여 부모 process id가 1인 child 프로세스가 됨.
    * 또한 session 그룹의 leader가 아닌 프로세스가 됨
    */
    if(fork() != 0) {
        /* child process가 아니면 종료 */
        exit(0);
    }
    
    /*
    * 새로운 session을 생성하여 session그룹의 leader가 됨
    * 새로운 session은 terminal의 영향을 받지 않음.
    */
    if(setsid() == -1) {
        fprintf(stderr, "NEW SESSION ERROR: %s\n", strerror(errno));
        exit(1);
    }
    
    /*
    * 프로세스의 home directory를 ROOT디렉토리로 변경함.
    */
    if(chdir("/") == -1) {
        fprintf(stderr, "CHDIR ERROR: %s\n", strerror(errno));
        exit(1);
    }
    

    /*
    * 표준입력/표준출력/표준오류 처리를 할 수 없도록 하기 위하여 /dev/null 파일을 open합니다.
    */
    if((fd = open("/dev/null", O_RDWR)) == -1) {
        exit(1);
    }
    
    // 표준입력의 동작을 못하도록 redirect 처리함.
    // scanf(), fgets(..., stdin) 등의 함수는 정상동작하나 입력은 안됨.
    dup2(fd, 0); 

    // 표준출력을 출력하지 못하도록 redirect 처리함.
    // printf() 등의 함수는 정상동작하나 출력은 안됨
    dup2(fd, 1); 
    
    // 표준오류를 출력하지 못하도록 redirect 처리함.
    // fprintf(stderr, ...)등의 함수는 정상동작하나 출력은 안됨
    dup2(fd, 2); 

    close(fd);   // 불필요한 fd는 close함

    
    /*
    * signal 처리 필수
    */
    memset(&act_ignore, 0x00, sizeof(act_ignore));
    act_ignore.sa_handler    = SIG_IGN;
    sigaction(SIGCLD,  &act_ignore, NULL);
    sigaction(SIGSEGV, &act_ignore, NULL);
    
    if((sockfd = TCPIPserver(SERVICE_PORT)) == -1) {
        /* TO-DO : 오류 로그 생성 */
        exit(1);
    }
    
    while(1) {
        if((acpt_sock = TCPIPaccept(sockfd, &client)) == -1) {
            /* TO-DO : 오류 로그 생성 */
            continue;
        }

        if(fork() == 0) {
            child_main();
            exit(0); // 반드시 종료하게 처리해야 함.
        } else {
            /* 부모 프로세스는 접속된 socket이 필요없으므로 close함. */
            close(acpt_sock);
        }
    }
    return 0;
}

void child_main(void)
{
    /* TO-DO: 이 함수는 실제 업무 로직을 구현합니다. */

    /* 
    * TO-DO: 변수는 업무에 맞게 데이터 송수신 구조체를 정의하여 사용합니다.
    * 필요시 전역변수를 선언하여 사용합니다.
    */
    int recv_len;
    char recv_buf[4096];
    char send_buf[4096];
    
    while(1) {
        if((recv_len = TCPIPrecv(acpt_sock, recv_buf, 2048, 0)) == -1) {
            /* TO-DO : 오류 로그 생성 */
            exit(1);
        }
        
        /* TO-DO : 입력한 데이터로 처리하는 로직  */
        // ......
        
        if((TCPIPsend(acpt_sock, send_buf, 2048, 0)) == -1) {
            /* TO-DO : 오류 로그 생성 */
            exit(1);
        }
    }
}
