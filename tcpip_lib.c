#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <tcpip_lib.h>

/*
* 주소 정보가 IP address인지 domain명인지를 판단하기 위한 함수
* IP address이면 1 그렇지 않으면 0을 return 함
*/
static int is_ipaddr(const char *addr)
{
    while(*addr) {
        if(('0' <= *addr && *addr <= '9') || *addr == '.') {
            addr++;
            continue;
        } else {
            return 0;
        }
    }
    return 1;
}

/*===================================================================================
* client socket을 생성하여 server socket으로 접속합니다.
* 파라미터: 
*       addr : 접속할 서버의 주소
*              IP address 또는 domain 형식 (ex. 127.0.0.1 또는 downman.tistory.com)
*       port : 접속할 port 번호
* return :
*       0이상 : 접속한 socket descriptor
*       -1    : 오류발생, 상세 오류는 errno 전역변수 확인
===================================================================================*/
int TCPIPconnect(const char *addr, int port)
{
    struct hostent *he;
    struct sockaddr_in server_addr;
    int  sock;

    memset(&server_addr, 0x00, sizeof(struct sockaddr_in));
    server_addr.sin_family    = AF_INET;
    server_addr.sin_port      = htons(port);

    if(is_ipaddr(addr)) {  // IP 주소로 접속할려면...
        server_addr.sin_addr.s_addr  = inet_addr(addr);
    } else { // domain명으로 접속하려면...
        if((he = gethostbyname(addr)) == NULL) {
            return -1;
        }

        memcpy(&server_addr.sin_addr.s_addr, he->h_addr, he->h_length);
    }
    
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return -1;
    }

    if(connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) == -1) {
        close(sock);
        return -1;
    }
    return sock;
}

/*===================================================================================
* server socket을 생성합니다.
* 파라미터 :
*        port : 서버 소켓의 port 번호
* return :
*        0 이상 : socket()/bind()가 완료된 server socket 
*        -1     : 오류발생, 상세 오류는 errno 전역변수 확인
===================================================================================*/
int TCPIPserver(int port)
{
    int    server_fd;
    int    reuse        = 1;
    struct sockaddr_in server;

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return -1;
    }

    /* 
    * 프로그램을 restart해야하는 경우에 프로그램 종료후 바로 재실행하는 경우 
    * SO_REUSEADDR을 설정하지 않으면 1분이상 port 번호가 해제되지 않아서 bind 오류가 발생함.
    */
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    server.sin_family      = AF_INET;
    server.sin_port        = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_fd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1) {
        return -1;
    }

    if(listen(server_fd, 50) == -1) {
        return -1;
    }
    
    return server_fd;
}

/*===================================================================================
* TCPIPaccept : client의 접속 요청을 받아들여 client와 연결된 새로운 socket을 생성합니다.
* 파라미터 :
*        server_fd : TCPIPserver()가 return한 server socket descriptor
*        client    : 접속한 client의 IP 및 port 정보
* return :
*        0 이상 : client와 접속한 socket descirptor 
*        -1     : 오류발생, 상세 오류는 errno 전역변수 확인
===================================================================================*/
int TCPIPaccept(int server_fd, struct sockaddr_in *client)
{
    int connected_fd;
    socklen_t addr_len = 0;
    
    if(client != NULL) {
        addr_len = sizeof(struct sockaddr_in);
    }
    if((connected_fd = accept(server_fd, (struct sockaddr *)client, &addr_len)) == -1) {
        return -1;   
    }
    return connected_fd;
}


/*===================================================================================
* TCPIPsend : 연결된 socket으로 데이터를 전송합니다.
* 파라미터 :
*        sockfd : TCPIPconnect() 또는 TCPIPaccept()가 reutrn한 socket descriptor
*        buf    : 전송할 데이터
*        len    : 전송할 데이터 byte수
*        flags  : 일반적으로 0을 설정합니다.  
*                 MSG_DONTROUTE, MSG_DONTWAIT, MSG_MORE, MSG_OOB 등을 사용할 수 있습니다.
* return :
*        0 이상 : 실제 전송된 데이터 byte수 
*        -1     : 오류발생, 상세 오류는 errno 전역변수 확인
===================================================================================*/
ssize_t TCPIPsend(int sockfd, const void *buf, size_t len, int flags)
{
    ssize_t size;
    
    while((size = send(sockfd, buf, len, flags)) == -1 && errno != EINTR );
    
    return size;
}

/*===================================================================================
* TCPIPrecv : 연결된 socket으로부터 데이터를 수신합니다.
* 파라미터 :
*        sockfd : TCPIPconnect() 또는 TCPIPaccept()가 reutrn한 socket descriptor
*        buf    : 수신후 저장할 데이터 buffer
*        len    : 수신할 데이터 byte 수
*        flags  : 일반적으로 0을 설정합니다.  
*                 MSG_DONTROUTE, MSG_DONTWAIT, MSG_MORE, MSG_OOB 등을 사용할 수 있습니다.
* return :
*        0 이상 : 실제 수신한 데이터 byte수 
*        -1     : 오류발생, 상세 오류는 errno 전역변수 확인
===================================================================================*/
ssize_t TCPIPrecv(int sockfd, void *buf, size_t len, int flags)
{
    ssize_t size;
    
    while((size = recv(sockfd, buf, len, flags)) == -1 && errno != EINTR );
    
    return size;
}


/*===================================================================================
* TCPIPclose : 생성된 socket을 close합니다.
* 파라미터 :
*        sockfd : TCPIPconnect(), TCPIPaccept(), TCPIPserver() 등으로 생성한 socket descriptor
* return :
*        0      : 정상적으로 close 됨.
*        -1     : 오류 발생
===================================================================================*/
int TCPIPclose(int sockfd)
{
    return close(sockfd);
}


/*===================================================================================
* TCPIPgetSockName : socket의 local의 정보(IP, port번호)를 얻습니다.
* 파라미터 :
*        sockfd : TCPIPconnect(), TCPIPaccept(), TCPIPserver() 등으로 생성한 socket descriptor
*        info   : local TCP/IP 정보를 저장할 buffer
* return :
*        0      : 정상처리됨
*        -1     : 오류 발생
===================================================================================*/
int TCPIPgetSockName(int sockfd, struct sockaddr_in *info)
{
    int addr_len = sizeof(struct sockaddr_in);

    return getsockname(sockfd, (struct sockaddr *)info, &addr_len);
}

/*===================================================================================
* TCPIPgetLocalIpAddress : 자신의 IP 주소를 dotted-decimal notation으로 얻습니다.
* 파라미터 :
*        ip_addr : IP 주소를 저장할 문자열 buffer, 16바이트 이상 할당되어 있어야 함.
*        sockfd  : socket descriptor 
* return :
*        ip address 
===================================================================================*/
char * TCPIPgetLocalIpAddress(char *ip_addr, int sockfd)
{
    struct sockaddr_in info;
    TCPIPgetSockName(sockfd, &info);
    
    return TCPIPgetIpAddress(ip_addr, &info);
}

/*===================================================================================
* TCPIPgetLocalPortNo : 자신의 port번호를 얻습니다.
* 파라미터 :
*        ip_addr : IP 주소를 저장할 문자열 buffer, 16바이트 이상 할당되어 있어야 함.
*        sockfd  : socket descriptor 
* return :
*        ip address 
===================================================================================*/

int TCPIPgetLocalPortNo(int sockfd)
{
    struct sockaddr_in info;
    TCPIPgetSockName(sockfd, &info);
    
    return TCPIPgetPortNo(&info);
}

/*===================================================================================
* TCPIPgetPeerName : socket의 peer(상대방)의 정보(IP, port번호)를 얻습니다.
* 파라미터 :
*        sockfd : TCPIPconnect(), TCPIPaccept() 등으로 생성한 socket descriptor
*        info   : local TCP/IP 정보를 저장할 buffer
* return :
*        0      : 정상처리됨
*        -1     : 오류 발생
===================================================================================*/
int TCPIPgetPeerName(int sockfd, struct sockaddr_in *info)
{
    int addr_len = sizeof(struct sockaddr_in);
    
    return getpeername(sockfd, (struct sockaddr *)info, &addr_len);
}


/*===================================================================================
* TCPIPgetPeerIpAddress : 상대방의 IP 주소를 dotted-decimal notation으로 얻습니다.
* 파라미터 :
*        ip_addr : IP 주소를 저장할 문자열 buffer, 16바이트 이상 할당되어 있어야 함.
*        sockfd  : socket descriptor 
* return :
*        ip address 
===================================================================================*/
char * TCPIPgetPeerIpAddress(char *ip_addr, int sockfd)
{
    struct sockaddr_in info;
    TCPIPgetPeerName(sockfd, &info);
    
    return TCPIPgetIpAddress(ip_addr, &info);
}


/*===================================================================================
* TCPIPgetPeerPortNo : 상대방의 port번호를 얻습니다.
* 파라미터 :
*        sockfd  : socket descriptor 
* return :
*        상대방의 port 번호
===================================================================================*/
int TCPIPgetPeerPortNo(int sockfd)
{
    struct sockaddr_in info;
    TCPIPgetPeerName(sockfd, &info);
    
    return TCPIPgetPortNo(&info);
}


/*===================================================================================
* TCPIPgetIpAddress : IP 주소를 dotted-decimal notation으로 얻습니다.
* 파라미터 :
*        ip_addr : IP 주소를 저장할 문자열 buffer, 16바이트 이상 할당되어 있어야 함.
*        info    : TCP/IP 정보가 저장된 구조체
*                  TCPIPaccept(), TCPIPgetPeerName(), TCPIPgetSockName() 등으로 얻습니다.
* return :
*        ip address 
===================================================================================*/
char * TCPIPgetIpAddress(char *ip_addr, const struct sockaddr_in *info)
{
    strcpy(ip_addr, inet_ntoa(info->sin_addr));

    return ip_addr;
}

/*===================================================================================
* TCPIPgetPortNo : port 번호를 얻습니다.
* 파라미터 :
*        info   : TCP/IP 정보가 저장된 구조체
*                 TCPIPaccept(), TCPIPgetpeername(), TCPIPgetsockname() 등으로 얻습니다.
* return :
*        port번호 
===================================================================================*/
int TCPIPgetPortNo(const struct sockaddr_in *info)
{
    return ntohs(info->sin_port);
}

 

 