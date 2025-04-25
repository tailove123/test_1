#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
 
#define BUF_SIZE 256	// 메시지 버퍼 길이
#define NAME_SIZE  256 	// 방이름의 길이

void * send_msg(void * arg);	// 쓰레드 함수 - 송신용
void * recv_msg(void * arg);	// 쓰레드 함수 - 수신용
void error_handling(char * msg);	// 예외처리 함수

char name[NAME_SIZE]="[DEFAULT]";	// 방 이름 - 초기값은 "[DEFAULT]"
char msg[BUF_SIZE];			// 메시지 버퍼

int main(int argc, char *argv[])	// 인자로 IP와 port 를 받는다
{
 int sock;				// 통신용 소켓 파일 디스크립터
 struct sockaddr_in serv_addr;	// 서버 주소 구조체 변수
 pthread_t snd_thread, rcv_thread;	// 송신 쓰레드, 수신 쓰레드
 void * thread_return;		// 쓰레드 반환 갑인가
 if(argc!=4) {				// 사용자가 프로그램 잘 못 시작시켰으면
  printf("Usage : %s <IP> <port> <name>\n", argv[0]);	// 사용법 안내
  exit(1);				// 프로그램 비정상 종료
  }

 sprintf(name, "[%s]", argv[3]);			// 사용자의 이름
 sock=socket(PF_INET, SOCK_STREAM, 0);	// TCP 통신 소켓 설정

 memset(&serv_addr, 0, sizeof(serv_addr));	// 접속할 서버 주소 설정
 serv_addr.sin_family=AF_INET;
 serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
 serv_addr.sin_port=htons(atoi(argv[2]));

// 서버에 접속 시도
 if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
  error_handling("connect() error");

 pthread_create(&snd_thread, NULL, send_msg, (void*)&sock); // 송신 쓰레드 시작
 pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock); //  수신 쓰레드 시작
 pthread_join(snd_thread, &thread_return); // 메인 함수는 아무것도 안하고 기다린다.
 pthread_join(rcv_thread, &thread_return); // 통신은 2개의 쓰레드가 처리한다.
 close(sock); // 프로그램 종료할 때 소켓 소멸 시키고 종료
 return 0;
}

void * send_msg(void * arg)   // send thread main
{
 int sock=*((int*)arg);	// 소켓을 받는다.
 char name_msg[BUF_SIZE];
 while(1)	// 무한 루프 돌면서
 {
  fgets(msg, BUF_SIZE, stdin);	// 키보드 입력을 받아서
  if(!strcmp(msg,"q\n")||!strcmp(msg,"Q\n"))	// q나 Q면 종료하고
  {
   close(sock);
   exit(0);
  }
	// 정상 적인 입력을 하면
  sprintf(name_msg,"%s %s", name, msg); // 이름 + 공백 + 메시지 순으로 전송
  write(sock, name_msg, BUF_SIZE);	// 서버로 메시지 보내기
 }
 return NULL;
}

// 수신쪽 쓰레드 함수
void * recv_msg(void * arg)   // read thread main
{
 int sock=*((int*)arg);	// 통신용 소켓을 받고
 char name_msg[BUF_SIZE];	// 이름 + 메시지 버퍼
 int str_len;			// 문자열 길이
 while(1)	// 무한 루프 돌면서
 {
  str_len=read(sock, name_msg, BUF_SIZE);	// 메시지가 들어오면
  if(str_len==-1)	// 만약 통신이 끊겼으면
   return (void*)-1;	// 쓰레드 종료
  fputs(name_msg, stdout);	// 화면에 수신된 메시지 표시
 }
 return NULL;
}

// 예외 처리 함수
void error_handling(char *msg)
{
 fputs(msg, stderr);	// 오류 메시지를 표시하고
 fputc('\n', stderr);
 exit(1);		// 비정상 종료 처리
}