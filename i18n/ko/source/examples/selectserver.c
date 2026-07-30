/*
** selectserver.c -- 간단한 다중 사용자 채팅 서버
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
 * 리스닝 소켓을 반환합니다
 */
int get_listener_socket(void)
{
	struct addrinfo hints, *ai, *p;
	int yes=1;    // 아래의 setsockopt() SO_REUSEADDR용
	int rv;
	int listener;

	// 소켓을 얻고 바인드합니다
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
 * 새로 들어오는 연결을 적절한 집합에 추가합니다
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
		FD_SET(newfd, master); // master 집합에 추가합니다
		if (newfd > *fdmax) {  // 최댓값을 추적합니다
			*fdmax = newfd;
		}
		printf("selectserver: new connection from %s on "
			"socket %d\n",
			inet_ntop2(&remoteaddr, remoteIP, sizeof remoteIP),
			newfd);
	}
}

/*
 * 모든 클라이언트에 메시지를 브로드캐스트합니다
 */
void broadcast(char *buf, int nbytes, int listener, int s,
               fd_set *master, int fdmax)
{
	for(int j = 0; j <= fdmax; j++) {
		// 모두에게 보냅니다!
		if (FD_ISSET(j, master)) {
			// 리스너와 자신은 제외합니다
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

	// 클라이언트의 데이터를 처리합니다
	if ((nbytes = recv(s, buf, sizeof buf, 0)) <= 0) {
		// 오류가 났거나 클라이언트가 연결을 닫았습니다
		if (nbytes == 0) {
			// 연결 종료
			printf("selectserver: socket %d hung up\n", s);
		} else {
			perror("recv");
		}
		close(s); // 잘 가!
		FD_CLR(s, master); // master 집합에서 제거합니다
	} else {
		// 클라이언트에게서 데이터를 받았습니다
		broadcast(buf, nbytes, listener, s, master, fdmax);
	}
}

/*
 * 메인 함수
 */
int main(void)
{
	fd_set master;    // master 파일 설명자 목록
	fd_set read_fds;  // select()용 임시 파일 설명자 목록
	int fdmax;        // 최대 파일 설명자 번호

	int listener;     // 리스닝 소켓 설명자

	FD_ZERO(&master);    // master와 임시 집합을 비웁니다
	FD_ZERO(&read_fds);

	listener = get_listener_socket();

	// 리스너를 master 집합에 추가합니다
	FD_SET(listener, &master);

	// 가장 큰 파일 설명자를 추적합니다
	fdmax = listener; // 지금까지는 이것입니다

	// 주 루프
	for(;;) {
		read_fds = master; // 복사합니다
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(4);
		}

		// 기존 연결을 순회하면서 읽을 데이터를 찾습니다
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
