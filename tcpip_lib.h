#ifndef __TCPIP_LIB_H__
#define __TCPIP_LIB_H__

#include <netinet/in.h>
#include <netdb.h>

/* tcp client가 tcp 서버로 접속합니다. */
int     TCPIPconnect(const char *addr, int port);

/* tcp server socket을 생성후 bind합니다. */
int     TCPIPserver(int port);

/* tcp client 접속을 accept하여 새로운 socket을 생성합니다. */
int     TCPIPaccept(int server_fd, struct sockaddr_in *client);

/* 접속된 socket으로 데이터를 전송합니다.  */
ssize_t TCPIPsend(int sockfd, const void *buf, size_t len, int flags);

/* 접속된 socket으로부터 데이터를 수신합니다.  */
ssize_t TCPIPrecv(int sockfd, void *buf, size_t len, int flags);

/* 생성된 socket을 close합니다.  */
int     TCPIPclose(int sockfd);

/* socket의 local의 정보(IP, port번호)를 얻습니다.  */
int     TCPIPgetSockName(int sockfd, struct sockaddr_in *info);

/* socket descriptor로 부터 자신의 IP 주소를 dotted-decimal notation으로 얻습니다. */
char *  TCPIPgetLocalIpAddress(char *ip_addr, int sockfd);

/* 자신의 port번호를 얻습니다. */
int     TCPIPgetLocalPortNo(int sockfd);

/* socket의 peer(상대방)의 정보(IP, port번호)를 얻습니다. */
int     TCPIPgetPeerName(int sockfd, struct sockaddr_in *info);

/* socket descriptor로 부터 상대방 IP 주소를 dotted-decimal notation으로 얻습니다. */
char *  TCPIPgetPeerIpAddress(char *ip_addr, int sockfd);

/* socket descriptor로 부터 상대방 port번호를 얻습니다. */
int     TCPIPgetPeerPortNo(int sockfd);

/* IP 주소를 dotted-decimal notation으로 얻습니다. */
char *  TCPIPgetIpAddress(char *ip_addr, const struct sockaddr_in *info);

/* port 번호를 얻습니다. */
int     TCPIPgetPortNo(const struct sockaddr_in *info);


#endif /* end of __TCPIP_LIB_H__ */

 

 