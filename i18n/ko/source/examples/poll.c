#include <stdio.h>
#include <poll.h>

int main(void)
{
	struct pollfd pfds[1]; // 더 많이 감시하려면 더 크게 만드세요

	pfds[0].fd = 0;          // 표준 입력
	pfds[0].events = POLLIN; // 읽을 준비가 되면 알려줍니다

	// 다른 것도 감시해야 한다면:
	//pfds[1].fd = some_socket; // 어떤 소켓 설명자
	//pfds[1].events = POLLIN;  // 읽을 준비가 되면 알려줍니다

	printf("Hit RETURN or wait 2.5 seconds for timeout\n");

	int num_events = poll(pfds, 1, 2500); // 2.5초 제한 시간

	if (num_events == 0) {
		printf("Poll timed out!\n");
	} else {
		int pollin_happened = pfds[0].revents & POLLIN;

		if (pollin_happened) {
			printf("File descriptor %d is ready to read\n",
                    pfds[0].fd);
		} else {
			printf("Unexpected event occurred: %d\n",
                    pfds[0].revents);
		}
	}

	return 0;
}
