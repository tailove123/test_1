#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100 // 최대 글자 수
#define MAX_CLNT 256 // 최대 동시 접속 가능 수
#define NAME_SIZE 20


void *send_msg_1(void *arg);
void *recv_msg_1(void *arg);
void error_handling(char *msg);


void *handle_clnt(void *arg);
void send_msg(char *msg, int len);

// 접속한 클라이언트 수
int clnt_cnt = 0;
// 여러 명의 클라이언트가 접속하므로, 클라이언트 소켓은 배열이다.
// 멀티쓰레드 시, 이 두 전역변수, clnt_cnt, clnt_socks 에 여러 쓰레드가 동시 접근할 수 있기에 
// 두 변수의 사용이 들어간다면 무조건 임계영역이다.
int clnt_socks[MAX_CLNT]; // 클라이언트 최대 256명
pthread_mutex_t mutx; // mutex 선언 - 다중 스레드끼리 전역변수 사용시 데이터의 혼선 방지

// 채팅창에 보여질 이름의 형태
char name[NAME_SIZE] = "[DEFAULT]"; // 본인 닉네임 20자 제한
char msg[BUF_SIZE];

int main(int argc, char *argv[])
{

    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    int clnt_adr_sz;
    pthread_t t_id; // thread 선언

    // 소켓 옵션 설정을 위한 두 변수
    int option;
    socklen_t optlen;

    int sock;
    struct sockaddr_in serv_addr;
    // 송신 쓰레드와 수신 쓰레드로 총 2개의 쓰레드 선언
    // 내 메세지를 보내야하고, 상대방의 메세지도 받아야 한다.
    pthread_t snd_thread, rcv_thread;
    // pthread_join 에 사용된다.
    void *thread_return;



    if (strcmp(argv[2], "-s") == 0)
    {
        if (argc != 4)  //인자갯수가 올바르지 않다면,
        {
            printf(" Usage : %s <-s/-c> <-p> <portnum>\n", argv[0]);    //최초실행시 argv[0]은 server_c(실행파일 이름)
            exit(1);    //프로그램 실행을 위한 입력방식을 안내하고, 프로그램 종료.
        }

        // 뮤텍스 만들기
        pthread_mutex_init(&mutx, NULL);
        serv_sock = socket(PF_INET, SOCK_STREAM, 0);

        // Time-wait 해제
        // SO_REUSEADDR 를 0에서 1로 변경
        optlen = sizeof(option);
        option = 1;
        setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void *)&option, optlen);

        // IPv4 , IP, Port 할당
        memset(&serv_adr, 0, sizeof(serv_adr));
        serv_adr.sin_family = AF_INET;
        serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_adr.sin_port = htons(atoi(argv[3]));

        // 주소 할당
        if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
            error_handling("bind() error");

        // 5 로 지정했으니 5명까지만 사용 가능한 채팅인가?
        // 아니다. 큐의 크기일 뿐인데, 운영체제가 여유가 된다면 "알아서" accept 할 것
        // 즉, 총 256명까지 접속 가능한 것.
        // 웹서버같이 수천명의 클라이언트로 바쁠 경우, 15로 잡는 경우가 보통임
        if (listen(serv_sock, 5) == -1)
            error_handling("listen() error");

        // 종료조건 없음. ctrl + c 로만 종료
        while (1)
        {
            clnt_adr_sz = sizeof(clnt_adr);
            clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);

            // clnt_socks[], clnt_cnt 전역변수를 사용하기 위해 뮤텍스 잠금
            pthread_mutex_lock(&mutx);
            // 클라이언트 카운터 올리고, 소켓 배정 . 첫 번째 클라이언트라면, clnt_socks[0] 에 들어갈 것
            clnt_socks[clnt_cnt++] = clnt_sock;
            // 뮤텍스 잠금해제
            pthread_mutex_unlock(&mutx);
            // 쓰레드 생성. 쓰레드의 main 은 handle_clnt
            // 네 번째 파라미터로 accept 이후 생겨난 소켓의 파일 디스크립터 주소값을 넣어주어
            // handle_clnt 에서 파라미터로 받을 수 있도록 함
            pthread_create(&t_id, NULL, handle_clnt, (void *)&clnt_sock);
            // 이걸 호출했다고 해서 끝나지도 않은 쓰레드가 종료되진 않음
            // 즉, t_id 로 접근했을 때, 해당 쓰레드가 NULL 값을 리턴한 경우가 아니라면 무시하고 진행됨
            // 만약 해당 쓰레드가 NULL 값을 리턴했다면, 쓰레드 종료
            pthread_detach(t_id);
            // inet_ntoa 는 int32 형으로 된 IP 를 캐릭터로 친절하게 보여주는 역할
            printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
        }
        // ctrl + c 로 프로그램 종료 시, 서버 소켓 종료
        close(serv_sock);
    }
    else if (strcmp(argv[1], "-c") == 0)
    {
        // 이번엔 ip, port 뿐만 아니라, 사용자 이름까지 넣어줘야 한다.
        if (argc!=6)    //인자의 갯수가 6개가 아닐 때,
        {
            printf(" Usage : %s <-s/-c> <-p> <port> <ip> <name>\n", argv[0]);
            exit(1);    //프로그램 실행을 위한 입력방식을 안내하고, 프로그램 종료.
        }

        // argv[3] 이 Jony 라면, "[Jony]" 가 name 이 됨
        sprintf(name, "[%s]", argv[5]);

        sock = socket(PF_INET, SOCK_STREAM, 0);

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(argv[4]);
        serv_addr.sin_port = htons(atoi(argv[3]));

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
            error_handling("connect() error");

        // 두 개의 쓰레드 생성하고, 각각의 main 은 send_msg, recv_meg
        pthread_create(&snd_thread, NULL, send_msg_1, (void *)&sock);
        pthread_create(&rcv_thread, NULL, recv_msg_1, (void *)&sock);

        // 쓰레드 종료 대기 및 소멸 유도
        pthread_join(snd_thread, &thread_return);
        pthread_join(rcv_thread, &thread_return);

        // 클라이언트 연결 종료
        close(sock);

    }

    return 0;
}

void *handle_clnt(void *arg)
{
    // 소켓 파일 디스크립터가 void 포인터로 들어왔으므로, int 로 형변환
    int clnt_sock = *((int *)arg);
    int str_len = 0;
    char msg[BUF_SIZE];

    // 클라이언트에서 보낸 메세지 받음.
    // 클라이언트에서 EOF 보내서, str_len 이 0이 될때까지 반복
    // EOF 를 보내는 순간은 언제인가? 클라이언트에서, 소켓을 close 했을 때이다!
    // 즉, 클라이언트가 접속을 하고 있는 동안에, while 문을 벗어나진 않는다.
    while ((str_len = read(clnt_sock, msg, sizeof(msg))) != 0)
        // 접속한 모두에게 메세지 보내기
        send_msg(msg, str_len);

    // while 문 탈출했다는 건, 현재 담당하는 소켓의 연결이 끊어졌다는 뜻임.
    // 그러면 당연히, clnt_socks[] 에서 삭제하고, 쓰레드도 소멸시켜야.

    // 전역변수 clnt_cnt 와 clnt_socks[] 를 건드릴 것이기에, 뮤텍스 잠금
    pthread_mutex_lock(&mutx);
    // 연결 끊어진 클라이언트인 "현재 쓰레드에서 담당하는 소켓" 삭제
    for (int i = 0; i < clnt_cnt; i++)
    {
        // 현재 담당하는 클라이언트 소켓의 파일 디스크립터 위치를 찾으면,
        if (clnt_sock == clnt_socks[i])
        {
        // 현재 소켓이 원래 위치했던 곳을 기준으로
        // 뒤의 클라이언트들을 땡겨옴
        while (i++ < clnt_cnt - 1) // 쓰레드 1개 삭제할 것이기 때문에 -1 해줘야 함
            clnt_socks[i] = clnt_socks[i + 1];
        break;
        }
    }
    // 클라이언트 수 하나 줄임
    clnt_cnt--;
    // 뮤텍스 잠금해제
    pthread_mutex_unlock(&mutx);
    // 서버의 쓰레드에서 담당하는 클라이언트 소켓 종료
    close(clnt_sock);
    return NULL;
}

// 접속한 모두에게 메세지 보내기
void send_msg(char *msg, int len)
{
    // clnt_cnt, clnt_socks[] 사용 위해 뮤텍스 잠금
    pthread_mutex_lock(&mutx);
    for (int i = 0; i < clnt_cnt; i++)
    // 모두에게 메세지 보냄
    write(clnt_socks[i], msg, len);
    // 뮤텍스 잠금 해제
    pthread_mutex_unlock(&mutx);
}
void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}


// snd_thread 의 쓰레드 main
void *send_msg_1(void *arg)
{
	// void형 int형으로 전환
    int sock = *((int *)arg);
    // 사용자 아이디와 메세지를 "붙여서" 한 번에 보낼 것이다
    char name_msg[NAME_SIZE + BUF_SIZE];
    while (1)
    {
        // 입력받음
        fgets(msg, BUF_SIZE, stdin);
        // Q 입력 시 종료
        if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
        {
        // 서버에 EOF 를 보냄
        close(sock);
        exit(0);
        }
        // id 를 "Jony", msg 를 "안녕 얘들아" 로 했다면, => [Jony] 안녕 얘들아
        // 이것이 name_msg 로 들어가서 출력됨
        sprintf(name_msg, "%s %s", name, msg);
        // 서버로 메세지 보냄
        write(sock, name_msg, strlen(name_msg));
    }
    return NULL;
}

// rcv_thread 의 쓰레드 main
void *recv_msg_1(void *arg)
{
    int sock = *((int *)arg);
    char name_msg[NAME_SIZE + BUF_SIZE];
    int str_len;
    while (1)
    {
        // 서버에서 들어온 메세지 수신
        str_len = read(sock, name_msg, NAME_SIZE + BUF_SIZE - 1);
        // str_len 이 -1 이라는 건, 서버 소켓과 연결이 끊어졌다는 뜻임
        // 왜 끊어졌는가? send_msg 에서 close(sock) 이 실행되고,
        // 서버로 EOF 가 갔으면, 서버는 그걸 받아서 "자기가 가진" 클라이언트 소켓을 close 할 것
        // 그러면 read 했을 때 결과가 -1 일 것.
        if (str_len == -1)
        // 종료를 위한 리턴값. thread_return 으로 갈 것
        return (void *)-1; // pthread_join를 실행시키기 위해
      
        // 버퍼 맨 마지막 값 NULL
        name_msg[str_len] = 0;
        // 받은 메세지 출력
        fputs(name_msg, stdout);
    }
    return NULL;
}

