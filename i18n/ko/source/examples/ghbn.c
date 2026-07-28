/*
** ghbn.c -- 호스트 이름 조회 데모
**
** 이것은 호스트 이름을 얻는 구식 방법입니다
** 대신 getaddrinfo()를 쓰세요.
*/

#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
	int i;
	struct hostent *he;
	struct in_addr **addr_list;

	if (argc != 2) {  // 명령줄 오류 확인
		fprintf(stderr,"usage: ghbn hostname\n");
		return 1;
	}

	if ((he = gethostbyname(argv[1])) == NULL) {  // 호스트 정보 얻기
		herror("gethostbyname");
		return 2;
	}

	// 이 호스트에 대한 정보를 출력합니다:
	printf("Official name is: %s\n", he->h_name);
	printf("    IP addresses: ");
	addr_list = (struct in_addr **)he->h_addr_list;
	for(i = 0; addr_list[i] != NULL; i++) {
		printf("%s ", inet_ntoa(*addr_list[i]));
	}
	printf("\n");

	return 0;
}
