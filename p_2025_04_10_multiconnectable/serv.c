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
//서버
void * handle_clnt(void *arg);
void send_msg(char *msg, int len);
void error_handling(char *msg);
char* serverState(int count);
void menu(char port[]);
//클라이언트
void* send_msg_1(void* arg);
void* recv_msg_1(void* arg);
void error_handling_1(char* msg);
 
void menu_1();
void changeName_1();
void menuOptions_1(); 
 
/****************************/
 
char name[NORMAL_SIZE]="[DEFALT]";     // name
char msg_form[NORMAL_SIZE];            // msg form
char serv_time[NORMAL_SIZE];        // server time
char msg[BUF_SIZE];                    // msg
char serv_port[NORMAL_SIZE];        // server port number
char clnt_ip[NORMAL_SIZE];            // client ip address
 
int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

int main(int argc, char *argv[])
{
    char choice[100];
    printf("명령어를 입려해주세요.\n");
    printf("서버접속하기 -s\n클라이언트 접속하기 -c\n");
    scanf("%s",choice);
    
    if (strcmp(choice,"-s") == 0){
        int serv_sock, clnt_sock;
        struct sockaddr_in serv_adr, clnt_adr;
        int clnt_adr_sz;
        pthread_t t_id;
        void* thread_return;

        int sock;
        // struct sockaddr_in serv_addr;
        pthread_t snd_thread, rcv_thread;
     
        /** time log **/
        struct tm *t;
        time_t timer = time(NULL);
        t=localtime(&timer);
     
        if (argc != 2)
        {
            printf(" Usage : %s <port>\n", argv[0]);
            exit(1);
        }
     
        menu(argv[1]);
     
        pthread_mutex_init(&mutx, NULL);
        serv_sock=socket(PF_INET, SOCK_STREAM, 0);
        
        memset(&serv_adr, 0, sizeof(serv_adr));
        serv_adr.sin_family=AF_INET;
        serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
        serv_adr.sin_port=htons(atoi(argv[1]));
     
        if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
            error_handling("bind() error");
        if (listen(serv_sock, 5)==-1)
            error_handling("listen() error");
     
        while(1)
        {
            t=localtime(&timer);
            clnt_adr_sz=sizeof(clnt_adr);
            clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
     
            pthread_mutex_lock(&mutx);
            clnt_socks[clnt_cnt++]=clnt_sock;
            pthread_mutex_unlock(&mutx);
     
            pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
            pthread_detach(t_id);
            printf(" Connceted client IP : %s ", inet_ntoa(clnt_adr.sin_addr));
            printf("(%d-%d-%d %d:%d)\n", t->tm_year+1900, t->tm_mon+1, t->tm_mday,
            t->tm_hour, t->tm_min);
            printf(" chatter (%d/100)\n", clnt_cnt);

            
            

            /*here*/
            pthread_create(&snd_thread, NULL, send_msg_1, (void*)&clnt_sock);
            pthread_create(&rcv_thread, NULL, recv_msg_1, (void*)&clnt_sock);
            pthread_join(snd_thread, &thread_return);
            pthread_join(rcv_thread, &thread_return);

        }
        close(serv_sock);
        return 0;
    }
    else if (strcmp(choice,"-c") == 0){
        int sock;
        struct sockaddr_in serv_addr;
        pthread_t snd_thread, rcv_thread;
        void* thread_return;
    
        if (argc!=4)
        {
            printf(" Usage : %s <ip> <port> <name>\n", argv[0]);
            exit(1);
        }
    
        /** local time **/
        struct tm *t;
        time_t timer = time(NULL);
        t=localtime(&timer);
        sprintf(serv_time, "%d-%d-%d %d:%d", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour,
        t->tm_min);
    
        /** 닉네임 */
        sprintf(name, "[%s]", argv[3]);
        /**클라아이피 */
        sprintf(clnt_ip, "%s", argv[1]);
        /** 접속할 서버의 포트 */
        sprintf(serv_port, "%s", argv[2]);
        /* 소켓 생성*/
        sock=socket(PF_INET, SOCK_STREAM, 0);
    
        /*정보 세팅*/
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family=AF_INET;
        serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
        serv_addr.sin_port=htons(atoi(argv[2]));
    
        /* 연결 */
        if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
            error_handling_1(" conncet() error");
    
        /** call menu **/
        menu_1();
            
        /** 스레드 생성 1. 구분을 위한id, null(기본) , 함수주소, 앞의 함수에 전달될 인자변수의 주소 */
        pthread_create(&snd_thread, NULL, send_msg_1, (void*)&sock);
        pthread_create(&rcv_thread, NULL, recv_msg_1, (void*)&sock);
        /** 스레드 종료 대기  */
        pthread_join(snd_thread, &thread_return);
        pthread_join(rcv_thread, &thread_return);
        close(sock);
        return 0;
    }
    else 
        printf("다시입력해주세요,");
        system("clear");
}
 
void *handle_clnt(void *arg)
{
    int clnt_sock=*((int*)arg);
    int str_len=0, i;
    char msg[BUF_SIZE];
 
    while((str_len=read(clnt_sock, msg, sizeof(msg)))!=0)
    {
        printf("%s",msg);
        send_msg(msg, str_len);
        
    }   
 
    // remove disconnected client
    pthread_mutex_lock(&mutx);
    for (i=0; i<clnt_cnt; i++)
    {
        if (clnt_sock==clnt_socks[i])
        {
            while(i++<clnt_cnt-1)
                clnt_socks[i]=clnt_socks[i+1];
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);
    close(clnt_sock);
    return NULL;
}
 
void send_msg(char* msg, int len)
{
    int i;
    pthread_mutex_lock(&mutx);
    for (i=0; i<clnt_cnt; i++)
        write(clnt_socks[i], msg, len);
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
 
void menu(char port[])
{
    system("clear");
    printf(" **** moon/sun chat server ****\n");
    printf(" server port    : %s\n", port);
    printf(" server state   : %s\n", serverState(clnt_cnt));
    printf(" max connection : %d\n", MAX_CLNT);
    printf(" ****          Log         ****\n\n");

    
}
// 클라이언트
void* send_msg_1(void* arg)
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
        sprintf(name_msg, "%s %s", name,msg);

        // printf("[log]send_msg_1 257line %s", name_msg);

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
        // printf("[log]recv_msg_! 279line %s", name_msg);
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
