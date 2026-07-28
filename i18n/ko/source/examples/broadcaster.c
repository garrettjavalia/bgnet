/*
** broadcaster.c -- talker.c와 같은 데이터그램 "클라이언트",
**                  다만 이 프로그램은 브로드캐스트를 할 수 있습니다
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVERPORT 4950	// 사용자들이 연결할 포트

int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in their_addr; // 연결자의 주소 정보
	struct hostent *he;
	int numbytes;
	int broadcast = 1;
	//char broadcast = '1'; // 동작하지 않으면 이것을 써 보세요

	if (argc != 3) {
		fprintf(stderr,"usage: broadcaster hostname message\n");
		exit(1);
	}

	if ((he=gethostbyname(argv[1])) == NULL) {  // 호스트 정보를 얻습니다
		perror("gethostbyname");
		exit(1);
	}

	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	// 이 호출이 브로드캐스트 패킷을 보낼 수 있게 만듭니다:
	if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast,
		sizeof broadcast) == -1) {
		perror("setsockopt (SO_BROADCAST)");
		exit(1);
	}

	their_addr.sin_family = AF_INET;	 // 호스트 바이트 순서
	their_addr.sin_port = htons(SERVERPORT); // 네트워크 바이트 순서
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

	numbytes = sendto(sockfd, argv[2], strlen(argv[2]), 0,
			 (struct sockaddr *)&their_addr, sizeof their_addr);

	if (numbytes == -1) {
		perror("sendto");
		exit(1);
	}

	printf("sent %d bytes to %s\n", numbytes,
		inet_ntoa(their_addr.sin_addr));

	close(sockfd);

	return 0;
}
