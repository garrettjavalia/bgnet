/*
** getip.c -- 호스트 이름 조회 데모
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
	struct hostent *h;

	if (argc != 2) {  // 명령줄 오류 확인
		fprintf(stderr,"usage: getip address\n");
		exit(1);
	}

	if ((h=gethostbyname(argv[1])) == NULL) {  // 호스트 정보 얻기
		herror("gethostbyname");
		exit(1);
	}

	printf("Host name  : %s\n", h->h_name);
	printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *)h->h_addr)));
   
   return 0;
}
