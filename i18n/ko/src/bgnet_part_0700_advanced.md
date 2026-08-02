# 약간 더 고급스러운 기술

이것들은 _진짜로_ 고급 기술인 것은 아니지만 우리가 지금까지 다룬 기본적인 것들을
벗어나고 있습니다. 사실 여러분이 여기까지 왔다면 스스로가 기본적인 유닉스 네트워크
프로그래밍을 꽤 잘 한다고 생각하셔도 됩니다. 축하합니다.

이제 여러분이 소켓에 대해서 배우고 싶어 할 조금 더 난해한 것들의 세상으로
용감하게 가봅시다. 시작합니다!

## 블로킹 {#blocking}

[i[Blocking]<]

블로킹. 한 번쯤 들어보셨을 것입니다. 그게 무엇일까요? 간단히 말씀드리자면
호출이 데이터를 기다리느라 반환하지 못하는 상태입니다. 기술적으로는 이 호출이
"블록된다"고 합니다. 위에서 `listener`를 실행하면 패킷이 도착할 때까지 그대로
기다리고 있다는 것을 눈치채셨을 것입니다. 내부적으로는 `recvfrom()`이 호출되었지만
데이터가 없어서 데이터가 도착할 때까지 블록됩니다. (즉 그동안 대기합니다.)

많은 함수가 블록됩니다. `accept()`와 모든 `recv()` 함수도 블록됩니다.
이 함수들이 블록되도록 허용되어 있기 때문입니다.
`socket()`으로 소켓 설명자를 처음 만들 때 커널이 이 소켓을 블로킹
소켓으로 설정합니다. [i[Non-blocking sockets]] 소켓이 블록되지 않게 하려면
않길 원한다면 [i[`fcntl()` function]] `fcntl()`을 호출해야 합니다:

```{.c .numberLines}
#include <unistd.h>
#include <fcntl.h>
.
.
.
sockfd = socket(PF_INET, SOCK_STREAM, 0);
fcntl(sockfd, F_SETFL, O_NONBLOCK);
.
.
.
```

소켓을 논블로킹으로 설정하면 정보를 얻기 위해서 실질적으로 소켓을 "폴링"
할 수 있습니다. 논블로킹 소켓을 읽으려고 할 때
정보가 없다면 블록될 수 없으므로 `-1`을
반환할 것이고 `errno`는 [i[`EAGAIN` macro]] `EAGAIN`이나 [i[`EWOULDBLOCK` macro]]
`EWOULDBLOCK`로 설정될 것입니다.

(잠깐, [i[`EAGAIN` macro]] `EAGAIN` _이나_ [i[`EWOULDBLOCK` macro]] `EWOULDBLOCK`
를 반환한다니 무엇을 확인해야 한다는 말일까요? 명세서에는 사실 여러분의 시스템이 어떤 값을
반환해야 하는지 정의되어 있지 않습니다. 그러므로 이식성을 위해서는 둘을 모두 확인해야 합니다.)

그러나 일반적으로 말하자면 이런 방식의 조사는 좋은 생각이 아닙니다. 여러분의 프로그램이
소켓의 데이터를 기다리면서 바쁜 대기 상태가 되면 여러분의 프로그램은 보통의 프로그램보다 훨씬
CPU 시간을 많이 사용할 것입니다. (역자 주: 특별한 제한을 걸지 않으면 최대 단일 코어
하나를 100% 점유할 수 있습니다.) 읽을 데이터가 있는지 확인하는 더
더 나은 방법은 [i[`poll()` function]] `poll()`을 다루는 다음 절에 있습니다.

[i[Blocking]>]

## `poll()`---동기 입출력 다중화 {#poll}

[i[poll()]<]

여러분이 정말로 해야 하는 일은 소켓 *한 무더기*를 한 번에 감시하고 그 중에
데이터가 준비된 것을 처리하는 것입니다. 이런 방식을 통해서 여러분은 모든 소켓을
지속적으로 조사하지 않아도 여러 개의 소켓 중 어떤 것이 데이터가 준비되었는지 알
수 있습니다.

> 경고 : `poll()`은 엄청나게 많은 수의 연결을 처리할 때 끔찍하게 느려집니다.
> 이런 상황에서는 [fl[libevent|https://libevent.org/]] 같은 이벤트 라이브러리
> 를 사용하면 더 좋은 성능을 얻을 수 있습니다. 이런 라이브러리는 여러분의 운영체제에서
> 사용할 수 있는 가장 빠른 방법을 사용하려고 시도할 것입니다.

그래서 어떻게 폴링을 피할 수 있을까요? 약간 아이러니하게도 `poll()` 시스템 함수를
사용해서 폴링을 피할 수 있습니다. 간단히 말하자면 모든 번거로운 작업을 운영체제에
맡기고, 어떤 소켓에 읽을 데이터가 준비되면
알려달라고 부탁하는 것입니다. 그 동안 우리의 프로세스는 대기 상태가 될 수 있고 시스템
자원을 아낄 수 있습니다.

전체적인 계획은 우리가 감시하고 싶은 소켓 설명자와 우리가 감시하고 싶은 이벤트의
종류에 대한 정보를 담은 `struct pollfd`의 배열을 보관하는 것입니다.
운영체제는 해당하는 종류의 이벤트가 발생(예를 들어 "소켓에서 데이터를 읽을 수 있다!"
같은 이벤트)하거나 사용자가 지정한 제한 시간이 지날 때까지 `poll()` 호출은 블록됩니다.

유용하게도 `listen()` 상태인 소켓은 새로운 연결이 `accept()`될 수 있는 상태가
되었을 때 "읽을 준비됨"을 반환할 것입니다.

이만하면 충분히 이야기했습니다. 이것을 쓰는 방법은 어떨지 봅시다.

```{.c}
#include <poll.h>

int poll(struct pollfd fds[], nfds_t nfds, int timeout);
```

`fds`는 우리의 정보(어떤 소켓을 무엇을 위해 감시할지)의 배열입니다.
`nfds`는 배열에 담긴 요소의 개수입니다. `timeout`은 밀리초 단위의 제한 시간입니다.
이 함수는 이벤트가 발생한 요소의 개수를 반환합니다.

위에 등장하는 구조체는 무엇인지 살펴봅시다.

[i[`struct pollfd` type]]

```{.c}
struct pollfd {
    int fd;         // 소켓 설명자
    short events;   // 우리가 관심 있는 이벤트의 비트맵
    short revents;  // poll()이 반환하는 시점에 발생한 이벤트의 비트맵
};
```

이 구조체 타입의 배열을 하나 설정하고 각각의 `fd` 필드를 우리가 관찰하고 싶은 소켓 설명자로
설정합니다. 그리고 각각의 `events` 필드는 우리가 관심 있는 이벤트로 설정하는 것입니다.

`events` 필드는 아래 값들을 비트 단위 OR로 묶은 결과값입니다.

| 매크로    | 설명                                                            |
| --------- | --------------------------------------------------------------- |
| `POLLIN`  | 이 소켓이 데이터를 `recv()`할 준비가 되면 알려줍니다.           |
| `POLLOUT` | 이 소켓에 대기하지 않고 데이터를 `send()`할 수 있으면 알려줍니다. |
| `POLLHUP` | 원격 쪽이 연결을 닫으면 알려줍니다.                             |

`struct pollfd`의 배열을 준비하면 `poll()`에 그것을 넘길 수 있습니다. 배열의 크기와
밀리초 단위의 제한 시간도 같이 넘겨야 합니다. (영원히 기다리려면 음수를 지정하면 됩니다.)

`poll()`이 반환하면 이벤트가 발생했음을 나타내는 `POLLIN`이나 `POLLOUT`이
설정되었는지 보기 위해서 `revents` 필드를 확인할 수 있습니다.

(실제로는 `poll()` 호출로 할 수 있는 것들이 더 있습니다. 자세한 내용은
[아래의 `poll()` 맨페이지](#pollman)를 참고하세요.)

여기 표준 입력에서 데이터를 읽어 들일 수 있을 때까지(예를 들어 여러분이 줄바꿈을 입력할 때까지)
2.5초를 기다리는 [flx[예제|poll.c]]가 있습니다.

```{.c .numberLines}
#include <stdio.h>
#include <poll.h>

int main(void)
{
    struct pollfd pfds[1]; // 더 많은 것들을 관찰하고 싶다면 더 크게 하세요.

    pfds[0].fd = 0;          // 표준 입력
    pfds[0].events = POLLIN; // 읽을 준비가 되면 알려줍니다.

    // 만약 다른 것들도 관찰하고 싶다면
    //pfds[1].fd = some_socket; // 임의의 소켓 설명자
    //pfds[1].events = POLLIN;  // 읽을 준비가 되면 알려줍니다.

    printf("엔터 키를 누르거나 시간 초과될 때까지 2.5초를 기다리세요\n");

    int num_events = poll(pfds, 1, 2500); // 2.5초 제한 시간

    if (num_events == 0) {
        printf("Poll 시간 초과!\n");
    } else {
        int pollin_happened = pfds[0].revents & POLLIN;

        if (pollin_happened) {
            printf("파일 설명자 %d를 읽을 준비가 되었습니다\n", pfds[0].fd);
        } else {
            printf("예상하지 못한 이벤트가 발생했습니다: %d\n", pfds[0].revents);
        }
    }

    return 0;
}
```

`poll()`이 `pfds` 배열에서 이벤트가 발생한 요소의 개수를 반환한다는 것을 다시
기억하세요. 배열의 *어떤* 요소에서 이벤트가 발생했는지는 알려주지 않지만 몇 개의
`revents` 필드가 0이 아닌 값으로 설정되었는지는 알려줍니다. 이것을 활용해서
반환된 숫자만큼의 0이 아닌 `revents`를 읽은 후에는 스캔을 중단할 수 있습니다.

몇 가지 질문이 떠오를 것입니다. `poll()`에 넘겨준 집합에 새 파일 설명자를 추가하려면
어떻게 해야 할까요? 이를 위해서 단순히 여러분의 모든 필요에 부합할 만큼 충분한
크기의 배열을 만들거나 추가적인 공간이 필요할 때 `realloc()`을 사용하세요.

집합에서 요소를 제거하려면 어떻게 해야 할까요? 이것을 위해서 여러분은 배열의 마지막
요소를 삭제할 요소에 덮어씌우고, `poll()`의 개수 인수에는 하나 더 적은 값을 전달하세요.
(역자 주: 이것은 배열에서 임의의 요소 1개를 빠르게 제거하기 위해 일반적으로
사용하는 방법입니다.) 다른 한 가지 방법은 `fd` 필드를 음수로 설정하는 것이며
`poll()`은 해당 요소를 무시할 것입니다.

이 모든 것을 여러분이 `telnet`할 수 있는 하나의 채팅 서버에 합치려면 어떻게
해야 할까요?

우리가 할 일은 리스너 소켓을 시작한 후에 그것을 `poll()`할 파일 설명자
집합에 추가하는 일입니다. (그 파일 설명자는 들어오는 연결이 있을 때 읽기 준비된
상태가 될 것입니다.)

그 후에 새로운 연결을 우리의 `struct pollfd` 배열에 추가하면 됩니다.
만약 배열의 크기가 부족하다면 동적으로 키우면 됩니다.

연결이 닫힌 후에는 그것을 배열로부터 제거합니다.

연결이 읽기 준비되면 우리는 그것에서 데이터를 읽어들인 후 다른 모든 연결에
전송합니다. 그렇게 해서 사용자들은 서로가 입력한 내용을 볼 수 있습니다.

이제 [flx[이 폴 서버|pollserver.c]]를 한 번 시험해보세요. 이것을 하나의
창에서 실행한 후 몇 개의 다른 터미널 창에서 `telnet localhost 9034`를
실행해 보세요. 여러분이 하나의 창에서 입력하는 것을 (여러분이 엔터 키를 누른 후에)
다른 창들에서 볼 수 있어야 합니다.

그뿐 아니라 여러분이 `CTRL-]`를 누른 후 `quit`을 입력해서 `telnet`을 종료할
경우 서버는 연결 종료를 감지하고 그 연결을 파일 설명자 배열에서 제거할 것입니다.

```{.c .numberLines}
/*
** pollserver.c -- 허술한 다중 사용자 대화 서버
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#define PORT "9034"   // 리스닝할 포트

/*
 * 소켓 주소를 IP 주소 문자열로 변환합니다.
 * addr: struct sockaddr_in 또는 struct sockaddr_in6
 */
const char *inet_ntop2(void *addr, char *buf, size_t size)
{
	struct sockaddr_storage *sas = addr;
	struct sockaddr_in *sa4;
	struct sockaddr_in6 *sa6;
	void *src;

	switch (sas->ss_family) {
		case AF_INET:
			sa4 = addr;
			src = &(sa4->sin_addr);
			break;
		case AF_INET6:
			sa6 = addr;
			src = &(sa6->sin6_addr);
			break;
		default:
			return NULL;
	}

	return inet_ntop(sas->ss_family, src, buf, size);
}

/*
 * 리스닝 소켓을 반환합니다.
 */
int get_listener_socket(void)
{
	int listener;	 // 리스닝 소켓 설명자
	int yes=1;		// 아래의 setsockopt() SO_REUSEADDR용
	int rv;

	struct addrinfo hints, *ai, *p;

	// 소켓을 얻어서 바인드합니다
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
		fprintf(stderr, "pollserver: %s\n", gai_strerror(rv));
		exit(1);
	}

	for(p = ai; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol);
		if (listener < 0) {
			continue;
		}

		// 성가신 "address already in use" 오류 메시지를 피합니다
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			continue;
		}

		break;
	}

	// 여기까지 왔다면 바인드하지 못했다는 뜻입니다
	if (p == NULL) {
		return -1;
	}

	freeaddrinfo(ai); // 이 구조체는 이제 필요 없습니다

	// 리스닝합니다
	if (listen(listener, 10) == -1) {
		return -1;
	}

	return listener;
}

/*
 * 집합에 새 파일 설명자를 추가합니다.
 */
void add_to_pfds(struct pollfd **pfds, int newfd, int *fd_count,
		int *fd_size)
{
	// 공간이 부족하면 pfds 배열 공간을 늘립니다
	if (*fd_count == *fd_size) {
		*fd_size *= 2; // 두 배로 늘립니다
		*pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
	}

	(*pfds)[*fd_count].fd = newfd;
	(*pfds)[*fd_count].events = POLLIN; // 읽을 준비 상태를 확인합니다
	(*pfds)[*fd_count].revents = 0;

	(*fd_count)++;
}

/*
 * 집합의 지정한 인덱스에서 파일 설명자를 제거합니다.
 */
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
	// 마지막 요소를 이 위치로 복사합니다
	pfds[i] = pfds[*fd_count-1];

	(*fd_count)--;
}

/*
 * 들어오는 연결을 처리합니다.
 */
void handle_new_connection(int listener, int *fd_count,
		int *fd_size, struct pollfd **pfds)
{
	struct sockaddr_storage remoteaddr; // 클라이언트 주소
	socklen_t addrlen;
	int newfd;  // 새로 accept()한 소켓 설명자
	char remoteIP[INET6_ADDRSTRLEN];

	addrlen = sizeof remoteaddr;
	newfd = accept(listener, (struct sockaddr *)&remoteaddr,
			&addrlen);

	if (newfd == -1) {
		perror("accept");
	} else {
		add_to_pfds(pfds, newfd, fd_count, fd_size);

		printf("pollserver: new connection from %s on socket %d\n",
				inet_ntop2(&remoteaddr, remoteIP, sizeof remoteIP),
				newfd);
	}
}

/*
 * 일반 클라이언트 데이터와 연결 종료를 처리합니다.
 */
void handle_client_data(int listener, int *fd_count,
		struct pollfd *pfds, int *pfd_i)
{
	char buf[256];	// 클라이언트 데이터용 버퍼

	int nbytes = recv(pfds[*pfd_i].fd, buf, sizeof buf, 0);

	int sender_fd = pfds[*pfd_i].fd;

	if (nbytes <= 0) { // 오류가 발생했거나 클라이언트가 연결을 닫았습니다
		if (nbytes == 0) {
			// 연결이 닫혔습니다
			printf("pollserver: socket %d hung up\n", sender_fd);
		} else {
			perror("recv");
		}

		close(pfds[*pfd_i].fd); // 잘 가!

		del_from_pfds(pfds, *pfd_i, fd_count);

		// 방금 삭제한 슬롯을 다시 검사합니다
		(*pfd_i)--;

	} else { // 클라이언트에서 정상 데이터를 받았습니다
		printf("pollserver: recv from fd %d: %.*s", sender_fd,
				nbytes, buf);
		// 모두에게 보냅니다!
		for(int j = 0; j < *fd_count; j++) {
			int dest_fd = pfds[j].fd;

			// 리스너와 자기 자신은 제외합니다
			if (dest_fd != listener && dest_fd != sender_fd) {
				if (send(dest_fd, buf, nbytes, 0) == -1) {
					perror("send");
				}
			}
		}
	}
}

/*
 * 기존 연결들을 모두 처리합니다.
 */
void process_connections(int listener, int *fd_count, int *fd_size,
		struct pollfd **pfds)
{
	for(int i = 0; i < *fd_count; i++) {

		// 읽을 준비가 된 항목이 있는지 확인합니다
		if ((*pfds)[i].revents & (POLLIN | POLLHUP)) {
			// 하나 찾았습니다!!

			if ((*pfds)[i].fd == listener) {
				// 리스너라면 새 연결입니다
				handle_new_connection(listener, fd_count, fd_size,
						pfds);
			} else {
				// 그렇지 않으면 일반 클라이언트입니다
				handle_client_data(listener, fd_count, *pfds, &i);
			}
		}
	}
}

/*
 * 메인: 리스너와 연결 집합을 만들고 영원히 반복하면서
 * 연결을 처리합니다.
 */
int main(void)
{
	int listener;	 // 리스닝 소켓 설명자

	// 5개 연결을 담을 공간으로 시작합니다
	// (필요하면 realloc할 것입니다)
	int fd_size = 5;
	int fd_count = 0;
	struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

	// 리스닝 소켓을 설정하고 얻습니다
	listener = get_listener_socket();

	if (listener == -1) {
		fprintf(stderr, "error getting listening socket\n");
		exit(1);
	}

	// 리스너를 집합에 추가합니다.
	// 들어오는 연결을 읽을 준비 이벤트로 보고받습니다
	pfds[0].fd = listener;
	pfds[0].events = POLLIN;

	fd_count = 1; // 리스너용

	puts("pollserver: waiting for connections...");

	// 메인 루프
	for(;;) {
		int poll_count = poll(pfds, fd_count, -1);

		if (poll_count == -1) {
			perror("poll");
			exit(1);
		}

		// 연결들을 순회하며 읽을 데이터를 찾습니다
		process_connections(listener, &fd_count, &fd_size, &pfds);
	}

	free(pfds);
}
```

다음 절에서는 비슷하지만 오래된 함수인 `select()`를 살펴볼 것입니다.
`select()`와 `poll()` 모두 비슷한 기능과 성능을 제공하고 쓰는 방식만
조금 다릅니다. `select()` 쪽이 조금 더 이식성이 좋을지도 모르나 사용하기에는
조금 더 어색할 것입니다. 여러분의 시스템에서 지원되기만 한다면 더 마음에
드는 쪽을 선택하세요.

[i[poll()]>]

## `select()`---동기화된 I/O 멀티플렉싱, 예전 방식 {#select}

[i[`select()` 함수]<]

이 함수는 이상하지만 아주 유용합니다. 다음과 같은 상황을 생각해보세요. 여러분은
서버이고 들어오는 연결을 감지함과 동시에 이미 가진 연결로부터 계속 데이터를
읽어들이고 싶습니다.

별 문제가 없다고 말씀하실지도 모릅니다. 그냥 `accept()`와 몇 개의 `recv()`를
쓰면 될 뿐입니다. 하지만 정말로 그럴까요? `accept()` 호출이 블록되었다면
어떻게 할까요? 어떻게 `recv()`로 동시에 데이터를 받을 수 있을까요?
"논블로킹 소켓을 써봅시다!" 역시 해결책이 아닙니다. CPU를 모조리 쓰고 싶지는 않을
것입니다. 그럼 어떻게 해야 할까요?

`select()`가 여러 소켓을 동시에 관찰할 수 있는 능력을 줍니다. 그것이 어떤 것이
읽을 준비가 되었는지, 어떤 것이 쓸 준비가 되었는지, 그리고 정말로 관심이
있다면 어떤 것에 오류가 발생했는지까지 알려줄 것입니다.

> 경고 한마디: `select()`가 이식성이 아주 좋지만 연결이 아주 많은 상황에서는
> 끔찍하게 느려집니다. 그런 상황에서는 여러분의 시스템에서 쓸 수 있는 가장 빠른
> 방법을 시도하는 [fl[libevent|https://libevent.org/]] 같은 이벤트
> 라이브러리를 쓰면 더 나은 성능을 얻을 수 있습니다.

잡담은 그만하고 `select()`의 개요를 제시하겠습니다.

```{.c}
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int select(int numfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout);
```

이 함수는 `readfds`와 `writefds`, 그리고 `exceptfds`라는
파일 설명자의 "집합들"을 관찰합니다. 만약 여러분이 표준 입력과 몇 개의 소켓
설명자로부터 읽어 들일 수 있는지 확인하고 싶다면 `readfds` 집합에 0과
`sockfd`를 추가하세요. `numfds`는 가장 큰 파일 설명자에 1을 더한 값으로
설정해야 합니다. 이 예제에서는 `sockfd+1`이 될 것이며 이유는 당연히 그것이
표준 입력(`0`)보다 클 것이기 때문입니다.

`select()`가 반환할 때 `readfds`는 여러분이 선택한 파일 설명자 중에서 읽을
준비가 된 것들을 반영하도록 바뀌어 있을 것입니다. 여러분은 그것들을
아래에 나오는 `FD_ISSET()` 매크로로 검사할 수 있습니다.

더 진행하기 전에 이 집합들을 어떻게 조작하는지에 대해 이야기할 것입니다. 각 집합은
`fd_set`형입니다. 이 자료형에 대해서 아래의 매크로들을 쓸 수 있습니다.

| 함수                                                    | 설명                                   |
| ------------------------------------------------------- | -------------------------------------- |
| [i[`FD_SET()` macro]]`FD_SET(int fd, fd_set *set);`     | `fd`를 `set`에 더합니다.               |
| [i[`FD_CLR()` macro]]`FD_CLR(int fd, fd_set *set);`     | `fd`를 `set`에서 제거합니다.           |
| [i[`FD_ISSET()` macro]]`FD_ISSET(int fd, fd_set *set);` | `fd`가 `set`에 있다면 참을 반환합니다. |
| [i[`FD_ZERO()` macro]]`FD_ZERO(fd_set *set);`           | `set`의 모든 요소를 제거합니다.        |

[i[`struct timeval` 형]<]

마지막으로 이 이상한 `struct timeval`은 무엇일까요? 누군가 여러분에게 데이터를
보낼 때까지 무한히 기다리고 싶지 않을 때가 있습니다. 아마도 매 96초마다
실제로는 아무 일도 일어나지 않았어도 "진행 중..."이라고 출력하고 싶을 수도 있습니다.
이 시간 구조체가 제한 시간을 지정할 수 있도록 해 줍니다. 시간이 초과되고 `select()`
가 준비된 파일 설명자를 찾지 못할 경우, 그것은 반환하고 여러분은 처리를 계속할
수 있습니다.

`struct timeval`는 아래와 같은 필드를 가지고 있습니다.

```{.c}
struct timeval {
    int tv_sec;     // 초
    int tv_usec;    // 마이크로초
};
```

단순히 `tv_sec`을 기다리고 싶은 초로, `tv_usec`을 기다리고 싶은 마이크로초로
설정하세요. 그렇습니다. *마이크로*초입니다. 밀리초가 아닙니다. 1밀리초는 1,000마이크로초입니다.
그리고 1,000밀리초는 1초입니다. 그러므로 1초는 1,000,000마이크로초입니다. 왜 "usec"일까요?
"u"는 우리가 "마이크로"를 뜻하기 위해서 쓰는 그리스 문자 μ(뮤)와 닮았기 때문입니다.
또 함수가 반환할 때 `timeout`은 남아있는 시간을 보여주기 위해서 업데이트 _될 수도_
있습니다. 이것은 여러분이 실행 중인 유닉스의 종류에 따라 다릅니다.

와! 마이크로초 해상도의 타이머를 쓸 수 있군요! 사실 별로 기대하지 않는 것이 좋습니다.
여러분이 `struct timeval`을 아무리 작게 설정해도 여러분의 표준 유닉스 타임슬라이스
(역자 주: 커널이 프로세스 스케줄링의 최소 단위로 쓰는 시간)만큼은 기다려야 합니다.

다른 흥미로운 것들도 있습니다. `struct timeval`을 `0`으로 설정하면 `select()`는 여러분의 집합에
있는 모든 파일 설명자를 폴링한 뒤 즉시 시간초과가 될 것입니다. 매개변수 `timeout`을 `NULL`
로 설정하면 절대 시간초과가 되지 않으며 파일 설명자가 준비될 때까지 기다릴 것입니다.
마지막으로 만약 특정 집합을 기다릴 필요가 없다면 그 집합은 `select()`를 호출할 때
NULL로 설정하면 됩니다.

[flx[아래의 코드 조각|select.c]]은 표준 입력에 뭔가 나타날 때까지 2.5초를 기다립니다.

```{.c .numberLines}
/*
** select.c -- select()의 예시
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

    // writefds와 exceptfds는 신경 쓰지 않습니다
    select(STDIN+1, &readfds, NULL, NULL, &tv);

    if (FD_ISSET(STDIN, &readfds))
        printf("키가 눌렸습니다!\n");
    else
        printf("시간이 초과되었습니다.\n");

    return 0;
}
```

만약 여러분이 줄 단위로 버퍼 처리되는 터미널을 사용한다면 엔터를 누르지 않으면
제한 시간이 초과될 것입니다.

이제 여러분 중 일부는 이것이 데이터그램 소켓의 데이터를 기다리는 아주 훌륭한
방법이라고 생각할 것입니다. 그리고 맞습니다. 맞을 _수도_ 있습니다. 일부 유닉스에서는
select를 이 목적으로 쓸 수 있고, 일부에서는 그럴 수 없습니다. 그 방식을 시도하고
싶다면 여러분의 로컬 맨페이지 내용을 참고해야 합니다.

일부 유닉스는 제한 시간이 초과되기까지 남은 시간을 반영하기 위해 여러분의
`struct timeval`를 업데이트합니다. 그러나 다른 것들은 그렇게 하지 않습니다.
만약 이식성 있는 코드를 작성하고자 한다면 그것에 의존해서는 안 됩니다.
(경과한 시간을 알고 싶다면 [i[`gettimeofday()`function]] `gettimeofday()`
을 사용하세요. 실망스럽겠지만 그것이 올바른 방법입니다.)

[i[`struct timeval` 형]>]

만약 읽기 집합에 있는 소켓이 연결을 닫는다면 어떤 일이 생길까요? 그 경우
`select()`는 그 소켓 설명자를 "읽기 준비된 상태"로 설정할 것입니다. 실제로
그 소켓에 `recv()`하면 `recv()`는 `0`을 반환할 것입니다. 그것이 클라이언트가
연결을 닫았음을 알아내는 방법입니다.

`select()`에 관한 흥미로운 이야기가 하나 더 있습니다. 만약
[i[`select()` function-->with `listen()`]]
[i[`listen()` function-->with `select()`]]
`listen()` 작업 중인 소켓을 가지고 있을 경우, 그 소켓의 파일 설명자를 `readfds`
집합에 넣어서 새로운 연결이 있는지 알 수 있습니다.

지금까지 전능한 `select()` 함수에 대해 간단히 알아보았습니다.

그러나 대중적 요구가 있으므로 아래에 심도 있는 예제를 첨부합니다. 불행하게도
위의 아주 단순한 예제와 아래의 예제에는 상당한 차이가 있습니다. 그렇지만 한 번
살펴보고 뒤따르는 설명을 읽어보세요.

[flx[이 프로그램|selectserver.c]]은 단순한 다중 사용자 채팅 서버처럼 동작합니다.
하나의 창에서 이것을 실행한 후 다른 창에서 `telnet`을 통해 접속하세요. ("`telnet
hostname 9034`") 하나의 `telnet` 세션에서 뭔가 입력하면 나머지 모두에서 그 내용이
나타나야 합니다.

```{.c .numberLines}
/*
** selectserver.c -- 허술한 다중 사용자 대화 서버
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT "9034"   // 리스닝할 포트

/*
 * 소켓 주소를 IP 주소 문자열로 변환합니다.
 * addr: struct sockaddr_in 또는 struct sockaddr_in6
 */
const char *inet_ntop2(void *addr, char *buf, size_t size)
{
	struct sockaddr_storage *sas = addr;
	struct sockaddr_in *sa4;
	struct sockaddr_in6 *sa6;
	void *src;

	switch (sas->ss_family) {
		case AF_INET:
			sa4 = addr;
			src = &(sa4->sin_addr);
			break;
		case AF_INET6:
			sa6 = addr;
			src = &(sa6->sin6_addr);
			break;
		default:
			return NULL;
	}

	return inet_ntop(sas->ss_family, src, buf, size);
}

/*
 * 리스닝 소켓을 반환합니다.
 */
int get_listener_socket(void)
{
	struct addrinfo hints, *ai, *p;
	int yes=1;    // 아래의 setsockopt() SO_REUSEADDR용
	int rv;
	int listener;

	// 소켓을 얻어서 바인드합니다
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}

	for(p = ai; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol);
		if (listener < 0) {
			continue;
		}

		// 성가신 "address already in use" 오류 메시지를 피합니다
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			continue;
		}

		break;
	}

	// 여기까지 왔다면 바인드하지 못했다는 뜻입니다
	if (p == NULL) {
		fprintf(stderr, "selectserver: failed to bind\n");
		exit(2);
	}

	freeaddrinfo(ai); // 이 구조체는 이제 필요 없습니다

	// 리스닝합니다
	if (listen(listener, 10) == -1) {
		perror("listen");
		exit(3);
	}

	return listener;
}

/*
 * 새로 들어온 연결을 적절한 집합에 추가합니다
 */
void handle_new_connection(int listener, fd_set *master, int *fdmax)
{
	socklen_t addrlen;
	int newfd;        // 새로 accept()한 소켓 설명자
		struct sockaddr_storage remoteaddr; // 클라이언트 주소
	char remoteIP[INET6_ADDRSTRLEN];

	addrlen = sizeof remoteaddr;
	newfd = accept(listener,
		(struct sockaddr *)&remoteaddr,
		&addrlen);

	if (newfd == -1) {
		perror("accept");
	} else {
		FD_SET(newfd, master); // 마스터 집합에 추가합니다
		if (newfd > *fdmax) {  // 최댓값을 기록합니다
			*fdmax = newfd;
		}
		printf("selectserver: new connection from %s on "
			"socket %d\n",
			inet_ntop2(&remoteaddr, remoteIP, sizeof remoteIP),
			newfd);
	}
}

/*
 * 모든 클라이언트에 메시지를 방송합니다
 */
void broadcast(char *buf, int nbytes, int listener, int s,
               fd_set *master, int fdmax)
{
	for(int j = 0; j <= fdmax; j++) {
		// 모두에게 보냅니다!
		if (FD_ISSET(j, master)) {
			// 리스너와 자기 자신은 제외합니다
			if (j != listener && j != s) {
				if (send(j, buf, nbytes, 0) == -1) {
					perror("send");
				}
			}
		}
	}
}

/*
 * 클라이언트 데이터와 연결 종료를 처리합니다
 */
void handle_client_data(int s, int listener, fd_set *master,
                        int fdmax)
{
	char buf[256];    // 클라이언트 데이터용 버퍼
	int nbytes;

	// 클라이언트에서 온 데이터를 처리합니다
	if ((nbytes = recv(s, buf, sizeof buf, 0)) <= 0) {
		// 오류가 발생했거나 클라이언트가 연결을 닫았습니다
		if (nbytes == 0) {
			// 연결이 닫혔습니다
			printf("selectserver: socket %d hung up\n", s);
		} else {
			perror("recv");
		}
		close(s); // 잘 가!
		FD_CLR(s, master); // 마스터 집합에서 제거합니다
	} else {
		// 클라이언트에서 데이터를 받았습니다
		broadcast(buf, nbytes, listener, s, master, fdmax);
	}
}

/*
 * 메인
 */
int main(void)
{
	fd_set master;    // 마스터 파일 설명자 목록
	fd_set read_fds;  // select()용 임시 파일 설명자 목록
	int fdmax;        // 가장 큰 파일 설명자 번호

	int listener;     // 리스닝 소켓 설명자

	FD_ZERO(&master);    // 마스터 집합과 임시 집합을 비웁니다
	FD_ZERO(&read_fds);

	listener = get_listener_socket();

	// 리스너를 마스터 집합에 추가합니다
	FD_SET(listener, &master);

	// 가장 큰 파일 설명자를 기록합니다
	fdmax = listener; // 지금까지는 이것입니다

	// 메인 루프
	for(;;) {
		read_fds = master; // 복사합니다
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(4);
		}

		// 기존 연결을 순회하며 읽을 데이터를 찾습니다
		for(int i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // 하나 찾았습니다!!
				if (i == listener)
					handle_new_connection(i, &master, &fdmax);
				else
					handle_client_data(i, listener, &master, fdmax);
			}
		}
	}

	return 0;
}
```

코드에 `master`와 `read_fds` 두 개의 파일 설명자 집합이 있음에 주목하세요.
전자인 `master`는 새 연결을 리스닝 소켓 설명자와 현재 연결된 모든 소켓의
설명자를 가집니다.

`master`를 가지는 이유는 `select()`가 사실 여러분이 넘기는 집합을 _변형해서_
읽기 준비된 소켓을 반영하기 때문입니다. 우리는 하나의 `select()` 호출과 다음
호출 사이에서 연결들을 계속 기억해야 하므로 이것들은 다른 곳에 안전하게
보관해야 하는 것입니다. 그래서 우리는 실제로 쓰기 전에 `master`를 `read_fds`에
복사하고 `select()`를 호출하는 것입니다.

하지만 그것은 우리가 새로운 연결을 받을 때마다 그것을 `master` 집합에 추가해야
한다는 의미일까요? 맞습니다! 그리고 연결이 닫힐 때마다 `master` 집합에서 제거해야
할까요? 그렇습니다. 그렇게 해야 합니다.

`listener` 소켓이 읽을 준비가 되었는지 확인한다는 사실에 주목하세요. 준비가 되어 있다면
대기 중인 새로운 연결이 있다는 의미이고, `accept()`한 후에 `master` 집합에 추가합니다.
비슷하게 클라이언트 연결을 읽을 준비가 되고 `recv()`가 `0`을 반환한다면 클라이언트가
연결을 닫았다는 사실을 알 수 있고, 우리는 그 연결을 `master` 집합에서 제거해야 합니다.

클라이언트에 대한 `recv()`가 0이 아닌 값을 반환한다면 우리는 어떤 데이터가 도착했다는
것을 알 수 있습니다. 그러므로 데이터를 받은 후에 `master` 목록을 순회하면서 모든 나머지
연결된 클라이언트들에게 그 데이터를 보냅니다.

지금까지 전능한 `select()` 함수에 대한 그다지 단순하지 않은 개관이었습니다.

모든 리눅스 팬들을 위한 짧은 이야기가 있습니다. 드문 몇몇 상황에서 때때로 리눅스의 `select()`
는 실제로는 읽을 준비가 되어 있지 않음에도 "읽을 준비가 되었다"고 하면서 반환합니다.
이것은 `select()`가 읽기 동작에 대해 대기하지 않을 것이라고 말함에도 `read()`
가 블록될 것임을 의미합니다. 이런! 아무튼 해결책은 읽을 소켓에 [i[`O_NONBLOCK` macro]]
`O_NONBLOCK` 플래그를 설정해서 `EWOULDBLOCK`오류가 발생하도록 하는 것입니다.
(이 오류가 생겨도 무시해도 됩니다.) 소켓을 논블로킹 모드로 설정하는 방법에 대해서는
[`fcntl()` 참조 페이지](#fcntlman)를 참고하세요.

추가로 약간의 여담을 하자면 `select()`와 상당히 비슷한 일을 하지만 파일 설명자
집합을 다른 방식으로 처리하는 [i[`poll()` function]] `poll()`이라는 다른 함수가 있습니다.
[한 번 확인해 보세요!](#pollman)

[i[`select()` 함수]>]

## 부분적인 `send()` 처리하기 {#sendall}

위에서 다룬 [`send()`에 대한 절](#sendrecv)에서 `send()`가 여러분이 전송 요청한 바이트들을
모두 보내지는 않을 수도 있다고 말한 것을 기억하시나요? 여러분은 512바이트를 보내길 원해도 복귀값은
412일 수 있다는 의미입니다. 남은 100바이트에는 무슨 일이 생긴 것일까요?

음, 그것들은 여전히 여러분의 작은 버퍼에 남아서 보내지길 기다리고 있습니다. 여러분의 통제 밖에 있는
상황 때문에 커널이 데이터를 한 덩어리로 전부 보내지는 않기로 결정한 것입니다. 그리고 이제 남은
데이터를 보내는 일은 여러분에게 달려 있습니다.

[i[`sendall()` 함수]<]
그 일을 하는 함수를 만드는 한 가지 방법은 이렇습니다.

```{.c .numberLines}
#include <sys/types.h>
#include <sys/socket.h>

int sendall(int s, char *buf, int *len)
{
    int total = 0;        // 몇 바이트를 보냈는가
    int bytesleft = *len; // 보내야 하는 데이터는 얼마나 남아 있는가
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // 실제로 보낸 바이트 수를 기록해 반환합니다

    return n==-1?-1:0; // 실패 시에는 -1을, 성공 시에는 0을 반환합니다
}
```

이 예제에서 `s`는 여러분이 데이터를 보내고 싶은 소켓이고 `buf`는 데이터를 담은 버퍼입니다.
`len`은 버퍼에 담긴 바이트의 개수를 담은 `int`에 대한 포인터입니다.

함수는 오류가 발생하면 `-1`을 반환합니다(`errno`는 `send()`에 대한 호출로 인해 여전히
설정되어 있습니다). 또한 실제로 전송된 바이트의 개수가 `len`을 통해 반환됩니다. 이 값은 오류가
발생하지 않는 이상 여러분이 전송하라고 요청한 바이트의 수와 같습니다. `sendall()`은 데이터를
전송하기 위해서 최선을 다하지만 오류가 발생하면 바로 여러분에게 알려줄 것입니다.

완성도를 위해 이 함수에 대한 호출 예제를 하나 더 보겠습니다.

```{.c .numberLines}
char buf[10] = "Beej!";
int len;

len = strlen(buf);
if (sendall(s, buf, &len) == -1) {
    perror("sendall");
    printf("오류가 발생해서 %d바이트만 보냈습니다!\n", len);
}
```

[i[`sendall()` 함수]>]

수신자 측에 패킷의 일부가 도착하면 무슨 일이 벌어질까요? 만약 패킷이 가변 길이일 경우 수신자는
어떻게 하나의 패킷이 끝나고 다른 하나가 시작되는 것을 알까요? 그렇습니다. 현실 세계의 시나리오는
정말 골치 아픕니다. 여러분은 아마도 [i[Data encapsulation]]*캡슐화*를 해야 할 것입니다.
(시작부에 있던 [데이터 캡슐화 절](#lowlevel)을 기억하시나요?) 자세한 내용을 알고 싶다면 계속
읽어보세요.

## 직렬화---데이터를 포장하는 방법 {#serialization}

[i[Serialization]<]

네트워크를 통해 문자열 데이터를 보내는 것은 꽤 쉽다는 것을 이제 알 것입니다. 하지만 만약
`int`나 `float` 같은 "이진" 데이터를 전송하려고 하면 어떻게 해야 할까요? 몇 가지 방법이
있습니다.

1. `sprintf()`같은 함수를 써서 수를 텍스트로 변환하고 텍스트를 전송합니다. 수신자는
   `strtol()`같은 함수를 써서 텍스트를 다시 숫자로 변환합니다.

2. `send()`에 데이터를 가리키는 포인터를 전달해서 원시 데이터를 그대로 전송합니다.

3. 데이터를 호환성 있는 바이너리 형태로 인코드합니다. 수신자는 디코드합니다.

오늘 밤 한정 미리보기!

[_커튼이 올라간다_]

Beej가 말합니다: "저는 위의 세 번째 방법을 좋아합니다."

[_끝_]

(이 절을 본격적으로 시작하기에 앞서, 이 일을 하기 위한 라이브러리들이 이미 있다는 말을 미리
해 두겠습니다. 이식성 있고 오류가 없는 여러분만의 라이브러리를 만드는 작업은 꽤 어려운 일이 될
것입니다. 그러므로 직접 구현하기로 결정하기 전에 충분히 찾아보고 조사하는
것이 좋습니다. 저는 그런 것이 어떻게 동작하는지 궁금할 독자들을 위해서 관련된
내용을 여기에 담을 뿐입니다.)

사실 위에 언급한 모든 방법에 각각의 장점과 단점이 있습니다. 그러나 위에 말한 대로, 저는 일반적으로
세 번째 방법을 선호합니다. 그러나 먼저 다른 두 가지 방법의 장단점에 대해서 조금 더 알아봅시다.

수를 보내기 전에 텍스트로 인코딩하는 첫 번째 방법은 랜선을 타고 오는 정보를 출력하고 읽어 보기가
쉽다는 장점이 있습니다. [i[IRC]] [fl[Internet
Relay Chat (IRC)|https://en.wikipedia.org/wiki/Internet_Relay_Chat]]
처럼 인간이 읽을 수 있는 프로토콜은 때때로 전송 대역폭에 민감하지 않은 상황에서 사용하기에 아주
훌륭합니다. 그러나 변환이 느리다는 단점이 있고, 결과는 거의 언제나 원본 수보다 더 많은
공간을 차지한다는 단점이 있습니다.

두 번째 방법: 원시 데이터를 전송하기. 이것은 꽤 간단(하고 위험)합니다.
보낼 데이터에 대한 포인터를 얻은 후 그것으로 `send()`를 호출합니다.

```{.c}
double d = 3490.15926535;

send(s, &d, sizeof d, 0);  /* 위험--이식성 없음! */
```

수신자는 이것을 아래와 같이 받습니다.

```{.c}
double d;

recv(s, &d, sizeof d, 0);  /* 위험--이식성 없음! */
```

빠르고, 간단합니다. 문제 될 것이 없어 보이지요? 사실, 모든 아키텍처들이 `double`
(이나 다른 예로는 `int`)을 동일한 비트 표현이나 심지어 동일한 바이트 순서로 표시하는
것은 아니라는 문제가 있습니다! 위의 코드는 절대로 이식성이 없습니다. (잠깐---이식성이 필요 없는
상황도 있지 않을까요? 그 경우에 이 방식은 좋고 빠른 방법이 됩니다.)

정수 자료형을 포장할 때 [i[`htons()` function]] `htons()` 계열 함수가 수를 [i[Byte ordering]]
네트워크 바이트 순서로 변환해서 이식성을 지키는 데 어떻게 도움을 주는지, 그리고 그것이 왜 필요한지
이미 살펴보았습니다. 불행하게도 `float` 자료형에 대해서는 유사한 함수가 없습니다. 희망이 없는 것일까요?

두려워하지 마세요!(잠시 무섭지 않았나요? 정말로 조금도 무섭지 않았다고요?) 우리에게 방법이 있습니다.
우리는 데이터를 원격지에서 풀어낼 수 있는 알려진 방식으로 포장(pack)(또는 "marshal",
"직렬화", 그것도 아니면 그런 일에 대한 10억 개의 다른 이름)할 수 있습니다.

"알려진 이진 형식"은 무엇일까요? 우리는 이미 `htons()`의 예제를 보았습니다. 그것은 수를 호스트의
형식이 무엇이든 간에 네트워크 바이트 순서로 변환(또는 "인코드", 이것이 더 이해하기 쉽다면)합니다.
수를 원래대로 돌려놓기(디코드) 위해서 수신자는 `ntohs()`를 호출해야 합니다.

하지만 제가 조금 전에 비-정수 타입에 대해서는 그런 함수가 없다고 했지요? 그렇습니다.
그리고 C 언어에서는 이것을 처리하는 표준 방법이 없기 때문에 조금 까다로운 일입니다. 하지만
파이썬에서는 누워서 피클 먹기입니다. (역자 주: 파이썬에는 직렬화와 역직렬화를 처리하는
`pickle` 모듈이 있습니다.)

필요한 작업은 데이터를 알려진 형식으로 포장하고 네트워크에 실어 보내는 것입니다. 예를 들어서
`float`를 포장하는 작업을 위한 [flx[간단하고 지저분하고 개선할 점이 많은 예제 코드|pack.c]]
가 있습니다.

```{.c .numberLines}
#include <stdint.h>

uint32_t htonf(float f)
{
    uint32_t p;
    uint32_t sign;

    if (f < 0) { sign = 1; f = -f; }
    else { sign = 0; }

    p = ((((uint32_t)f)&0x7fff)<<16) | (sign<<31); // 정수 부분과 부호
    p |= (uint32_t)(((f - (int)f) * 65536.0f))&0xffff; // 소수 부분

    return p;
}

float ntohf(uint32_t p)
{
    float f = ((p>>16)&0x7fff); // 정수 부분
    f += (p&0xffff) / 65536.0f; // 소수 부분

    if (((p>>31)&0x1) == 0x1) { f = -f; } // 부호 비트 설정

    return f;
}
```

위의 코드는 `float`를 32비트 수에 저장하기 위한 단순한 구현입니다. 최상위 비트(31)가 수의 부호를
저장하기 위해 쓰입니다. ("1"이 음수를 의미합니다.) 다음 15비트(30-16)(역자 주: 원문에서는 7비트라고
적혀 있으나 코드의 내용상 오타로 보임)가 `float`의 전체 수 부분을 저장하기 위해서 쓰입니다. 마지막으로
남은 비트들(15-0)이 수의 소수 부분을 기록하기 위해서 쓰입니다.

사용법은 꽤 직관적입니다.

```{.c .numberLines}
#include <stdio.h>

int main(void)
{
    float f = 3.1415926, f2;
    uint32_t netf;

    netf = htonf(f);  // "네트워크" 형식으로 변환
    f2 = ntohf(netf); // 시험을 위해 원래대로 변환

    printf("Original: %f\n", f);        // 3.141593
    printf(" Network: 0x%08X\n", netf); // 0x0003243F
    printf("Unpacked: %f\n", f2);       // 3.141586

    return 0;
}
```

장점을 보자면, 이 코드는 작고 간단하며 빠릅니다. 단점을 보자면 이 방식은 공간을 효율적으로
쓰지 않으며 표현 범위가 상당히 제한되어 있습니다. 32767보다 큰 수를 저장하려고 하면 이
방법은 제대로 동작하지 않을 것입니다! 또한 여러분은 위의 예제에서 소수점의 마지막 2자리가
제대로 보존되지 않은 것을 볼 수 있습니다.

이것을 해결하려면 어떻게 해야 할까요? 사실 부동 소수점 수를 저장하기 위한 *표준*은 [i[IEEE-754]]
[fl[IEEE-754|https://en.wikipedia.org/wiki/IEEE_754]]로 알려져 있습니다.
대부분의 컴퓨터는 부동 소수점 계산을 위해서 내부적으로 이 형식을 사용합니다.
그러므로 그런 경우라면 엄밀히 말하자면 변환을 수행할 필요는 없습니다. 그러나
여러분의 소스 코드가 이식성이 있기를 바란다면 반드시 그런 가정을 할 수는 없습니다.

정말 그럴까요? 여러분의 시스템은 정수에 대해 아마 2의 보수 표현을 쓰는 것처럼,
부동 소수점에 대해서도 매우 높은 확률로 IEEE-754를 쓸 것입니다. 그래서 그 사실을
알고 있다면 데이터를 그대로 네트워크로 넘길 수 있습니다. 다만 `htonl()`이나 알맞은
함수로 엔디언은 맞춰야 합니다. `float`에도 엔디언이 있습니다. 변환이 필요 없는
빅 엔디언 시스템에서 `htons()`와 그 계열 함수들이 하는 일이 바로 이것입니다.

[flx[하지만 혹시 IEEE-754가 아닌 시스템을 쓰고 있을 경우를 대비해서, 여기 `float`와
`double`을 IEEE-754 형식으로 인코드하는 코드가 있습니다|ieee754.c]]. (엄밀히는 거의 대부분을 인코드합니다. 이 코드는 NaN이나
Infinity를 처리하지 않습니다. 그러나 그런 처리가 가능하게 수정할 수도 있습니다.)

```{.c .numberLines}
#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

uint64_t pack754(long double f, unsigned bits, unsigned expbits)
{
    long double fnorm;
    int shift;
    long long sign, exp, significand;
    unsigned significandbits = bits - expbits - 1; // 부호 비트를 위해 1을 뺍니다

    if (f == 0.0) return 0; // 특별한 경우의 처리

    // 부호를 확인하고 정규화를 시작합니다
    if (f < 0) { sign = 1; fnorm = -f; }
    else { sign = 0; fnorm = f; }

    // 정규화된 형태의 f를 얻어내고 지수를 추적합니다
    shift = 0;
    while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
    while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
    fnorm = fnorm - 1.0;

    // 실수부의 부동 소수점이 아닌 이진 표현을 구합니다
    significand = fnorm * ((1LL<<significandbits) + 0.5f);

    // 바이어스를 더한 지수부를 구합니다
    // (역자 주: IEEE754는 실제 지수에 일정한 바이어스를 더한 값을 부호 없는 정수처럼
    // 저장합니다. 이렇게 하면 음수 지수를 따로 부호화하지 않아도 되고, 지수 값의 크기
    // 비교도 단순해집니다.)
    exp = shift + ((1<<(expbits-1)) - 1); // shift + bias

    // 최종 값을 반환합니다
    return (sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand;
}

long double unpack754(uint64_t i, unsigned bits, unsigned expbits)
{
    long double result;
    long long shift;
    unsigned bias;
    unsigned significandbits = bits - expbits - 1; // 부호 비트를 위해서 -1

    if (i == 0) return 0.0;

    // 유효숫자부를 뽑아냅니다
    result = (i&((1LL<<significandbits)-1)); // 마스크 처리
    result /= (1LL<<significandbits); // 부동 소수점으로 변환
    result += 1.0f; // 1을 다시 더합니다

    // 지수부를 처리합니다
    bias = (1<<(expbits-1)) - 1;
    shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
    while(shift > 0) { result *= 2.0; shift--; }
    while(shift < 0) { result /= 2.0; shift++; }

    // 부호 처리
    result *= (i>>(bits-1))&1? -1.0: 1.0;

    return result;
}
```

32비트(아마도 `float`)와 64비트(아마도 `double`) 수를 위한 패킹과
언패킹 매크로를 위에 넣어두었습니다. 그러나 `bits`크기의 데이터를 인코드
하기 위해서 `pack754()`함수를 직접 호출할 수도 있을 것입니다.(`expbits`
만큼의 지수부가 정규화된 수의 지수로 보존될 것입니다.)

여기 사용 예시가 있습니다.

```{.c .numberLines}

#include <stdio.h>
#include <stdint.h> // uintN_t 형들을 정의합니다
#include <inttypes.h> // PRIx 매크로들을 정의합니다

int main(void)
{
    float f = 3.1415926, f2;
    double d = 3.14159265358979323, d2;
    uint32_t fi;
    uint64_t di;

    fi = pack754_32(f);
    f2 = unpack754_32(fi);

    di = pack754_64(d);
    d2 = unpack754_64(di);

    printf("float before : %.7f\n", f);
    printf("float encoded: 0x%08" PRIx32 "\n", fi);
    printf("float after  : %.7f\n\n", f2);

    printf("double before : %.20lf\n", d);
    printf("double encoded: 0x%016" PRIx64 "\n", di);
    printf("double after  : %.20lf\n", d2);

}
```

위의 코드는 아래의 출력을 생성합니다.

```
float before : 3.1415925
float encoded: 0x40490FDA
float after  : 3.1415925

double before : 3.14159265358979311600
double encoded: 0x400921FB54442D18
double after  : 3.14159265358979311600
```

여러분이 하실 만한 또 다른 질문은 `struct`를 어떻게 포장하느냐입니다.
불행히도 컴파일러는 `struct`의 모든 곳에 자유롭게 패딩을 넣을 수 있습니다.
그리고 그것은 구조체 전체를 한 번에 네트워크에 이식성 있게 전송할 수는 없다는 것을
의미합니다. ("이건 되고", "이건 안되고"를 듣는 것이 지겨운가요? 죄송합니다.
제 친구의 말을 빌리자면 "뭔가 잘못되면 그건 전부 마이크로소프트 때문입니다."
이 경우에는 아마도 마이크로소프트의 잘못만은 아닐 것입니다. 그러나 제 친구의
말은 완전히 옳습니다.)

다시 주제로 돌아가서, `struct`를 전송하는 가장 좋은 방법은 각각의 필드를
독립적으로 포장한 다음 반대편에 도착하면 다시 `struct` 안에 풀어 넣는 것입니다.

여러분은 이것이 굉장히 큰 작업일 것이라 예상할 것입니다. 맞습니다. 여러분이 할 일은
데이터를 포장하는 일을 도와줄 도우미 함수를 작성하는 것입니다. 재미있을 것입니다!
정말로!!

Kernighan과 Pike가 지은 [flr[_The Practice of Programming_|tpop]]
에서 그들은 바로 그 일을 하도록 `printf()`와 유사한 `pack()`과 `unpack()` 함수를 작성했습니다.
그것에 대한 링크를 제공하고 싶지만 그 함수들과 책의 다른 소스 코드는 온라인으로
제공되지 않고 있습니다.

(_The Practice of Programming_은 아주 좋은 책입니다. 제가 그 책을 추천할
때마다 제우스가 고양이를 한 마리씩 구합니다.)
(역자 주: 신이 보답을 할 만큼 아주 좋은 선행이라는 뜻입니다.)

이 시점에서 저는 [fl[프로토콜 버퍼의 C 구현체|https://github.com/protobuf-c/protobuf-c]]
에 대한 링크를 제공하려 합니다. 저는 이것을 써 본 적이 없으나 훌륭한 코드로
보입니다. 파이썬과 펄 프로그래머들은 같은 일을 하기 위해서 그들의 언어가
가진 `pack()`과 `unpack()` 함수를 확인해 보길 바랍니다. 자바는 유사한 방식으로
사용할 수 있는 Serializable 인터페이스를 가지고 있습니다.

그러나 만약 여러분이 자신만의 패킹 유틸리티를 C 언어로 작성하고 싶다면, K&P의
해결책은 패킷을 만들기 위해서 가변 길이 매개변수 목록을 활용하는 `printf()`와
유사한 함수를 만드는 것입니다. [flx[여기 제가 직접 만든 버전이 있으며|pack2.c]]
여러분이 그런 것이 어떻게 동작하는지 알기에 충분할 것입니다.

(이 코드는 위의 `pack754()` 함수를 참조합니다. `packi*()` 함수는 또 다른 정수가
아닌 `char` 배열에 수를 담는다는 점을 제외하면 `htons()` 계열 함수와 유사하게
동작합니다.)

```{.c .numberLines}
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

/*
** packi16() -- 16비트 정수를 char 버퍼에 저장합니다. (htons()처럼)
*/
void packi16(unsigned char *buf, unsigned int i)
{
    *buf++ = i>>8; *buf++ = i;
}

/*
** packi32() -- 32비트 정수를 char 버퍼에 저장합니다. (htonl()처럼)
*/
void packi32(unsigned char *buf, unsigned long int i)
{
    *buf++ = i>>24; *buf++ = i>>16;
    *buf++ = i>>8;  *buf++ = i;
}

/*
** packi64() -- 64비트 정수를 char 버퍼에 저장합니다. (htonl()처럼)
*/
void packi64(unsigned char *buf, unsigned long long int i)
{
    *buf++ = i>>56; *buf++ = i>>48;
    *buf++ = i>>40; *buf++ = i>>32;
    *buf++ = i>>24; *buf++ = i>>16;
    *buf++ = i>>8;  *buf++ = i;
}

/*
** unpacki16() -- 16비트 정수를 char 버퍼에서 풀어냅니다. (ntohs()처럼)
*/
int unpacki16(unsigned char *buf)
{
    unsigned int i2 = ((unsigned int)buf[0]<<8) | buf[1];
    int i;

    // 부호 없는 수를 부호 있는 수로 바꿉니다
    if (i2 <= 0x7fffu) { i = i2; }
    else { i = -1 - (unsigned int)(0xffffu - i2); }

    return i;
}

/*
** unpacku16() -- 16비트 부호 없는 정수를 char 버퍼에서 풀어냅니다. (ntohs()처럼)
*/
unsigned int unpacku16(unsigned char *buf)
{
    return ((unsigned int)buf[0]<<8) | buf[1];
}

/*
** unpacki32() -- 32비트 정수를 char 버퍼에서 풀어냅니다. (ntohl()처럼)
*/
long int unpacki32(unsigned char *buf)
{
    unsigned long int i2 = ((unsigned long int)buf[0]<<24) |
                           ((unsigned long int)buf[1]<<16) |
                           ((unsigned long int)buf[2]<<8)  |
                           buf[3];
    long int i;

    // 부호 없는 수를 부호 있는 수로 바꿉니다
    if (i2 <= 0x7fffffffu) { i = i2; }
    else { i = -1 - (long int)(0xffffffffu - i2); }

    return i;
}

/*
** unpacku32() -- 32비트 부호 없는 정수를 char 버퍼에서 풀어냅니다. (ntohl()처럼)
*/
unsigned long int unpacku32(unsigned char *buf)
{
    return ((unsigned long int)buf[0]<<24) |
           ((unsigned long int)buf[1]<<16) |
           ((unsigned long int)buf[2]<<8)  |
           buf[3];
}

/*
** unpacki64() -- 64비트 정수를 char 버퍼에서 풀어냅니다. (ntohl()처럼)
*/
long long int unpacki64(unsigned char *buf)
{
    unsigned long long int i2 = ((unsigned long long int)buf[0]<<56) |
                                ((unsigned long long int)buf[1]<<48) |
                                ((unsigned long long int)buf[2]<<40) |
                                ((unsigned long long int)buf[3]<<32) |
                                ((unsigned long long int)buf[4]<<24) |
                                ((unsigned long long int)buf[5]<<16) |
                                ((unsigned long long int)buf[6]<<8)  |
                                buf[7];
    long long int i;

    // 부호 없는 수를 부호 있는 수로 바꿉니다
    if (i2 <= 0x7fffffffffffffffu) { i = i2; }
    else { i = -1 -(long long int)(0xffffffffffffffffu - i2); }

    return i;
}

/*
** unpacku64() -- 64비트 부호 없는 정수를 char 버퍼에서 풀어냅니다. (ntohl()처럼)
*/
unsigned long long int unpacku64(unsigned char *buf)
{
    return ((unsigned long long int)buf[0]<<56) |
           ((unsigned long long int)buf[1]<<48) |
           ((unsigned long long int)buf[2]<<40) |
           ((unsigned long long int)buf[3]<<32) |
           ((unsigned long long int)buf[4]<<24) |
           ((unsigned long long int)buf[5]<<16) |
           ((unsigned long long int)buf[6]<<8)  |
           buf[7];
}

/*
** pack() -- 형식 문자열이 지시한 방식으로 버퍼에 데이터를 저장합니다
**
**   bits |signed   unsigned   float   string
**   -----+----------------------------------
**      8 |   c        C
**     16 |   h        H         f
**     32 |   l        L         d
**     64 |   q        Q         g
**      - |                               s
**
**  (16비트 부호 없는 길이가 자동으로 문자열의 앞에 붙습니다.)
*/

unsigned int pack(unsigned char *buf, char *format, ...)
{
    va_list ap;

    signed char c;              // 8비트
    unsigned char C;

    int h;                      // 16비트
    unsigned int H;

    long int l;                 // 32비트
    unsigned long int L;

    long long int q;            // 64비트
    unsigned long long int Q;

    float f;                    // 부동 소수점
    double d;
    long double g;
    unsigned long long int fhold;

    char *s;                    // 문자열
    unsigned int len;

    unsigned int size = 0;

    va_start(ap, format);

    for(; *format != '\0'; format++) {
        switch(*format) {
        case 'c': // 8비트
            size += 1;
            c = (signed char)va_arg(ap, int); // 자료형 승급
            *buf++ = c;
            break;

        case 'C': // 부호 없는 8비트
            size += 1;
            C = (unsigned char)va_arg(ap, unsigned int); // 자료형 승급
            *buf++ = C;
            break;

        case 'h': // 16비트
            size += 2;
            h = va_arg(ap, int);
            packi16(buf, h);
            buf += 2;
            break;

        case 'H': // 부호 없는 16비트
            size += 2;
            H = va_arg(ap, unsigned int);
            packi16(buf, H);
            buf += 2;
            break;

        case 'l': // 32비트
            size += 4;
            l = va_arg(ap, long int);
            packi32(buf, l);
            buf += 4;
            break;

        case 'L': // 부호 없는 32비트
            size += 4;
            L = va_arg(ap, unsigned long int);
            packi32(buf, L);
            buf += 4;
            break;

        case 'q': // 64비트
            size += 8;
            q = va_arg(ap, long long int);
            packi64(buf, q);
            buf += 8;
            break;

        case 'Q': // 부호 없는 64비트
            size += 8;
            Q = va_arg(ap, unsigned long long int);
            packi64(buf, Q);
            buf += 8;
            break;

        case 'f': // 부동 소수점 16비트
            size += 2;
            f = (float)va_arg(ap, double); // 자료형 승급
            fhold = pack754_16(f); // IEEE 754로 변환
            packi16(buf, fhold);
            buf += 2;
            break;

        case 'd': // 부동 소수점 32비트
            size += 4;
            d = va_arg(ap, double);
            fhold = pack754_32(d); // IEEE 754로 변환
            packi32(buf, fhold);
            buf += 4;
            break;

        case 'g': // 부동 소수점 64비트
            size += 8;
            g = va_arg(ap, long double);
            fhold = pack754_64(g); // IEEE 754로 변환
            packi64(buf, fhold);
            buf += 8;
            break;

        case 's': // 문자열
            s = va_arg(ap, char*);
            len = strlen(s);
            size += len + 2;
            packi16(buf, len);
            buf += 2;
            memcpy(buf, s, len);
            buf += len;
            break;
        }
    }

    va_end(ap);

    return size;
}

/*
** unpack() -- 형식 문자열이 지정하는 대로 버퍼에 데이터를 풀어놓습니다
**
**   bits |signed   unsigned   float   string
**   -----+----------------------------------
**      8 |   c        C
**     16 |   h        H         f
**     32 |   l        L         d
**     64 |   q        Q         g
**      - |                               s
**
**  (문자열은 저장된 길이에 근거해서 추출됩니다. 단, `s` 앞에 최대 길이를 지정할 수 있습니다.)
*/
void unpack(unsigned char *buf, char *format, ...)
{
    va_list ap;

    signed char *c;              // 8비트
    unsigned char *C;

    int *h;                      // 16비트
    unsigned int *H;

    long int *l;                 // 32비트
    unsigned long int *L;

    long long int *q;            // 64비트
    unsigned long long int *Q;

    float *f;                    // 부동 소수점
    double *d;
    long double *g;
    unsigned long long int fhold;

    char *s;
    unsigned int len, maxstrlen=0, count;

    va_start(ap, format);

    for(; *format != '\0'; format++) {
        switch(*format) {
        case 'c': // 8비트
            c = va_arg(ap, signed char*);
            if (*buf <= 0x7f) { *c = *buf;} // 부호를 다시 붙입니다
            else { *c = -1 - (unsigned char)(0xffu - *buf); }
            buf++;
            break;

        case 'C': // 부호 없는 8비트
            C = va_arg(ap, unsigned char*);
            *C = *buf++;
            break;

        case 'h': // 16비트
            h = va_arg(ap, int*);
            *h = unpacki16(buf);
            buf += 2;
            break;

        case 'H': // 부호 없는 16비트
            H = va_arg(ap, unsigned int*);
            *H = unpacku16(buf);
            buf += 2;
            break;

        case 'l': // 32비트
            l = va_arg(ap, long int*);
            *l = unpacki32(buf);
            buf += 4;
            break;

        case 'L': // 부호 없는 32비트
            L = va_arg(ap, unsigned long int*);
            *L = unpacku32(buf);
            buf += 4;
            break;

        case 'q': // 64비트
            q = va_arg(ap, long long int*);
            *q = unpacki64(buf);
            buf += 8;
            break;

        case 'Q': // 부호 없는 64비트
            Q = va_arg(ap, unsigned long long int*);
            *Q = unpacku64(buf);
            buf += 8;
            break;

        case 'f': // 부동 소수점
            f = va_arg(ap, float*);
            fhold = unpacku16(buf);
            *f = unpack754_16(fhold);
            buf += 2;
            break;

        case 'd': // 32비트 부동 소수점
            d = va_arg(ap, double*);
            fhold = unpacku32(buf);
            *d = unpack754_32(fhold);
            buf += 4;
            break;

        case 'g': // 64비트 부동 소수점
            g = va_arg(ap, long double*);
            fhold = unpacku64(buf);
            *g = unpack754_64(fhold);
            buf += 8;
            break;

        case 's': // 문자열
            s = va_arg(ap, char*);
            len = unpacku16(buf);
            buf += 2;
            if (maxstrlen > 0 && len > maxstrlen) count = maxstrlen - 1;
            else count = len;
            memcpy(s, buf, count);
            s[count] = '\0';
            buf += len;
            break;

        default:
            if (isdigit(*format)) { // 최대 문자열 길이를 기록합니다
                maxstrlen = maxstrlen * 10 + (*format-'0');
            }
        }

        if (!isdigit(*format)) maxstrlen = 0;
    }

    va_end(ap);
}
```

위 코드의 [flx[시연 프로그램|pack2.c]]입니다. 이 프로그램은 `buf`에 데이터를
조금 포장한 후 다시 변수에 풀어놓습니다. `unpack()`을 문자열 매개변수로 호출하는
경우(형식 지정자 "`s`")에는 버퍼 오버런을 막기 위해 "`96s`"처럼 최대 길이를 앞에
붙이는 것이 좋습니다. 네트워크를 통해 받은 데이터를 풀어놓을 때에는 주의해야 합니다.
악의적인 사용자가 여러분의 시스템을 공격하려고 잘못 구성된 패킷을 보낼 수 있습니다!

```{.c .numberLines}
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

// C23 컴파일러가 있다면
#if __STDC_VERSION__ >= 202311L
#include <stdfloat.h>
#else
// 아니면 직접 정의합니다.
// 아키텍처에 따라 다릅니다! 하지만 아마 다음과 같을 것입니다.
typedef float float32_t;
typedef double float64_t;
#endif

int main(void)
{
    uint8_t buf[1024];
    int8_t magic;
    int16_t monkeycount;
    int32_t altitude;
    float32_t absurdityfactor;
    char *s = "Great unmitigated Zot!  You've found the Runestaff!";
    char s2[96];
    int16_t packetsize, ps2;

    packetsize = pack(buf, "chhlsf", (int8_t)'B', (int16_t)0,
            (int16_t)37, (int32_t)-5, s, (float32_t)-3490.6677);
    packi16(buf+1, packetsize); // 재미삼아 패킷 크기를 넣어 둡니다

    printf("packet is %" PRId32 " bytes\n", packetsize);

    unpack(buf, "chhl96sf", &magic, &ps2, &monkeycount, &altitude,
            s2, &absurdityfactor);

    printf("'%c' %" PRId32" %" PRId16 " %" PRId32
            " \"%s\" %f\n", magic, ps2, monkeycount,
            altitude, s2, absurdityfactor);

    return 0;
}
```

여러분이 직접 만든 코드를 쓰건 다른 사람이 작성한 것을 쓰건, 매번 각 비트를
수동으로 포장하기보다는 버그를 통제하기 위해 일반적인 데이터 패킹 루틴을
사용하는 것이 좋은 습관입니다.

데이터를 포장할 때에 쓰기 좋은 형식은 무엇일까요? 아주 좋은 질문입니다. 다행히도
[i[XDR]] [flrfc[RFC 4506|4506]], 외부 데이터 표현 표준이 부동 소수점과 정수,
배열 등 다양한 자료형에 대해서 이진 형식을 정의합니다. 만약 데이터를 직접 처리할
생각이라면 이것을 준수하는 것을 권장합니다. 그러나 반드시 그래야 하는 것은 아닙니다.
패킷 경찰들이 문 앞까지 찾아오는 것은 아닙니다. 최소한 저는 그렇지 않을 것이라고
_생각합니다._

어떤 경우에도, 데이터를 보내기 전에 어떤 식으로든 인코딩하는 것이 올바른 방식입니다.

[i[Serialization]>]

## 망할 데이터 캡슐화 {#sonofdataencap}

아무튼 데이터 캡슐화가 정말로 의미하는 것은 무엇일까요? 가장 단순한 경우 그것은
여러분이 데이터에 약간의 식별 정보나 패킷 길이 혹은 둘 모두를 담은 헤더를
붙여둔다는 뜻이 됩니다.

헤더가 어떤 모양을 하고 있어야 할까요? 사실 여러분의 프로젝트를 끝내기 위해서
필요하다고 느끼는 어떤 이진 데이터면 됩니다.

와. 정말 막연한 이야기입니다.

좋습니다. 예를 들자면 `SOCK_STREAM`을 사용하는 다중 사용자 대화 프로그램이 있다고 합시다.
한 사용자가 뭔가 입력한다면, 두 조각의 정보가 서버에 전달되어야 합니다. 무엇을 말했는지,
그리고 누가 말했는지가 그것입니다.

여기까지는 좋아 보입니다. "그럼 무엇이 문제인가요?"라고 여러분은 질문할 것입니다.

문제는 메시지가 가변 길이일 수 있다는 점입니다. "Tom"이라는 사용자가 "Hi"라고
말할 수 있고 "Benjamin"이라는 또 다른 사용자가 "Hey guys what is up?"이라고
말할 수도 있습니다.

그것이 들어오는 대로 클라이언트에게 `send()`한다고 합시다. 여러분의 송신 데이터 스트림은
아래와 같을 것입니다.

```
t o m H i B e n j a m i n H e y g u y s w h a t i s u p ?
```

이런 식일 것입니다. 클라이언트가 어떻게 하면 메시지의 시작과 끝을 알 수 있을까요?
원한다면 모든 메시지가 같은 길이를 갖도록 하고 우리가 구현한 [i[`sendall()` function]] `sendall()`
함수를 그냥 호출할 수 있을 것입니다. 그러나 그렇게 하면 대역폭을 낭비하게 됩니다!
"tom"이 "Hi"라고 말하는 일을 위해서 1024바이트를 `send()`하고 싶지는 않을
것입니다.

그래서 우리는 데이터를 작은 헤더와 패킷 구조에 *캡슐화*합니다. 클라이언트와
서버 모두 이 데이터를 어떻게 포장하고 풀어내는지(때때로 "marshal"과 "unmarshal"
이라고 부릅니다) 알고 있습니다. 어느새 클라이언트와 서버가 어떻게 통신하는지를
설명하는 *프로토콜*을 정의하기 시작한 셈입니다!

지금은 사용자의 이름이 `'\0'`으로 패드된 고정된 8개의 문자라고 가정합시다.
데이터는 최대 128개 문자로 구성되는 가변 길이 형태라고 합시다. 이 상황에서
쓸 수 있는 예제 패킷 구조를 살펴봅시다.

1. `len` (1바이트, 부호 없음)---패킷의 전체 길이, 8바이트의 사용자 이름과
   대화 데이터의 길이를 센다.

2. `name` (8바이트)---사용자의 이름, 필요한 경우 NUL로 덧댑니다.

3. `chatdata` (*n*바이트)---데이터 자체, 최대 128바이트. 패킷의 길이는 이
   데이터의 길이에 8을 더한 값으로 계산되어야 합니다.(위에서 언급한 이름 필드의 길이)

필자가 8바이트와 128바이트를 필드의 길이 제한으로 선택한 이유가 궁금한가요?
특별한 이유는 없고, 충분히 길 것이라고 생각했습니다. 그러나 아마도 8바이트는 여러분의
필요에는 조금 못 미칠 수도 있습니다. 그런 경우에는 이름 필드의 길이를 30바이트나
다른 값으로 설정할 수 있습니다. 선택은 여러분의 몫입니다.

위의 패킷 정의를 사용하는 첫 번째 패킷은 아래와 같은 정보로 구성될 수 있습니다.
(16진수와 아스키 코드로 표시되었습니다.)

```
   0A     74 6F 6D 00 00 00 00 00      48 69
(길이)  T  o  m    (패딩)         H  i
```

두 번째 패킷도 비슷합니다.

```
   18     42 65 6E 6A 61 6D 69 6E      48 65 79 20 67 75 79 73 20 77 ...
(길이)  B  e  n  j  a  m  i  n       H  e  y     g  u  y  s     w  ...
```

(길이는 물론 네트워크 바이트 순서로 기록되어 있습니다. 이 경우 길이가 단일 바이트이므로
그것이 중요하지는 않지만, 일반적으로는 여러분의 패킷이 가지는 모든
이진 정수가 네트워크 바이트 순서로 기록되기를 원할 것입니다.)

이 데이터를 보낼 때 여러분은 위에서 제시된 [`sendall()`](#sendall)과 비슷한
함수를 쓸 수 있습니다. 그렇게 하면 데이터를 모두 전송하기 위해서 `send()`를 여러
번 호출하는 한이 있어도 모든 데이터가 전송되는 것을 확신할 수 있습니다.

마찬가지로 이 데이터를 받을 때에도 약간의 추가적인 작업이 필요합니다. 이 데이터를
받을 때에도 부분적인 패킷(예를 들어 위의 벤자민으로부터 `recv()`로 받은 것이
"`18 42 65 6E 6A`"뿐일 수도 있습니다.)을 받을 가능성을 염두에 둬야 합니다. 전체 패킷을
받을 때까지 `recv()`를 반복적으로 호출해야 합니다.

그러나 어떻게 해야 할까요? 우리는 패킷이 완성되기 위해서 받아야 하는 바이트의
총 개수를 알고 있습니다. 개수가 패킷의 앞쪽에 붙어 있기 때문입니다. 우리는 또한 패킷의
최대 크기가 1 + 8 + 128, 즉 137바이트라는 것을 알고 있습니다. (우리가 그렇게 정의했기
때문입니다.)

여기에서는 몇 가지 방식으로 일을 할 수 있습니다. 모든 패킷이 길이 정보로 시작한다는 것을
알고 있으므로 패킷의 길이를 얻기 위해서 `recv()`를 호출할 수 있습니다. 그리고
길이를 가지고 있으면 전체 패킷을 받을 때까지 남은 길이를 명시하면서 `recv()`
를(아마도 반복적으로) 호출하는 것입니다. 이 방식의 장점은 하나의 패킷을 담기에 충분한
크기의 버퍼만 있으면 된다는 것이고, 단점은 모든 데이터를 받기 위해서 `recv()`를
최소 두 번 호출해야 한다는 것입니다.

다른 옵션은 `recv()`를 호출할 때 한 패킷의 최대 크기만큼을 받겠다고 지정하는 것입니다.
그 후에 받은 데이터를 버퍼의 뒤쪽에 쌓아두고, 패킷이 완성되었는지 확인합니다. 물론
다음 패킷의 일부를 받을 수 있으므로 그것을 위한 여분의 공간이 필요합니다.

이를 위해서 두 개의 패킷을 담기에 충분한 배열을 선언하면 됩니다. 이것은 패킷이
도착하는 대로 재구성하는 일에 사용할 작업 공간입니다.

데이터를 `recv()`로 받을 때마다 그것을 작업 버퍼에 덧붙이고 패킷이 완성되었는지
확인합니다. 버퍼에 담긴 바이트의 개수가 헤더에 명시된 길이보다 많거나 같은지
확인한다는 뜻입니다(사실은 헤더에 헤더 자신의 길이가 포함되지 않으므로 +1을
해야 합니다). 만약 버퍼의 바이트 수가 1보다 적다면 물론 패킷은 완성되지 않은
것입니다. 또한 이 경우 버퍼의 첫 바이트를 읽어들인다고 해도 그것은 쓰레기값이므로
그것을 감안한 처리를 해야 합니다.

패킷이 완성되면 여러분은 그것으로 여러분이 원하는 일을 할 수 있습니다. 패킷을
사용하거나, 그것을 여러분의 작업 버퍼에서 제거할 수 있습니다.

휴! 머릿속으로 아직 잘 따라오고 있나요? 여기 두 번째 문제가 있습니다. 한 번의
`recv()` 호출로 한 패킷을 넘어서는 분량을 읽어들일 수가 있습니다. 즉 작업 버퍼에
하나의 완전한 패킷과 다음 패킷의 불완전한 부분이 있을 수 있다는 것입니다.
제기랄. (그러나 이런 경우를 처리하기 위해서 여러분의 작업 버퍼를 _두_ 개의
패킷을 담기에 충분한 크기로 만들어둔 것입니다.)

첫 패킷의 길이를 헤더를 통해 알고 있고 작업 버퍼에 있는 바이트의 수를 추적하고
있으므로, 뺄셈을 해서 작업 버퍼에 있는 바이트 중 몇 개가 다음(미완성된) 패킷에
속해 있는지 계산할 수 있습니다. 첫 번째 패킷을 처리한 후에는 그것을 작업 버퍼에서
제거하고 부분적인 두 번째 패킷을 버퍼의 앞쪽으로 옮겨서 다음 `recv()`를
처리할 준비를 할 수 있습니다.

(독자 여러분 중 일부는 부분적인 두 번째 패킷을 작업 버퍼의 앞쪽으로 옮기는 것에
시간이 걸리고, 환형 버퍼를 사용하면 그 작업이 필요하지 않다는 것에 주목할 것입니다.
다른 독자들에게는 불행하게도, 환형 버퍼에 대한 논의는 이 글의 범위를 벗어납니다.
흥미가 있다면 자료 구조 책을 집어 들고 거기서부터 시작하면 됩니다.)

쉽다고 한 적은 없습니다. 사실 앞에서 쉽다고 말했습니다. 또 실제로도 그렇습니다.
단지 연습이 필요하고, 머지않아 자연스럽게 익숙해질 것입니다.
[i[Excalibur]] 엑스칼리버에 맹세합니다!

## 브로드캐스트(Broadcast) 패킷 --- Hello, World!

지금까지 이 안내서에서는 데이터를 하나의 호스트에서 다른 하나의 호스트로 보내는
일에 대해서 이야기했습니다. 그러나 적절한 권한이 있다면 _한 번에_ 여러 호스트에게
데이터를 보낼 수도 있습니다!

[i[UDP]] UDP(TCP는 안 됩니다)와 표준 IPv4에서 이것은 [i[Broadcast]]
*브로드캐스팅(Broadcasting)*이라는 메커니즘으로 가능합니다. IPv6에서 브로드캐스팅은
지원되지 않으며, 보통은 더 나은 기술인 *멀티캐스팅(Multicasting)*을 사용해야 합니다.
그러나 이번에는 그것에 대해서 다루지 않을 것입니다. 눈부신 미래에 대해서는
그만 이야기합시다. 우리는 32비트의 현재에 갇혀 있습니다.

잠깐! 그러나 무작정 브로드캐스팅을 시작할 수는 없습니다. 네트워크에 브로드캐스트 패킷을
전송하기 전에 [i[`SO_BROADCAST` macro]] `SO_BROADCAST` 소켓 옵션을
[i[`setsockopt()` function]] 설정해야 합니다. 이것은 미사일 발사 스위치에 달아두는
플라스틱 덮개 같은 것입니다. 그만큼 강력한 도구라는 뜻입니다.

아무튼 진지하게 말하자면 브로드캐스트 패킷을 쓰는 일에는 위험이 따릅니다. 브로드캐스트
패킷을 받는 모든 시스템은 반드시 그 데이터가 어떤 포트를 목적지로 삼는지 알아내기
위해서 패킷의 데이터 캡슐화 계층이라는 양파 껍질을 벗겨내야 한다는 점이 바로 그것입니다.
그렇게 하고 난 후에야 시스템은 데이터를 포트에 건네줄지 아니면 무시할지를 결정합니다.
어떤 경우건 그것은 브로드캐스트 패킷을 받는 각 시스템에 큰 작업이고, 상당히 많은
시스템이 불필요한 작업을 할 수 있습니다. 게임 둠이 처음 세상에 나왔을 때 그것의 네트워크
코드에 대한 불평이 이런 것이었습니다.

자, 고양이 가죽을 벗기는 방법은 하나만 있는 게 아니라는 말도 있죠[^6178]...
잠깐만요. 정말 고양이 가죽을 벗기는 방법이 여러 가지라는 말을 하고 있는 건가요?
대체 그런 표현은 왜 있는 걸까요? 아무튼 마찬가지로, 브로드캐스트 패킷을 보내는
방법도 하나만 있는 것은 아닙니다. 핵심을 말하자면 이것입니다. 어떻게 브로드캐스트 메시지의
목적지 주소를 지정할 수 있을까요? 두 개의 일반적인 방법이 있습니다.

[^6178]: 기록 차원에서 말해 두자면, 저는 고양이를 좋아합니다. 최고죠. 평생 여러
    마리를 사랑하며 함께 살았습니다. 기원을 알 수 없는 이 음울한 비유 표현을 싫어하는
    분들이 있다는 것도 알지만, 이 안내서의 이 부분에는 이 표현이 가장 잘 맞는다고 생각합니다.
    (역자 주: 각주 번호 6178은 숫자를 180도 돌려 읽으면 "glib"처럼 보이는 말장난입니다.
    "glib"은 말만 번지르르하고 깊이가 없다는 뜻입니다.)

1. 데이터를 특정 서브넷의 브로드캐스트 주소로 보냅니다. 이것은 주소의 모든 호스트
   부분 비트가 1로 설정된 서브넷 네트워크 주소입니다. 예를 들어 저의 네트워크는
   집에서 `192.168.1.0`이고 넷마스크는 `255.255.255.0`입니다. 그러므로 주소의
   마지막 바이트가 호스트 번호입니다(넷마스크에 따라 첫 세 바이트가 네트워크
   주소이기 때문입니다). 그러므로 저의 브로드캐스트 주소는 `192.168.1.255`
   입니다. 유닉스에서는 `ifconfig` 명령이 이 모든 정보를 줄 것입니다. (궁금한 분을
   위해 적자면 브로드캐스트 주소를 얻기 위한 비트단위 논리연산은 `네트워크 번호`
   OR (NOT `넷마스크`)입니다.) 여러분은 이 종류의 브로드캐스트 패킷을
   로컬 네트워크뿐 아니라 원격 네트워크에도 보낼 수 있습니다. 그러나 이 경우
   목적지의 라우터가 패킷을 무시할 가능성이 존재합니다. (이런 패킷을 무시하지
   않으면 공격자가 브로드캐스트 통신을 과다하게 발송할 수 있습니다.)

2. 데이터를 "전역" 브로드캐스트 주소로 보냅니다. 이것은 [i[`255.255.255.255`]]
   `255.255.255.255`, 통칭 [i[`INADDR_BROADCAST`
   macro]] `INADDR_BROADCAST`입니다. 많은 시스템은 이것을 자동으로 여러분의 네트워크
   번호와 비트단위 AND연산 해서 네트워크 브로드캐스트 주소로 변환할 것입니다.
   그러나 일부는 그렇게 하지 않을 것입니다. 이는 시스템마다 다릅니다. 역설적이게도
   라우터들은 이 종류의 브로드캐스트 패킷을 로컬 네트워크 너머로 전송하지 않습니다.

`SO_BROADCAST` 소켓 옵션을 지정하지 않고 브로드캐스트 주소에 데이터를 보내려고 하면
어떤 일이 생길까요? 오래됐지만 유용한 [`talker`와 `listener`](#datagram) 를
실행해보고 무슨 일이 생기는지 봅시다.

```
$ talker 192.168.1.2 foo
sent 3 bytes to 192.168.1.2
$ talker 192.168.1.255 foo
sendto: Permission denied
$ talker 255.255.255.255 foo
sendto: Permission denied
```

별로 좋지 않은 상황입니다. `SO_BROADCAST`을 설정하지 않았기 때문입니다. 설정을
한 뒤에는 원하는 곳 어디에든 `sendto()`를 할 수 있습니다.

사실 그것이 브로드캐스트를 할 수 있는 UDP 응용프로그램과 그렇지 않은
응용프로그램의 *유일한 차이*입니다. 그러니 오래된 `talker` 응용프로그램에
`SO_BROADCAST` 소켓 옵션을 설정하는 부분을 하나 추가해봅시다. 이 프로그램을
[flx[`broadcaster.c`|broadcaster.c]]라고 부를 것입니다.

```{.c .numberLines}
/*
** broadcaster.c -- talker.c와 같은 데이터그램 클라이언트, 다만
**                  이 프로그램은 브로드캐스트를 할 수 있습니다.
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

#define SERVERPORT 4950 // 사용자들이 연결할 포트

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in their_addr; // 연결 대상의 주소 정보
    struct hostent *he;
    int numbytes;
    int broadcast = 1;
    //char broadcast = '1'; // 동작하지 않으면 이것을 써 보세요

    if (argc != 3) {
        fprintf(stderr,"usage: broadcaster hostname message\n");
        exit(1);
    }

    if ((he=gethostbyname(argv[1])) == NULL) {  // 호스트 정보를 받아옵니다
        perror("gethostbyname");
        exit(1);
    }

    if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // 이 호출이 브로드캐스트 패킷을 보낼 수 있게 만듭니다
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast,
        sizeof broadcast) == -1) {
        perror("setsockopt (SO_BROADCAST)");
        exit(1);
    }

    their_addr.sin_family = AF_INET;     // 호스트 바이트 순서
    their_addr.sin_port = htons(SERVERPORT); // short, 네트워크 바이트 순서
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
```

이 프로그램과 "평범한" UDP 클라이언트/서버의 상황에는 어떤 차이가 있을까요?
아무것도 없습니다! (이 경우에는 클라이언트가 브로드캐스트 패킷을 보낼 수 있다는
점을 빼면) 앞서와 마찬가지로 이전에 언급한 UDP [`listener`](#datagram)를
한 창에 실행하고 다른 창에 `broadcaster`를 실행하세요. 위에서 실패한 전송이
성공할 것입니다.

```
$ broadcaster 192.168.1.2 foo
sent 3 bytes to 192.168.1.2
$ broadcaster 192.168.1.255 foo
sent 3 bytes to 192.168.1.255
$ broadcaster 255.255.255.255 foo
sent 3 bytes to 255.255.255.255
```

`listener`가 수신한 패킷에 반응하는 것을 볼 수 있어야 합니다. (만약 `listener`가
반응하지 않는다면 그것이 IPv6 주소에 바인드되어서 그럴 수 있습니다. `listener.c`의
`AF_INET6`를 `AF_INET`으로 바꿔서 IPv4를 강제해 보세요.)

자, 여기까지도 조금 재미있었습니다. 그러나 같은 네트워크에 있는 다른 컴퓨터에서
`listener`를 실행해서 각 컴퓨터에서 하나씩 실행되게 한 후에
`broadcaster`에 브로드캐스트 주소를 넣고 다시 실행해봅시다. `sendto()`를 한 번만
실행했음에도 두 개의 `listener` 모두가 패킷을 받습니다! 멋지네요!

만약 `listener`가 실행 중인 컴퓨터의 IP 주소를 목적지로 발송된 데이터는 받는데
브로드캐스트 주소로 보낸 데이터는 받지 못한다면 아마도 [i[Firewall]] 방화벽이
컴퓨터에서 패킷을 막고 있을 것입니다. (그래요, [i[Pat]] Pat, [i[Bapper]] Bapper.
이게 제 샘플 코드가 동작하지 않았던 이유라는 것을 저보다 먼저 깨달아 줘서
고마워요. 안내서에 여러분을 언급하겠다고 했지요. 바로 이 부분에 적었습니다.)

다시 말하지만 브로드캐스트 패킷을 다룰 때에는 주의하세요. LAN에 있는 모든 시스템이
`recvfrom()` 수행 여부와 상관없이 패킷을 처리해야 하므로
전체 컴퓨터 네트워크에 상당한 부하를 줄 수 있습니다. 브로드캐스트 패킷은 반드시
가끔씩만, 그리고 적절한 상황에서만 쓰여야 합니다.
