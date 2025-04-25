#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
#include<time.h>
#include<stdlib.h>
#define NORMAL_SIZE 20
#define BUF_SIZE 100
#define MAX_CLNT 100
#define MAX_IP 30
#define CLEARBUFFER clearbuffer();





typedef struct Config{
    char ip[20];
    char port[10];
    char nick[100];
}network_config;









//서버
void config_read(network_config* config, char* path);
void * handle_clnt(void *arg);      //클라이언트 수 체크, 제어
void send_msg_to_all(char *msg, int len);  //메세지 보내기
void error_handling(char *msg);     //에러메시지 출력
char* serverState(int count);       //서버상태 확인
void menu(char port[]);             //메뉴 출력
//클라이언트
void* send_msg_1(void* arg);        //메세지 보내기
void* recv_msg_1(void* arg);        //메세지 받기
void error_handling_1(char* msg);   //에러메시지 출력

void clearbuffer(){                 //클리어버퍼
    while(getchar() != '\n'){
        getchar();
    }
}
 
void menu_1();                      //클라이언트 메뉴 출력
void changeName_1();                //이름바꾸기
void menuOptions_1();               //메뉴옵션
void* send_msg_server(void* arg);
 
/****************************/
 
char name[NORMAL_SIZE]="[방장]";       // name
char msg_form[NORMAL_SIZE];            // msg form
char serv_time[NORMAL_SIZE];           // server time
char msg[BUF_SIZE];                    // msg
char serv_port[NORMAL_SIZE];           // server port number
char clnt_ip[NORMAL_SIZE];             // client ip address
 
int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

int serv_sock;





int main(int argc, char *argv[])
{
    int clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    int clnt_adr_sz;
    pthread_t t_id;
    char choice[100];


    //서버일때`
    if (strcmp(argv[1], "-s") == 0){

        int serv_sock, clnt_sock;
        struct sockaddr_in serv_adr, clnt_adr;
        int clnt_adr_sz;
        pthread_t t_id;
        void* thread_return;

        int sock;
        struct sockaddr_in serv_addr;
        pthread_t snd_thread, rcv_thread;
     
        /** time log **/
        struct tm *t;
        time_t timer = time(NULL);
        t=localtime(&timer);
     

        if (argc != 4)  //인자갯수가 올바르지 않다면,
        {
            printf(" Usage : %s <-s/-c> <-p> <portnum>\n", argv[0]);    //최초실행시 argv[0]은 server_c(실행파일 이름)
            exit(1);    //프로그램 실행을 위한 입력방식을 안내하고, 프로그램 종료.
        }
     

        menu(argv[3]);  //menu에 포트번호 전달 
     
        pthread_mutex_init(&mutx, NULL);    //mutex 초기화.
        
        //[1] 소켓 생성 socket()    - 서버소켓 생성
        serv_sock= socket(PF_INET, SOCK_STREAM, 0);      
        
        //serv_adr 초기화.          - 
        memset(&serv_adr, 0, sizeof(serv_adr));
        serv_adr.sin_family=AF_INET;
        serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
        serv_adr.sin_port=htons(atoi(argv[3]));
     
        //[2] bind - 소켓에 주소정보 할당
        if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)     
            error_handling("bind() error");

        //[3] listen - 수신연결
        if (listen(serv_sock, 5)==-1)       
            error_handling("listen() error");
        
        
        while(1)
        {

            clnt_adr_sz=sizeof(clnt_adr);
            //[4] accept - 수신연결요청 수락상태로 설정.
            clnt_sock= accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);     

            pthread_mutex_lock(&mutx);
            clnt_socks[clnt_cnt++]=clnt_sock;   //clnt_socks 배열에 clnt_sock 정보 저장.
            pthread_mutex_unlock(&mutx);
            
            //clnt_sock: 클라이언트 fd정보.
            pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);    //메세지 보내기
            pthread_create(&t_id, NULL, send_msg_server, (void*)&clnt_sock);
            pthread_detach(t_id);         //쓰레드 메모리할당 해제 함수

            printf(" Connceted client IP : %s ", inet_ntoa(clnt_adr.sin_addr));     //inet_ntoa함수: 32비트 ip주소를 문자열로 바꿔줌.
            printf(" chatter (%d/100)\n", clnt_cnt);    //클라이언트 수 카운트 후 출력.


        }


    
        close(serv_sock);
        return 0;
    }
    
    //클라이언트일때
    else if (strcmp(argv[1], "-c") == 0){
        int sock;                         //소켓
        struct sockaddr_in serv_addr;     //서버주소
        pthread_t snd_thread, rcv_thread; //send쓰레드, receive쓰레드
        void* thread_return;              //쓰레드 리턴
        char port[10], ip[30], nick[100];
        network_config myconf;

        if (argc<6)    //인자의 갯수가 6개가 아닐 때,
        {
            char answer[1];

            printf("클라이언트 실행하기 위한 매개변수가 부족합니다.");
            printf("미리 설정된 정보를 사용하시겠습니까?");
            scanf("%c", answer);
            
            //유저가 conf파일 사용을 원함
            if(strcmp(answer, "y")==0)
            {
            
                char *path ="./config.conf";
                network_config myconf;    
                config_read(&myconf, path);

                printf("1> ip: %s\n",myconf.ip);
                printf("2> port: %s\n",myconf.port);
                printf("3> nic: %s\n",myconf.nick);
                
                strcpy(port, myconf.port);
                strcpy(ip, myconf.ip);
                strcpy(nick, myconf.nick);

            }
            else{
                 //유저가 conf파일 사용을 원치 않음
                printf(" Usage : %s <-s/-c> <-p> <port> <ip> <name>\n", argv[0]);
                exit(1);

            }

        //인자가 우리가 설계한 대로 매개변수를 다 입력했을시에
        }else{
            strcpy(port, argv[3]);
            strcpy(ip, argv[4]);
            strcpy(nick, argv[5]);
        }
    
        /** local time **/
        struct tm *t;
        time_t timer = time(NULL);
        t=localtime(&timer);
        sprintf(serv_time, "%d-%d-%d %d:%d", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour,
        t->tm_min);
    
        sprintf(name, "[%s]", nick);         //argv[5]번인 이름을 버퍼에 출력한다.
        sprintf(clnt_ip, "%s", ip);        //argv[4]  ~  ip번호    ~
        sprintf(serv_port, "%s", port);      //argv[3]  ~  포트번호   ~
        sock=socket(PF_INET, SOCK_STREAM, 0);   //socket함수를 통해 socket을 생성.
    
        memset(&serv_addr, 0, sizeof(serv_addr));     //서버주소를 모두 0으로 초기화.
        serv_addr.sin_family=AF_INET;                 //주소 체계: AF_INET
        serv_addr.sin_addr.s_addr=inet_addr(clnt_ip); //32 비트 IP 주소
        serv_addr.sin_port=htons(atoi(serv_port));      //포트번호(0~65535)
    
        /*connect 함수: 연결 성공 시 0 리턴 , 연결 실패 시 0보다 작은 값 리턴.*/
        if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
            error_handling_1(" connect() error ");
    
        /** call menu for client **/
        menu_1();
    
        /** 스레드 생성 1. 구분을 위한id, null(기본) , 함수주소, 앞의 함수에 전달될 인자변수의 주소 */
        pthread_create(&snd_thread, NULL, send_msg_1, (void*)&sock);
        pthread_create(&rcv_thread, NULL, recv_msg_1, (void*)&sock);
        /** 스레드 종료 대기  */
        pthread_join(snd_thread, &thread_return);
        pthread_join(rcv_thread, &thread_return);
        //https://www.ibm.com/docs/ko/aix/7.3?topic=programming-joining-threads
        close(sock);
        return 0;
    }
    else 
    printf(" Usage : %s <-s/-c> <-p> <port> <ip> <name>\n", argv[0]);
    exit(1);    //프로그램 실행을 위한 입력방식을 안내하고, 프로그램 종료.
}
 
void *handle_clnt(void *arg)
{
    int clnt_sock=*((int*)arg);
    int str_len=0, i;
    char msg[BUF_SIZE];
 
    while((str_len= read(clnt_sock, msg, sizeof(msg)))!=0)
        send_msg_to_all(msg, str_len);
 
    // remove disconnected client
    pthread_mutex_lock( &mutx );
    for (i=0; i<clnt_cnt; i++)
    {
        if (clnt_sock == clnt_socks[i])
        {
            while( i++ < clnt_cnt-1 )
                clnt_socks[i] = clnt_socks[i+1];
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);
    close(clnt_sock);
    return NULL;
}
 
//각각의 클라이언트의 소켓정보에 맞게, 클라이언트의 수만큼 메세지를 전송하는 함수.
void send_msg_to_all(char* msg, int len)
{
    int i;
    pthread_mutex_lock(&mutx);
    for (i=0; i<clnt_cnt; i++)
        write(clnt_socks[i], msg, len);
    write(serv_sock, msg, len);
    pthread_mutex_unlock(&mutx);
}
 
void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
 
char* serverState(int count)
{
    char* stateMsg = malloc(sizeof(char) * 20);
    strcpy(stateMsg ,"None");
    
    if (count < 5)
        strcpy(stateMsg, "Good");
    else
        strcpy(stateMsg, "Bad");
    
    return stateMsg;
}        
 
void menu(char port[])  //argv의 포트번호를 전달받는다.
{
    system("clear");
    printf(" **** moon/sun chat server ****\n");
    printf(" server port    : %s\n", port);
    printf(" server state   : %s\n", serverState(clnt_cnt));
    printf(" max connection : %d\n", MAX_CLNT);
    printf(" ****          Log         ****\n\n");



    
}


void* send_msg_server(void* arg) //쓰레드 안에서 구동하는 메세지 보내기 함수.
{
    int sock=*((int*)arg);
    char name_msg[NORMAL_SIZE+BUF_SIZE];
    char myInfo[BUF_SIZE];
    char* who = NULL;
    char temp[BUF_SIZE];
 
    /** send join messge **/
    printf(" >> join the chat !! \n");
    sprintf(myInfo, "%s's join. IP_%s\n",name , clnt_ip);
    write(sock, myInfo, strlen(myInfo));
 
    while(1)
    {
        fgets(msg, BUF_SIZE, stdin);
 
        // menu_mode command -> !menu
        if (!strcmp(msg, "!menu\n"))
        {
            menuOptions_1();
            continue;
        }
        else if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
        {
            close(sock);
            
            exit(0);
        }
 
        // send message
        
        for (int i = 0; i < clnt_cnt; i++)
        {
            sprintf(name_msg, "%s %s", name,msg);
            write(clnt_socks[i], name_msg, strlen(name_msg));
            
        }
        
        
    }
    return NULL;
}
 

// 클라이언트
void* send_msg_1(void* arg) //쓰레드 안에서 구동하는 메세지 보내기 함수.
{
    int sock=*((int*)arg);
    char name_msg[NORMAL_SIZE+BUF_SIZE];
    char myInfo[BUF_SIZE];
    char bye_msg[BUF_SIZE];
    char* who = NULL;
    char temp[BUF_SIZE];
 
    /** send join messge **/
    printf(" >> join the chat !! \n");
    sprintf(myInfo, "%s's join. IP_%s\n",name , clnt_ip);
    write(sock, myInfo, strlen(myInfo));
 
    while(1)
    {
        fgets(msg, BUF_SIZE, stdin);
 
        // menu_mode command -> !menu
        if (!strcmp(msg, "!menu\n"))    //msg는 입력.
        {
            menuOptions_1();
            continue;
        }
 
        else if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
        {
            sprintf(bye_msg, "%s's exit. IP_%s\n",name , clnt_ip);
            write(sock, bye_msg, strlen(bye_msg));
            close(sock);
            exit(0);
        }
 
        // send message
        sprintf(name_msg, "%s %s", name,msg);
        write(sock, name_msg, strlen(name_msg));
    }
    return NULL;
}
 
void* recv_msg_1(void* arg)
{
    int sock=*((int*)arg);
    char name_msg[NORMAL_SIZE+BUF_SIZE];
    int str_len;
 
    while(1)
    {
        str_len=read(sock, name_msg, NORMAL_SIZE+BUF_SIZE-1);
        if (str_len==-1)
            return (void*)-1;
        name_msg[str_len]=0;
        fputs(name_msg, stdout);
    }
    return NULL;
}
 
 
void menuOptions_1() 
{
    int select;
    // print menu
    printf("\n\t**** menu mode ****\n");
    printf("\t1. change name\n");
    printf("\t2. clear/update\n\n");
    printf("\tthe other key is cancel");
    printf("\n\t*******************");
    printf("\n\t>> ");
    scanf("%d", &select);
    getchar();
    switch(select)
    {
        // change user name
        case 1:
        changeName_1();
        break;
 
        // console update(time, clear chatting log)
        case 2:
        menu_1();
        break;
 
        // menu error
        default:
        printf("\tcancel.");
        break;
    }
}
 
 
/** change user name **/
void changeName_1()
{
    char nameTemp[100];
    printf("\n\tInput new name -> ");
    scanf("%s", nameTemp);
    sprintf(name, "[%s]", nameTemp);
    printf("\n\tComplete.\n\n");
}
 
void menu_1()
{
    system("clear");
    printf(" **** moon/sum chatting client ****\n");
    printf(" server port : %s \n", serv_port);
    printf(" client IP   : %s \n", clnt_ip);
    printf(" chat name   : %s \n", name);
    printf(" server time : %s \n", serv_time);
    printf(" ************* menu ***************\n");
    printf(" if you want to select menu -> !menu\n");
    printf(" 1. change name\n");
    printf(" 2. clear/update\n");
    printf(" **********************************\n");
    printf(" Exit -> q & Q\n\n");
}    
 
void error_handling_1(char* msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}


void config_read(network_config* config, char* path)
{

    FILE* in = NULL;  
    in =  fopen(path, "r");
    char* ptr[256];
    char* str[128];
    // 파일이 안열린다.
    if(in ==0){
        printf("파일을 열 수 없습니다.");
        exit(1);
    }else{
        
        const int max = 100;
        char line[max], ip[20], port_[20], nic_name[50];
        
    
        // printf("허허?");
        fgets(line,max,in);
        sscanf(line, "%s %s %s", ip, port_,nic_name);
        // printf("%s\n", line);
        
        strcpy(config->ip, ip);
        
        strcpy(config->port, port_);
        
        strcpy(config->nick, nic_name);          
    }

}