/*
** select.c -- select() 데모
*/

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define STDIN 0  // 표준 입력의 파일 설명자

int main(void)
{
	struct timeval tv;
	fd_set readfds;

	tv.tv_sec = 2;
	tv.tv_usec = 500000;

	FD_ZERO(&readfds);
	FD_SET(STDIN, &readfds);

	// writefds와 exceptfds는 신경 쓰지 않습니다:
	select(STDIN+1, &readfds, NULL, NULL, &tv);

	if (FD_ISSET(STDIN, &readfds))
		printf("A key was pressed!\n");
	else
		printf("Timed out.\n");

	return 0;
}
