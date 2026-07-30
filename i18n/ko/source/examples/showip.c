/*
** showip.c
**
** 명령줄로 받은 호스트의 IP 주소를 보여줍니다
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
	struct addrinfo hints, *res, *p;
	int status;
	char ipstr[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"usage: showip hostname\n");
	    return 1;
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;  // IPv4와 IPv6 어느 쪽이든
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}

	printf("IP addresses for %s:\n\n", argv[1]);

	for(p = res;p != NULL; p = p->ai_next) {
		void *addr;
		char *ipver;
        struct sockaddr_in *ipv4;
        struct sockaddr_in6 *ipv6;

		// 주소 자체에 대한 포인터를 얻습니다.
		// IPv4와 IPv6에서는 필드가 다릅니다:
		if (p->ai_family == AF_INET) { // IPv4
			ipv4 = (struct sockaddr_in *)p->ai_addr;
			addr = &(ipv4->sin_addr);
			ipver = "IPv4";
		} else { // IPv6
			ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			addr = &(ipv6->sin6_addr);
			ipver = "IPv6";
		}

		// IP를 문자열로 변환해서 출력합니다:
		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
		printf("  %s: %s\n", ipver, ipstr);
	}

	freeaddrinfo(res); // 연결 리스트를 해제합니다

	return 0;
}
