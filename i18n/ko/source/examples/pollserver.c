/*
** pollserver.c -- 간단한 다중 사용자 채팅 서버
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
 * 소켓을 IP 주소 문자열로 변환합니다.
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

	// 소켓을 얻고 바인드합니다
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
	// 공간이 부족하면 pfds 배열에 공간을 더 추가합니다
	if (*fd_count == *fd_size) {
		*fd_size *= 2; // 두 배로 늘립니다
		*pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
	}

	(*pfds)[*fd_count].fd = newfd;
	(*pfds)[*fd_count].events = POLLIN; // 읽을 준비가 되었는지 확인
	(*pfds)[*fd_count].revents = 0;

	(*fd_count)++;
}

/*
 * 주어진 인덱스의 파일 설명자를 집합에서 제거합니다.
 */
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
	// 끝에 있는 항목을 이 위치로 복사합니다
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
 * 일반 클라이언트 데이터나 클라이언트 연결 종료를 처리합니다.
 */
void handle_client_data(int listener, int *fd_count,
		struct pollfd *pfds, int *pfd_i)
{
	char buf[256];	// 클라이언트 데이터용 버퍼

	int nbytes = recv(pfds[*pfd_i].fd, buf, sizeof buf, 0);

	int sender_fd = pfds[*pfd_i].fd;

	if (nbytes <= 0) { // 오류가 났거나 클라이언트가 연결을 닫았습니다
		if (nbytes == 0) {
			// 연결 종료
			printf("pollserver: socket %d hung up\n", sender_fd);
		} else {
			perror("recv");
		}

		close(pfds[*pfd_i].fd); // 잘 가!

		del_from_pfds(pfds, *pfd_i, fd_count);

		// 방금 삭제한 슬롯을 다시 검사합니다
		(*pfd_i)--;

	} else { // 클라이언트에게서 정상 데이터를 받았습니다
		printf("pollserver: recv from fd %d: %.*s", sender_fd,
				nbytes, buf);
		// 모두에게 보냅니다!
		for(int j = 0; j < *fd_count; j++) {
			int dest_fd = pfds[j].fd;

			// 리스너와 자신은 제외합니다
			if (dest_fd != listener && dest_fd != sender_fd) {
				if (send(dest_fd, buf, nbytes, 0) == -1) {
					perror("send");
				}
			}
		}
	}
}

/*
 * 기존 연결을 모두 처리합니다.
 */
void process_connections(int listener, int *fd_count, int *fd_size,
		struct pollfd **pfds)
{
	for(int i = 0; i < *fd_count; i++) {

		// 누군가 읽을 준비가 되었는지 확인합니다
		if ((*pfds)[i].revents & (POLLIN | POLLHUP)) {
			// 하나 찾았습니다!!

			if ((*pfds)[i].fd == listener) {
				// 리스너라면 새 연결입니다
				handle_new_connection(listener, fd_count, fd_size,
						pfds);
			} else {
				// 아니면 일반 클라이언트입니다
				handle_client_data(listener, fd_count, *pfds, &i);
			}
		}
	}
}

/*
 * 메인 함수: 리스너와 연결 집합을 만들고, 영원히 반복하면서
 * 연결을 처리합니다.
 */
int main(void)
{
	int listener;	 // 리스닝 소켓 설명자

	// 연결 5개를 담을 공간으로 시작합니다
	// (필요하면 realloc합니다)
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
	// 들어오는 연결이 있으면 읽을 준비가 되었다고 보고합니다
	pfds[0].fd = listener;
	pfds[0].events = POLLIN;

	fd_count = 1; // 리스너용

	puts("pollserver: waiting for connections...");

	// 주 루프
	for(;;) {
		int poll_count = poll(pfds, fd_count, -1);

		if (poll_count == -1) {
			perror("poll");
			exit(1);
		}

		// 연결을 순회하면서 읽을 데이터를 찾습니다
		process_connections(listener, &fd_count, &fd_size, &pfds);
	}

	free(pfds);
}
