# 약간 더 고급스러운 기술

이것들은 _진짜로_ 고급 기술인 것은 아니지만 우리가 지금까지 다룬 기본적인 것들을
벗어나고 있습니다. 사실 여러분이 여기까지 왔다면 스스로가 기본적인 유닉스 네트워크 프로그래밍을
꽤 잘 한다고 생각해도 됩니다. 축하합니다.

이제 여러분이 소켓에 대해서 배우고 싶어할 좀 더 비밀스러운 것들의 세상으로
용감하게 가보자. 열심히 해 보자!

## 블로킹 {#blocking}

[i[Blocking]<]

블로킹. 그것에 대해서 이미 들어보았다. 그것은 도대체 무엇인가? 간단히 말하면
"블록"은 "잠자기"에 대한 기술적 용어입니다. 위에서 `listener`를 실행하면
패킷이 도착할 때까지 그대로 기다리고 있다는 것을 눈치챘을 것입니다. 내부적으로는
`recvfrom()`이 호출되고, 데이터가 없으므로 `recvfrom()`는 데이터가 도착할
때까지 "블록"된 상태인 것입니다.(즉 그대로 잠들어 있게 됩니다.)

많은 함수들이 블록상태가 됩니다. `accept()`는 블로킹 함수입니다. 모든 `recv()`
함수들도 블록 동작을 하는 함수입니다.(역자 주 : 원문에서는 block자체를 동사로
쓴다.) 그것들이 블록동작을 할 수 있는 이유는 그렇게 할 수 있게 허락을 받았기
때문입니다. `socket()`으로 소켓 설명자를 처음 만들 때 커널이 이 소켓을 블로킹
소켓으로 설정합니다. [i[Non-blocking sockets]] 만약 소켓이 블록 동작을 하지
않길 원한다면 [i[`fcntl()` funtion]] `fcntl()` 에 대한 호출을 해야합니다. :

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

소켓을 논블로킹으로 설정하면 정보를 얻기 위해서 소켓을 효과적으로 "조사(원문 : poll)"
할 수 있습니다. 논 블로킹 소켓을 읽으려고 할 때 정보가 없다면 그것이 블록 동작을
하는 것은 허락되지 않는다. 그것은 -1을 반환할 것이고 `errno`은 [i[`EAGAIN` macro]]
`EAGAIN` 이나 [i[`EWOULDBLOCK` macro]] `EWOULDBLOCK`로 설정될 것입니다.

(잠깐, [i[`EAGAIN` macro]] `EAGAIN` _이나_ [i[`EWOULDBLOCK` macro]] `EWOULDBLOCK`
를 돌려준다니 무엇을 확인해야 한다는 말인가? 명세서에는 사실 당신의 시스템이 어떤 값을
돌려줘야 하는지가 정의되어 있지 않다. 그러므로 이식성을 위해서는 둘을 모두 확인해야 합니다.)

그러나 일반적으로 말하자면 이런 방식의 조사는 좋은 생각이 아니다. 당신의 프로그램이
소켓의 자료를 기다리면서 바쁜 대기 상태가 되면 당신은 보통의 프로그램보다 훨씬
CPU시간을 많이 사용할 것입니다. (역자 주 : 특별한 제한을 걸지 않으면 최대 단일 코어
하나를 100% 점유할 수 있습니다.) 읽기 작업을 기다리는 정보가 있는지 확인하기 위한
조금 더 우아한 해결책은 [i[`poll()` funtion]] `poll()`에 대해 다루는 다음 절에 있습니다.

[i[Blocking]>]

## `poll()`---동기 입출력 다중화 {#poll}

[i[poll()]<]

여러분이 정말로 하고자 해야하는 일은 소켓 _한 무더기_ 를 한 번에 감시하고 그 중에
데이터가 준비된 것을 처리하는 것입니다. 이런 방식을 통해서 당신은 모든 소켓을
지속적으로 조사하지 않아도 여러 개의 소켓 중 어떤 것이 데이터가 준비되었는지 알
수 있습니다.

> 경고 : `poll()`은 엄청나게 많은 수의 연결을 처리할 때 끔찍하게 느려진다.
> 이런 상황에서는 [fl[libevent|https://libevent.org/]] 같은 이벤트 라이브러리
> 를 사용하면 더 좋은 성능을 얻을 수 있습니다. 이런 라이브러리는 당신의 운영체제에서
> 사용할 수 있는 가장 빠른 방법을 사용하려고 시도할 것입니다.

그래서 어떻게 조사를 피할 수 있는가? 놀랍게도 `poll()` 시스템 함수를 사용해서 조사를
피할 수 있습니다. (역자 주 : poll은 투표, 설문, 그러한 부류의 조사라는 뜻입니다.) 간단히
말하자면 운영체제에게 모든 번거로운 작업을 우리 대신 하고 어떤 소켓에
자료가 도착하면 알려달라고 부탁하는 것입니다. 그 동안 우리의 프로세스는 대기 상태가
될 수 있고 시스템 자원을 아낄 수 있습니다.

전체적인 계획은 우리가 감시하고 싶은 소켓 설명자와 우리가 감시하고 싶은 이벤트의
종류에 대한 정보를 담은 `struct pollfd`의 배열을 보관하는 것입니다.
운영체제는 해당하는 종류의 이벤트가 발생(예를 들어 "소켓에 읽을 자료가 있다!"
같은 이벤트)하거나 사용자가 지정한 제한 시간이 지날 때까지 `poll()` 호출을 블록할
것입니다.

유용하게도 `listen()` 상태인 소켓은 새로운 연결이 `accept()`될 수 있는 상태가
되었을 때 "ready to read"를 반환할 것입니다.

이만하면 충분히 떠들었다. 이것을 쓰는 방법은 어떨지 보자.

```{.c}
#include <poll.h>

int poll(struct pollfd fds[], nfds_t nfds, int timeout);
```

`fds`는 우리의 정보(어떤 소켓을 무엇을 위해 감시할지)의 배열입니다.
`nfds`는 배열에 담긴 요소의 갯수입니다. `timeout`은 밀리초 단위의 제한시간입니다.
이것은 이벤트가 발생한 요소의 갯수를 돌려줍니다.

위에 등장하는 구조체는 무엇인지 살펴보자:

[i[`struct pollfd` type]]

```{.c}
struct pollfd {
    int fd;         // 소켓 설명자
    short events;   // 우리가 관심있는 이벤트의 비트맵
    short revents;  // poll()이 반환하는 시점에 발생한 이벤트의 비트맵
};
```

즉 이것의 배열을 하나 설정하고 각각의 `fd`필드를 우리가 관찰하고 싶은 소켓 설명자로
설정합니다. 그리고 각각의 `events`필드는 우리가 관심있는 이벤트로 설정하는 것입니다.

`events`필드는 아래 값들을 비트단위 논리합 계산한 결과값입니다.

| 매크로    | 설명                                                          |
| --------- | ------------------------------------------------------------- |
| `POLLIN`  | 이 소켓이 데이터를 `recv()`할 준비가 되면 알려달라.           |
| `POLLOUT` | 이 소켓에 블로킹 없이 데이터를 `send()`할 수 있으면 알려달라. |

`struct pollfd`의 배열을 준비하면 `poll()`에 그것을 넘길 수 있습니다. 배열의 크기와
밀리초 단위의 제한시간도 같이 넘겨야 합니다.(영원히 기다리려면 음수를 지정하면 됩니다.)

`poll()`이 반환하면 이벤트가 발생했음을 나타내는 `POLLIN`이나 `POLLOUT`이
설정되었는지 보기위해서 `revents` 필드를 확인할 수 있습니다.

(실제로는 `poll()`호출로 할 수 있는 것들이 더 있습니다. 자세한 내용은
[아래의 `poll()` 맨페이지](#pollman)를 참고하세요.)

여기 표준 입력에서 데이터를 읽어들일 수 있을 때까지(예를 들어 여러분이 줄바꿈을 입력할 때까지)
2.5초를 기다리는 예제가 있습니다.[flx[an example|poll.c]]

```{.c .numberLines}
#include <stdio.h>
#include <poll.h>

int main(void)
{
    struct pollfd pfds[1]; // 더 많은 것들을 관찰하고 싶다면 더 크게 하세요.

    pfds[0].fd = 0;          // 표준 입력
    pfds[0].events = POLLIN; // 읽을 준비가 되면 알려달라.

    // 만약 다른 것들도 관찰하고 싶다면
    //pfds[1].fd = some_socket; // 임의의 소켓 설명자
    //pfds[1].events = POLLIN;  // 읽을 준비가 되면 알려달라.

    printf("엔터키를 누르거나 제한시간 도달을 위해 2.5초를 기다리라\n");

    int num_events = poll(pfds, 1, 2500); // 2.5초 제한 시간

    if (num_events == 0) {
        printf("Poll 시간 초과!\n");
    } else {
        int pollin_happened = pfds[0].revents & POLLIN;

        if (pollin_happened) {
            printf("파일 설명자 %d을 읽을 준비가 되었다\n", pfds[0].fd);
        } else {
            printf("예상하지 못한 이벤트가 발생했다: %d\n", pfds[0].revents);
        }
    }

    return 0;
}
```

`poll()`이 `pfds`배열에서 이벤트가 발생한 요소의 갯수를 돌려준다는 것을 다시
기억하세요. 배열의 _어떤_ 요소에서 이벤트가 발생했는지는 알려주지 않지만 몇 개의
`revents` 필드가 0이 아닌 값으로 설정되었는지는 알려줍니다. 이것을 활용해서
반환된 숫자만큼의 0이 아닌 `revents`를 읽은 후에는 스캔을 중단할 수 있습니다.

몇 가지 질문이 떠오를 것이다: `poll()`에 넘겨준 집합에 새 파일 설명자를 추가하려면
어떻게 해야하는가? 이를 위해서 단순히 당신의 모든 필요에 부합할 만큼 충분한
크기의 배열을 만들거나 추가적인 공간이 필요할 때 `realloc()`을 사용하세요.

집합에서 요소를 제거하려면 어떻게 해야하는가? 이것을 위해서 당신은 배열의 마지막
요소를 삭제할 요소에 덮어씌우고, `poll()`의 count에 하나 더 적은 값을 전달하세요.
(역자 주 : 이것은 배열에서 임의의 요소 1개를 빠르게 제거하기 위해서 일반적으로
사용하는 방법입니다.) 다른 한 가지 방법은 `fd`필드를 음수로 설정하는 것이며
`poll()`은 해당 요소를 무시할 것입니다.

이 모든 것을 여러분이 `telnet`할 수 있는 하나의 채팅 서버에 합치려면 어떻게
해야할까?

우리가 할 일은 리스너 소켓을 시작한 후에 그것을 `poll()`할 파일 설명자
집합에 추가하는 일입니다. (그 파일설명자는 들어오는 연결이 있을 때 읽기 준비된
상태가 될 것입니다.)

그 후에 새로운 연결을 우리의 `struct pollfd` 배열에 추가하면 됩니다.
만약 배열의 크기가 부족하다면 동적으로 키우면 됩니다.

연결이 닫힌 후에는 그것을 배열로부터 제거합니다.

연결이 읽기 준비되면 우리는 그것에서 데이터를 읽어들인 후 다른 모든 연결에
전송합니다. 그렇게 해서 사용자들은 서로가 입력한 내용을 볼 수 있습니다.

이제 [flx[이 폴 서버|pollserver.c]]를 한 번 시험해보라. 이것을 하나의
창에서 실행한 후 몇 개의 다른 터미널 창에서 `telnet localhost 9034`를
실행해보라. 여러분이 하나의 창에서 입력하는 것을 (여러분이 엔터키를 누른 후에)
다른 창들에서 볼 수 있어야합니다.

그것 뿐 아니라 여러분이 `CTRL-]`를 누른 후 `quit`을 입력해서 `telnet`을 종료할
경우 서버는 연결종료를 감지하고 그 연결을 파일 설명자 배열에서 제거할 것입니다.

```{.c .numberLines}
/*
** pollserver.c -- 형편없는 다인 대화 서버
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

#define PORT "9034"   // 우리가 듣는(listening) 포트

// Get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Return a listening socket
int get_listener_socket(void)
{
    int listener;     // 듣는 소켓 설명자
    int yes=1;        // setsockopt() SO_REUSEADDR을 위해서는 아래를 보라
    int rv;

    struct addrinfo hints, *ai, *p;

    // 소켓을 받아서 바인드하자
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // 귀찮은 "주소가 이미 사용중입니다"에러메시지를 제거한다
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    freeaddrinfo(ai); // 더 이상 필요없다.

    // 여기가 실행되면 우리가 바인드하지 못했다는 의미다
    if (p == NULL) {
        return -1;
    }

    // 리슨
    if (listen(listener, 10) == -1) {
        return -1;
    }

    return listener;
}

// 집합에 새 파일 설명자를 추가한다
void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size)
{
    // 공간이 부족하면 pfds 배열을 늘린다.
    if (*fd_count == *fd_size) {
        *fd_size *= 2; // 두배로 합니다.

        *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
    }

    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN; // 읽을 준비가 되었는지 확인

    (*fd_count)++;
}

// 집합에서 하나의 인덱스를 제거한다
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
    // 마지막에서 하나를 삭제 대상 인덱스로 복사해온다
    pfds[i] = pfds[*fd_count-1];

    (*fd_count)--;
}

// 메인
int main(void)
{
    int listener;     // 리슨 소켓 설명자

    int newfd;        // 새롭게 accept()한 소켓 설명자
    struct sockaddr_storage remoteaddr; // 클라이언트 주소
    socklen_t addrlen;

    char buf[256];    // 클라이언트 데이터를 위한 버퍼

    char remoteIP[INET6_ADDRSTRLEN];

    // 5개의 연결을 위한 공간을 가지고 시작한다
    // (필요해지면 realloc할 것입니다.)
    int fd_count = 0;
    int fd_size = 5;
    struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

    // 초기화 후 리스닝 소켓을 얻는다
    listener = get_listener_socket();

    if (listener == -1) {
        fprintf(stderr, "리스닝 소켓 얻기 실패\n");
        exit(1);
    }

    // 리스너를 집합에 추가
    pfds[0].fd = listener;
    pfds[0].events = POLLIN; // 들어오는 연결을 읽을 준비가 되면 보고하라

    fd_count = 1; // 리스너를 위한 설정

    // 주 반복문
    for(;;) {
        int poll_count = poll(pfds, fd_count, -1);

        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }

        // 읽어들일 데이터를 찾기 위해서 존재하는 연결을 순회
        for(int i = 0; i < fd_count; i++) {

            // 무엇인가 읽을 준비가 되었는지 확인
            if (pfds[i].revents & POLLIN) { // 하나를 찾았다!!

                if (pfds[i].fd == listener) {
                    // 리스너를 읽을 준비가 되었다면 새 연결을 처리한다

                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        add_to_pfds(&pfds, newfd, &fd_count, &fd_size);

                        printf("폴서버: 새로운 연결 %s"
                            " 소켓 %d\n",
                            inet_ntop(remoteaddr.ss_family,
                                get_in_addr((struct sockaddr*)&remoteaddr),
                                remoteIP, INET6_ADDRSTRLEN),
                            newfd);
                    }
                } else {
                    // 리스너가 아닐 경우 일반적인 클라이언트다
                    int nbytes = recv(pfds[i].fd, buf, sizeof buf, 0);

                    int sender_fd = pfds[i].fd;

                    if (nbytes <= 0) {
                        // 오류가 발생했거나 연결이 클라이언트에 의해 닫혔다
                        if (nbytes == 0) {
                            // 연결이 닫혔다.
                            printf("폴서버: 소켓 %d 이 끊어짐\n", sender_fd);
                        } else {
                            perror("recv");
                        }

                        close(pfds[i].fd); // 잘가!

                        del_from_pfds(pfds, i, &fd_count);

                    } else {
                        // 클라이언트로부터 뭔가 좋은 데이터를 받았다

                        for(int j = 0; j < fd_count; j++) {
                            // 모두에게 보내자!
                            int dest_fd = pfds[j].fd;

                            // 리스너와 보낸 사람을 제외한다
                            if (dest_fd != listener && dest_fd != sender_fd) {
                                if (send(dest_fd, buf, nbytes, 0) == -1) {
                                    perror("send");
                                }
                            }
                        }
                    }
                } // 클라이언트로부터 온 데이터를 처리하는 부분의 끝
            } // poll()에서 읽을 준비가 된 부분의 끝
        } // 파일 설명자 순회의 끝
    } // for(;;)의 끝--절대 안 끝나겠지만!

    return 0;
}
```

다음 절에서는 비슷하지만 오래된 함수인 `select()`를 살펴볼 것입니다.
`select()`와 `poll()` 모두 비슷한 기능과 성능을 제공하고 쓰는 방식만
조금 다르다. `select()`쪽이 조금 더 이식성이 좋을지도 모르나 사용하기에는
조금 더 어색할 것입니다. 당신의 시스템에서 지원되기만 한다면 더 마음에
드는 쪽을 선택하세요.

[i[poll()]>]

## `select()`---동기화된 I/O 멀티플렉싱, 예전 방식 {#select}

[i[`select()` 함수]<]

이 함수는 이상하지만 아주 유용합니다. 다음과 같은 상황을 생각해보라: 당신은
서버이고 들어오는 연결을 감지함과 동시에 이미 가진 연결로부터 계속 자료를
읽어들이고 싶다.

별 문제가 없다고 말할지도 모른다. 그냥 `accept()`와 몇 개의 `recv()`를
쓰면 될 뿐입니다. 정말로 그럴까? `accept()`호출이 블록 상태로 들어갔다면
어떻게 하겠는가? 어떻게 `recv()`로 동시에 데이터를 받을 수 있겠는가?
"논 블로킹 소켓을 써라!" 역시 안 될 말입니다. CPU를 모조리 쓰고 싶지는 않을
것입니다. 그럼 어떻게 해야하는가?

`select()`가 여러 소켓을 동시에 관찰할 수 있는 능력을 줍니다. 그것이 어떤 것이
읽을 준비가 되었는지, 어떤 것이 쓸 준비가 되었는지, 그리고 정말로 관심이
있다면 어떤 것에 오류가 발생했는지까지 알려줄 것입니다.

> 경고 한마디: `select()`가 이식성이 아주 좋지만 연결이 아주 많은 상황에서는
> 끔찍하게 느려진다. 그런 상황에서는 당신의 시스템에서 쓸 수 있는 가장 빠른
> 방법을 시도하는 [fl[libevent|https://libevent.org/]] 같은 이벤트
> 라이브러리를 쓰면 더 나은 성능을 얻을 수 있습니다.

잡담은 그만하고 `select()`의 개요를 제시하겠다.

```{.c}
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int select(int numfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout);
```

이 함수는 특정한 `readfds`과 `writefds` 그리고 `exceptfds`로 이루어진
파일 설명자의 "집합들"을 관찰합니다. 만약 여러분이 표준 입력과 몇 개의 소켓
설명자로부터 읽어들일 수 있는지 확인하고 싶다면 `readfds` 집합에 0과
`sockfd`를 추가하세요. `numfds`는 가장 큰 파일 설명자에 1을 더한 값으로
설정해야 합니다. 이 예제에서는 `sockfd+1`이 될 것이며 이유는 당연히 그것이
표준 입력(`0`)보다 클 것이기 때문입니다.

`select()`가 반환할 때 `readfds`는 여러분이 선택한 파일 설명자 중에서 읽기를
위해 준비된 것들을 반영하기 위해서 변해있을 것입니다. 당신은 그것들을
아래에 나오는 `FD_ISSET()`매크로로 검사할 수 있습니다.
When `select()` returns, `readfds` will be modified to reflect which of
the file descriptors you selected which is ready for reading. You can
test them with the macro `FD_ISSET()`, below.

더 진행하기 전에 이 집합들을 어떻게 조작하는지에 대해 이야기할 것입니다. 각 집합은
`fd_set`형입니다. 이 자료형에 대해서 아래의 매크로들을 쓸 수 있습니다.

| 함수                                                    | 설명                                   |
| ------------------------------------------------------- | -------------------------------------- |
| [i[`FD_SET()` macro]]`FD_SET(int fd, fd_set *set);`     | `fd`를 `set`에 더합니다.               |
| [i[`FD_CLR()` macro]]`FD_CLR(int fd, fd_set *set);`     | `fd`를 `set`에서 제거합니다.           |
| [i[`FD_ISSET()` macro]]`FD_ISSET(int fd, fd_set *set);` | `fd`이 `set`에 있다면 참을 돌려줍니다. |
| [i[`FD_ZERO()` macro]]`FD_ZERO(fd_set *set);`           | `set`의 모든 요소를 제거합니다.        |

[i[`struct timeval` 형]<]

마지막으로 이 이상한 `struct timeval`은 무엇일까? 누군가 당신에게 자료를
보낼 때까지 무한히 기다리고 싶지 않을 때가 있습니다. 아마도 매 96초마다
실제로는 아무 일도 일어나지 않았어도 "진행중..."이라고 출력하고 싶을 수도 있습니다.
이 시간 구조체가 제한시간을 지정할 수 있도록 해 줍니다. 시간이 초과하고 `select()`
가 준비된 파일 설명자를 찾지 못할 경우, 그것은 반환하고 당신은 처리를 계속할
수 있습니다.

`struct timeval`는 아래와 같은 필드를 가지고 있다:

```{.c}
struct timeval {
    int tv_sec;     // 초
    int tv_usec;    // 마이크로초
};
```

단순히 `tv_sec`을 기다리고 싶은 초로, `tv_usec`을 기다리고 싶은 마이크로초로
설정하세요. 그렇다. _마이크로_ 초다. 밀리초가 아니다. 1밀리초는 1,000마이크로초다.
그리고 1,000밀리초는 1초입니다. 그러므로 1초는 1,000,000초입니다. 왜 "usec"일까?
"u"는 우리가 "마이크로"를 뜻하기 위해서 쓰는 그리스 문자 μ(뮤)와 닮았기 때문입니다.
또 함수가 반환할 때 `timeout`은 남아있는 시간을 보여주기 위해서 업데이트 될 수도
있습니다. 이것은 여러분이 실행중인 유닉스의 종류에 따라 다르다.

와! 우리는 마이크로초 해상도의 타이머를 가졌다! 사실 별로 기대하지 않는 것이 좋습니다.
여러분이 `struct timeval`을 아무리 작게 설정해도 당신의 표준 유닉스 타임슬라이스
(역자 주 : 커널이 프로세스 스케쥴링의 최소 단위로 쓰는 시간)만큼은 기다려야 합니다.

다른 흥미로운 것들: `struct timeval`을 `0`으로 설정하면 `select()`는 당신의 집합에
있는 모든 파일 설명자를 조사한 즉시 시간초과가 될 것입니다. 매개변수 `timeout`을 NULL
로 설정하면 절대 시간초과가 되지 않으며 파일 설명자가 준비될 때까지 기다릴 것입니다.
마지막으로 만약 특정 집합을 기다릴 필요가 없다면 그 집합은 `select()`를 호출할 때
NULL로 설정하면 됩니다.

[flx[아래의 코드 조각|select.c]]은 표준 입력에 뭔가 나타날 때까지 2.5초를 기다린다:

```{.c .numberLines}
/*
** select.c -- a select() demo
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

    // writefds와 exceptfds는 신경쓰지 않는다:
    select(STDIN+1, &readfds, NULL, NULL, &tv);

    if (FD_ISSET(STDIN, &readfds))
        printf("키가 눌렸다!\n");
    else
        printf("시간이 초과되었다.\n");

    return 0;
}
```

만약 여러분이 줄 단위로 버퍼처리되는 터미널을 사용한다면 엔터를 누르지 않으면
제한시간이 초과될 것입니다.

이제 여러분 중 일부는 이것이 데이터그램 소켓의 데이터를 기다리는 아주 훌륭한
방법이라고 생각할 것입니다. 그리고 맞다. 맞을 _수도_ 있습니다. 일부 유닉스에서는
select를 이 목적으로 쓸 수 있고, 일부에서는 그럴 수 없다. 그 방식을 시도하고
싶다면 당신의 로컬 맨페이지 내용을 참고해야 합니다.

일부 유닉스는 제한시간이 초과되기까지 남은 시간을 반영하기 위해서 당신의
`struct timeval`를 업데이트합니다. 그러나 다른 것들은 그렇게 하지 않는다.
만약 이식성있는 코드를 작성하고자 한다면 그것에 의존해서는 안됩니다.
(경과한 시간을 알고싶다면[i[`gettimeofday()`funtion]]`gettimeofday()`
을 사용하세요. 실망스럽겠지만 그것이 올바른 방법입니다.)

[i[`struct timeval` 형]>]

만약 읽기 집합에 잇는 소켓이 연결을 닫는다면 어떤 일이 생길까? 그 경우
`select()`는 그 소켓 설명자를 "읽기 준비된 상태"로 설정할 것입니다. 실제로
그 소켓에 `recv()`하면 `recv()`는 `0`을 돌려줄 것입니다. 그것이 클라이언트가
연결을 닫았음을 알아내는 방법입니다.

`select()`에 관한 흥미로운 이야기 하나 더: 만약
[i[`select()` function-->with `listen()`]]
[i[`listen()` function-->with `select()`]]
`listen()`작업중인 소켓을 가지고 있을 경우, 그 소켓의 파일 설명자를 `readfds`
집합에 넣어서 새로운 연결이 있는지 알 수 있습니다.

지금까지 전능한 `select()`함수에 대한 간략한 개관이었다.

그러나 대중적 요구가 있으므로 아래에 심도있는 예제를 첨부합니다. 불행하게도
위의 아주 단순한 예제와 아래의 에제에는 상당한 차이가 있습니다. 그렇지만 한 번
살펴보고 뒤따르는 설명을 읽어보라.

[flx[이 프로그램|selectserver.c]] 은 단순한 다중 사용자 챗 서버처럼 동작합니다.
하나의 창에서 이것을 실행한 후 다른 창에서 `telnet`을 통해 접속하세요. ("`telnet
hostname 9034`") 하나의 `telnet`세션에서 뭔가 입력하면 나머지 모두에서 그 내용이
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

#define PORT "9034"   // 우리가 듣는 포트

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    fd_set master;    // 마스터 파일 설명자 리스트
    fd_set read_fds;  // select()를 위한 임시 파일 설명자 리스트
    int fdmax;        // 가장 큰 파일 설명자 번호

    int listener;     // 듣는 소켓 설명자
    int newfd;        // 새롭게 accept() 처리한 소켓 설명자
    struct sockaddr_storage remoteaddr; // 클라이언트 주소
    socklen_t addrlen;

    char buf[256];    // 클라이언트 데이터를 위한 버퍼
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // setsockopt() SO_REUSEADDR를 위해서는 아래를 보라
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // 마스터와 임시 집합을 초기화
    FD_ZERO(&read_fds);

    // 소켓을 하나 받아와서 바인드합니다.
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // 짜증나는 "address already in use" 오류 메시지를 제거합니다.
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // 이곳이 실행되면 바인드가 되지 않은 것입니다.
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // 더 이상 필요없다.

    // 듣는다.
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // 리스너를 마스터 집합에 추가합니다.
    FD_SET(listener, &master);

    // 가장 큰 파일 설명자를 기록합니다.
    fdmax = listener; // 현재까지는 이것입니다.

    // 주 반복문
    for(;;) {
        read_fds = master; // 복사합니다.
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // 존재하는 연결을 순회하며 읽을 데이터가 있는지 확인합니다.
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // 새 연결을 처리합니다.
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // 마스터 집합에 추가합니다.
                        if (newfd > fdmax) {    // 가장 큰 것을 기록합니다.
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
                            "socket %d\n",
                            inet_ntop(remoteaddr.ss_family,
                                get_in_addr((struct sockaddr*)&remoteaddr),
                                remoteIP, INET6_ADDRSTRLEN),
                            newfd);
                    }
                } else {
                    // 클라이언트에서 온 자료를 처리합니다.
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // 오류가 발생했거나 클라이언트에 의해 연결이 종료되었다.
                        if (nbytes == 0) {
                            // 연결이 종료되었다.
                            printf("selectserver: socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // 잘가!
                        FD_CLR(i, &master); // 마스터 집합에서 삭제
                    } else {
                        // 클라이언트로부터 데이터가 들어왔다.
                        for(j = 0; j <= fdmax; j++) {
                            // 모두에게 보낸다!
                            if (FD_ISSET(j, &master)) {
                                // 리스너와 그 자신을 제외
                                if (j != listener && j != i) {
                                    if (send(j, buf, nbytes, 0) == -1) {
                                        perror("send");
                                    }
                                }
                            }
                        }
                    }
                } // 클라이언트로부터 온 데이터를 다루는 부분의 끝
            } // 새 연결을 얻는 부분의 끝
        } // 파일 설명자 순회 코드의 끝
    } // 무한루프의 끝. 절대 끝나지 않는다고 생각할 것이다!

    return 0;
}
```

코드에 `master`와 `read_fds` 두 개의 파일 설명자 집합이 있음에 주목하세요.
전자인 `master`는 새 연결을 듣는 소켓 설명자와 현재 연결된 모든 소켓의
설명자를 가진다.

`master`를 가지는 이유는 `select()`가 사실 여러분이 넘기는 집합을 _변형해서_
읽기 준비된 소켓을 반영하기 때문입니다. 우리는 하나의 `select()`호출과 다음
호출 사이에서 연결들을 계속 기억해야 하므로 이것들은 다른 곳에 안전하게
보관해야 하는 것입니다. 그래서 우리는 실제로 쓰기 전에 `master`를 `read_fds`에
복사하고 `select()`를 호출하는 것입니다.

하지만 그것은 우리가 새로운 연결을 받을 때마다 그것을 `master`집합에 추가해야
한다는 의미가 아닌가? 맞다! 그리고 연결이 닫힐 때마다 `master`집합에서 제거해야
하지않은가? 맞다, 그렇게 해야합니다.

`listener`소켓이 읽을 준비가 되었는지 확인한다는 사실에 주목하세요. 준비가 되어있다면
대기중인 새로운 연결이 있다는 의미이고, `accept()`한 후에 `master`집합에 추가합니다.
비슷하게 클라이언트 연결을 읽을 준비가 되고 `recv()`가 `0`을 돌려준다면 클라이언트가
연결을 닫았다는 사실을 알 수 있고, 우리는 그 연결을 `master` 집합에서 제거해야 합니다.

클라이언트에 대한 `recv()`가 0이 아닌 값을 돌려준다면 우리는 어떤 데이터가 도착했다는
것을 알 수 있습니다. 그러므로 자료를 받은 후에 `master`목록을 순회하면서 모든 나머지
연결된 클라이언트들에게 그 자료를 보낸다.

지금까지 전능한 `select()`함수에 대한 별로 단순하지 않은 개관이었다.

모든 리눅스 팬들을 위한 짧은 이야기: 드문 몇몇 상황에서 때때로 리눅스의 `select()`
는 실제로는 읽을 준비가 되어있지 않음에도 "읽을 준비가 되었다"고 하면서 반환합니다.
이것은 `select()`가 읽기 동작에 대한 블로킹이 없을 것이라고 말함에도 `read()`
가 블록될 것임을 의미합니다. 아무튼 해결책은 읽을 소켓에 [i[`O_NONBLOCK` macro]]
`O_NONBLOCK` 플래그를 설정해서 `EWOULDBLOCK`오류가 발생하도록 하는 것입니다.
(이 오류가 생겨도 무시해도 됩니다.) 소켓을 논블로킹 모드로 설정하는 방법에 대해서는
[`fcntl()` 참조 페이지](#fcntlman)를 참고하세요.

추가로 약간의 여담을 하자면 `select()`와 상당히 비슷한 일을 하지만 파일 설명자
집합을 다른 방식으로 처리하는 [i[`poll()` funtion]] `poll()`이라는 다른 함수가 있습니다.
[한 번 확인해 보라!](#pollman)

[i[`select()` 함수]>]

## 부분적인 `send()` 처리하기 {#sendall}

위에서 다룬 [`send()`에 대한 절](#sendrecv)에서 `send()`가 여러분이 전송 요청한 바이트들을
모두 보내지는 않을 수도 있다고 말한 것을 기억하는가? 당신은 512바이트를 보내길 원해도 복귀값은
412일 수 있다는 의미입니다. 남은 100바이트에는 무슨 일이 생긴 것인가?

음, 그것들은 여전히 당신의 작은 버퍼에 남아서 보내지길 기다리고 있습니다. 당신의 통제 밖에 있는
상황때문에 커널이 데이터를 한 덩어리로 전부 보내지는 않기로 결정한 것입니다. 그리고 친구여, 이제 남은
데이터를 보내는 일은 당신에게 달려있는 것입니다.

[i[`sendall()` 함수]<]
그 일을 하는 함수를 만드는 한 가지 방법은 이렇다:

```{.c .numberLines}
#include <sys/types.h>
#include <sys/socket.h>

int sendall(int s, char *buf, int *len)
{
    int total = 0;        // 몇 바이트를 보냈는가
    int bytesleft = *len; // 보내야하는 데이터는 얼마나 남아있는가
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // 실제로 보낸 바이트 수를 기록해서 돌려줍니다.

    return n==-1?-1:0; // 실패시에는 -1을, 성공시에는 0을 돌려줍니다.
}
```

이 예제에서 `s`는 여러분이 데이터를 보내고 싶은 소켓이고 `buf`는 자료를 담은 버퍼입니다.
`len`은 버퍼에 담긴 바이트의 갯수를 담은 `int`에 대한 포인터입니다.

함수는 오류가 발생하면 `-1`을 돌려준다(`errno`는 `send()`에 대한 호출로 인해 여전히
설정되어 있다). 또한 실제로 전송된 바이트의 갯수가 `len`을 통해 반환됩니다. 이 값은 오류가
발생하지 않는 이상 여러분이 전송하라고 요청한 바이트의 수와 같다. `sendall()`은 데이터를
전송하기 위해서 최선을 다하지만 오류가 발생하면 바로 당신에게 알려줄 것입니다.

완결성을 위해 이 함수에 대한 호출 예제가 여기 있다:

```{.c .numberLines}
char buf[10] = "Beej!";
int len;

len = strlen(buf);
if (sendall(s, buf, &len) == -1) {
    perror("sendall");
    printf("오류가 발생해서 %d바이트만 전송했습니다!\n", len);
}
```

[i[`sendall()` 함수]>]

수신자 측에 패킷의 일부가 도착하면 무슨 일이 벌어질까? 만약 패킷이 가변 길이일 경우 수신자는
어떻게 하나의 패킷이 끝나고 다른 하나가 시작되는 것을 알까? 그렇다. 실세계의 시나리오는 [i[Donkeys]]
뒤지게 고통스럽다. 당신은 아마도 [i[Data encapsulation]]_캡슐화_ 를 해야 할 것입니다.
(시작부에 있던 [데이터 캡슐화 절](#lowlevel)을 기억하는가?) 자세한 내용을 알고싶다면 계속
읽어보자!

## 직렬화---데이터를 포장하는 방법 {#serialization}

[i[Serialization]<]

네트워크를 통해 문자열 데이터를 보내는 것은 꽤 쉽다는 것을 이제 알 것입니다. 하지만 만약
`int`나 `float`같은 "이진" 자료를 전송하려고 하면 어떻게 해야하는가? 몇 가지 방법이
있습니다.

1. `sprintf()`같은 함수를 써서 수를 텍스트로 변환하고 텍스트를 전송합니다. 수신자는
   `strtol()`같은 함수를 써서 텍스트를 다시 숫자로 변환합니다.

2. `send()`에 데이터를 가리키는 포인터를 전달해서 원시 데이터를 그대로 전송합니다.

3. 데이터를 호환성 있는 바이너리 형태로 인코드합니다. 수신자는 디코드합니다.

오늘밤의 특별 사시회!

[_커튼이 올라간다_]

Beej가 말합니다: "저는 위의 세 번째 방법을 좋아합니다."

[_끝_]

(이 절을 기쁘게 시작하기에 앞서, 이 일을 하기 위한 라이브러리들이 이미 있다는 말을 미리
해야겠다. 이식성 있고 오류가 없는 당신만의 라이브러리를 만드는 작업은 꽤 어려운 일이 될
것입니다. 그러므로 그런 작업을 직접 하기 전에 그것들을 살펴보고 해야하는 다른 일을 처리하는
것이 나을 것입니다. 필자는 그런 것이 어떻게 동작하는지 궁금할 독자들을 위해서 관련된
내용을 여기에 담을 뿐입니다.)

사실 위에 언급한 모든 방법에 각각의 장점과 단점이 있습니다. 그러나 위에 말한대로, 필자는 일반적으로
세 번째 방법을 선호합니다. 그러나 먼저 다른 두 가지 방법의 장단점에 대해서 조금 더 알아보자.

수를 보내기 전에 텍스트로 인코딩하는 첫 번재 방법은 랜선을 타고 오는 정보를 출력하고 읽어보기가
쉽다는 장점이 있습니다. [i[IRC]] [fl[Internet
Relay Chat (IRC)|https://en.wikipedia.org/wiki/Internet_Relay_Chat]]
처럼 인간이 읽을 수 있는 프로토콜은 때때로 전송 대역폭에 민감하지 않은 상황에서 사용하기에 아주
훌륭합니다. 그러나 그것은 변환이 느리다는 단점이 있고, 결과물은 언제나 원본 수보다 더 많은
공간을 차지한다는 단점이 있습니다.

두 번째 방법: 원시 데이터(원문: raw data)를 넘기기. 이것은 꽤 간단(하고 위험)하다:
보낼 데이터에 대한 포인터를 얻은 후 그것에 send를 호출합니다.

```{.c}
double d = 3490.15926535;

send(s, &d, sizeof d, 0);  /* 위험--이식성 없음!(역자 주 : 이식성은 컴퓨터 프로그램이나 소스코드가 서로 다른 구조를 가진 컴퓨터에서 동작하는 특성을 의미합니다.) */
```

수신자는 이것을 아래와 같이 받는다:

```{.c}
double d;

recv(s, &d, sizeof d, 0);  /* 위험--이식성 없음! */
```

빠르고, 간단하다---문제될 것이 없지않은가? 사실, 모든 아키텍처들이 `double`
(이나 다른 예로는 `int`)을 동일한 비트 표현이나 심지어 동일한 바이트 순서로 표시하는
것은 아니라는 문제가 있다! 위의 코드는 절대로 이식성이 없다. (잠깐---이식성이 필요 없는
상황도 있지 않을까? 그렇다면 이 방식은 좋고 빠른 방법이 됩니다.)

정수 자료형을 포장할 때 [i[`htons()` function]] `htons()`-수를 [i[Byte ordering]]
네트워크 바이트 순서로 변환해주는 종류의 함수를 어떻게 쓰는지, 그리고 그것이 왜 필요한지
이미 살펴보았다. 불행하게도 `float`자료형에 대해서는 유사한 함수가 없다. 희망이 없는 것일까?

두려워 말라!(잠시 두려움을 느꼈는가? 두렵지 않았는가? 아주 조금도?) 우리에게 방법이 있다:
우리는 데이터를 원격지에서 풀어낼 수 있는 알려진 방식으로 포장(원문 : pack)(또는 "marshal"
또는 "직렬화" 그것도 아니면 그런 일에 대한 천만개의 다른 이름)을 할 수 있습니다.

"알려진 이진 형식"은 무엇일까? 우리는 이미 `htons()`의 예제를 보았다. 그것은 수를 호스트의
형식이 무엇이든간에 네트워크 바이트 순서로 변환(또는 "인코드", 이것이 더 이해하기 쉽다면)합니다.
수를 원래대로 돌려놓기(디코드) 위해서 수신자는 `ntohs()`를 호출해야 합니다.

하지만 필자가 조금 전에 비-정수 타입에 대해서는 그런 함수가 없다고 하지 않았던가? 그렇다.
그리고 C에서 이것을 처리하는 표준 방법이 없기 때문에 이것은 조금 까다로운 일입니다. (파이썬
팬들은 이런 작업을 할 필요가 없을 것입니다.)

필요한 작업은 데이터를 알려진 형식으로 포장하고 유선상으로 실어보내는 것입니다. 예를 들어서
`float`를 포장하는 작업을 위한 [flx[간단하고 지저분하고 개선할 점이 많은 예제코드|pack.c]]
가 있다:

```{.c .numberLines}
#include <stdint.h>

uint32_t htonf(float f)
{
    uint32_t p;
    uint32_t sign;

    if (f < 0) { sign = 1; f = -f; }
    else { sign = 0; }

    p = ((((uint32_t)f)&0x7fff)<<16) | (sign<<31); // 전체 부분과 부호
    p |= (uint32_t)(((f - (int)f) * 65536.0f))&0xffff; // 소수점

    return p;
}

float ntohf(uint32_t p)
{
    float f = ((p>>16)&0x7fff); // 전체
    f += (p&0xffff) / 65536.0f; // 소수점

    if (((p>>31)&0x1) == 0x1) { f = -f; } // 부호 비트 설정

    return f;
}
```

위의 코드는 `float`를 32비트 수에 저장하기 위한 단순한 구현입니다. 최상위 비트(31)가 수의 부호를
저장하기 위해 쓰인다. ("1"이 음수를 의미합니다.) 다음 15비트(30-16)(역자 주 : 원문에서는 7비트라고
적혀 있으나 코드의 내용상 오타로 보임)가 `float`의 전체 수 부분을 저장하기 위해서 쓰인다. 마지막으로
남은 비트들(15-0)이 수의 소수점 부분을 기록하기 위해서 쓰인다.

사용법은 꽤 직관적이다:

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

장점을 보자면, 이 코드는 작고 간단하며 빠르다. 단점을 보자면 이 방식은 공간을 효율적으로
쓰지 않으며 표현 범위가 상당히 제한되어 있습니다.---32767보다 큰 수를 저장하려고 하면 이
방법은 제대로 동작하지 않을 것이다! 또한 여러분은 위의 예제에서 소수점의 마지막 2자리가
제대로 보존되지 않은 것을 볼 수 있습니다.

이것을 해결하려면 어떻게 해야할까? 사실 부동소수점 수를 저장하기 위한 _표준_ 은 [i[IEEE-754]]
[fl[IEEE-754|https://en.wikipedia.org/wiki/IEEE_754]]로 알려져 있습니다.
대부분의 컴퓨터는 부동 소수점 계산을 위해서 내부적으로 이 형식을 사용합니다.
그러므로 그런 경우라면 엄밀히 말하자면 변환을 수행할 필요는 없다. 그러나
여러분의 소스코드가 이식성이 있기를 바란다면 그런 가정을 할 수는 없다.
(한 편으로 만약 속도를 원한다면 변환이 필요없는 플랫폼에서는 변환 작업을
제거하는 최적화를 해야함을 의미합니다. `htons()`및 그와 유사한 함수들은 그런
방식으로 동작합니다.)

[flx[여기 단정밀도 부동소수점 및 배정밀도 부동소수점 타입을 IEEE-754로 인코드하는
코드가 있다|ieee754.c]]. (엄밀히는 거의 대부분을 인코드합니다. 이 코드는 NaN이나
Infinity를 처리하지 않는다. 그러나 그런 처리가 가능하게 수정할 수도 있습니다.)

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
    unsigned significandbits = bits - expbits - 1; // 부호 비트를 위해 1을 뺀다.

    if (f == 0.0) return 0; // 특별한 경우의 처리

    // 부호를 확인하고 정규화를 시작합니다.
    if (f < 0) { sign = 1; fnorm = -f; }
    else { sign = 0; fnorm = f; }

    // 정규화된 형태의 f를 얻어내고 지수를 추적합니다.
    shift = 0;
    while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
    while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
    fnorm = fnorm - 1.0;

    // 실수부의 부동소수점이 아닌 이진 표현을 구합니다.
    significand = fnorm * ((1LL<<significandbits) + 0.5f);

    // 바이어스를 더한 지수부를 구합니다.
    // (역자 주 : IEEE754에서는 지수부를 일정 비트의 정수로 나타내며,
    // 바이어스보다 큰 수는 바이어스와 계산한 차의 절대값 만큼의 양의 지수,
    // 바이어스 미만은 바이어스와 계산한 차의 절대값만큼의 음의 지수를 나타낸다.)
    exp = shift + ((1<<(expbits-1)) - 1); // shift + bias

    // 최종 값을 돌려줍니다.
    return (sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand;
}

long double unpack754(uint64_t i, unsigned bits, unsigned expbits)
{
    long double result;
    long long shift;
    unsigned bias;
    unsigned significandbits = bits - expbits - 1; // 부호 비트를 위해서 -1

    if (i == 0) return 0.0;

    // 실수부를 뽑아낸다.
    result = (i&((1LL<<significandbits)-1)); // 마스크 처리
    result /= (1LL<<significandbits); // 부동소수점으로 변환
    result += 1.0f; // 1을 다시 더합니다.

    // 지수부를 처리합니다.
    bias = (1<<(expbits-1)) - 1;
    shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
    while(shift > 0) { result *= 2.0; shift--; }
    while(shift < 0) { result /= 2.0; shift++; }

    // 부호처리
    result *= (i>>(bits-1))&1? -1.0: 1.0;

    return result;
}
```

32비트(아마도 `float`)와 64비트(아마도 `double`) 수를 위한 패킹과
언패킹 매크로를 위에 넣어두었다. 그러나 `bits`크기의 데이터를 인코드
하기위해서 `pack754()`함수를 직접 호출할 수도 있을 것입니다.(`expbits`
만큼의 지수부가 정규화된 수의 지수로 보존될 것입니다.)

여기 사용 예시가 있다:

```{.c .numberLines}

#include <stdio.h>
#include <stdint.h> // uintN_t 형들을 정의합니다.
#include <inttypes.h> // PRIx 매크로들을 정의합니다.

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

    return 0;
}
```

위의 코드는 아래의 출력을 생성한다:

```
float before : 3.1415925
float encoded: 0x40490FDA
float after  : 3.1415925

double before : 3.14159265358979311600
double encoded: 0x400921FB54442D18
double after  : 3.14159265358979311600
```

여러분이 가질 수 있는 또 다른 질문은 어떻게 `sturct`를 포장하는가입니다.
불행히도 컴파일러는 `struct`의 모든 곳에 자유롭게 패딩을 넣을 수 있습니다.
그리고 그것은 구조체 전체를 한 번에 네트워크에 전송할 수는 없다는 것을
의미합니다. ("이건 되고", "이건 안되고"를 듣는 것이 지겨운가? 미안합니다.
내 친구의 말을 빌리자면 "뭔가 잘못되면 나는 늘 마이크로소프트를 탓합니다."
이 경우에는 아마도 마이크로소프트의 잘못만은 아닐 것입니다. 그러나 내 친구의
선언은 완전히 옳다.)

다시 주제로 돌아가서: `stuct`를 전송하기 위한 최고의 방법은 각각의 필드를
독립적으로 포장한 다음 반대편에 도착하면 다시 `struct`안에 풀어넣는 것입니다.

여러분은 이것이 굉장히 큰 작업일 것이라 예상할 것입니다. 맞다. 여러분이 할 일은
데이터를 포장하는 일을 도와줄 도우미 함수를 작성하는 것입니다. 재미있을 것이다!
정말로!!

Kernighan(역자 주 : 커니건)과 Pike가 지은 [flr[_The Practice of Programming_|tpop]]
에서 그들은 바로 그 일을 하도록 `printf()`와 유사한 `pack()`과 `unpack()`함수를 작성했다.
그것에 대한 링크를 제공하고 싶지만 그 함수들과 책의 다른 소스코드는 온라인으로
제공되지 않고 있습니다.

(The Practice of Programming은 아주 좋은 책입니다. 필자가 그 책을 추천할
때마다 제우스 신이 고양이를 한 마리씩 구합니다. (역자 주 : 아주 좋은 선행이라는 뜻))

이 시점에서 필자는 [fl[프로토콜 버퍼의 C 구현체|https://github.com/protobuf-c/protobuf-c]]
에 대한 링크를 제공하려 합니다. 필자는 이것을 써 본 적이 없으나 훌륭한 코드로
보인다. 파이썬과 펄 프로그래머들은 같은 일을 하기 위해서 그들의 언어가
가진 `pack()`과 `unpack()` 함수를 확인해보길 바란다. 자바는 유사한 방식으로
사용할 수 있는 Serializable 인터페이스를 가지고 있습니다.

그러나 만약 여러분이 자신만의 패킹 유틸리티를 C언어로 작성하고 싶다면, K&P의
해결책은 패킷을 만들기 위해서 가변 길이 매개변수를 활용하는 `printf()`와
유사한 함수를 만드는 것입니다. [flx[여기 필자가 만든 버전이 있으며|pack2.c]]
여러분이 그런 것이 어떻게 동작하는지 알기에 충분할 것입니다.

(이 코드는 위의 `pack754()`함수를 참조합니다. `packi*()`함수는 또 다른 정수가
아닌 `char`의 배열에 수를 담는다는 점을 제외하면 `htons()`계열 함수와 유사하게
동작합니다.)

```{.c .numberLines}
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

/*
** packi16() -- 16비트를 char 버퍼에 저장합니다. (htons()처럼)
*/
void packi16(unsigned char *buf, unsigned int i)
{
    *buf++ = i>>8; *buf++ = i;
}

/*
** packi32() -- 32비트를 char 버퍼에 저장합니다. (htonl()처럼)
*/
void packi32(unsigned char *buf, unsigned long int i)
{
    *buf++ = i>>24; *buf++ = i>>16;
    *buf++ = i>>8;  *buf++ = i;
}

/*
** packi64() -- 64비트를 char 버퍼에 저장합니다. (htonl()처럼)
*/
void packi64(unsigned char *buf, unsigned long long int i)
{
    *buf++ = i>>56; *buf++ = i>>48;
    *buf++ = i>>40; *buf++ = i>>32;
    *buf++ = i>>24; *buf++ = i>>16;
    *buf++ = i>>8;  *buf++ = i;
}

/*
** unpacki16() -- 16비트 정수를 char 버퍼에서 풀어낸다. (ntohs()처럼)
*/
int unpacki16(unsigned char *buf)
{
    unsigned int i2 = ((unsigned int)buf[0]<<8) | buf[1];
    int i;

    // change unsigned numbers to signed
    if (i2 <= 0x7fffu) { i = i2; }
    else { i = -1 - (unsigned int)(0xffffu - i2); }

    return i;
}

/*
** unpacku16() -- 16비트 부호없는 정수를 char 버퍼에서 풀어낸다. (ntohs()처럼)
*/
unsigned int unpacku16(unsigned char *buf)
{
    return ((unsigned int)buf[0]<<8) | buf[1];
}

/*
** unpacki32() -- 32비트 정수를 char 버퍼에서 풀어낸다. (ntohl()처럼)
*/
long int unpacki32(unsigned char *buf)
{
    unsigned long int i2 = ((unsigned long int)buf[0]<<24) |
                           ((unsigned long int)buf[1]<<16) |
                           ((unsigned long int)buf[2]<<8)  |
                           buf[3];
    long int i;

    // change unsigned numbers to signed
    if (i2 <= 0x7fffffffu) { i = i2; }
    else { i = -1 - (long int)(0xffffffffu - i2); }

    return i;
}

/*
** unpacku32() -- 32비트 부호없는 정수를 char 버퍼에서 풀어낸다. (ntohl()처럼)
*/
unsigned long int unpacku32(unsigned char *buf)
{
    return ((unsigned long int)buf[0]<<24) |
           ((unsigned long int)buf[1]<<16) |
           ((unsigned long int)buf[2]<<8)  |
           buf[3];
}

/*
** unpacki64() -- 32비트 정수를 char 버퍼에서 풀어낸다. (ntohl()처럼)
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

    // change unsigned numbers to signed
    if (i2 <= 0x7fffffffffffffffu) { i = i2; }
    else { i = -1 -(long long int)(0xffffffffffffffffu - i2); }

    return i;
}

/*
** unpacku64() -- 64비트 부호없는 정수를 char 버퍼에서 풀어낸다. (ntohl()처럼)
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
** pack() -- 버퍼의 형식화 문자열이 지시한 방식으로 데이터를 저장합니다.
**
**   bits |signed   unsigned   float   string
**   -----+----------------------------------
**      8 |   c        C
**     16 |   h        H         f
**     32 |   l        L         d
**     64 |   q        Q         g
**      - |                               s
**
**  (16비트 부호없는 길이가 자동으로 문자열의 앞에 붙는다.)
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

    float f;                    // 부동소수점
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

        case 'C': // 부호없는 8비트
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

        case 'H': // 부호없는 16비트
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

        case 'L': // 부호없는 32비트
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

        case 'Q': // 부호없는 64비트
            size += 8;
            Q = va_arg(ap, unsigned long long int);
            packi64(buf, Q);
            buf += 8;
            break;

        case 'f': // 부동소수점 16비트
            size += 2;
            f = (float)va_arg(ap, double); // 자료형 승급
            fhold = pack754_16(f); // IEEE 754로 변환
            packi16(buf, fhold);
            buf += 2;
            break;

        case 'd': // 부동소수점 32비트
            size += 4;
            d = va_arg(ap, double);
            fhold = pack754_32(d); // IEEE 754로 변환
            packi32(buf, fhold);
            buf += 4;
            break;

        case 'g': // 부동소수점 64비트
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
** unpack() -- 형식화 문자열이 지정하는대로 버퍼에 데이터를 풀어놓는다.
**
**   bits |signed   unsigned   float   string
**   -----+----------------------------------
**      8 |   c        C
**     16 |   h        H         f
**     32 |   l        L         d
**     64 |   q        Q         g
**      - |                               s
**
**  (문자열은 저장된 길이에 근거해서 추출됩니다. 그러나 `s`의 앞에 최대 길이를 앞에 지정할 수 있습니다.)
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

    float *f;                    // 부동소수점
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
            if (*buf <= 0x7f) { *c = *buf;} // 부호를 다시 붙인다
            else { *c = -1 - (unsigned char)(0xffu - *buf); }
            buf++;
            break;

        case 'C': // 부호없는 8비트
            C = va_arg(ap, unsigned char*);
            *C = *buf++;
            break;

        case 'h': // 16비트
            h = va_arg(ap, int*);
            *h = unpacki16(buf);
            buf += 2;
            break;

        case 'H': // 부호없는 16비트
            H = va_arg(ap, unsigned int*);
            *H = unpacku16(buf);
            buf += 2;
            break;

        case 'l': // 32비트
            l = va_arg(ap, long int*);
            *l = unpacki32(buf);
            buf += 4;
            break;

        case 'L': // 부호없는 32비트
            L = va_arg(ap, unsigned long int*);
            *L = unpacku32(buf);
            buf += 4;
            break;

        case 'q': // 64비트
            q = va_arg(ap, long long int*);
            *q = unpacki64(buf);
            buf += 8;
            break;

        case 'Q': // 부호없는 64비트
            Q = va_arg(ap, unsigned long long int*);
            *Q = unpacku64(buf);
            buf += 8;
            break;

        case 'f': // 부동소수점
            f = va_arg(ap, float*);
            fhold = unpacku16(buf);
            *f = unpack754_16(fhold);
            buf += 2;
            break;

        case 'd': // 32비트 부동소수점
            d = va_arg(ap, double*);
            fhold = unpacku32(buf);
            *d = unpack754_32(fhold);
            buf += 4;
            break;

        case 'g': // 64비트 부동소수점
            g = va_arg(ap, long double*);
            fhold = unpacku64(buf);
            *g = unpack754_64(fhold);
            buf += 8;
            break;

        case 's': // 문자열
            s = va_arg(ap, char*);
            len = unpacku16(buf);
            buf += 2;
            if (maxstrlen > 0 && len >= maxstrlen) count = maxstrlen - 1;
            else count = len;
            memcpy(s, buf, count);
            s[count] = '\0';
            buf += len;
            break;

        default:
            if (isdigit(*format)) { // 최대 문자열 길이를 기록
                maxstrlen = maxstrlen * 10 + (*format-'0');
            }
        }

        if (!isdigit(*format)) maxstrlen = 0;
    }

    va_end(ap);
}
```

그리고 위의 코드를 [flx[시연하는 프로그램|pack2.c]]이 여기에 있습니다. 이 프로그램은
`buf`에 약간의 데이터를 포장한 후 다시 변수에 풀어놓는다. `unpack()`을 문자열
매개변수로 호출하는 경우(형식 지정자 "`s`") 버퍼 오버런을 방지하기 위해서
"`96s`"처럼 최대 길이를 앞에 붙이는 것이 현명하다는 것을 기억하세요. 네트워크를
통해 받은 데이터를 풀어놓을 때에는 주의해야 합니다. 악의적인 사용자가 당신의
시스템을 공격하기 위해서 악의적으로 구성된 패킷을 보낼 수 있다!

```{.c .numberLines}
#include <stdio.h>

// 부동 소수점 형의 다양한 비트의 변종
// 아키텍처 별로 다르다.
typedef float float32_t;
typedef double float64_t;

int main(void)
{
    unsigned char buf[1024];
    int8_t magic;
    int16_t monkeycount;
    int32_t altitude;
    float32_t absurdityfactor;
    char *s = "Great unmitigated Zot! You've found the Runestaff!";
    char s2[96];
    int16_t packetsize, ps2;

    packetsize = pack(buf, "chhlsf", (int8_t)'B', (int16_t)0, (int16_t)37,
            (int32_t)-5, s, (float32_t)-3490.6677);
    packi16(buf+1, packetsize); // 시작을 위해 패킷 사이즈를 패킷에 넣어둔다.

    printf("packet is %" PRId32 " bytes\n", packetsize);

    unpack(buf, "chhl96sf", &magic, &ps2, &monkeycount, &altitude, s2,
        &absurdityfactor);

    printf("'%c' %" PRId32" %" PRId16 " %" PRId32
            " \"%s\" %f\n", magic, ps2, monkeycount,
            altitude, s2, absurdityfactor);

    return 0;
}
```

여러분이 직접 만든 코드를 쓰건 다른 사람이 작성한 것을 쓰건, 매번 각 비트를
수동으로 포장하기보다는 버그를 쉽게 잡아내기 위해서 일반적인 데이터 패킹 루틴을
사용하는 것이 좋은 습관입니다.

데이터를 포장할 때에 쓰기 좋은 형식은 무엇일까? 아주 좋은 질문입니다. 다행히도
[i[XDR]] [flrfc[RFC 4506|4506]], 외부 데이터 표현 표준이 부동소수점과 정수,
배열 등 다양한 자료형에 대해서 이진 형식을 정의합니다. 만약 데이터를 직접 처리할
생각이라면 이것을 준수하는 것을 권장합니다. 그러나 반드시 그래야 하는 것은 아니다.
패킷 경찰들이 문 앞에 지키고 서 있는 것은 아니다. 최소한 필자는 그렇지 않을 것이라고
_생각합니다._

어떤 경우에도, 데이터를 보내기 전에 인코드 하는 것이 옳은 일입니다.

[i[Serialization]>]

## 망할 데이터 캡슐화 {#sonofdataencap}

아무튼 데이터 캡슐화가 정말로 의미하는 것은 무엇인가? 가장 단순한 경우 그것은
여러분이 데이터에 약간의 식별 정보나 패킷 길이 혹은 둘 모두를 담은 헤더를
붙여둔다는 뜻이 됩니다.

헤더가 어떤 모양을 하고 있어야 할까? 사실 여러분의 프로젝트를 끝내기 위해서
필요하다고 느끼는 어떤 이진 데이터면 됩니다.

와. 정말 막연한 이야기다.

좋습니다. 예를 들자면 `SOCK_STREAM`을 사용하는 다중 사용자 대화 프로그램이 있다고 하자.
한 사용자가 뭔가 입력한다면, 두 조각의 정보가 서버에 전달되어야 합니다. 무엇을 말했는지,
그리고 누가 말했는지.

여기까지는 좋은가? "그럼 무엇이 문제인가?"라고 여러분은 질문할 것입니다.

문제는 메시지가 가변 길이일 수 있다는 점입니다. "Tom"이라는 사용자가 "Hi"라고
말할 수 있고 "Benjamin"이라는 또다른 사용자가 "Hey guys what is up?"이라고
말할 수도 있습니다.

그것이 들어오는대로 클라이언트에게 `send()`한다고 하자. 여러분의 송출 자료 스트림은
아래와 같을 것입니다.

```
t o m H i B e n j a m i n H e y g u y s w h a t i s u p ?
```

이런 식일 것입니다. 클라이언트가 어떻게 하면 메시지의 시작과 끝을 알 수 있겠는가?
원한다면 모든 메시지가 같은 길이를 갖도록 하고 우리가 구현한 [i[`sendall()` function]] `sendall()`
함수를 그냥 호출할 수 있을 것입니다. 그러나 그렇게 하면 대역폭을 낭비하게 된다!
"tom"이 "Hi"라고 말하는 일을 위해서 1024바이트를 `send()`하고싶지는 않을
것입니다.

그래서 우리는 데이터를 작은 헤더와 패킷 구조에 _캡슐화_ 합니다. 클라이언트와
서버 모두 이 데이터를 어떻게 포장하고 풀어내는지(때때로 "marshal"과 "unmarshal"
이라고 부른다) 알고있습니다. 지금 이해할 필요는 없지만 우리는 클라이언트와
서버가 어떻게 통신하는지를 정의하는 _프로토콜_ 을 정의하려고 하고있습니다.

지금은 사용자의 이름이 `'\0'`으로 패드된 고정된 8개의 문자라고 가정하자.
데이터는 최대 128개 문자로 구성되는 가변길이 형태라고 하자. 이 상황에서
쓸 수 있는 예제 패킷 구조를 살펴보자.

1. `len` (1바이트, 부호 없음)---패킷의 전체 길이, 8바이트의 사용자 이름과
   대화 데이터의 길이를 센다.

2. `name` (8 바이트)---사용자의 이름, 필요한 경우 0이 덧대진다.

3. `chatdata` (_n_ 바이트)---데이터 자체, 최대 128바이트. 패킷의 길이는 이
   데이터의 길이에 8을 더한 값으로 계산되어야 합니다.(위에서 언급한 이름 필드의 길이)

필자가 8바이트와 128바이트를 필드의 길이 제한으로 선택한 이유가 궁금한가?
특별한 이유는 없고, 충분히 길 것이라 생각했다. 그러나 아마도 8바이트는 여러분의
필요에는 조금 못 미칠수도 있습니다. 그런 경우에는 이름 필드의 길이를 30바이트나
다른 값으로 설정할 수 있습니다. 선택은 여러분의 몫입니다.

위의 패킷 정의를 사용하는 첫 번째 패킷은 아래와 같은 정보로 구성될 수 있습니다.
(16진수와 아스키 코드로 표시되었다.):

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

그러나 어떻게 해야할까? 우리는 패킷이 완성되기 위해서 받아야 하는 바이트의
총 갯수를 알고있습니다. 갯수가 패킷의 앞쪽에 붙어있기 때문입니다. 우리는 또한 패킷의
최대 크기가 1 + 8 + 128, 즉 137바이트라는 것을 알고있습니다. (우리가 그렇게 정의했기
때문입니다.)

여기에서는 몇 가지 방식으로 일을 할 수 있습니다. 모든 패킷이 길이 정보로 시작한다는 것을
알고있으므로 패킷의 길이를 얻기 위해서 `recv()`를 호출할 수 있습니다. 그리고
길이를 가지고 있으면 전체 패킷을 받을 때까지 남은 길이를 명시하면서 `recv()`
를(아마도 반복적으로) 호출하는 것입니다. 이 방식의 장점은 하나의 패킷을 담기에 충분한
크기의 버퍼만 있으면 된다는 것이고, 단점은 모든 데이터를 받기 위해서 `recv()`를
최소 두 번 호출해야 한다는 것입니다.

다른 옵션은 `recv()`을 호출할 때 한 패킷의 최대 크기만큼을 받겠다고 지정하는 것입니다.
그 후에 받은 자료를 버퍼의 뒤쪽에 쌓아두고, 패킷이 완성되었는지 확인합니다. 물론
다음 패킷의 일부를 받을 수 있으므로 그것을 위한 여분의 공간이 필요합니다.

이를 위해서 두 개의 패킷을 담기에 충분한 배열을 선언하면 됩니다. 이것은 패킷이
도착하는대로 재구성하는 일에 사용할 작업 공간입니다.

자료를 `recv()`처리할 때마다 그것을 작업 버퍼에 덧붙이고 패킷이 완성되었는지
확인합니다. 버퍼에 담긴 바이트의 갯수가 헤더에 명시된 길이보다 많거나 같은지
확인한다는 뜻이다(사실은 헤더에 헤더 자신의 길이가 포함되지 않으므로 +1을
해야한다). 만약 버퍼의 바이트 수가 1보다 적다면 물론 패킷은 완성되지 않은
것입니다. 또한 이 경우 버퍼의 첫 바이트를 읽어들인다고 해도 그것은 쓰레기값이므로
그것을 감안한 처리를 해야합니다.

패킷이 완성되면 여러분은 그것으로 여러분이 원하는 일을 할 수 있습니다. 패킷을
사용하거나, 그것을 여러분의 작업 버퍼에서 제거할 수 있습니다.

휴! 아직 머릿속이 어지러운가? 여기 원투펀치의 두 번째 부분이 있습니다. 한 번의
`recv()`호출로 한 패킷을 넘어서는 분량을 읽어들일 수가 있습니다. 즉 작업버퍼에
하나의 완전한 패킷과 다음 패킷의 불완전한 부분이 있을 수 있다는 것입니다.
제기랄. (그러나 이런 경우를 처리하기 위해서 여러분의 작업 버퍼를 _두_ 개의
패킷을 담기에 충분한 크기로 만들어둔 것입니다.)

첫 패킷의 길이를 헤더를 통해 알고있고 작업 버퍼에 있는 바이트의 수를 추적하고
있으므로, 뺄셈을 해서 작업 버퍼에 있는 바이트 중 몇 개가 다음(미완성된) 패킷에
속해있는지 계산할 수 있습니다. 첫 번째 패킷을 처리한 후에는 그것을 작업 버퍼에서
제거하고 부분적인 두 번째 패킷을 버퍼의 앞쪽으로 옮겨서 다음 `recv()`를
처리할 준비를 할 수 있습니다.

(독자 여러분 중 일부는 부분적인 두 번째 패킷을 작업 버퍼의 앞쪽으로 옮기는 것에
시간이 걸리고, 환형 버퍼를 사용하면 그 작업이 필요하지 않다는 것에 주목할 것입니다.
다른 독자들에게는 불행하게도, 환형 버퍼에 대한 논의는 이 글의 범위를 벗어난다.
흥미가 있다면 데이터 구조 책을 집어들고 거기서부터 시작할 수 있을 것입니다.)

쉽다고 한 적은 없다. 사실은, 쉽다고 했다. 또 실제로도 그렇다. 단지 연습이 필요하고
오래지않아 익숙해질 것입니다. [i[Excalibur]] 엑스칼리버에 맹세한다!

## 브로드캐스트(Broadcast) 패킷 --- Hello, World!

지금까지 이 안내서에서는 데이터를 하나의 호스트에서 다른 호스트로 보내는 일에
대해서 이야기했다. 그러나 적절한 권한이 있다면 _한 번에_ 여러 호스트에게
자료를 보낼 수 있다!

[i[UDP]] UDP(TCP는 안 된다)와 표준 IPv4에서 이것은 [i[Broadcast]]
_브로드캐스팅(Broadcasting)_ 이라는 매커니즘으로 가능합니다. IPv6에서 브로드캐스팅은
지원되지 않으며, 더 상위의 기술인 *멀티캐스팅(Multicasting)*을 사용해야 합니다.
그러나 이번에는 그것에 대해서 다루지 않을 것입니다. 촉망받는 미래에 대해서는
그만 이야기하자. 우리는 32비트의 현재에 갇혀있습니다.

잠깐! 그러나 무작정 브로드캐스팅을 시작할 수는 없다. 네트워크에 브로드캐스트 패킷을
전송하기 전에 [i[`SO_BROADCAST` macro]] `SO_BROADCAST` 소켓 옵션을
[i[`setsockopt()` function]] 설정해야 합니다. 이것은 미사일 발사 스위치에 달아두는
플라스틱 덮개같은 것입니다. 그만큼 강력한 도구라는 뜻입니다.

아무튼 진지하게 말하자면 브로드캐스트 패킷을 쓰는 일에는 위험이 따른다. 브로드캐스트
패킷을 받는 모든 시스템은 반드시 그 데이터가 어떤 포트를 목적지로 삼는지 알아내기
위해서 패킷의 데이터 캡슐화 계층이라는 양파껍질을 벗겨내야 한다는 점이 바로 그것입니다.
그렇게 하고 난 후에야 시스템은 데이터를 포트에 건네줄지 아니면 무시할지를 결정합니다.
어떤 경우건 그것은 브로드캐스트 패킷을 받는 각각의 장치에게 큰 작업이고, 상당히 많은
장치들이 불필요한 작업을 할 수 있습니다. 게임 둠이 처음 세상에 나왔을 때 그것의 네트워크
코드에 대한 불평이 이런 것이었다.

자, 고양이 가죽을 벗기는 일에도 여러 방법이 있을 수 있으니 잠깐 기다려보자.
(역자 주 : 서양의 속담) 잠깐, 무슨 그런 속담이 다 있는가? 정말로 고양이
가죽을 벗기는 일에 여러 방법이 있는가? 그리고 브로드캐스트 패킷을 보내는
일에도 여러 방법이 있는가? 핵심을 말하자면 이것입니다. 어떻게 브로드캐스트 메시지의
목적지 주소를 지정할 수 있는가? 두 개의 일반적인 방법이 있습니다.

1. 데이터를 특정 서브넷의 브로드캐스트 주소로 보낸다. 이것은 주소의 모든 호스트
   부분 비트가 1로 설정된 서브넷 네트워크 주소입니다. 예를 들어 필자의 네트워크는
   집에서 `192.168.1.0`이고 넷마스크는 `255.255.255.0`입니다. 그러므로 주소의
   마지막 바이트가 호스트 번호이다(넷마스크에 따라 첫 세 바이트가 네트워크
   주소이기 때문이다). 그러므로 필자의 브로드캐스트 주소는 `192.168.1.255`
   입니다. 유닉스에서는 `ifconfig` 명령이 이 모든 정보를 줄 것입니다. (궁금한 분을
   위해 적자면 브로드캐스트 주소를 얻기 위한 비트단위 논리연산은 `네트워크 번호`
   OR (NOT `넷마스크`)입니다.) 여러분은 이 종류의 브로드캐스트 패킷을
   로컬 네트워크 뿐 아니라 원격 네트워크에도 보낼 수 있습니다. 그러나 이 경우
   목적지의 라우터가 패킷을 무시할 가능성이 존재합니다. (이런 패킷을 무시하지
   않으면 공격자가 브로드캐스트 통신을 과다하게 발송할 수 있습니다.)

2. 데이터를 "전역" 브로드캐스트 주소로 보낸다. 이것은 [i[`255.255.255.255`]]
   `255.255.255.255`, 통칭 [i[`INADDR_BROADCAST`
   macro]] `INADDR_BROADCAST`다. 많은 장치들은 이것을 자동으로 여러분의 네트워크
   번호와 비트단위 AND연산 해서 네트워크 브로드캐스트 주소로 변환할 것입니다.
   그러나 일부는 그렇게 하지 않을 것입니다. 그것은 장치별로 다르다. 역설적이게도
   라우터들은 이 종류의 브로드캐스트 패킷을 로컬 네트워크 너머로 전송하지 않는다.

`SO_BROADCAST` 소켓 옵션을 지정하지 않고 브로드캐스트 주소에 데이터를 보내려고 하면
어떤 일이 생길까? 오래됐지만 유용한 [`talker`와 `listener`](#datagram) 를
실행해보고 무슨 일이 생기는지 보자.

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
응용프로그램의 *유일한 차이*다. 그러나 오래된 `talker` 응용프로그램에
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
    struct sockaddr_in their_addr; // 연결자(Connector)의 주소 정보
    struct hostent *he;
    int numbytes;
    int broadcast = 1;
    //char broadcast = '1'; // 동작하지 않으면 이것을 써 보라

    if (argc != 3) {
        fprintf(stderr,"usage: broadcaster hostname message\n");
        exit(1);
    }

    if ((he=gethostbyname(argv[1])) == NULL) {  // 호스트 정보를 받아온다
        perror("gethostbyname");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // 이 호출이 브로드캐스트 패킷을 보낼 수 있게 만든다
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast,
        sizeof broadcast) == -1) {
        perror("setsockopt (SO_BROADCAST)");
        exit(1);
    }

    their_addr.sin_family = AF_INET;     // 호스트 바이트 순서
    their_addr.sin_port = htons(SERVERPORT); // 숏, 네트워크 바이트 순서
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

    if ((numbytes=sendto(sockfd, argv[2], strlen(argv[2]), 0,
             (struct sockaddr *)&their_addr, sizeof their_addr)) == -1) {
        perror("sendto");
        exit(1);
    }

    printf("sent %d bytes to %s\n", numbytes,
        inet_ntoa(their_addr.sin_addr));

    close(sockfd);

    return 0;
}
```

이 프로그램과 "평범한" UDP 클라이언트/서버의 상황에는 어떤 차이가 있을까?
아무 것도 없다! (이 경우에는 클라이언트가 브로드캐스트 패킷을 보낼 수 있다는
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
반응하지 않는다면 그것이 IPv6주소에 연결되어서 그럴 수 있습니다. `listener.c`의
`AF_INET6`를 `AF_INET`로 바꿔서 IPv4를 강제해보라.)

자, 여기까지도 조금 재미있었다. 그러나 여러분이 가진 다른 장치 중 같은
네트워크에 있는 것에서 `listener`를 실행해서 각 장치에 1개씩 실행되게 한 후에
`broadcaster`에 브로드캐스트 주소를 넣고 다시 실행해봅시다. `sendto()`를 한 번만
실행했음에도 두 개의 `listener` 모두가 패킷을 받는다! 멋지다!

만약 `listener`가 실행중인 장치의 아이피 주소를 목적지로 발송된 데이터는 받는데
브로드캐스트 주소로 보낸 데이터는 받지 못한다면 아마도 [i[Firewall]] 방화벽이
장치에 있어서 패킷을 막고 있을 것입니다. (그래요, [i[Pat]] Pat, [i[Bapper]] Bapper.
이게 제 샘플 코드가 동작하지 않을 수 있는 이유라는 것을 나보다 먼저 깨달아줘서
고마워요. 안내서에 여러분을 언급하겠다고 했지요. 바로 이 부분에 적었습니다.)

다시 말하지만 브로드캐스트 패킷을 다룰 때에는 주의하세요. LAN(역자 주 : Local Area Network)
에 있는 모든 장치들이 `recvfrom()` 수행 여부와 상관없이 패킷을 처리해야 하므로
전체 컴퓨터 네트워크에 상당한 부하를 줄 수 있습니다. 브로드캐스트 패킷은 반드시
가끔씩만, 그리고 적절한 상황에서만 쓰여야 합니다.
