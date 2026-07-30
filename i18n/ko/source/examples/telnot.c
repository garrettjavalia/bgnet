/*
** telnot.c -- telnet은 아니지만, 안내서 데모에서 telnet 대신
**             사용할 수 있습니다.
**
** 이 프로그램은 telnet 프로토콜을 전혀 구현하지 않습니다.
**
** 사용법: telnot hostname port
**
** 그런 다음 내용을 입력하고 RETURN을 눌러 보내세요. (현재 터미널의
** 라인 규칙을 사용하며, 아마 줄 단위 버퍼링이 적용되어 RETURN을 누르기 전에는
** 아무것도 전송되지 않을 것입니다.) 데이터를 받으면 표준 출력으로 출력합니다.
**
** 빠져나가려면 ^C를 누르세요.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>

#include <arpa/inet.h>

#define BUFSIZE 1024

/**
 * sockaddr를 얻습니다. IPv4 또는 IPv6
 */
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/**
 * 메인 함수
 */
int main(int argc, char *argv[])
{
	int sockfd;  
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 3) {
	    fprintf(stderr,"usage: telnot hostname port\n");
	    exit(1);
	}

	char *hostname = argv[1];
	char *port = argv[2];

	// 연결을 시도합니다

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// 모든 결과를 순회하면서 연결 가능한 첫 번째 것에 연결합니다
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			//perror("telnot: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			//perror("telnot: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	// 연결되었습니다!

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);

	printf("Connected to %s port %s\n", s, port);
	printf("Hit ^C to exit\n");

	freeaddrinfo(servinfo); // 이 구조체는 이제 필요 없습니다

	// 들어오는 데이터(읽기 준비됨)를 위해 stdin과 sockfd를 poll합니다
	struct pollfd fds[2];

	fds[0].fd = 0;
	fds[0].events = POLLIN;

	fds[1].fd = sockfd;
	fds[1].events = POLLIN;

	// 주 루프
	for(;;) {
		if (poll(fds, 2, -1) == -1) {
			perror("poll");
			exit(1);
		}

		for (int i = 0; i < 2; i++) {

			// 읽을 준비가 되었는지 확인합니다
			if (fds[i].revents & POLLIN) {

				int readbytes, writebytes;
				char buf[BUFSIZE];

				// 데이터를 어디에 쓸지 계산합니다. stdin(0)이면
				// sockfd에 씁니다. sockfd라면 stdout(1)에 씁니다.
				int outfd = fds[i].fd == 0? sockfd: 1;

				// 여기에서는 read()와 write()를 사용합니다. 이 함수들은
				// 소켓뿐 아니라 모든 fd에서 동작하기 때문입니다. stdin과
				// stdout은 소켓이 아니므로 send()와 recv()는 실패할 것입니다.
				if ((readbytes = read(fds[i].fd, buf, BUFSIZE)) == -1) {
					perror("read");
					exit(2);
				}

				char *p = buf;
				int remainingbytes = readbytes;

				// 모든 데이터를 씁니다
				while (remainingbytes > 0) {
					if ((writebytes = write(outfd, p, remainingbytes)) == -1) {
						perror("write");
						exit(2);
					}

					p += writebytes;
					remainingbytes -= writebytes;
				}
			}
		}
	}

	// 여기에 도달하지 않습니다. 종료하려면 ^C를 쓰세요.

	return 0;
}
