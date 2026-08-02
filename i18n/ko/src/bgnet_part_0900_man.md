# 맨페이지

[i[man pages]<]

유닉스의 세상에는 많은 설명서들이 있습니다. 거기에는 여러분이 쓸 수 있는 개별
함수에 대한 짧은 절이 있습니다.

물론 `manual`은 타자로 치기에 너무 길 것입니다. 그 말인즉 저를 포함해
유닉스 세상의 누구도 그만큼을 치고 싶어하지 않습니다. 물론 저는
제가 얼마나 간결한 것을 선호하는지에 대해서 이렇게 저렇게 길게 늘여서
말할 수 있을 것입니다. 그러나 저는 제가 얼마나 완전히 놀라우리만치
사실상 총체적인 모든 상황에서 간결하기를 선호하는지에 대해서 긴 웅변을
늘어놓는 대신 짧게 말하도록 하겠습니다.

_[박수 소리]_

(박수쳐주셔서) 감사합니다. 제가 말하고자 하는 바는 이런 페이지들이
유닉스 세상에서 "맨페이지"라고 불린다는 것입니다. 그리고 독자 여러분의 읽는
재미를 위해서 저의 개인적인 축약판을 여기에 포함해두었습니다. 그 말인즉
이 함수들 대부분은 저의 사용법보다 훨씬 더 범용적이지만 저는 인터넷
소켓 프로그래밍에 연관된 부분만을 제시할 것이라는 의미입니다.

그러나 잠깐! 저의 맨페이지에서 잘못된 부분은 이것만이 아닙니다.

- 이 문서들은 불완전하고 안내서의 기본적인 부분만을 보여줍니다.
- 현실 세계에는 여기에 있는 것보다 훨씬 많은 맨페이지가 있습니다.
- 여러분의 시스템에 있는 것과 다를 수 있습니다.
- 여러분의 시스템에 있는 특정 함수에 대해서는 헤더 파일이 다를 수 있습니다.
- 여러분의 시스템에 있는 특정 함수에 대해서는 함수 매개변수가 다를 수 있습니다.

진짜 정보를 원한다면 `man whatever`를 입력해서 여러분의 로컬 유닉스 맨페이지를
참고하세요. "whatever"는 예를 들자면 "`accept`"처럼 여러분이 큰 관심을 갖는 사안입니다.
(마이크로소프트 비주얼 스튜디오도 그들의 도움말 부분에 이것과 유사한 것을 가지고
있다고 생각합니다. 그러나 "man"이 "help"보다 1바이트 더 간결하므로 더 좋습니다.
이번에도 유닉스가 이겼습니다!)

그래서 이 정보에 흠이 있다면 애초에 안내서에 첨부한 이유는 무엇이냐고요?
말하자면 몇 가지 이유가 있습니다. 그러나 그 중 가장 좋은 이유는 바로
(a) 이 버전들은 네트워크 프로그래밍을 위해 특별히 재구성되었고 원본보다
이해하기 쉬우며 (b) 이 버전들에는 예제가 있다는 것입니다!

예제 이야기가 나왔으니 더 이야기하자면 코드의 길이가 너무 길어지는 점을 염려해서
오류 검사 코드를 모두 넣지는 않았습니다. 그러나 여러분은 여러분의 시스템 콜이
100% 성공할 것이라고 가정하지 않는 이상 오류 확인을 언제나 해야 합니다.
그리고 사실 그런 확신이 있어도 검사를 하는 것이 좋습니다!

[i[man pages]>]

[[manbreak]]

## `accept()` {#acceptman}

[i[`accept()` function]i]

리스닝 소켓에서 들어오는 연결을 받아들입니다

### 개요 {.unnumbered .unlisted}

```{.c}
#include <sys/types.h>
#include <sys/socket.h>

int accept(int s, struct sockaddr *addr, socklen_t *addrlen);
```

### 설명 {.unnumbered .unlisted}

`SOCK_STREAM` 소켓을 얻고 `listen()`으로 들어오는 연결을 받을 준비를 마쳤다면,
그 다음에는 `accept()`를 호출해서 새로 연결된 클라이언트와 이후 통신에 사용할
새 소켓 설명자를 실제로 얻습니다.

리스닝에 사용하던 기존 소켓은 여전히 남아 있으며, 이후 들어오는 연결에 대해
계속 `accept()`를 호출하는 데 쓰입니다.

| 매개변수 | 설명 |
| --------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `s`       | `listen()` 중인 소켓 설명자입니다. |
| `addr`    | 여러분에게 연결하는 쪽의 주소로 채워집니다. |
| `addrlen` | `addr` 매개변수로 반환된 구조체의 `sizeof()` 값으로 채워집니다. `addr`로 넘긴 타입이 `struct sockaddr_in`이고 그 타입이 돌아온다고 확신할 수 있다면 안전하게 무시할 수 있습니다. |

`accept()`는 보통 블록됩니다. 미리 `select()`를 사용해서 리스닝 소켓 설명자가
"읽을 준비"가 되었는지 살펴볼 수 있습니다. 그렇다면 `accept()`될 새 연결이
기다리고 있다는 뜻입니다! 야호! 다른 방법으로는 [i[`fcntl()` function]]
`fcntl()`을 사용해서 리스닝 소켓에 [i[`O_NONBLOCK` macro]] `O_NONBLOCK`
플래그를 설정할 수도 있습니다. 그러면 이 소켓은 블록되지 않고, 대신 `errno`를
`EWOULDBLOCK`으로 설정한 채 `-1`을 반환하게 됩니다.

`accept()`가 반환하는 소켓 설명자는 원격 호스트에 연결되어 열린 상태인 진짜
소켓 설명자입니다. 사용이 끝나면 `close()`해야 합니다.

### 반환값 {.unnumbered .unlisted}

`accept()`는 새로 연결된 소켓 설명자를 반환합니다. 오류가 발생하면 `-1`을
반환하고 `errno`를 적절히 설정합니다.

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
struct sockaddr_storage their_addr;
socklen_t addr_size;
struct addrinfo hints, *res;
int sockfd, new_fd;

// 먼저 getaddrinfo()로 주소 구조체를 채웁니다:

memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;  // IPv4와 IPv6 어느 쪽이든 사용
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE;     // 내 IP를 대신 채웁니다

getaddrinfo(NULL, MYPORT, &hints, &res);

// 소켓을 만들고, 바인드하고, 리스닝합니다:

sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
bind(sockfd, res->ai_addr, res->ai_addrlen);
listen(sockfd, BACKLOG);

// 이제 들어오는 연결을 받아들입니다:

addr_size = sizeof their_addr;
new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

// new_fd 소켓 설명자로 통신할 준비가 끝났습니다!
```

### 함께 보기 {.unnumbered .unlisted}

[`socket()`](#socketman), [`getaddrinfo()`](#getaddrinfoman),
[`listen()`](#listenman), [`struct sockaddr_in`](#structsockaddrman)

[[manbreak]]

## `bind()` {#bindman}

[i[`bind()` function]i]

소켓을 IP 주소 및 포트 번호와 연결합니다

### 개요 {.unnumbered .unlisted}

```{.c}
#include <sys/types.h>
#include <sys/socket.h>

int bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen);
```

### 설명 {.unnumbered .unlisted}

원격 호스트가 여러분의 서버 프로그램에 연결하려면 두 가지 정보가 필요합니다.
IP 주소와 포트 번호입니다. `bind()` 호출은 바로 그 일을 할 수 있게 해 줍니다.

먼저 `getaddrinfo()`를 호출해서 목적지 주소와 포트 정보가 담긴 `struct sockaddr`를
불러옵니다. 그런 다음 `socket()`을 호출해서 소켓 설명자를 얻고, 그 소켓과 주소를
`bind()`에 넘깁니다. 그러면 IP 주소와 포트가 마법처럼(진짜 마법으로) 소켓에
묶입니다!

여러분의 IP 주소를 모르거나, 시스템에 IP 주소가 하나뿐임을 알고 있거나, 그 시스템의
어떤 IP 주소가 쓰이든 신경 쓰지 않는다면 `getaddrinfo()`의 `hints` 매개변수에
`AI_PASSIVE` 플래그를 넘기면 됩니다. 이 플래그는 `struct sockaddr`의 IP 주소
부분에 특별한 값을 넣는데, 그 값은 `bind()`에게 이 호스트의 IP 주소를 자동으로
채우라고 알려줍니다.

뭐라고요? 현재 호스트 주소를 자동으로 채우게 만들려면 `struct sockaddr`의 IP 주소에
어떤 특별한 값을 넣어야 하느냐고요? 알려드리겠습니다. 하지만 이것은 여러분이
`struct sockaddr`를 손으로 직접 채우는 경우에만 해당합니다. 그렇지 않다면 위에서
말한 대로 `getaddrinfo()`의 결과를 쓰세요. IPv4에서는 `struct sockaddr_in` 구조체의
`sin_addr.s_addr` 필드를 `INADDR_ANY`로 설정합니다. IPv6에서는 `struct
sockaddr_in6` 구조체의 `sin6_addr` 필드에 전역 변수 `in6addr_any`를 대입합니다.
또는 새 `struct in6_addr`를 선언하는 중이라면 `IN6ADDR_ANY_INIT`으로 초기화할 수
있습니다.

마지막으로 `addrlen` 매개변수는 `sizeof my_addr`로 설정되어야 합니다.

### 반환값 {.unnumbered .unlisted}

성공하면 0을 반환합니다. 오류가 발생하면 `-1`을 반환하고 `errno`를 적절히
설정합니다.

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
// getaddrinfo()를 사용하는 현대적인 방식

struct addrinfo hints, *res;
int sockfd;

// 먼저 getaddrinfo()로 주소 구조체를 채웁니다:

memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;  // IPv4와 IPv6 어느 쪽이든 사용
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE;     // 내 IP를 대신 채웁니다

getaddrinfo(NULL, "3490", &hints, &res);

// 소켓을 만듭니다:
// (실제로는 "res" 연결 리스트를 순회하고 오류 확인도 해야 합니다!)

sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

// getaddrinfo()에 넘긴 포트에 바인드합니다:

bind(sockfd, res->ai_addr, res->ai_addrlen);
```

```{.c .numberLines}
// 구조체를 직접 채우는 예제, IPv4

struct sockaddr_in myaddr;
int s;

myaddr.sin_family = AF_INET;
myaddr.sin_port = htons(3490);

// IP 주소를 지정할 수도 있고:
inet_pton(AF_INET, "63.161.169.137", &(myaddr.sin_addr));

// 자동으로 고르게 할 수도 있습니다:
myaddr.sin_addr.s_addr = INADDR_ANY;

s = socket(PF_INET, SOCK_STREAM, 0);
bind(s, (struct sockaddr*)&myaddr, sizeof myaddr);
```

### 함께 보기 {.unnumbered .unlisted}

[`getaddrinfo()`](#getaddrinfoman), [`socket()`](#socketman), [`struct
sockaddr_in`](#structsockaddrman), [`struct in_addr`](#structsockaddrman)

[[manbreak]]

## `connect()` {#connectman}

[i[`connect()` function]i]

소켓을 서버에 연결합니다

### 개요 {.unnumbered .unlisted}

```{.c}
#include <sys/types.h>
#include <sys/socket.h>

int connect(int sockfd, const struct sockaddr *serv_addr,
            socklen_t addrlen);
```

### 설명 {.unnumbered .unlisted}

`socket()` 호출로 소켓 설명자를 만들었다면, 이름도 딱 맞는 `connect()` 시스템
호출을 사용해서 그 소켓을 원격 서버에 연결할 수 있습니다. 여러분이 해야 할 일은
소켓 설명자와, 좀 더 친해지고 싶은 서버의 주소를 넘기는 것뿐입니다. (아, 그리고
이런 함수에 흔히 넘기는 주소의 길이도 필요합니다.)

보통 이 정보는 `getaddrinfo()` 호출의 결과로 얻습니다. 하지만 원한다면 직접
`struct sockaddr`를 채워도 됩니다.

아직 그 소켓 설명자에 대해 `bind()`를 호출하지 않았다면, 그 소켓은 여러분의 IP 주소와
무작위 로컬 포트에 자동으로 바인드됩니다. 서버가 아니라면 보통 이것으로 충분합니다.
여러분은 로컬 포트가 무엇인지에는 별 관심이 없고, `serv_addr` 매개변수에 넣어야 할
원격 포트가 무엇인지만 신경 쓰기 때문입니다. 클라이언트 소켓이 특정 IP 주소와 포트에
있어야 한다면 `bind()`를 호출할 _수는_ 있지만, 그런 일은 꽤 드뭅니다.

소켓이 `connect()`되면, 마음껏 그 소켓에서 데이터를 `send()`하고 `recv()`할 수
있습니다.

[i[`connect()`-->on datagram sockets]] 특별 참고: `SOCK_DGRAM` UDP 소켓을
원격 호스트에 `connect()`하면 `sendto()`와 `recvfrom()`뿐 아니라 `send()`와
`recv()`도 사용할 수 있습니다. 원한다면요.

### 반환값 {.unnumbered .unlisted}

성공하면 0을 반환합니다. 오류가 발생하면 `-1`을 반환하고 `errno`를 적절히
설정합니다.

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
// www.example.com의 80번 포트(http)에 연결합니다

struct addrinfo hints, *res;
int sockfd;

// 먼저 getaddrinfo()로 주소 구조체를 채웁니다:

memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;  // IPv4와 IPv6 어느 쪽이든 사용
hints.ai_socktype = SOCK_STREAM;

// 다음 줄에서 "http" 대신 "80"을 넣을 수도 있습니다:
getaddrinfo("www.example.com", "http", &hints, &res);

// 소켓을 만듭니다:

sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

// getaddrinfo()에 넘겼던 주소와 포트에 연결합니다:

connect(sockfd, res->ai_addr, res->ai_addrlen);
```

### 함께 보기 {.unnumbered .unlisted}

[`socket()`](#socketman), [`bind()`](#bindman)

[[manbreak]]

## `close()` {#closeman}

[i[`close()` function]i]

소켓 설명자를 닫습니다

### 개요 {.unnumbered .unlisted}

```{.c}
#include <unistd.h>

int close(int s);
```

### 설명 {.unnumbered .unlisted}

여러분이 꾸며낸 어떤 미친 계획에든 소켓을 다 사용했고, 더 이상 그 소켓으로
`send()`나 `recv()`를 하고 싶지 않거나, 사실 _그 어떤 일도_ 하고 싶지 않다면
`close()`할 수 있습니다. 그러면 그 소켓은 해제되어 다시는 쓰이지 않습니다.

원격 쪽은 이런 일이 일어났는지 두 가지 방법 중 하나로 알 수 있습니다. 첫째:
원격 쪽이 `recv()`를 호출하면 `0`이 반환됩니다. 둘째: 원격 쪽이 `send()`를
호출하면 [i[`SIGPIPE` macro]] `SIGPIPE` 신호를 받고, `send()`는 `-1`을 반환하며
`errno`는 [i[`EPIPE` macro]] `EPIPE`로 설정됩니다.

[i[Windows]] **Windows 사용자**: 여러분이 사용해야 하는 함수는 `close()`가 아니라
[i[`closesocket()` function]i] `closesocket()`입니다. 소켓 설명자에 `close()`를
쓰려고 하면 Windows가 화를 낼 수도 있습니다... 그리고 화난 Windows는 별로
마음에 들지 않을 것입니다.

### 반환값 {.unnumbered .unlisted}

성공하면 0을 반환합니다. 오류가 발생하면 `-1`을 반환하고 `errno`를 적절히
설정합니다.

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
s = socket(PF_INET, SOCK_DGRAM, 0);
.
.
.
// 아주 많은 일들...*부우우우웅!*
.
.
.
close(s);  // 정말로 별것 없습니다.
```

### 함께 보기 {.unnumbered .unlisted}

[`socket()`](#socketman), [`shutdown()`](#shutdownman)

[[manbreak]]

## `getaddrinfo()`, `freeaddrinfo()`, `gai_strerror()` {#getaddrinfoman}

[i[`getaddrinfo()` function]i]
[i[`freeaddrinfo()` function]i]
[i[`gai_strerror()` function]i]

호스트 이름 및/또는 서비스에 대한 정보를 얻고 그 결과로 `struct sockaddr`를 채웁니다.

### 개요 {.unnumbered .unlisted}

```{.c}
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int getaddrinfo(const char *nodename, const char *servname,
                const struct addrinfo *hints, struct addrinfo **res);

void freeaddrinfo(struct addrinfo *ai);

const char *gai_strerror(int ecode);

struct addrinfo {
  int     ai_flags;          // AI_PASSIVE, AI_CANONNAME, ...
  int     ai_family;         // AF_xxx
  int     ai_socktype;       // SOCK_xxx
  int     ai_protocol;       // 0(자동) 또는 IPPROTO_TCP, IPPROTO_UDP

  socklen_t  ai_addrlen;     // ai_addr의 길이
  char   *ai_canonname;      // nodename의 정규 이름
  struct sockaddr  *ai_addr; // 이진 주소
  struct addrinfo  *ai_next; // 연결 리스트의 다음 구조체
};
```

### 설명 {.unnumbered .unlisted}

`getaddrinfo()`는 특정 호스트 이름에 대한 정보(예를 들면 IP 주소)를 반환하고,
그 지저분한 세부사항(IPv4인지 IPv6인지 같은 것)을 알아서 처리해 `struct sockaddr`를
채워 주는 훌륭한 함수입니다. 이 함수는 오래된 `gethostbyname()`과
`getservbyname()`을 대체합니다. 아래 설명에는 조금 겁먹을 만큼 많은 정보가 있지만,
실제 사용법은 꽤 단순합니다. 예제를 먼저 살펴보는 것도 좋습니다.

관심 있는 호스트 이름은 `nodename` 매개변수에 들어갑니다. 주소는
"www.example.com" 같은 호스트 이름일 수도 있고, 문자열로 넘긴 IPv4 또는 IPv6 주소일
수도 있습니다. `AI_PASSIVE` 플래그를 사용한다면(아래를 보세요) 이 매개변수는
`NULL`일 수도 있습니다.

`servname` 매개변수는 기본적으로 포트 번호입니다. 포트 번호("80"처럼 문자열로
넘깁니다)일 수도 있고, "http", "tftp", "smtp", "pop" 같은 서비스 이름일 수도
있습니다. 잘 알려진 서비스 이름은 [fl[IANA 포트
목록|https://www.iana.org/assignments/port-numbers]]이나 여러분의 `/etc/services`
파일에서 찾을 수 있습니다.

마지막 입력 매개변수로는 `hints`가 있습니다. 사실상 여기에서 `getaddrinfo()` 함수가
무엇을 할지 정합니다. 사용하기 전에 `memset()`으로 구조체 전체를 0으로 비우세요.
사용 전에 설정해야 하는 필드들을 살펴봅시다.

`ai_flags`에는 여러 가지 값을 설정할 수 있지만, 여기 중요한 몇 가지가 있습니다.
(여러 플래그는 `|` 연산자로 비트 OR해서 지정할 수 있습니다.) 전체 플래그 목록은
여러분의 맨페이지를 확인하세요.

`AI_CANONNAME`은 결과의 `ai_canonname`을 호스트의 정규(진짜) 이름으로 채우게
합니다. `AI_PASSIVE`는 결과의 IP 주소를 `INADDR_ANY`(IPv4) 또는
`in6addr_any`(IPv6)로 채우게 합니다. 그러면 나중에 `bind()`를 호출할 때
`struct sockaddr`의 IP 주소가 현재 호스트의 주소로 자동 채워집니다. 주소를
하드코딩하고 싶지 않은 서버 설정에 아주 좋습니다.

`AI_PASSIVE` 플래그를 사용한다면 `nodename`에 `NULL`을 넘길 수 있습니다.
(`bind()`가 나중에 여러분 대신 채울 것이기 때문입니다.)

입력 매개변수를 계속 보자면, `ai_family`는 아마 `AF_UNSPEC`으로 설정하고 싶을
것입니다. 이것은 `getaddrinfo()`에게 IPv4와 IPv6 주소를 모두 찾아보라고 말합니다.
`AF_INET`이나 `AF_INET6`으로 둘 중 하나만 쓰게 제한할 수도 있습니다.

다음으로 `socktype` 필드는 원하는 소켓의 종류에 따라 `SOCK_STREAM` 또는
`SOCK_DGRAM`으로 설정해야 합니다.

마지막으로 프로토콜 종류를 자동 선택하게 하려면 `ai_protocol`은 그냥 `0`으로
두세요.

이제 그 모든 내용을 채우고 나면 _마침내_ `getaddrinfo()`를 호출할 수 있습니다!

물론 재미는 여기서 시작됩니다. 이제 `res`는 `struct addrinfo`들의 연결 리스트를
가리키고, 여러분은 이 목록을 순회하면서 `hints`에 넘긴 조건과 맞는 모든 주소를
얻을 수 있습니다.

어떤 이유로든 동작하지 않는 주소가 일부 나올 수 있으므로, Linux 맨페이지의 방식은
목록을 순회하면서 성공할 때까지 `socket()`과 `connect()`를 호출하는 것입니다.
(`AI_PASSIVE` 플래그로 서버를 설정하는 중이라면 `connect()` 대신 `bind()`를
호출합니다.)

마지막으로 연결 리스트를 다 썼다면 `freeaddrinfo()`를 호출해서 메모리를 해제해야
합니다. (그러지 않으면 메모리가 새고, 어떤 사람들은 화를 낼 것입니다.)

### 반환값 {.unnumbered .unlisted}

성공하면 0을 반환하고 오류가 발생하면 0이 아닌 값을 반환합니다. 0이 아닌 값이
반환되면 `gai_strerror()` 함수를 사용해서 반환값 안의 오류 코드를 출력 가능한
형태로 얻을 수 있습니다.

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
// 서버에 연결하는 클라이언트 코드
// 구체적으로는 www.example.com의 80번 포트(http)에 대한 스트림 소켓
// IPv4와 IPv6 어느 쪽이든

int sockfd;
struct addrinfo hints, *servinfo, *p;
int rv;

memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC; // IPv6만 강제하려면 AF_INET6 사용
hints.ai_socktype = SOCK_STREAM;

rv = getaddrinfo("www.example.com", "http", &hints, &servinfo);
if (rv != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    exit(1);
}

// 모든 결과를 순회하면서 연결 가능한 첫 번째 것에 연결합니다
for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
            p->ai_protocol)) == -1) {
        perror("socket");
        continue;
    }

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        perror("connect");
        close(sockfd);
        continue;
    }

    break; // 여기까지 왔다면 연결에 성공한 것입니다
}

if (p == NULL) {
    // 연결하지 못한 채 목록 끝까지 순회했습니다
    fprintf(stderr, "failed to connect\n");
    exit(2);
}

freeaddrinfo(servinfo); // 이 구조체는 이제 필요 없습니다
```

```{.c .numberLines}
// 연결을 기다리는 서버 코드
// 구체적으로는 이 호스트의 IP에 있는 3490번 포트의 스트림 소켓
// IPv4와 IPv6 어느 쪽이든.

int sockfd;
struct addrinfo hints, *servinfo, *p;
int rv;

memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC; // IPv6만 강제하려면 AF_INET6 사용
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE; // 내 IP 주소를 사용

if ((rv = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    exit(1);
}

// 모든 결과를 순회하면서 바인드 가능한 첫 번째 것에 바인드합니다
for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
            p->ai_protocol)) == -1) {
        perror("socket");
        continue;
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        close(sockfd);
        perror("bind");
        continue;
    }

    break; // 여기까지 왔다면 바인드에 성공한 것입니다
}

if (p == NULL) {
    // 성공적으로 바인드하지 못한 채 목록 끝까지 순회했습니다
    fprintf(stderr, "failed to bind socket\n");
    exit(2);
}

freeaddrinfo(servinfo); // 이 구조체는 이제 필요 없습니다
```

### 함께 보기 {.unnumbered .unlisted}

[`gethostbyname()`](#gethostbynameman), [`getnameinfo()`](#getnameinfoman)

[[manbreak]]

## `gethostname()` {#gethostnameman}

[i[`gethostname()` function]i]

시스템의 이름을 반환합니다

### 개요 {.unnumbered .unlisted}

```{.c}
#include <sys/unistd.h>

int gethostname(char *name, size_t len);
```

### 설명 {.unnumbered .unlisted}

여러분의 시스템에는 이름이 있습니다. 모두 그렇습니다. 이것은 지금까지 이야기한
네트워크다운 것들보다는 약간 더 유닉스다운 일이지만, 그래도 쓸모가 있습니다.

예를 들어 호스트 이름을 얻은 다음 [i[`gethostbyname()` function]] `gethostbyname()`을
호출해서 IP 주소를 알아낼 수 있습니다.

`name` 매개변수는 호스트 이름을 담을 버퍼를 가리켜야 하고, `len`은 그 버퍼의
바이트 단위 크기입니다. `gethostname()`은 버퍼 끝을 넘어 덮어쓰지 않습니다.
(오류를 반환하거나, 그냥 쓰기를 멈출 수 있습니다.) 그리고 버퍼 안에 공간이 있다면
문자열을 `NUL`로 끝냅니다.

### 반환값 {.unnumbered .unlisted}

성공하면 0을 반환합니다. 오류가 발생하면 `-1`을 반환하고 `errno`를 적절히
설정합니다.

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
char hostname[128];

gethostname(hostname, sizeof hostname);
printf("My hostname: %s\n", hostname);
```

### 함께 보기 {.unnumbered .unlisted}

[`gethostbyname()`](#gethostbynameman)

[[manbreak]]

## `gethostbyname()`, `gethostbyaddr()` {#gethostbynameman}

[i[`gethostbyname()` function]i]
[i[`gethostbyaddr()` function]i]

호스트 이름에 대한 IP 주소를 얻거나, 그 반대 작업을 합니다

### 개요 {.unnumbered .unlisted}

```{.c}
#include <sys/socket.h>
#include <netdb.h>

struct hostent *gethostbyname(const char *name); // 더 이상 권장되지 않습니다!
struct hostent *gethostbyaddr(const char *addr, int len, int type);
```

### 설명 {.unnumbered .unlisted}

_주의하세요: 이 두 함수는 `getaddrinfo()`와 `getnameinfo()`로 대체되었습니다!_
특히 `gethostbyname()`은 IPv6과 잘 동작하지 않습니다.

이 함수들은 호스트 이름과 IP 주소를 서로 변환합니다. 예를 들어 "www.example.com"이
있다면 `gethostbyname()`을 사용해서 그 IP 주소를 얻고 `struct in_addr`에 저장할
수 있습니다.

반대로 `struct in_addr`나 `struct in6_addr`가 있다면 `gethostbyaddr()`를 사용해서
호스트 이름을 되찾을 수 있습니다. `gethostbyaddr()`는 IPv6과 호환되기는 _하지만_,
대신 더 새롭고 반짝이는 `getnameinfo()`를 쓰는 편이 좋습니다.

(점과 숫자 형식의 IP 주소 문자열이 있고 그 호스트 이름을 찾고 싶다면,
`AI_CANONNAME` 플래그와 함께 `getaddrinfo()`를 쓰는 편이 낫습니다.)

`gethostbyname()`은 "www.yahoo.com" 같은 문자열을 받아 IP 주소를 포함한 많은
정보가 들어 있는 `struct hostent`를 반환합니다. (다른 정보로는 공식 호스트 이름,
별칭 목록, 주소 타입, 주소 길이, 주소 목록이 있습니다. 사용법을 보고 나면 우리의
구체적인 목적에는 꽤 쉽게 쓸 수 있는 범용 구조체입니다.)

`gethostbyaddr()`는 `struct in_addr`나 `struct in6_addr`를 받아 그에 대응하는
호스트 이름이 있다면 가져옵니다. 그러니까 일종의 `gethostbyname()` 반대입니다.
매개변수를 보자면, `addr`은 `char*`이지만 실제로는 `struct in_addr`에 대한 포인터를
넘기고 싶을 것입니다. `len`은 `sizeof(struct in_addr)`이어야 하고, `type`은
`AF_INET`이어야 합니다.

그러면 반환되는 [i[`struct hostent` type]i] `struct hostent`는 무엇일까요?
여기에는 해당 호스트에 대한 정보를 담은 여러 필드가 있습니다.

| 필드 | 설명 |
| -------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `char *h_name`       | 실제 정규 호스트 이름입니다. |
| `char **h_aliases`   | 배열처럼 접근할 수 있는 별칭 목록입니다. 마지막 요소는 `NULL`입니다. |
| `int h_addrtype`     | 결과의 주소 타입입니다. 우리의 목적에서는 사실상 `AF_INET`이어야 합니다. |
| `int length`         | 주소의 바이트 단위 길이입니다. IP 버전 4 주소에서는 4입니다. |
| `char **h_addr_list` | 이 호스트의 IP 주소 목록입니다. `char**`이지만 실제로는 `struct in_addr*` 배열이 변장한 것입니다. 마지막 배열 요소는 `NULL`입니다. |
| `h_addr`             | 흔히 정의되어 있는 `h_addr_list[0]`의 별칭입니다. 이 호스트의 아무 IP 주소나 원한다면(네, 둘 이상 있을 수 있습니다) 이 필드를 쓰면 됩니다. |

### 반환값 {.unnumbered .unlisted}

성공하면 결과 `struct hostent`에 대한 포인터를 반환하고, 오류가 발생하면 `NULL`을
반환합니다.

오류 보고에 보통 쓰는 `perror()` 같은 것들 대신, 이 함수들은 `h_errno` 변수에
별도의 결과를 둡니다. 이것은 [i[`herror()` function]i] `herror()`나
[i[`hstrerror()` function]i] `hstrerror()` 함수를 사용해서 출력할 수 있습니다.
이 함수들은 여러분에게 익숙한 고전적인 `errno`, `perror()`, `strerror()` 함수와
비슷하게 동작합니다.

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
// 이것은 호스트 이름을 얻는 구식 방법입니다
// 대신 getaddrinfo()를 쓰세요!

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

    if (argc != 2) {
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
```

```{.c .numberLines}
// 이것은 대체되었습니다
// 대신 getnameinfo()를 쓰세요!

struct hostent *he;
struct in_addr ipv4addr;
struct in6_addr ipv6addr;

inet_pton(AF_INET, "192.0.2.34", &ipv4addr);
he = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
printf("Host name: %s\n", he->h_name);

inet_pton(AF_INET6, "2001:db8:63b3:1::beef", &ipv6addr);
he = gethostbyaddr(&ipv6addr, sizeof ipv6addr, AF_INET6);
printf("Host name: %s\n", he->h_name);
```

### 함께 보기 {.unnumbered .unlisted}

[`getaddrinfo()`](#getaddrinfoman), [`getnameinfo()`](#getnameinfoman),
[`gethostname()`](#gethostnameman), [`errno`](#errnoman),
[`perror()`](#perrorman), [`strerror()`](#perrorman), [`struct
in_addr`](#structsockaddrman)

[[manbreak]]

## `getnameinfo()` {#getnameinfoman}

[i[`getnameinfo()` function]i]

주어진 `struct sockaddr`에 대한 호스트 이름과 서비스 이름 정보를 찾습니다.

### 개요 {.unnumbered .unlisted}

```{.c}
#include <sys/socket.h>
#include <netdb.h>

int getnameinfo(const struct sockaddr *sa, socklen_t salen,
                char *host, size_t hostlen,
                char *serv, size_t servlen, int flags);
```

### 설명 {.unnumbered .unlisted}

이 함수는 `getaddrinfo()`의 반대입니다. 다시 말해 이미 채워진 `struct sockaddr`를
받아 그에 대해 이름과 서비스 이름 조회를 수행합니다. 이 함수는 오래된
`gethostbyaddr()`와 `getservbyport()` 함수를 대체합니다.

`sa` 매개변수에는 `struct sockaddr`에 대한 포인터를 넘겨야 합니다. 실제로는 아마
형변환한 `struct sockaddr_in`이나 `struct sockaddr_in6`일 것입니다. 그리고 `salen`에는
그 `struct`의 길이를 넘깁니다.

결과 호스트 이름과 서비스 이름은 `host`와 `serv` 매개변수가 가리키는 영역에
쓰입니다. 물론 `hostlen`과 `servlen`으로 이 버퍼들의 최대 길이를 지정해야 합니다.

마지막으로 넘길 수 있는 플래그가 몇 가지 있는데, 여기 괜찮은 것 몇 개가 있습니다.
`NI_NOFQDN`은 `host`에 전체 도메인 이름이 아니라 호스트 이름만 들어가게 합니다.
`NI_NAMEREQD`는 DNS 조회로 이름을 찾을 수 없을 때 함수가 실패하게 만듭니다.
(이 플래그를 지정하지 않았고 이름을 찾을 수 없다면 `getnameinfo()`는 대신 IP 주소의
문자열 버전을 `host`에 넣습니다.)

늘 그렇듯 전체 이야기는 여러분의 로컬 맨페이지를 확인하세요.

### 반환값 {.unnumbered .unlisted}

성공하면 0을 반환하고 오류가 발생하면 0이 아닌 값을 반환합니다. 반환값이 0이
아니라면 `gai_strerror()`에 넘겨 사람이 읽을 수 있는 문자열을 얻을 수 있습니다.
더 자세한 내용은 `getaddrinfo()`를 보세요.

[[book-pagebreak]]

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
struct sockaddr_in6 sa; // 원한다면 IPv4일 수도 있습니다
char host[1024];
char service[20];

// sa에 호스트와 포트에 대한 좋은 정보가 가득하다고 가정합니다...

getnameinfo(&sa, sizeof sa, host, sizeof host, service,
            sizeof service, 0);

printf("   host: %s\n", host);    // 예: "www.example.com"
printf("service: %s\n", service); // 예: "http"
```

### 함께 보기 {.unnumbered .unlisted}

[`getaddrinfo()`](#getaddrinfoman), [`gethostbyaddr()`](#gethostbynameman)

[[manbreak]]

## `getpeername()` {#getpeernameman}

[i[`getpeername()` function]i]

연결의 원격 쪽에 대한 주소 정보를 반환합니다

### 개요 {.unnumbered .unlisted}

```{.c}
#include <sys/socket.h>

int getpeername(int s, struct sockaddr *addr, socklen_t *len);
```

### 설명 {.unnumbered .unlisted}

원격 연결을 `accept()`했거나 서버에 `connect()`했다면, 여러분에게는 이제 _피어_라고
부르는 것이 생깁니다. 피어는 간단히 말해 여러분이 연결된 컴퓨터이며, IP 주소와 포트로
식별됩니다. 그래서...

`getpeername()`은 여러분이 연결된 호스트의 정보로 채워진 `struct sockaddr_in`을
그냥 반환합니다.

왜 "name"이라고 부를까요? 음, 이 안내서에서 사용하는 인터넷 소켓뿐 아니라 여러
종류의 소켓이 있기 때문에, "name"은 모든 경우를 포괄하는 괜찮은 일반 용어였습니다.
하지만 우리의 경우 피어의 "name"은 IP 주소와 포트입니다.

이 함수는 결과 주소의 크기를 `len`에 반환하지만, 호출 전에는 `len`에 `addr`의 크기를
미리 넣어 두어야 합니다.

### 반환값 {.unnumbered .unlisted}

성공하면 0을 반환합니다. 오류가 발생하면 `-1`을 반환하고 `errno`를 적절히
설정합니다.

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
// s가 연결된 소켓이라고 가정합니다

socklen_t len;
struct sockaddr_storage addr;
char ipstr[INET6_ADDRSTRLEN];
int port;

len = sizeof addr;
getpeername(s, (struct sockaddr*)&addr, &len);

// IPv4와 IPv6을 모두 처리합니다:
if (addr.ss_family == AF_INET) {
    struct sockaddr_in *s = (struct sockaddr_in *)&addr;
    port = ntohs(s->sin_port);
    inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
} else { // AF_INET6
    struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
    port = ntohs(s->sin6_port);
    inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
}

printf("Peer IP address: %s\n", ipstr);
printf("Peer port      : %d\n", port);
```

### 함께 보기 {.unnumbered .unlisted}

[`gethostname()`](#gethostnameman), [`gethostbyname()`](#gethostbynameman),
[`gethostbyaddr()`](#gethostbynameman)

[[manbreak]]

## `errno` {#errnoman}

[i[`errno` variable]i]

마지막 시스템 호출의 오류 코드를 담습니다

### 개요 {.unnumbered .unlisted}

```{.c}
#include <errno.h>

int errno;
```

### 설명 {.unnumbered .unlisted}

이 변수는 많은 시스템 호출의 오류 정보를 담습니다. 기억하시겠지만 `socket()`이나
`listen()` 같은 함수는 오류가 발생하면 `-1`을 반환하고, 정확히 어떤 오류가
일어났는지 알려주기 위해 `errno` 값을 설정합니다.

헤더 파일 `errno.h`에는 `EADDRINUSE`, `EPIPE`, `ECONNREFUSED` 같은 오류에 대한
상수 심볼 이름이 많이 나열되어 있습니다. 여러분의 로컬 맨페이지는 어떤 코드가
오류로 반환될 수 있는지 알려줄 것이고, 여러분은 런타임에 이 값들을 사용해서 서로
다른 오류를 서로 다른 방식으로 처리할 수 있습니다.

또는 더 흔하게는 [i[`perror()` function]] `perror()`나 [i[`strerror()` function]]
`strerror()`를 호출해서 사람이 읽을 수 있는 오류 표현을 얻을 수 있습니다.

멀티스레딩 애호가들을 위해 한 가지 적자면, 대부분의 시스템에서 `errno`는 스레드
안전한 방식으로 정의됩니다. (다시 말해 실제로는 전역 변수가 아니지만, 단일 스레드
환경에서는 전역 변수처럼 동작합니다.)

### 반환값 {.unnumbered .unlisted}

이 변수의 값은 가장 최근에 일어난 오류입니다. 마지막 동작이 성공했다면 "성공"을
나타내는 코드일 수도 있습니다.

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
s = socket(PF_INET, SOCK_STREAM, 0);
if (s == -1) {
    perror("socket"); // 또는 strerror()를 사용
}

tryagain:
if (select(n, &readfds, NULL, NULL) == -1) {
    // 오류가 발생했습니다!!

    // 단순히 인터럽트된 것뿐이라면 select() 호출을 다시 시작합니다:
    if (errno == EINTR) goto tryagain;  // 으아아! goto!!!

    // 그렇지 않다면 더 심각한 오류입니다:
    perror("select");
    exit(1);
}
```

### 함께 보기 {.unnumbered .unlisted}

[`perror()`](#perrorman), [`strerror()`](#perrorman)

[[manbreak]]

## `fcntl()` {#fcntlman}

[i[`fcntl()` function]i]

소켓 설명자를 제어합니다

### 개요 {.unnumbered .unlisted}

```{.c}
#include <sys/unistd.h>
#include <sys/fcntl.h>

int fcntl(int s, int cmd, long arg);
```

### 설명 {.unnumbered .unlisted}

이 함수는 보통 파일 잠금이나 다른 파일 중심 작업에 사용되지만, 가끔 보거나 사용하게 될
소켓 관련 기능도 몇 가지 가지고 있습니다.

`s` 매개변수는 작업하려는 소켓 설명자입니다. `cmd`는 [i[`F_SETFL` macro]i]
`F_SETFL`로 설정해야 하고, `arg`에는 아래 명령 중 하나가 올 수 있습니다. (말했듯이
`fcntl()`에는 제가 여기에서 드러낸 것보다 더 많은 기능이 있지만, 저는 소켓 중심으로
이야기하려고 합니다.)

| `cmd`                                | 설명 |
| ------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| [i[`O_NONBLOCK` macro]i]`O_NONBLOCK` | 소켓을 논블로킹으로 설정합니다. 더 자세한 내용은 [블로킹](#blocking) 절을 보세요. |
| [i[`O_ASYNC` macro]i]`O_ASYNC`       | 소켓이 비동기 I/O를 하도록 설정합니다. 소켓에서 `recv()`할 데이터가 준비되면 [i[`SIGIO` signal]] `SIGIO` 신호가 발생합니다. 보기 드문 방식이고 이 안내서의 범위를 벗어납니다. 또한 특정 시스템에서만 사용할 수 있는 것으로 생각합니다. |

### 반환값 {.unnumbered .unlisted}

성공하면 0을 반환합니다. 오류가 발생하면 `-1`을 반환하고 `errno`를 적절히
설정합니다.

`fcntl()` 시스템 호출은 실제로 사용법에 따라 서로 다른 반환값을 가지지만, 소켓과
관련되지 않은 내용이므로 여기에서는 다루지 않았습니다. 더 자세한 정보는 여러분의
로컬 `fcntl()` 맨페이지를 보세요.

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
int s = socket(PF_INET, SOCK_STREAM, 0);

fcntl(s, F_SETFL, O_NONBLOCK);  // 논블로킹으로 설정
fcntl(s, F_SETFL, O_ASYNC);     // 비동기 I/O로 설정
```

### 함께 보기 {.unnumbered .unlisted}

[Blocking](#blocking), [`send()`](#sendman)

[[manbreak]]

## `htons()`, `htonl()`, `ntohs()`, `ntohl()` {#htonsman}

[i[`htons()` function]i]
[i[`htonl()` function]i]
[i[`ntohs()` function]i]
[i[`ntohl()` function]i]

여러 바이트 정수 타입을 호스트 바이트 순서와 네트워크 바이트 순서 사이에서 변환합니다

### 개요 {.unnumbered .unlisted}

```{.c}
#include <netinet/in.h>

uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);
```

### 설명 {.unnumbered .unlisted}

여러분을 정말로 불행하게 만들기 위해, 서로 다른 컴퓨터들은 여러 바이트 정수(즉,
`char`보다 큰 정수)를 내부적으로 서로 다른 바이트 순서로 사용합니다. 그 결과
Intel 컴퓨터에서 Mac으로(그 Mac들도 Intel 컴퓨터가 되기 전을 말합니다) 2바이트
`short int`를 `send()`하면, 한 컴퓨터가 숫자 `1`이라고 생각하는 것을 다른 컴퓨터는
`256`이라고 생각할 수 있고 그 반대도 가능합니다.

[i[Byte ordering]] 이 문제를 피하는 방법은 모두가 차이를 내려놓고 Motorola와 IBM이
옳았으며 Intel이 이상한 방식으로 했다는 데 동의한 뒤, 내보내기 전에 모두 바이트
순서를 "빅엔디언"으로 변환하는 것입니다. Intel은 "리틀엔디언" 시스템이므로, 우리가
선호하는 바이트 순서를 "네트워크 바이트 순서"라고 부르는 편이 훨씬 정치적으로
올바릅니다. 그래서 이 함수들은 여러분의 네이티브 바이트 순서에서 네트워크 바이트
순서로, 그리고 그 반대로 변환합니다.

(이 말은 Intel에서는 이 함수들이 모든 바이트를 뒤집고, PowerPC에서는 바이트가 이미
네트워크 바이트 순서이므로 아무 일도 하지 않는다는 뜻입니다. 하지만 누군가가 Intel
시스템에서 빌드해도 올바르게 동작하기를 원할 수 있으므로, 여러분은 코드에서 늘 이
함수들을 사용해야 합니다.)

여기에서 다루는 타입은 32비트(4바이트, 아마 `int`)와 16비트(2바이트, 거의 확실히
`short`) 숫자라는 점에 주목하세요.

여러 시스템에는 64비트 변종이 있습니다. 사용할 수 있다면 `<endian.h>` 안의
[flm[`htobe64()`|htobe64]] 함수와 그 친척들을 확인해 보세요. (MacOS에는 없는 것
같습니다.) 그리고 GCC에는 128비트까지 다루는 [fl[바이트 스와핑
내장 함수|https://gcc.gnu.org/onlinedocs/gcc/Byte-Swapping-Builtins.html]]도
있습니다. [flx[아니면 직접 만들 수도 있습니다|htonll.c]]. 단, 실제 스왑은
리틀엔디언 시스템에서만 하세요!

어쨌든 이 함수들이 동작하는 방식은 이렇습니다. 먼저 호스트(여러분 시스템의) 바이트
순서에서 변환하는지, 아니면 네트워크 바이트 순서에서 변환하는지를 결정합니다.
"host"라면 호출할 함수의 첫 글자는 "h"입니다. 그렇지 않으면 "network"의 "n"입니다.
함수 이름의 가운데는 언제나 "to"입니다. 하나에서 다른 하나로 변환하기 때문입니다.
그리고 끝에서 두 번째 글자는 무엇으로 변환하는지를 보여줍니다. 마지막 글자는 데이터의
크기로, short는 "s", long은 "l"입니다. 그래서:

| 함수      | 설명                          |
| --------- | ----------------------------- |
| `htons()` | `h`ost `to` `n`etwork `s`hort |
| `htonl()` | `h`ost `to` `n`etwork `l`ong  |
| `ntohs()` | `n`etwork `to` `h`ost `s`hort |
| `ntohl()` | `n`etwork `to` `h`ost `l`ong  |

### 반환값 {.unnumbered .unlisted}

각 함수는 변환된 값을 반환합니다.

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
uint32_t some_long = 10;
uint16_t some_short = 20;

uint32_t network_byte_order;

// 변환하고 전송
network_byte_order = htonl(some_long);
send(s, &network_byte_order, sizeof(uint32_t), 0);

some_short == ntohs(htons(some_short)); // 이 표현식은 참입니다
```

[[manbreak]]

## `inet_ntoa()`, `inet_aton()`, `inet_addr` {#inet_ntoaman}

[i[`inet_ntoa()` function]i]
[i[`inet_aton()` function]i]
[i[`inet_addr()` function]i]

IP 주소를 점과 숫자로 된 문자열에서 `struct in_addr`로, 또는 그 반대로 변환합니다

### 개요 {.unnumbered .unlisted}

```{.c}
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// 이것들은 모두 구식입니다!
// 대신 inet_pton()이나 inet_ntop()을 쓰세요!

char *inet_ntoa(struct in_addr in);
int inet_aton(const char *cp, struct in_addr *inp);
in_addr_t inet_addr(const char *cp);
```

### 설명 {.unnumbered .unlisted}

_이 함수들은 IPv6을 처리하지 못하기 때문에 구식입니다! 대신
[`inet_ntop()`](#inet_ntopman)이나 [`inet_pton()`](#inet_ntopman)을 쓰세요!
여기 포함한 이유는 여전히 현장에서 발견될 수 있기 때문입니다._

이 함수들은 모두 `struct in_addr`(아마 여러분의 `struct sockaddr_in`의 일부일
것입니다)에서 점과 숫자 형식의 문자열(예: "192.168.5.10")로, 또는 그 반대로
변환합니다. 명령줄 같은 곳에서 IP 주소를 넘겨받았다면, `connect()` 등에 쓸
`struct in_addr`를 얻는 가장 쉬운 방법입니다. 더 강력한 기능이 필요하다면
`gethostbyname()` 같은 DNS 함수들을 시도하거나, 여러분 나라에서 _쿠데타_를 시도해
보세요.

`inet_ntoa()` 함수는 `struct in_addr` 안의 네트워크 주소를 점과 숫자 형식의 문자열로
변환합니다. "ntoa"의 "n"은 network를 뜻하고, "a"는 역사적인 이유로 ASCII를 뜻합니다.
(그러니까 "Network To ASCII"입니다. "toa" 접미사에는 C 라이브러리에 `atoi()`라는
비슷한 친구가 있는데, 이것은 ASCII 문자열을 정수로 변환합니다.)

`inet_aton()` 함수는 그 반대로, 점과 숫자 문자열을 `in_addr_t`로 변환합니다.
(`in_addr_t`는 여러분의 `struct in_addr` 안에 있는 `s_addr` 필드의 타입입니다.)

마지막으로 `inet_addr()` 함수는 기본적으로 `inet_aton()`과 같은 일을 하는 더 오래된
함수입니다. 이론적으로는 구식이지만, 많이 보게 될 것이고 사용한다고 경찰이 잡으러
오지는 않을 것입니다.

### 반환값 {.unnumbered .unlisted}

`inet_aton()`은 주소가 유효하면 0이 아닌 값을 반환하고, 주소가 유효하지 않으면 0을
반환합니다.

`inet_ntoa()`는 점과 숫자 문자열을 정적 버퍼에 담아 반환합니다. 이 버퍼는 함수를
호출할 때마다 덮어써집니다.

`inet_addr()`는 주소를 `in_addr_t`로 반환하고, 오류가 발생하면 `-1`을 반환합니다.
(그 값은 유효한 IP 주소인 [i[`255.255.255.255`]] "`255.255.255.255`" 문자열을
변환하려고 했을 때와 같은 결과입니다. 그래서 `inet_aton()`이 더 낫습니다.)

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
struct sockaddr_in antelope;
char *some_addr;

inet_aton("10.0.0.1", &antelope.sin_addr); // antelope에 IP 저장

some_addr = inet_ntoa(antelope.sin_addr); // IP 반환
printf("%s\n", some_addr); // "10.0.0.1" 출력

// 그리고 이 호출은 위의 inet_aton() 호출과 같습니다:
antelope.sin_addr.s_addr = inet_addr("10.0.0.1");
```

### 함께 보기 {.unnumbered .unlisted}

[`inet_ntop()`](#inet_ntopman), [`inet_pton()`](#inet_ntopman),
[`gethostbyname()`](#gethostbynameman), [`gethostbyaddr()`](#gethostbynameman)

[[manbreak]]

## `inet_ntop()`, `inet_pton()` {#inet_ntopman}

[i[`inet_ntop()` function]i]
[i[`inet_pton()` function]i]

IP 주소를 사람이 읽을 수 있는 형식으로, 또는 그 반대로 변환합니다.

### 개요 {.unnumbered .unlisted}

```{.c}
#include <arpa/inet.h>

const char *inet_ntop(int af, const void *src,
                      char *dst, socklen_t size);

int inet_pton(int af, const char *src, void *dst);
```

### 설명 {.unnumbered .unlisted}

이 함수들은 사람이 읽을 수 있는 IP 주소를 다루고, 그것을 여러 함수와 시스템 호출에서
쓸 수 있도록 이진 표현으로 변환합니다. "n"은 "network"를, "p"는 "presentation"을
뜻합니다. 또는 "text presentation"입니다. 하지만 "printable"이라고 생각해도 됩니다.
"ntop"은 "network to printable"입니다. 보이지요?

가끔은 IP 주소를 볼 때 이진 숫자 더미를 보고 싶지 않습니다. `192.0.2.180`이나
`2001:db8:8714:3a90::12`처럼 보기 좋게 출력 가능한 형식으로 보고 싶습니다.
그런 경우를 위한 것이 `inet_ntop()`입니다.

`inet_ntop()`은 `af` 매개변수로 주소 계열(`AF_INET` 또는 `AF_INET6`)을 받습니다.
`src` 매개변수는 문자열로 변환하려는 주소가 들어 있는 `struct in_addr` 또는
`struct in6_addr`에 대한 포인터여야 합니다. 마지막으로 `dst`와 `size`는 목적지
문자열에 대한 포인터와 그 문자열의 최대 길이입니다.

`dst` 문자열의 최대 길이는 얼마여야 할까요? IPv4와 IPv6 주소의 최대 길이는
얼마일까요? 다행히 여러분을 도와줄 매크로가 몇 개 있습니다. 최대 길이는
`INET_ADDRSTRLEN`과 `INET6_ADDRSTRLEN`입니다.

다른 경우에는 읽을 수 있는 형식의 IP 주소 문자열이 있고, 그것을 `struct sockaddr_in`
또는 `struct sockaddr_in6` 안에 넣고 싶을 수 있습니다. 그런 경우에는 반대 함수인
`inet_pton()`이 여러분이 찾는 것입니다.

`inet_pton()`도 `af` 매개변수로 주소 계열(`AF_INET` 또는 `AF_INET6`)을 받습니다.
`src` 매개변수는 출력 가능한 형식의 IP 주소가 담긴 문자열에 대한 포인터입니다.
마지막으로 `dst` 매개변수는 결과를 저장할 곳을 가리키며, 아마 `struct in_addr` 또는
`struct in6_addr`일 것입니다.

이 함수들은 DNS 조회를 하지 않습니다. 그 작업에는 `getaddrinfo()`가 필요합니다.

### 반환값 {.unnumbered .unlisted}

`inet_ntop()`은 성공하면 `dst` 매개변수를 반환합니다. 실패하면 `NULL`을 반환하고
`errno`를 설정합니다.

`inet_pton()`은 성공하면 `1`을 반환합니다. 오류가 있으면 `-1`을 반환하고(`errno`가
설정됩니다), 입력이 유효한 IP 주소가 아니면 `0`을 반환합니다.

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
// inet_ntop()과 inet_pton()의 IPv4 데모

struct sockaddr_in sa;
char str[INET_ADDRSTRLEN];

// 이 IP 주소를 sa에 저장합니다:
inet_pton(AF_INET, "192.0.2.33", &(sa.sin_addr));

// 이제 다시 꺼내 출력합니다
inet_ntop(AF_INET, &(sa.sin_addr), str, INET_ADDRSTRLEN);

printf("%s\n", str); // "192.0.2.33" 출력
```

```{.c .numberLines}
// inet_ntop()과 inet_pton()의 IPv6 데모
// (기본적으로 6이 여기저기 붙는 것 말고는 같습니다)

struct sockaddr_in6 sa;
char str[INET6_ADDRSTRLEN];

// 이 IP 주소를 sa에 저장합니다:
inet_pton(AF_INET6, "2001:db8:8714:3a90::12", &(sa.sin6_addr));

// 이제 다시 꺼내 출력합니다
inet_ntop(AF_INET6, &(sa.sin6_addr), str, INET6_ADDRSTRLEN);

printf("%s\n", str); // "2001:db8:8714:3a90::12" 출력
```

```{.c .numberLines}
// 사용할 수 있는 도우미 함수:

// struct sockaddr 주소를 문자열로 변환합니다. IPv4와 IPv6:

char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen)
{
    switch(sa->sa_family) {
        case AF_INET:
            inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
                    s, maxlen);
            break;

        case AF_INET6:
            inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
                    s, maxlen);
            break;

        default:
            strncpy(s, "Unknown AF", maxlen);
            return NULL;
    }

    return s;
}
```

### 함께 보기 {.unnumbered .unlisted}

[`getaddrinfo()`](#getaddrinfoman)

[[manbreak]]

## `listen()` {#listenman}

[i[`listen()` function]i]

소켓에게 들어오는 연결을 리스닝하라고 알려줍니다

### 개요 {.unnumbered .unlisted}

```{.c}
#include <sys/socket.h>

int listen(int s, int backlog);
```

### 설명 {.unnumbered .unlisted}

여러분은 (`socket()` 시스템 호출로 만든) 소켓 설명자에게 들어오는 연결을
리스닝하라고 말할 수 있습니다. 여러분, 이것이 서버와 클라이언트를 구분하는 것입니다.

`backlog` 매개변수는 사용 중인 시스템에 따라 몇 가지 다른 뜻을 가질 수 있지만,
대략적으로는 커널이 새 연결을 거부하기 전에 보류 중인 연결을 몇 개까지 둘 수 있는지를
의미합니다. 그러므로 새 연결이 들어오면 `backlog`가 꽉 차지 않도록 빠르게
`accept()`해야 합니다. 10 정도로 설정해 보고, 부하가 클 때 클라이언트가
"Connection refused"를 받기 시작하면 더 크게 설정하세요.

`listen()`을 호출하기 전에 서버는 `bind()`를 호출해서 자신을 특정 포트 번호에
붙여야 합니다. 그 포트 번호(서버의 IP 주소에 있는)가 클라이언트가 연결할 대상이 됩니다.

### 반환값 {.unnumbered .unlisted}

성공하면 0을 반환합니다. 오류가 발생하면 `-1`을 반환하고 `errno`를 적절히
설정합니다.

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
struct addrinfo hints, *res;
int sockfd;

// 먼저 getaddrinfo()로 주소 구조체를 채웁니다:

memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;  // IPv4와 IPv6 어느 쪽이든 사용
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE;     // 내 IP를 대신 채웁니다

getaddrinfo(NULL, "3490", &hints, &res);

// 소켓을 만듭니다:

sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

// getaddrinfo()에 넘긴 포트에 바인드합니다:

bind(sockfd, res->ai_addr, res->ai_addrlen);

listen(sockfd, 10); // sockfd를 서버 소켓으로 설정합니다

// 그런 다음 이 아래 어딘가에 accept() 루프를 둡니다
```

### 함께 보기 {.unnumbered .unlisted}

[`accept()`](#acceptman), [`bind()`](#bindman), [`socket()`](#socketman)

[[manbreak]]

## `perror()`, `strerror()` {#perrorman}

[i[`perror()` function]i]
[i[`strerror()` function]i]

오류를 사람이 읽을 수 있는 문자열로 출력합니다

### 개요 {.unnumbered .unlisted}

```{.c}
#include <stdio.h>
#include <string.h>   // strerror()용

void perror(const char *s);
char *strerror(int errnum);
```

### 설명 {.unnumbered .unlisted}

많은 함수가 오류가 발생하면 `-1`을 반환하고 [i[`errno` variable]] `errno` 변수의
값을 어떤 숫자로 설정하므로, 그것을 여러분이 이해할 수 있는 형태로 쉽게 출력할 수
있다면 분명 좋을 것입니다.

고맙게도 `perror()`가 그 일을 합니다. 오류 앞에 더 많은 설명을 출력하고 싶다면
`s` 매개변수가 그 문자열을 가리키게 하면 됩니다. (또는 `s`를 `NULL`로 두면 아무
추가 내용도 출력되지 않습니다.)

간단히 말해 이 함수는 `ECONNRESET` 같은 `errno` 값을 받아 "Connection reset by peer."
처럼 보기 좋게 출력합니다.

`strerror()` 함수는 `perror()`와 매우 비슷하지만, 주어진 값(보통 `errno` 변수를
넘깁니다)에 대한 오류 메시지 문자열의 포인터를 반환한다는 점이 다릅니다.

### 반환값 {.unnumbered .unlisted}

`strerror()`는 오류 메시지 문자열에 대한 포인터를 반환합니다.

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
int s;

s = socket(PF_INET, SOCK_STREAM, 0);

if (s == -1) { // 어떤 오류가 발생했습니다
    // "socket error: " + 오류 메시지를 출력합니다:
    perror("socket error");
}

// 비슷하게:
if (listen(s, 10) == -1) {
    // 이것은 "an error: " + errno의 오류 메시지를 출력합니다:
    printf("an error: %s\n", strerror(errno));
}
```

### 함께 보기 {.unnumbered .unlisted}

[`errno`](#errnoman)

[[manbreak]]

## `poll()` {#pollman}

[i[`poll()` function]i]

여러 소켓의 이벤트를 동시에 검사합니다

### 개요 {.unnumbered .unlisted}

```{.c}
#include <sys/poll.h>

int poll(struct pollfd *ufds, unsigned int nfds, int timeout);
```

### 설명 {.unnumbered .unlisted}

이 함수는 `select()`와 매우 비슷합니다. 둘 다 파일 설명자 집합을 감시하면서
`recv()`할 준비가 된 수신 데이터, 데이터를 `send()`할 준비가 된 소켓,
`recv()`할 준비가 된 out-of-band 데이터, 오류 같은 이벤트를 확인합니다.

기본 개념은 `ufds`에 `nfds`개의 `struct pollfd` 배열을 넘기고, 밀리초 단위의
타임아웃도 함께 넘기는 것입니다. (1초는 1000밀리초입니다.) 영원히 기다리고 싶다면
`timeout`은 음수일 수 있습니다. 타임아웃까지 어떤 소켓 설명자에서도 이벤트가
일어나지 않으면 `poll()`은 반환합니다.

`struct pollfd` 배열의 각 요소는 소켓 설명자 하나를 나타내며, 아래 필드들을
포함합니다:

[i[`struct pollfd` type]i]

```{.c}
struct pollfd {
    int fd;         // 소켓 설명자
    short events;   // 관심 있는 이벤트의 비트맵
    short revents;  // 반환 시점에 발생한 이벤트의 비트맵
};
```

`poll()`을 호출하기 전에 `fd`에 소켓 설명자를 넣으세요. (`fd`를 음수로 설정하면 이
`struct pollfd`는 무시되고 `revents` 필드는 0으로 설정됩니다.) 그런 다음 아래
매크로들을 비트 OR해서 `events` 필드를 구성합니다:

| 매크로    | 설명                                                                |
| --------- | ------------------------------------------------------------------- |
| `POLLIN`  | 이 소켓에서 `recv()`할 데이터가 준비되면 알려줍니다.                |
| `POLLOUT` | 이 소켓에 대기하지 않고 데이터를 `send()`할 수 있으면 알려줍니다.       |
| `POLLPRI` | 이 소켓에서 `recv()`할 out-of-band 데이터가 준비되면 알려줍니다.    |

`poll()` 호출이 반환되면 `revents` 필드는 위 필드들을 비트 OR한 값으로 구성되어,
어떤 설명자에서 실제로 이벤트가 발생했는지 알려줍니다. 추가로 아래 필드들이 나타날
수도 있습니다:

| 매크로     | 설명                                                                            |
| ---------- | ------------------------------------------------------------------------------- |
| `POLLERR`  | 이 소켓에서 오류가 발생했습니다.                                                |
| `POLLHUP`  | 연결의 원격 쪽이 끊었습니다.                                                    |
| `POLLNVAL` | 소켓 설명자 `fd`에 문제가 있습니다. 초기화되지 않았을지도 모릅니다.             |

### 반환값 {.unnumbered .unlisted}

이벤트가 발생한 `ufds` 배열 요소의 수를 반환합니다. 타임아웃이 발생했다면 0일 수
있습니다. 오류가 발생하면 `-1`을 반환하고 `errno`를 적절히 설정합니다.

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
int s1, s2;
int rv;
char buf1[256], buf2[256];
struct pollfd ufds[2];

s1 = socket(PF_INET, SOCK_STREAM, 0);
s2 = socket(PF_INET, SOCK_STREAM, 0);

// 이 시점에서 둘 다 서버에 연결되었다고 가정합니다
//connect(s1, ...)...
//connect(s2, ...)...

// 파일 설명자 배열을 설정합니다.
//
// 이 예제에서는 일반 데이터나 out-of-band 데이터가
// recv()로 수신될 준비가 되었는지 알고 싶습니다...

ufds[0].fd = s1;
ufds[0].events = POLLIN | POLLPRI; // 일반 데이터나 out-of-band 확인

ufds[1].fd = s2;
ufds[1].events = POLLIN; // 일반 데이터만 확인

// 소켓 이벤트를 기다립니다. 타임아웃은 3.5초
rv = poll(ufds, 2, 3500);

if (rv == -1) {
    perror("poll"); // poll()에서 오류 발생
} else if (rv == 0) {
    printf("Timeout occurred! No data after 3.5 seconds.\n");
} else {
    // s1의 이벤트 확인:
    if (ufds[0].revents & POLLIN) {
        recv(s1, buf1, sizeof buf1, 0); // 일반 데이터 수신
    }
    if (ufds[0].revents & POLLPRI) {
        recv(s1, buf1, sizeof buf1, MSG_OOB); // out-of-band 데이터
    }

    // s2의 이벤트 확인:
    if (ufds[1].revents & POLLIN) {
        recv(s1, buf2, sizeof buf2, 0);
    }
}
```

### 함께 보기 {.unnumbered .unlisted}

[`select()`](#selectman)

[[manbreak]]

## `recv()`, `recvfrom()` {#recvman}

[i[`recv()` function]i]
[i[`recvfrom()` function]i]

소켓에서 데이터를 받습니다

### 개요 {.unnumbered .unlisted}

```{.c}
#include <sys/types.h>
#include <sys/socket.h>

ssize_t recv(int s, void *buf, size_t len, int flags);
ssize_t recvfrom(int s, void *buf, size_t len, int flags,
                 struct sockaddr *from, socklen_t *fromlen);
```

### 설명 {.unnumbered .unlisted}

소켓을 만들고 연결했다면, `recv()`(TCP [i[`SOCK_STREAM` macro]] `SOCK_STREAM`
소켓용)와 `recvfrom()`(UDP [i[`SOCK_DGRAM` macro]] `SOCK_DGRAM` 소켓용)을 사용해서
원격 쪽에서 들어오는 데이터를 읽을 수 있습니다.

두 함수 모두 소켓 설명자 `s`, 버퍼에 대한 포인터 `buf`, 버퍼의 바이트 단위 크기
`len`, 함수 동작을 제어하는 `flags` 집합을 받습니다.

추가로 `recvfrom()`은 데이터가 어디에서 왔는지 알려줄 [i[`struct sockaddr` type]]
`struct sockaddr*`인 `from`을 받고, `fromlen`을 `struct sockaddr`의 크기로
채웁니다. (`fromlen`도 `from` 또는 `struct sockaddr`의 크기로 초기화해야 합니다.)

그러면 이 함수에 넘길 수 있는 놀라운 플래그에는 무엇이 있을까요? 일부는 아래와
같습니다. 더 자세한 정보와 여러분 시스템에서 실제로 지원하는 값은 로컬 맨페이지를
확인하세요. 이 값들을 비트 OR하거나, 평범한 기본 `recv()`를 원한다면 `flags`를
그냥 `0`으로 설정하면 됩니다.

| 매크로                                              | 설명 |
| --------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [i[Out-of-band data]][i[`MSG_OOB` macro]i]`MSG_OOB` | Out-of-band 데이터를 받습니다. `send()`에서 `MSG_OOB` 플래그로 여러분에게 보낸 데이터를 얻는 방법입니다. 수신 측에서는 긴급 데이터가 있다는 사실을 알려주는 [i[`SIGURG` macro]i] `SIGURG` 신호를 받게 됩니다. 그 신호 처리기 안에서 이 `MSG_OOB` 플래그와 함께 `recv()`를 호출할 수 있습니다. |
| [i[`MSG_PEEK` macro]i]`MSG_PEEK`                    | `recv()`를 "그냥 보는 척" 호출하고 싶다면 이 플래그를 사용할 수 있습니다. 이 플래그는 "진짜로" `recv()`를 호출할 때(_즉_ `MSG_PEEK` 플래그 없이 호출할 때) 버퍼에서 기다리고 있을 내용을 알려줍니다. 다음 `recv()` 호출을 살짝 미리 보는 것과 같습니다. |
| [i[`MSG_WAITALL` macro]i]`MSG_WAITALL`              | `len` 매개변수로 지정한 데이터가 모두 수신될 때까지 `recv()`가 반환하지 않게 합니다. 하지만 신호가 호출을 인터럽트하거나, 오류가 발생하거나, 원격 쪽이 연결을 닫는 등 극단적인 상황에서는 여러분의 바람을 무시할 것입니다. 너무 화내지는 마세요. |

`recv()`를 호출하면 읽을 데이터가 생길 때까지 블록됩니다. 블록되지 않기를 원한다면
소켓을 논블로킹으로 설정하거나, `recv()` 또는 `recvfrom()`을 호출하기 전에
`select()`나 `poll()`로 들어오는 데이터가 있는지 확인하세요.

### 반환값 {.unnumbered .unlisted}

실제로 받은 바이트 수를 반환합니다. 이 값은 `len` 매개변수로 요청한 것보다 작을 수
있습니다. 오류가 발생하면 `-1`을 반환하고 `errno`를 적절히 설정합니다.

원격 쪽이 연결을 닫았다면 `recv()`는 `0`을 반환합니다. 이것이 원격 쪽이 연결을
닫았는지 판단하는 일반적인 방법입니다. 평범함도 좋은 겁니다, 반항아 여러분!

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
// 스트림 소켓과 recv()

struct addrinfo hints, *res;
int sockfd;
char buf[512];
int byte_count;

// 호스트 정보를 얻고, 소켓을 만들고, 연결합니다
memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;  // IPv4와 IPv6 어느 쪽이든 사용
hints.ai_socktype = SOCK_STREAM;
getaddrinfo("www.example.com", "3490", &hints, &res);
sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
connect(sockfd, res->ai_addr, res->ai_addrlen);

// 좋습니다! 이제 연결되었으니 데이터를 받을 수 있습니다!
byte_count = recv(sockfd, buf, sizeof buf, 0);
printf("recv()'d %d bytes of data in buf\n", byte_count);
```

```{.c .numberLines}
// 데이터그램 소켓과 recvfrom()

struct addrinfo hints, *res;
int sockfd;
int byte_count;
socklen_t fromlen;
struct sockaddr_storage addr;
char buf[512];
char ipstr[INET6_ADDRSTRLEN];

// 호스트 정보를 얻고, 소켓을 만들고, 4950번 포트에 바인드합니다
memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;  // IPv4와 IPv6 어느 쪽이든 사용
hints.ai_socktype = SOCK_DGRAM;
hints.ai_flags = AI_PASSIVE;
getaddrinfo(NULL, "4950", &hints, &res);
sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
bind(sockfd, res->ai_addr, res->ai_addrlen);

// accept()는 필요 없습니다. 그냥 recvfrom()입니다:

fromlen = sizeof addr;
byte_count = recvfrom(sockfd, buf, sizeof buf, 0, &addr, &fromlen);

printf("recv()'d %d bytes of data in buf\n", byte_count);
printf("from IP address %s\n",
    inet_ntop(addr.ss_family,
        addr.ss_family == AF_INET?
            ((struct sockaddr_in *)&addr)->sin_addr:
            ((struct sockaddr_in6 *)&addr)->sin6_addr,
        ipstr, sizeof ipstr);
```

### 함께 보기 {.unnumbered .unlisted}

[`send()`](#sendman), [`sendto()`](#sendman), [`select()`](#selectman),
[`poll()`](#pollman), [블로킹](#blocking)

[[manbreak]]

## `select()` {#selectman}

[i[`select()` function]i]

소켓 설명자가 읽기/쓰기를 할 준비가 되었는지 확인합니다

### 개요 {.unnumbered .unlisted}

```{.c}
#include <sys/select.h>

int select(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
           struct timeval *timeout);

FD_SET(int fd, fd_set *set);
FD_CLR(int fd, fd_set *set);
FD_ISSET(int fd, fd_set *set);
FD_ZERO(fd_set *set);
```

### 설명 {.unnumbered .unlisted}

`select()` 함수는 여러 소켓을 동시에 확인할 방법을 제공합니다. 각 소켓에
`recv()`할 데이터가 기다리고 있는지, 대기하지 않고 데이터를 `send()`할 수 있는지,
또는 어떤 예외가 발생했는지 확인할 수 있습니다.

위의 `FD_SET()` 같은 매크로를 사용해서 소켓 설명자 집합을 채웁니다. 집합을 만들고
나면 아래 매개변수 중 하나로 함수에 넘깁니다. 집합 안의 소켓 중 어느 것이 데이터를
`recv()`할 준비가 되었는지 알고 싶다면 `readfds`, 어느 소켓에 데이터를 `send()`할
준비가 되었는지 알고 싶다면 `writefds`, 어느 소켓에서 예외(오류)가 발생했는지 알아야
한다면 `exceptfds`입니다. 이런 이벤트에 관심이 없다면 이 매개변수 중 일부 또는 전부가
`NULL`일 수 있습니다. `select()`가 반환된 뒤에는 집합의 값이 바뀌어 어떤 소켓이
읽기나 쓰기 준비가 되었는지, 어떤 소켓에 예외가 있는지를 보여줍니다.

첫 번째 매개변수 `n`은 가장 큰 번호의 소켓 설명자(그냥 `int`라는 점을 기억하세요)에
1을 더한 값입니다.

마지막에 있는 [i[`struct timeval` type]i] `struct timeval`인 `timeout`은
`select()`가 이 집합들을 얼마나 오래 확인할지 알려줍니다. 타임아웃이 지나거나
이벤트가 발생하면, 둘 중 먼저 일어난 시점에 반환합니다. `struct timeval`에는 두
필드가 있습니다. `tv_sec`은 초 단위 값이고, 여기에 마이크로초 단위 값인 `tv_usec`이
더해집니다. (1초는 1,000,000마이크로초입니다.)

도우미 매크로들은 아래 일을 합니다:

| 매크로                                                   | 설명                                 |
| -------------------------------------------------------- | ------------------------------------ |
| [i[`FD_SET()` macro]i]`FD_SET(int fd, fd_set *set);`     | `fd`를 `set`에 추가합니다.           |
| [i[`FD_CLR()` macro]i]`FD_CLR(int fd, fd_set *set);`     | `fd`를 `set`에서 제거합니다.         |
| [i[`FD_ISSET()` macro]i]`FD_ISSET(int fd, fd_set *set);` | `fd`가 `set` 안에 있으면 참을 반환합니다. |
| [i[`FD_ZERO()` macro]i]`FD_ZERO(fd_set *set);`           | `set`의 모든 항목을 비웁니다.        |

Linux 사용자를 위한 노트: Linux의 `select()`는 "읽을 준비가 됐다"고 반환했는데
실제로는 읽을 준비가 되어 있지 않아서, 뒤따르는 `read()` 호출이 블록될 수
있습니다. 수신 소켓에 [i[`O_NONBLOCK` macro]] `O_NONBLOCK` 플래그를 설정해서
이 경우 `EWOULDBLOCK` 오류가 나게 하고, 그 오류가 발생하면 무시하는 방식으로 이
버그를 우회할 수 있습니다. 소켓을 논블로킹으로 설정하는 방법은 [`fcntl()` 맨페이지](#fcntlman)를
참고하세요.

### 반환값 {.unnumbered .unlisted}

성공하면 집합 안에서 준비된 설명자의 수를 반환합니다. 타임아웃에 도달했다면 `0`,
오류가 발생했다면 `-1`을 반환하고 `errno`를 적절히 설정합니다. 또한 어떤 소켓이
준비되었는지를 보여주도록 집합이 수정됩니다.

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
int s1, s2, n;
fd_set readfds;
struct timeval tv;
char buf1[256], buf2[256];

// 이 시점에서 둘 다 서버에 연결되었다고 가정합니다
//s1 = socket(...);
//s2 = socket(...);
//connect(s1, ...)...
//connect(s2, ...)...

// 미리 집합을 비웁니다
FD_ZERO(&readfds);

// 설명자들을 집합에 추가합니다
FD_SET(s1, &readfds);
FD_SET(s2, &readfds);

// s2를 두 번째로 얻었으므로 이것이 "더 큰" 값입니다.
// select()의 n 매개변수에는 이 값을 사용합니다
n = s2 + 1;

// 둘 중 한 소켓에 recv()할 데이터가 준비될 때까지 기다립니다
// (타임아웃 10.5초)
tv.tv_sec = 10;
tv.tv_usec = 500000;
rv = select(n, &readfds, NULL, NULL, &tv);

if (rv == -1) {
    perror("select"); // select()에서 오류 발생
} else if (rv == 0) {
    printf("Timeout occurred! No data after 10.5 seconds.\n");
} else {
    // 설명자 하나 또는 둘 모두에 데이터가 있습니다
    if (FD_ISSET(s1, &readfds)) {
        recv(s1, buf1, sizeof buf1, 0);
    }
    if (FD_ISSET(s2, &readfds)) {
        recv(s2, buf2, sizeof buf2, 0);
    }
}
```

### 함께 보기 {.unnumbered .unlisted}

[`poll()`](#pollman)

[[manbreak]]

## `setsockopt()`, `getsockopt()` {#setsockoptman}

[i[`setsockopt()` function]i]
[i[`getsockopt()` function]i]

소켓의 여러 옵션을 설정합니다

### 개요 {.unnumbered .unlisted}

```{.c}
#include <sys/types.h>
#include <sys/socket.h>

int getsockopt(int s, int level, int optname, void *optval,
               socklen_t *optlen);
int setsockopt(int s, int level, int optname, const void *optval,
               socklen_t optlen);
```

### 설명 {.unnumbered .unlisted}

소켓은 꽤 많이 설정할 수 있는 녀석입니다. 사실 너무 많이 설정할 수 있어서 여기에서
전부 다루지는 않을 것입니다. 어차피 시스템에 따라 달라질 가능성도 큽니다. 하지만
기본은 이야기하겠습니다.

분명하겠지만, 이 함수들은 소켓의 특정 옵션을 얻고 설정합니다. Linux 시스템에서는 모든
소켓 정보가 7번 섹션의 `socket` 맨페이지에 있습니다. (이 정보를 모두 얻으려면
"`man 7 socket`"을 입력하세요.)

매개변수를 보자면 `s`는 이야기하고 있는 소켓이고, `level`은 [i[`SOL_SOCKET` macro]i]
`SOL_SOCKET`으로 설정해야 합니다. 그런 다음 `optname`을 관심 있는 이름으로
설정합니다. 다시 말하지만 모든 옵션은 맨페이지를 보세요. 여기에는 그중 재미있는 것
몇 가지가 있습니다:

| `optname`                                      | 설명 |
| ---------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [i[`SO_BINDTODEVICE` macro]i]`SO_BINDTODEVICE` | `bind()`로 IP 주소에 바인드하는 대신 이 소켓을 `eth0` 같은 심볼릭 장치 이름에 바인드합니다. 유닉스에서 장치 이름을 보려면 `ifconfig` 명령을 입력하세요. |
| [i[`SO_REUSEADDR` macro]i]`SO_REUSEADDR      ` | 이 포트에 이미 활성 리스닝 소켓이 바인드되어 있지 않다면, 다른 소켓도 이 포트에 `bind()`할 수 있게 합니다. 서버가 죽은 뒤 다시 시작하려 할 때 보이는 "Address already in use" 오류 메시지를 피할 수 있게 해 줍니다. |
| [i[`SO_BROADCAST` macro]i]`SO_BROADCAST`       | UDP 데이터그램(`SOCK_DGRAM`) 소켓이 브로드캐스트 주소로 오가는 패킷을 보내고 받을 수 있게 합니다. TCP 스트림 소켓에는 아무 일도, _아무 일도!!_ 하지 않습니다! 하하하! |

(역자 주: `SO_BINDTODEVICE`는 Linux 전용 옵션입니다. 최근 Linux에서는 인터페이스 이름을
확인할 때 `ifconfig` 대신 `ip link`를 흔히 사용합니다.)

`optval` 매개변수는 보통 해당 값을 나타내는 `int`에 대한 포인터입니다. 불리언의 경우
0은 거짓이고 0이 아니면 참입니다. 여러분 시스템에서 다르지 않은 한, 이것은 절대적
사실입니다. 넘길 매개변수가 없다면 `optval`은 `NULL`일 수 있습니다.

마지막 매개변수 `optlen`은 `optval`의 길이로 설정해야 합니다. 아마 `sizeof(int)`일
것이지만 옵션에 따라 달라집니다. `getsockopt()`의 경우 이것은 `socklen_t`에 대한
포인터이며, `optval`에 저장될 객체의 최대 크기를 지정한다는 점에 주목하세요.
(버퍼 오버플로를 막기 위해서입니다.) 그리고 `getsockopt()`는 실제로 설정된 바이트
수를 반영하도록 `optlen` 값을 수정합니다.

**경고**: 일부 시스템(특히 [i[SunOS]] [i[Solaris]] Sun과 [i[Windows]] Windows)에서는
옵션이 `int`가 아니라 `char`일 수 있으며, 예를 들어 `int` 값 `1` 대신 문자 값
`'1'`로 설정됩니다. 다시 말하지만 더 자세한 정보는 "`man setsockopt`"와
"`man 7 socket`"으로 여러분의 맨페이지를 확인하세요!

### 반환값 {.unnumbered .unlisted}

성공하면 0을 반환합니다. 오류가 발생하면 `-1`을 반환하고 `errno`를 적절히
설정합니다.

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
int optval;
int optlen;
char *optval2;

// 소켓의 SO_REUSEADDR을 참(1)으로 설정합니다:
optval = 1;
setsockopt(s1, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

// 소켓을 장치 이름에 바인드합니다(모든 시스템에서 동작하지는 않을 수 있습니다):
optval2 = "eth1"; // 길이가 4바이트이므로 아래에서 4:
setsockopt(s2, SOL_SOCKET, SO_BINDTODEVICE, optval2, 4);

// SO_BROADCAST 플래그가 설정되어 있는지 확인합니다:
getsockopt(s3, SOL_SOCKET, SO_BROADCAST, &optval, &optlen);
if (optval != 0) {
    print("SO_BROADCAST enabled on s3!\n");
}
```

### 함께 보기 {.unnumbered .unlisted}

[`fcntl()`](#fcntlman)

[[manbreak]]

## `send()`, `sendto()` {#sendman}

[i[`send()` function]i]
[i[`sendto()` function]i]

소켓으로 데이터를 보냅니다

### 개요 {.unnumbered .unlisted}

```{.c}
#include <sys/types.h>
#include <sys/socket.h>

ssize_t send(int s, const void *buf, size_t len, int flags);
ssize_t sendto(int s, const void *buf, size_t len,
               int flags, const struct sockaddr *to,
               socklen_t tolen);
```

### 설명 {.unnumbered .unlisted}

이 함수들은 소켓으로 데이터를 보냅니다. 일반적으로 `send()`는 TCP `SOCK_STREAM`
연결 소켓에 사용하고, `sendto()`는 UDP `SOCK_DGRAM` 비연결 데이터그램 소켓에
사용합니다. 비연결 소켓에서는 패킷을 보낼 때마다 목적지를 지정해야 하므로,
`sendto()`의 마지막 매개변수들이 패킷이 어디로 가는지를 정의합니다.

`send()`와 `sendto()`에서 `s` 매개변수는 소켓이고, `buf`는 보내려는 데이터에 대한
포인터이며, `len`은 보내려는 바이트 수입니다. `flags`를 사용하면 데이터를 어떻게
보낼지에 대한 추가 정보를 지정할 수 있습니다. "일반" 데이터를 원한다면 `flags`를
0으로 설정하세요. 흔히 쓰는 플래그 몇 가지는 아래와 같습니다. 더 자세한 내용은
여러분의 로컬 `send()` 맨페이지를 확인하세요:

| 매크로                                     | 설명 |
| ------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| [i[`MSG_OOB` macro]i]`MSG_OOB`             | [i[Out-of-band data]] "out of band" 데이터로 보냅니다. TCP는 이것을 지원하며, 이 데이터가 일반 데이터보다 더 높은 우선순위를 가진다고 수신 시스템에 알리는 방법입니다. 수신자는 [i[`SIGURG` macro]i] `SIGURG` 신호를 받고, 큐에 있는 일반 데이터를 모두 받기 전에 이 데이터를 받을 수 있습니다. |
| [i[`MSG_DONTROUTE` macro]i]`MSG_DONTROUTE` | 이 데이터를 라우터를 통해 보내지 않고 로컬에만 둡니다. |
| [i[`MSG_DONTWAIT` macro]i]`MSG_DONTWAIT`   | 나가는 트래픽이 막혀 `send()`가 대기하게 될 상황이면 [i[`EAGAIN` macro]] `EAGAIN`을 반환하게 합니다. "이번 `send()`에 대해서만 [i[Non-blocking sockets]] 논블로킹을 켠다"와 비슷합니다. 더 자세한 내용은 [블로킹](#blocking) 절을 보세요. |
| [i[`MSG_NOSIGNAL` macro]i]`MSG_NOSIGNAL`   | 더 이상 `recv()`하지 않는 원격 호스트에 `send()`하면 보통 [i[`SIGPIPE` macro]] `SIGPIPE` 신호를 받습니다. 이 플래그를 추가하면 그 신호가 발생하지 않습니다. |

### 반환값 {.unnumbered .unlisted}

실제로 보낸 바이트 수를 반환하거나, 오류가 발생하면 `-1`을 반환하고 `errno`를 적절히
설정합니다. 실제로 보낸 바이트 수는 여러분이 보내라고 요청한 수보다 작을 수 있다는
점에 주목하세요! 이것을 처리하는 도우미 함수는 [부분 `send()` 처리](#sendall) 절을
보세요.

또한 어느 한쪽에서 소켓을 닫았다면, `send()`를 호출한 프로세스는 `SIGPIPE` 신호를
받습니다. (`MSG_NOSIGNAL` 플래그와 함께 `send()`를 호출한 경우는 제외합니다.)

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
int spatula_count = 3490;
char *secret_message = "The Cheese is in The Toaster";

int stream_socket, dgram_socket;
struct sockaddr_in dest;
int temp;

// 먼저 TCP 스트림 소켓:

// 소켓이 만들어지고 연결되었다고 가정합니다
//stream_socket = socket(...
//connect(stream_socket, ...

// 네트워크 바이트 순서로 변환
temp = htonl(spatula_count);
// 데이터를 일반 방식으로 전송:
send(stream_socket, &temp, sizeof temp, 0);

// 비밀 메시지를 out-of-band로 전송:
send(stream_socket, secret_message, strlen(secret_message)+1,
        MSG_OOB);

// 이제 UDP 데이터그램 소켓:
//getaddrinfo(...
//dest = ... // "dest"가 목적지 주소를 담고 있다고 가정합니다
//dgram_socket = socket(...

// 비밀 메시지를 일반 방식으로 전송:
sendto(dgram_socket, secret_message, strlen(secret_message)+1, 0,
       (struct sockaddr*)&dest, sizeof dest);
```

### 함께 보기 {.unnumbered .unlisted}

[`recv()`](#recvman), [`recvfrom()`](#recvman)

[[manbreak]]

## `shutdown()` {#shutdownman}

[i[`shutdown()` function]i]

소켓에서 이후 송수신을 중단합니다

### 개요 {.unnumbered .unlisted}

```{.c}
#include <sys/socket.h>

int shutdown(int s, int how);
```

### 설명 {.unnumbered .unlisted}

됐습니다! 이제 지긋지긋합니다! 이 소켓에서는 더 이상 `send()`를 허용하지 않지만,
그래도 `recv()`로 데이터는 받고 싶습니다! 또는 그 반대일 수도 있습니다! 어떻게 해야
할까요?

소켓 설명자를 `close()`하면, 그 소켓의 읽기와 쓰기 양쪽이 모두 닫히고 소켓 설명자가
해제됩니다. 한쪽만 닫고 싶다면 `shutdown()` 호출을 사용할 수 있습니다.

매개변수를 보자면 `s`는 당연히 이 동작을 수행할 소켓이고, 어떤 동작을 할지는 `how`
매개변수로 지정할 수 있습니다. `how`는 이후 `recv()`를 막는
[i[`SHUT_RD` macro]i]`SHUT_RD`, 이후 `send()`를 금지하는
[i[`SHUT_WR` macro]i]`SHUT_WR`, 또는 둘 다 하는 [i[`SHUT_RDWR` macro]i]`SHUT_RDWR`
일 수 있습니다.

`shutdown()`은 소켓 설명자를 해제하지 않는다는 점에 주목하세요. 완전히 종료한
경우에도 결국 소켓을 `close()`해야 합니다.

이것은 드물게 쓰는 시스템 호출입니다.

### 반환값 {.unnumbered .unlisted}

성공하면 0을 반환합니다. 오류가 발생하면 `-1`을 반환하고 `errno`를 적절히
설정합니다.

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
int s = socket(PF_INET, SOCK_STREAM, 0);

// ...여기에서 send() 몇 번과 여러 일을 합니다...

// 이제 끝났으니 더 이상 send()를 허용하지 않습니다:
shutdown(s, SHUT_WR);
```

### 함께 보기 {.unnumbered .unlisted}

[`close()`](#closeman)

[[manbreak]]

## `socket()` {#socketman}

[i[`socket()` function]i]

소켓 설명자를 할당합니다

### 개요 {.unnumbered .unlisted}

```{.c}
#include <sys/types.h>
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```

### 설명 {.unnumbered .unlisted}

소켓다운 일을 할 때 사용할 수 있는 새 소켓 설명자를 반환합니다. 이것은 보통 소켓
프로그램을 작성하는 거대한 과정에서 첫 번째 호출이며, 그 결과를 이후 `listen()`,
`bind()`, `accept()` 또는 여러 다른 함수 호출에 사용할 수 있습니다.

일반적인 사용에서는 아래 예제처럼 `getaddrinfo()` 호출에서 이 매개변수들의 값을
얻습니다. 하지만 정말 원한다면 직접 채울 수도 있습니다.

| 매개변수   | 설명 |
| ---------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `domain`   | `domain`은 여러분이 관심 있는 소켓의 종류를 설명합니다. 믿어 주세요, 이것에는 정말 다양한 값이 올 수 있습니다. 하지만 이것은 소켓 안내서이므로 IPv4에는 [i[`PF_INET` macro]i] `PF_INET`, IPv6에는 `PF_INET6`이 될 것입니다. |
| `type`     | `type` 매개변수에도 여러 값이 올 수 있지만, 아마 신뢰성 있는 TCP 소켓(`send()`, `recv()`)을 위한 [i[`SOCK_STREAM` macro]i] `SOCK_STREAM`이나, 신뢰성은 없지만 빠른 UDP 소켓(`sendto()`, `recvfrom()`)을 위한 [i[`SOCK_DGRAM` macro]i] `SOCK_DGRAM` 중 하나로 설정할 것입니다. (또 다른 흥미로운 소켓 타입으로는 패킷을 직접 구성할 때 쓸 수 있는 [i[`SOCK_RAW` macro]i] `SOCK_RAW`가 있습니다. 꽤 멋집니다.) |
| `protocol` | 마지막으로 `protocol` 매개변수는 특정 소켓 타입에 어떤 프로토콜을 사용할지 알려줍니다. 이미 말했듯이 예를 들어 `SOCK_STREAM`은 TCP를 사용합니다. 다행히 `SOCK_STREAM`이나 `SOCK_DGRAM`을 쓸 때는 프로토콜을 그냥 0으로 설정하면 적절한 프로토콜을 자동으로 사용합니다. 그렇지 않다면 [i[`getprotobyname()` function]] `getprotobyname()`으로 적절한 프로토콜 번호를 찾을 수 있습니다. |

### 반환값 {.unnumbered .unlisted}

이후 호출에서 사용할 새 소켓 설명자를 반환합니다. 오류가 발생하면 `-1`을 반환하고
`errno`를 적절히 설정합니다.

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
struct addrinfo hints, *res;
int sockfd;

// 먼저 getaddrinfo()로 주소 구조체를 채웁니다:

memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;     // AF_INET, AF_INET6, 또는 AF_UNSPEC
hints.ai_socktype = SOCK_STREAM; // SOCK_STREAM 또는 SOCK_DGRAM

getaddrinfo("www.example.com", "3490", &hints, &res);

// getaddrinfo()에서 얻은 정보를 사용해 소켓을 만듭니다:
sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
```

### 함께 보기 {.unnumbered .unlisted}

[`accept()`](#acceptman), [`bind()`](#bindman),
[`getaddrinfo()`](#getaddrinfoman), [`listen()`](#listenman)

[[manbreak]]

## `struct sockaddr`와 친구들 {#structsockaddrman}

[i[`struct sockaddr` type]i]
[i[`struct sockaddr_in` type]i]
[i[`struct in_addr` type]i]
[i[`struct sockaddr_in6` type]i]
[i[`struct in6_addr` type]i]
[i[`struct sockaddr_storage` type]i]

인터넷 주소를 다루는 구조체들

### 개요 {.unnumbered .unlisted}

```{.c}
#include <netinet/in.h>

// 소켓 주소 구조체에 대한 모든 포인터는 여러 함수와 시스템 호출에서
// 사용되기 전에 이 타입에 대한 포인터로 형변환되는 경우가 많습니다:

struct sockaddr {
    unsigned short    sa_family;    // 주소 계열, AF_xxx
    char              sa_data[14];  // 14바이트 프로토콜 주소
};


// IPv4 AF_INET 소켓:

struct sockaddr_in {
    short            sin_family;   // 예: AF_INET, AF_INET6
    unsigned short   sin_port;     // 예: htons(3490)
    struct in_addr   sin_addr;     // 아래 struct in_addr를 보세요
    char             sin_zero[8];  // 원한다면 0으로 만드세요
};

struct in_addr {
    unsigned long s_addr;          // inet_pton()으로 채웁니다
};


// IPv6 AF_INET6 소켓:

struct sockaddr_in6 {
    u_int16_t       sin6_family;   // 주소 계열, AF_INET6
    u_int16_t       sin6_port;     // 포트 번호, 네트워크 바이트 순서
    u_int32_t       sin6_flowinfo; // IPv6 흐름 정보
    struct in6_addr sin6_addr;     // IPv6 주소
    u_int32_t       sin6_scope_id; // 스코프 ID
};

struct in6_addr {
    unsigned char   s6_addr[16];   // inet_pton()으로 채웁니다
};


// struct sockaddr_in 또는 struct sockaddr_in6 데이터를 담기에 충분히 큰
// 일반 소켓 주소 보관 구조체:

struct sockaddr_storage {
    sa_family_t  ss_family;     // 주소 계열

    // 이것들은 모두 패딩이고 구현에 따라 다릅니다. 무시하세요:
    char      __ss_pad1[_SS_PAD1SIZE];
    int64_t   __ss_align;
    char      __ss_pad2[_SS_PAD2SIZE];
};
```

### 설명 {.unnumbered .unlisted}

이것들은 인터넷 주소를 다루는 모든 시스템 호출과 함수의 기본 구조체입니다. 보통은
`getaddrinfo()`를 사용해 이 구조체들을 채우고, 필요할 때 읽게 될 것입니다.

메모리에서 `struct sockaddr_in`과 `struct sockaddr_in6`은 `struct sockaddr`와 같은
시작 구조를 공유합니다. 그래서 우주가 끝날 가능성만 빼면, 한 타입의 포인터를 다른
타입으로 자유롭게 형변환해도 해가 없습니다.

우주가 끝난다는 건 농담입니다... `struct sockaddr_in*`을 `struct sockaddr*`로
형변환했을 때 정말 우주가 끝난다면, 그건 순전히 우연일 뿐이니 걱정하지 않아도
된다고 약속드립니다.

그러니 이 점을 염두에 두고, 어떤 함수가 `struct sockaddr*`를 받는다고 할 때마다
여러분의 `struct sockaddr_in*`, `struct sockaddr_in6*`, 또는
`struct sockaddr_storage*`를 그 타입으로 쉽고 안전하게 형변환할 수 있음을
기억하세요.

`struct sockaddr_in`은 IPv4 주소(예: "192.0.2.10")에 사용하는 구조체입니다. 여기에는
주소 계열(`AF_INET`), `sin_port` 안의 포트, `sin_addr` 안의 IPv4 주소가 들어갑니다.

`struct sockaddr_in`에는 `sin_zero` 필드도 있는데, 어떤 사람들은 이것을 반드시 0으로
설정해야 한다고 주장합니다. 다른 사람들은 이것에 대해 아무 주장도 하지 않고(Linux
문서에서는 아예 언급하지도 않습니다), 실제로 0으로 설정할 필요가 있어 보이지도
않습니다. 그러니 마음이 내키면 `memset()`으로 0으로 설정하세요.

이제, 저 `struct in_addr`는 시스템에 따라 이상한 녀석입니다. 때로는 온갖 `#define`과
여러 잡동사니가 붙은 정신없는 `union`입니다. 하지만 여러분이 해야 할 일은 이 구조체의
`s_addr` 필드만 사용하는 것입니다. 많은 시스템이 그것 하나만 구현하기 때문입니다.

`struct sockaddr_in6`과 `struct in6_addr`도 매우 비슷하지만, IPv6에 사용된다는 점이
다릅니다.

`struct sockaddr_storage`는 IP 버전에 무관한 코드를 작성하려고 할 때, 그리고 새
주소가 IPv4일지 IPv6일지 모를 때 `accept()`나 `recvfrom()`에 넘길 수 있는
구조체입니다. `struct sockaddr_storage` 구조체는 원래의 작은 `struct sockaddr`와
달리 두 타입을 모두 담을 만큼 큽니다.

### 예제 {.unnumbered .unlisted}

```{.c .numberLines}
// IPv4:

struct sockaddr_in ip4addr;
int s;

ip4addr.sin_family = AF_INET;
ip4addr.sin_port = htons(3490);
inet_pton(AF_INET, "10.0.0.1", &ip4addr.sin_addr);

s = socket(PF_INET, SOCK_STREAM, 0);
bind(s, (struct sockaddr*)&ip4addr, sizeof ip4addr);
```

```{.c .numberLines}
// IPv6:

struct sockaddr_in6 ip6addr;
int s;

ip6addr.sin6_family = AF_INET6;
ip6addr.sin6_port = htons(4950);
inet_pton(AF_INET6, "2001:db8:8714:3a90::12", &ip6addr.sin6_addr);

s = socket(PF_INET6, SOCK_STREAM, 0);
bind(s, (struct sockaddr*)&ip6addr, sizeof ip6addr);
```

### 함께 보기 {.unnumbered .unlisted}

[`accept()`](#acceptman), [`bind()`](#bindman), [`connect()`](#connectman),
[`inet_aton()`](#inet_ntoaman), [`inet_ntoa()`](#inet_ntoaman)
