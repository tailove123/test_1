#include <stdio.h>              // 표준 입출력
#include <stdlib.h>             // 메모리/문자열 함수
#include <string.h>             // 문자열 처리 함수
#include <unistd.h>             // read(), write(), close() 등
#include <netinet/in.h>         // sockaddr_in 구조체 정의
#include <arpa/inet.h>          // inet_pton() 함수
#include <time.h>
#pragma pack(1)
#define SIZE 100
#define MAX_BOOKS 11000
#define PORT 9000  // 서버 포트
#define MAX_LOANS 100  // 대출 가능한 최대 도서 수
#define MAX_USERS 500
#define CLEARBUFFER clearbuffer();
#define PRESSENTER  pressenter();
#define CLEAR system("clear");

// 서버와 동일한 도서 구조체 정의


typedef struct {
    int no;                     // No
    char title[SIZE];            // 제목
    char author[SIZE];            // 저자
    char publisher[SIZE];              // 출판사
    int pub_year;              //출판년
    int num_books;            //권
    char isbn[SIZE];                 //ISBN
    char extra_n[SIZE];              //부가기호
    char kdc[SIZE];                //KDC
    char kdc_subject[SIZE];        //KDC 주제명
    int loan_frequency;      //대출 빈도
} Book;


typedef struct {
    char id[SIZE]; // id
    char title[SIZE];            // 제목
    char author[SIZE];            // 저자
    char publisher[SIZE];              // 출판사
    int pub_year;              //출판년
    int num_books;            //권
    char isbn[SIZE];                 //ISBN
    int loan_time;      // 대출시간
    char status[SIZE];        // 승인상태
    char loan_addr[SIZE]; // 주소
} Book2;

typedef struct {
    char id[50];
    char pw[50];
    char name[50];
    int age;
    char phone[50];
    char addr[50];
    int msc; //메세지 카운트
    int bsc; //불량자카운트
}User;
//유저 아이디 이름 나이 핸드폰번호 주소 
typedef struct {

    char date[100];
    char day_[50];
    int is_open;  // 1이면 영업 0이면 휴일
}bussiness_month;

void pressenter();
void clearbuffer();
void client_message(int sock, char *user_id);
void librarian_message(int sock);

void clear_newline(char *str) {

    str[strcspn(str, "\n")] = 0; // Remove the newline character
}

void open_close(){ // 오픈마감 시간 <<
    time_t rawtime;
    struct tm * timeinfo;

    // 현재 시간 얻기
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    int hour = timeinfo->tm_hour;
    int minute = timeinfo->tm_min;


    // 운영시간: 08:00 ~ 17:59
    if (hour >= 8 && hour < 18) {
        // printf("운영 중입니다.\n");
    } else {
        printf("마감 시간입니다.\n");
        exit(0);
    }
}

int main() {
    int sock;
    struct sockaddr_in serv_addr = {0};
    char choice[10], id[50], pw[50];
    char nickname[50], phone[50], address[100];
    int year;
    int message_a = 1; // 메세지 보낼수있는 횟수
    int bull_count = 0; // 불량자 구별카운트
    int plag_exit = 0;
    int d_count = 0;
    Book books[MAX_BOOKS];
    Book2 d_results[MAX_LOANS];  // 대출한 책 저장용
    User Users[MAX_USERS];
    
    while(1)
    {
        printf("1.로그인\n2.회원가입>> ");
        fgets(choice, sizeof(choice), stdin);
        clear_newline(choice); // Remove the newline from choice input
        printf("ID: "); fgets(id, sizeof(id), stdin); clear_newline(id);
        printf("PW: "); fgets(pw, sizeof(pw), stdin); clear_newline(pw);
        plag_exit = 0;

        while(plag_exit == 0)
        {
            // 회원가입일 경우 추가 정보 입력
            if (choice[0] == '2')
            {
                printf("이름: "); fgets(nickname, sizeof(nickname), stdin); clear_newline(nickname);
                printf("나이: "); scanf("%d", &year); getchar();
                if (year < 10)
                {
                    printf("10살 이하는 가입이안됩니다.\n");
                    break;
                }
                printf("전화번호: "); fgets(phone, sizeof(phone), stdin); clear_newline(phone);
                printf("주소: "); fgets(address, sizeof(address), stdin); clear_newline(address);
            }

            sock = socket(PF_INET, SOCK_STREAM, 0);
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(PORT);
            inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
            connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

            if (choice[0] == '1') send(sock, "login", 16, 0);
            else send(sock, "register", 16, 0);
            send(sock, id, sizeof(id), 0);
            send(sock, pw, sizeof(pw), 0);

            // 회원가입일 경우 추가 정보 전송
            if (choice[0] == '2')
            {
                send(sock, nickname, sizeof(nickname), 0);
                send(sock, &year, sizeof(int), 0);
                send(sock, phone, sizeof(phone), 0);
                send(sock, address, sizeof(address), 0);
                send(sock, &message_a, sizeof(int), 0);
                send(sock, &bull_count, sizeof(int), 0);
                printf("회원가입이 완료되었습니다.\n");
                break;
            }
            int result;
            read(sock, &result, sizeof(int));
            if (result != 1 && result != 2 && result != 3)
            {
                printf("없는 계정이거나 아이디 혹은 비밀번호가 틀리셨습니다.\n");
                close(sock);
                sleep(1);
                system("clear");
                break;
            }

            printf("성공적으로 로그인이 완료되었습니다.\n");

    // 명령어 루프
            if (result == 1)
            {
                while (plag_exit == 0)
                {
                    char cmd[16];
                    int num = 0;
                    printf("\n1.내정보확인하기\n2.온라인대출하기\n3.메세지보내기\n4.로그아웃");
                    fgets(cmd, sizeof(cmd), stdin);
                    cmd[strcspn(cmd, "\n")] = 0;
                    send(sock, cmd, sizeof(cmd), 0);
                    if (strcmp(cmd,"1") == 0)
                    {
                        User user_info;

                        send(sock, id, sizeof(id), 0);
                        
                        //서버로부터 응답을 받음
                        read(sock, user_info.id, sizeof(user_info.id));
                        read(sock, user_info.name, sizeof(user_info.name));
                        read(sock, &user_info.age, sizeof(int));
                        read(sock, user_info.phone, sizeof(user_info.phone));
                        read(sock, user_info.addr, sizeof(user_info.addr));
                        

                        printf("id :%s\n",id);
                        printf("유저명 :%s\n",user_info.name);
                        printf("나이 :%d\n",user_info.age);
                        printf("전화번호 :%s\n",user_info.phone);
                        printf("주소 :%s\n",user_info.addr);

                        //내 정보를 화면에 출력 + 내 대출목록 

                    }
                    else if (strcmp(cmd, "2") == 0)
                    {
                        char key[50], val[100];
                        printf("검색 기준(title/author): ");
                        fgets(key, sizeof(key), stdin);
                        key[strlen(key) - 1] = '\0';
                        // key[strcspn(key, "\n")] = 0;
                        printf("검색어 입력: ");
                        fgets(val, sizeof(val), stdin);
                        val[strlen(val) - 1] = '\0';
                        // val[strcspn(val, "\n")] = 0;
                        send(sock, key, sizeof(key), 0);
                        send(sock, val, sizeof(val), 0);
                        int count;

                        // memset(p_results, 0, sizeof(Book));
                        read(sock, &count, sizeof(int));
                        Book *p_results = NULL;
                        p_results = malloc(sizeof(Book)*count);
                        if (count>0)
                        {
                            for (int i = 0; i < count; i++)
                            {
                                read(sock, &p_results[i], sizeof(Book));
                            }
                            for (int i = 0; i < count; i++)
                            {
                                printf("[%d] 제목 : %s 글쓴이 : %s 출판사 : %s 출판년 : %d ISBN %s\n", i, p_results[i].title, p_results[i].author, p_results[i].publisher, p_results[i].pub_year,p_results[i].isbn);
                                
                            }
                        }
                        else
                        printf("검색 결과가 없습니다.\n");
                    }
                    else if (strcmp(cmd, "3") == 0)             //3. 메세지
                    {
                        // printf("message\n");
                        client_message(sock, id);
                        CLEARBUFFER
                    } 
                    else if (strcmp(cmd, "4") == 0)
                    {
                        plag_exit = 1;
                        break;
                    }
                }
            }
            else if (result == 2)
            {
                while(plag_exit == 0){
                    printf("관리자님 환영합니다.\n");
                    printf("1.도서관리\n2.모든계정관리\n3.도서관오픈관리\n4.대출자정보\n5.로그아웃\n");
                    char cmd[16];
                    fgets(cmd, sizeof(cmd), stdin);
                    cmd[strcspn(cmd, "\n")] = 0;
                    send(sock, cmd, sizeof(cmd), 0);
                    if (strcmp(cmd, "1") == 0) // 도서관리
                    {   
                        char cmp[16];
                        printf("1.도서목록\n2.도서검색\n3.도서수정\n4.도서추가\n5.도서삭제\n6.돌아가기");
                        fgets(cmp, sizeof(cmp), stdin);
                        cmp[strcspn(cmp, "\n")] = 0;
                        send(sock, cmp, sizeof(cmp), 0);
                        if(strcmp(cmp, "1") == 0)
                        {
                            int count;
                            read(sock, &count, sizeof(int));
                            Book *all = NULL;
                            all = malloc(sizeof(Book)*count);
                            if (count>0)
                            {
                                for (int i = 0; i < count; i++)
                                {
                                    read(sock, &all[i], sizeof(Book));
                                }
                                for (int i = 0; i < count; i++)
                                {
                                    printf(" %d 제목: %s ,저자: %s ,출판사: %s ,출판년: %d ,권: %d ,ISBN: %s ,부가기호: %s ,KDC: %s ,KDC주제명: %s ,대출빈도: %d\n", 
                                    all[i].no, all[i].title, all[i].author, all[i].publisher, all[i].pub_year,
                                    all[i].num_books, all[i].isbn, all[i].extra_n, all[i].kdc, all[i].kdc_subject, all[i].loan_frequency);
                                }
                            }
                            else
                            printf("검색 결과가 없습니다.\n");
                            free(all);
                        }
                        else if(strcmp(cmp, "2") == 0) // 검색기능
                        {
                            char key[50], val[100];
                            printf("검색 기준(title/author): ");
                            fgets(key, sizeof(key), stdin);
                            key[strlen(key) - 1] = '\0';
                            // key[strcspn(key, "\n")] = 0;
                            printf("검색어 입력: ");
                            fgets(val, sizeof(val), stdin);
                            val[strlen(val) - 1] = '\0';
                            // val[strcspn(val, "\n")] = 0;
                            send(sock, key, sizeof(key), 0);
                            send(sock, val, sizeof(val), 0);
                            int count;
    
                            // memset(p_results, 0, sizeof(Book));
                            read(sock, &count, sizeof(int));
                            Book *p_results = NULL;
                            p_results = malloc(sizeof(Book)*count);
                            if (count>0)
                            {
                                for (int i = 0; i < count; i++)
                                {
                                    read(sock, &p_results[i], sizeof(Book));
                                }
                                for (int i = 0; i < count; i++)
                                {
                                    printf("[%d] 제목 : %s 글쓴이 : %s 출판사 : %s 출판년 : %d ISBN %s\n", i-1, p_results[i].title, p_results[i].author, p_results[i].publisher, p_results[i].pub_year,p_results[i].isbn);
                
                                }
                            }
                            else
                            printf("검색 결과가 없습니다.\n");
                            free(p_results);
                        }
                        else if (strcmp(cmp, "3") == 0) //도서수정
                        {
                            Book b;
                            printf("수정할 도서 ISBN: "); fgets(b.isbn, sizeof(b.isbn), stdin); b.isbn[strcspn(b.isbn, "\n")] = 0;
                            printf("새 No: "); scanf("%d", &b.no); getchar();
                            printf("새 제목: "); fgets(b.title, sizeof(b.title), stdin); b.title[strcspn(b.title, "\n")] = 0;
                            printf("새 저자: "); fgets(b.author, sizeof(b.author), stdin); b.author[strcspn(b.author, "\n")] = 0;
                            printf("새 출판사: "); fgets(b.publisher, sizeof(b.publisher), stdin); b.publisher[strcspn(b.publisher, "\n")] = 0;
                            printf("새 출판년: "); scanf("%d", &b.pub_year); getchar();
                            printf("새 권 수: "); scanf("%d", &b.num_books); getchar();
                            printf("새 부가기호: "); fgets(b.extra_n, sizeof(b.extra_n), stdin); b.extra_n[strcspn(b.extra_n, "\n")] = 0;
                            printf("새 KDC: "); fgets(b.kdc, sizeof(b.kdc), stdin); b.kdc[strcspn(b.kdc, "\n")] = 0;
                            printf("새 KDC 주제: "); fgets(b.kdc_subject, sizeof(b.kdc_subject), stdin); b.kdc_subject[strcspn(b.kdc_subject, "\n")] = 0;
                            printf("새 대출빈도: "); scanf("%d", &b.loan_frequency); getchar();
    
                            send(sock, &b, sizeof(Book), 0);
                            read(sock, &result, sizeof(int));
                            printf(result ? "수정 성공\n" : "수정 실패\n");
    
                        }
                        else if (strcmp(cmp, "4") == 0) // 도서추가
                        {
                            Book b;
                            printf("No: "); scanf("%d", &b.no); getchar();
                            printf("제목: "); fgets(b.title, sizeof(b.title), stdin); b.title[strcspn(b.title, "\n")] = 0;
                            printf("저자: "); fgets(b.author, sizeof(b.author), stdin); b.author[strcspn(b.author, "\n")] = 0;
                            printf("출판사: "); fgets(b.publisher, sizeof(b.publisher), stdin); b.publisher[strcspn(b.publisher, "\n")] = 0;
                            printf("출판년: "); scanf("%d", &b.pub_year); getchar();
                            printf("권 수: "); scanf("%d", &b.num_books); getchar();
                            printf("ISBN: "); fgets(b.isbn, sizeof(b.isbn), stdin); b.isbn[strcspn(b.isbn, "\n")] = 0;
                            printf("부가기호: "); fgets(b.extra_n, sizeof(b.extra_n), stdin); b.extra_n[strcspn(b.extra_n, "\n")] = 0;
                            printf("KDC: "); fgets(b.kdc, sizeof(b.kdc), stdin); b.kdc[strcspn(b.kdc, "\n")] = 0;
                            printf("KDC 주제: "); fgets(b.kdc_subject, sizeof(b.kdc_subject), stdin); b.kdc_subject[strcspn(b.kdc_subject, "\n")] = 0;
                            printf("대출빈도: "); scanf("%d", &b.loan_frequency); getchar();
    
                            send(sock, &b, sizeof(Book), 0);
                            read(sock, &result, sizeof(int));
                            printf(result ? "도서 추가 성공\n" : "도서 추가 실패\n");
    
                        }
                        else if (strcmp(cmp, "5") == 0) // 도서삭제
                        {
                            char isbn[100];
                            printf("삭제할 도서 ISBN: "); fgets(isbn, sizeof(isbn), stdin); isbn[strcspn(isbn, "\n")] = 0;
                            send(sock, isbn, sizeof(char), 0);
                            read(sock, &result, sizeof(int));
                            printf(result ? "삭제 성공\n" : "삭제 실패\n");
                        }
                        else if (strcmp(cmp, "6") == 0) // 돌아가기
                        {
                            break;
                        }
                    }
                    if (strcmp(cmd, "2") == 0) // 모든계정관리
                    {
                        int count_2 = 0;
                        read(sock, &count_2, sizeof(int));
                        User *all_2 = NULL;
                        all_2 = malloc(sizeof(User)*count_2);
                        if (count_2>0)
                        {
                            for (int i = 0; i < count_2; i++)
                            {
                                read(sock, &all_2[i], sizeof(User));
                            }
                            for(int i = 2;i<count_2;i++){
                                printf("%d.아이디 %s, 이름 %s, 나이 %d, 핸드폰번호 %s 주소 %s\n",i-1,all_2[i].id,all_2[i].
                                name,all_2[i].age,all_2[i].phone,all_2[i].addr);
                            }
                            //    
                            printf("1.수정하기 2.추가하기");

                            scanf("%s", cmd);
                            char sub_action[16];
                            if(strcmp(cmd, "1")==0)
                            {
                                
                                strcpy(sub_action, "1");
                                send(sock, sub_action, sizeof(sub_action),0);

                                char  user_id_for_modify[16];
                                printf("수정할 유저의 아이디를 입력하세요\n");
                                scanf("%s", user_id_for_modify);
                                // send(sock, user_id_for_modify, sizeof(user_id_for_modify),0);
                                
                                char answer[16];
                                printf("입력한 아이디는?%s입니다 수정하시겠습니까?[y/n]\n", user_id_for_modify);
                                scanf("%s", answer);

                                getchar();
                                if(strcmp(answer, "y")==0)
                                {
                                    
                                    
                                    //유저 아이디를 보낸다 send1
                                    
                                    send(sock, user_id_for_modify, sizeof(user_id_for_modify),0);
                                    printf("y를 눌렀고 이미 1차 아이디를  보냄\n");
                                    // 서버로부터 받은 유저 검색결과 있으면 1없으면 0
                                    int user_result= 0;
                                    read(sock,&user_result, sizeof(result));
                                    printf("log3 해당유저가 있다?없다?%d\n", user_result);
                                    if(user_result)
                                    {
                                        User modi_user;
                                        printf("유저명을 입력해주세요\n");
                                        // scanf("%[^\n]s",modi_user.name);
                                        gets(modi_user.name);

                                        printf("유저 나이를 입력해주세요\n");
                                        char age[16];
                                        // scanf("%[^\n]s",age);
                                        gets(age);
                                        modi_user.age = atoi(age);
                                        printf("유저의 전화번호를 입력해주세요\n");
                                        // scanf("%[^\n]s",modi_user.phone);
                                        gets(modi_user.phone);
                                        printf("유저의 주소를 입력해주세요\n");
                                        // scanf("%[^\n]s",modi_user.addr);
                                        gets(modi_user.addr);
                                        modi_user.msc=1;

                                        //전송 보내기전 무결성검사 수행
                                        if(strlen(modi_user.name)<=0 || modi_user.name ==NULL )
                                        {
                                            printf("유저 이름이 입력되지 않았습니다.\n");
                                        }else if( 0 || modi_user.age <=0)
                                        {
                                            printf("유저 나이가 입력되지 않았습니다.\n");
                                        }
                                        else if( strlen(modi_user.phone)<=0 || modi_user.phone ==NULL )
                                        {
                                            printf("유저 전번이 입력되지 않았습니다.\n");
                                        }
                                        else if( strlen(modi_user.addr)<=0 || modi_user.addr ==NULL )
                                        {
                                            printf("유저 주소가 입력되지 않았습니다.\n");
                                        }
                                        //무결성 완료

                                        // send 2
                                        send(sock,&modi_user,sizeof(modi_user),0 );

                                        // 수정결과
                                        read(sock,&user_result, sizeof(user_result));

                                        if(user_result)
                                        {
                                            printf("수정이 완료되었습니다.\n");
                                        }

                                        
                                    
                                    }
                                    else
                                    {
                                        printf("찾고자하는 유저가 없습니다.");
                                    }

                                }   
                            }
                            //  계정추가
                            else if(strcmp(cmd, "2")==0)
                            {
                                strcpy(sub_action, "2");
                                send(sock, sub_action, sizeof(sub_action),0);
                                printf("유저를 추가를 선택하셨습니다.\n");

                                User user_for_add;
                                
                                printf("유저 아이디를 입력해주세요\n");
                                getchar();
                                gets(user_for_add.id);

                                printf("유저 패스워드를 입력해주세요\n");
                                getchar();
                                gets(user_for_add.pw);

                                printf("유저명을 입력해주세요\n");
                                // scanf("%[^\n]s",modi_user.name);
                                gets(user_for_add.name);

                                printf("유저 나이를 입력해주세요\n");
                                char age[16];
                                // scanf("%[^\n]s",age);
                                gets(age);
                                user_for_add.age = atoi(age);
                                printf("유저의 전화번호를 입력해주세요\n");
                                // scanf("%[^\n]s",modi_user.phone);
                                gets(user_for_add.phone);
                                printf("유저의 주소를 입력해주세요\n");
                                // scanf("%[^\n]s",modi_user.addr);
                                gets(user_for_add.addr);
                                
                                user_for_add.msc=1;

                                // 입력검증
                                if( (strlen(user_for_add.id)<=0) || (strlen(user_for_add.pw)<=0) || (strlen(user_for_add.name)<=0) || (user_for_add.age <= 0) || (strlen(user_for_add.phone)<=0) || (strlen(user_for_add.phone)<=0)) 
                                {
                                    printf("유저 입력이 잘못되었습니다.");
                                    sleep(1);
                                }
                                
                                // 구조체를 서버로 보내기 
                                send(sock,&user_for_add,sizeof(user_for_add),0 );
                                

                                

                                // 잘들어갔으면 결과를 받아서 출력
                                int user_add_result;
                                read(sock,&user_add_result, sizeof(user_add_result));

                                if(user_add_result)
                                {
                                    printf("계정이 잘 추가 되었습니다. \n");
                                }
                                    

                            }//계정 추가 끝
                        }// 유저의 수가 0보다 클때 if문 372라인 
                        else
                        {
                            printf("검색 결과가 없습니다.\n");

                        } 
                        free(all_2);
                    }  
                    if (strcmp(cmd, "3") == 0) // 도서관오픈관리
                    {
                       
                    }
                    if (strcmp(cmd, "4") == 0) // 대출자정보
                    {
                       
                    }
                    if (strcmp(cmd, "5") == 0) // 로그아웃
                    {
                        plag_exit = 1;
                        break;
                    }
                }
            }
            else if (result == 3)
            {
                while(plag_exit == 0){
                    printf("사서님 환영합니다.\n");
                    printf("1.도서관리\n2.대출정보\n3.모든계정관리\n4.메세지\n5.로그아웃\n"); 
                    char cmd[16];
                    fgets(cmd, sizeof(cmd), stdin);
                    cmd[strcspn(cmd, "\n")] = 0;
                    send(sock, cmd, sizeof(cmd), 0);

                    if (strcmp(cmd, "1") == 0) // 도서관리
                    {  
                        char cmp[16];
                        printf("1.도서검색\n2.도서추가\n3.돌아가기");
                        fgets(cmp, sizeof(cmp), stdin);
                        cmp[strcspn(cmp, "\n")] = 0;
                        send(sock, cmp, sizeof(cmp), 0);
                        if(strcmp(cmp, "1") == 0)
                        {
                                char key[50], val[100];
                            printf("검색 기준(title/author): ");
                            fgets(key, sizeof(key), stdin);
                            key[strlen(key) - 1] = '\0';
                            // key[strcspn(key, "\n")] = 0;
                            printf("검색어 입력: ");
                            fgets(val, sizeof(val), stdin);
                            val[strlen(val) - 1] = '\0';
                            // val[strcspn(val, "\n")] = 0;
                            send(sock, key, sizeof(key), 0);
                            send(sock, val, sizeof(val), 0);
                            int count;
    
                            // memset(p_results, 0, sizeof(Book));
                            read(sock, &count, sizeof(int));
                            Book *p_results = NULL;
                            p_results = malloc(sizeof(Book)*count);
                            if (count>0)
                            {
                                for (int i = 0; i < count; i++)
                                {
                                    read(sock, &p_results[i], sizeof(Book));
                                }
                                for (int i = 0; i < count; i++)
                                {
                                    printf("[%d] 제목 : %s 글쓴이 : %s 출판사 : %s 출판년 : %d ISBN %s\n", i-1, p_results[i].title, p_results[i].author, p_results[i].publisher, p_results[i].pub_year,p_results[i].isbn);
                
                                }
                            }
                            else
                            printf("검색 결과가 없습니다.\n");
                            free(p_results);
                        }
                        else if (strcmp(cmp, "2") == 0) // 도서추가
                        {
                        Book b;
                        printf("No: "); scanf("%d", &b.no); getchar();
                        printf("제목: "); fgets(b.title, sizeof(b.title), stdin); b.title[strcspn(b.title, "\n")] = 0;
                        printf("저자: "); fgets(b.author, sizeof(b.author), stdin); b.author[strcspn(b.author, "\n")] = 0;
                        printf("출판사: "); fgets(b.publisher, sizeof(b.publisher), stdin); b.publisher[strcspn(b.publisher, "\n")] = 0;
                        printf("출판년: "); scanf("%d", &b.pub_year); getchar();
                        printf("권 수: "); scanf("%d", &b.num_books); getchar();
                        printf("ISBN: "); fgets(b.isbn, sizeof(b.isbn), stdin); b.isbn[strcspn(b.isbn, "\n")] = 0;
                        printf("부가기호: "); fgets(b.extra_n, sizeof(b.extra_n), stdin); b.extra_n[strcspn(b.extra_n, "\n")] = 0;
                        printf("KDC: "); fgets(b.kdc, sizeof(b.kdc), stdin); b.kdc[strcspn(b.kdc, "\n")] = 0;
                        printf("KDC 주제: "); fgets(b.kdc_subject, sizeof(b.kdc_subject), stdin); b.kdc_subject[strcspn(b.kdc_subject, "\n")] = 0;
                        printf("대출빈도: "); scanf("%d", &b.loan_frequency); getchar();

                        send(sock, &b, sizeof(Book), 0);
                        read(sock, &result, sizeof(int));
                        printf(result ? "도서 추가 성공\n" : "도서 추가 실패\n");
                        }
                        else if(strcmp(cmp, "3") == 0)
                        {
                            break;
                        }
                    }
                    else if (strcmp(cmd, "2") == 0) // 대출정보 //eh
                    {
                        int count;
    
                            // memset(p_results, 0, sizeof(Book));
                        read(sock, &count, sizeof(int));
                        Book2 *p_results = NULL;
                        p_results = malloc(sizeof(Book2)*count);
                        if (count>0)
                        {
                            for (int i = 0; i < count; i++)
                            {
                                read(sock, &p_results[i], sizeof(Book2));
                            }
                            for (int i = 0; i < count; i++)
                            {
                                printf("ID.%s 제목 : %s 저자 : %s 출판사 : %s 출판년 : %d 권 : %d ISBN %s 대출시간: %d 상태 : %s 주소 : %s\n",p_results[i].id, p_results[i].title, p_results[i].author,
                                p_results[i].publisher, p_results[i].pub_year,p_results[i].num_books,p_results[i].isbn,p_results[i].loan_time,p_results[i].status,p_results[i].loan_addr);
                            }
                            while(1){
                                int choice_3;
                                printf("1.대출승인\n2.도서반납\n3.대출자구제\n");
                                scanf("%d",&choice_3);
                                send(sock,&choice_3,sizeof(int), 0);
                                while(getchar() != '\n');
                                if(choice_3 == 1){

                                }
                                else if(choice_3 == 2){
                                    char isbn[50];
                                    int found = 0;
                                    printf("반납할 ISBN을 입력해주세요.\n");
                                    scanf("%s", isbn);
                                    send(sock, isbn, sizeof(isbn), 0);
                                
                                    for (int i = 0; i < count; i++) 
                                    {
                                        if (strcmp(isbn, p_results[i].isbn) == 0) {
                                            found = 1;
                                            int return_count;
                                            printf("반납할 권수를 입력해주세요.\n");
                                            scanf("%d", &return_count);
                                            send(sock, &return_count, sizeof(int), 0);
                                
                                            if (p_results[i].num_books - return_count >= 0) {
                                                while(getchar() != '\n'); // 버퍼 클리어
                                                printf("반납처리가 완료되었습니다.\n");
                                            } else {
                                                printf("잘못된 입력입니다. 반납할 권수가 너무 많습니다.\n");
                                            }
                                            break; // 찾았으면 더 이상 반복할 필요 없음
                                        }
                                    }
                                
                                    if (!found) {
                                        printf("해당 ISBN에 해당하는 도서를 찾을 수 없습니다.\n");
                                    }
                                }
                                else if(choice_3 == 3){

                                }
                            }

                        }
                        else
                        printf("검색 결과가 없습니다.\n");
                        free(p_results); 
                    }
                    else if (strcmp(cmd, "3") == 0) // 모든계정관리
                    {
                        int count_2 = 0;
                        read(sock, &count_2, sizeof(int));
                        User *all_2 = NULL;
                        all_2 = malloc(sizeof(User)*count_2);
                        if (count_2>0)
                        {
                            for (int i = 0; i < count_2; i++)
                            {
                                read(sock, &all_2[i], sizeof(User));
                            }
                            for (int i =0; i <count_2;i++)
                            {
                                all_2[i].phone[3] = '*';
                                all_2[i].phone[4] = '*';
                                all_2[i].phone[5] = '*';
                                all_2[i].phone[6] = '*';
                            }
                            for(int i = 2;i<count_2;i++){
                                printf("%d.아이디 %s, 이름 %s, 나이 %d, 핸드폰번호 %s 주소 %s\n",i-1,all_2[i].id,all_2[i].
                                name,all_2[i].age,all_2[i].phone,all_2[i].addr);
                            }     
                        }
                        else
                        printf("검색 결과가 없습니다.\n");
                        free(all_2);
                    }
                    else if (strcmp(cmd, "4") == 0) // 메세지
                    {
                        librarian_message(sock);
                        
                    }
                    else if (strcmp(cmd, "5") == 0) // 로그아웃
                    {
                        plag_exit = 1;
                        break;
                    }
                }
            }
            close(sock);
        }
    }
}
void clearbuffer(){
    while (getchar() !='\n');
}

void pressenter() {
    clearbuffer();
    printf("<Press Enter>\n");
    scanf(".");
    getchar();
}

//메세지 - 사서
void librarian_message(int sock) {

    while(1) {

        CLEAR

        //choice 1
        printf("******* [ 메세지 ] *******\n");
        printf("[1] 유저에게 메세지 보내기\n");
        printf("[2] 메세지함 확인하기\n");
        printf("[3] 돌아가기\n");
        printf("user: ");

        int user_choice;
        scanf("%d", &user_choice);
        
        // send 1: int user_choice (void librarian) - choice 1
        send(sock, &user_choice, sizeof(user_choice), 0);

        if (user_choice == 1) {                 //[1] 유저에게 메세지 보내기

            printf("메세지를 보낼 유저의 ID를 입력해주세요:\n");
            
            clearbuffer();
            char user_id[30];
            scanf("%s", user_id);

            // printf("user id: %s\n", user_id);
                     
            // send 2: char user_id
            printf("user_id: %s\n", user_id);
            send(sock, user_id, sizeof(user_id), 0);

            int success;
            
            // read 1: int success;
            read(sock, &success, sizeof(success));

            // printf("read success: %d\n", success);
            if (success == 1) {

                CLEAR

                printf("*****[%s]에게 메세지 보내기*****\n", user_id);

                char title_msg[150];
                char script_msg[150];

                clearbuffer();
                printf("제목을 입력해주세요: \n");
                fgets(title_msg, sizeof(title_msg), stdin);
                clear_newline(title_msg);
                printf("내용을 입력해주세요:\n");
                clear_newline(script_msg);
                fgets(script_msg, sizeof(script_msg), stdin);

                //send 3: char title_msg / char script_msg
                send(sock, title_msg, sizeof(title_msg), 0);
                send(sock, script_msg, sizeof(script_msg), 0);


                //read 2: int success
                read(sock, &success, sizeof(success));
                if (success== 1) {
                    printf("메세지 전달 완료\n");
                    
                    PRESSENTER
                }

                                                
            } else if (success == 0) {
                printf("해당 유저가 존재하지 않습니다.\n");
                PRESSENTER
            }
            
            
        } else if (user_choice == 2){           //[2] 메세지함 확인하기
            while (1) {

                CLEAR

                //read 1: int success
                int success;
                read(sock, &success, sizeof(success));
                if (success == 0) {
                    printf("사서의 메세지함 열기 실패\n");
                    break;
                } else {
                    CLEAR
                    printf("*****[사서]의 메세지함*****\n");
                }

                //read 2: int count
                int count;
                read(sock, &count, sizeof(count));

                // printf("count: %d\n", count);

                char file_name[50][50];
                for (int i = 0; i< count; i++) {
                    //read 3: char file_name[]
                    read(sock, file_name[i], sizeof(file_name[i]));
                    printf("[%d]. %s\n", i+1, file_name[i]);
                }

                printf("***************************\n");
                printf("[번호+r]: 메세지 읽기\n");
                printf("[번호+k]: 메세지 삭제하기\n");
                printf("[  q  ]: 돌아가기\n");

                char choice_msg[20] = {0};                //유저가 선택한 파일(경로)
                char choices[5] = {0};                         //번호+r / 번호+k
                char file_path[50] = {0};

                clearbuffer();
                scanf("%s", choice_msg);

                int num= atoi(choice_msg);               //첫문자는 int형으로 바꾸기
                char form[1]={0};
                form[0]= choice_msg[strlen(choice_msg)-1];                          //r/k는 form문자열에 저장.
                int rk;

                printf("num: %d\n", num);
                printf("form: %s\n", form);


                if ( strcmp(form,"r")==0 && count > 0 ) {             //메세지 읽기

                    CLEAR

                    rk = 0;
                    // send 2: int rk   (int form)
                    send(sock, &rk, sizeof(rk), 0);

                    char title_msg[150];
                    char user_id[150];
                    char script_msg[150];

                    printf("file_name[num-1]: %s\n", file_name[num-1]);

                    // send 3: char file_name[i]
                    send(sock, file_name[num-1], sizeof(file_name[num-1]), 0);

                    //read 4: char title_msg, char user_id, char script_msg
                    read(sock, title_msg, sizeof(title_msg));
                    read(sock, user_id, sizeof(user_id));
                    read(sock, script_msg, sizeof(script_msg));

                    CLEAR

                    // printf("script_msg: %s\n", script_msg);
                    printf("[system] Now Loading...\n");
                    usleep(500000);

                    CLEAR

                    printf("[제목]: %s\n", title_msg);
                    printf("[보낸이]: %s\n", user_id);
                    printf("[내용]: %s\n", script_msg);

                    printf("\n<Press Enter>\n");
                    CLEARBUFFER
                    scanf(".");
                    int go=1;
                    send(sock, &go, sizeof(go), 0);

                } else if ( strcmp(form, "k")==0 && count > 0 ) {    //메세지 삭제하기
                    
                    CLEAR

                    // send 2: int rk   ( int form )
                    rk = 1;
                    send(sock, &rk, sizeof(rk), 0);

                    // send 3: char file_name[i] -> file_path
                    send(sock, file_name[num-1], sizeof(file_name[num-1]), 0);

                    // read 4: int result
                    int result;
                    read(sock, &result, sizeof(result));

                    if (result== 0) {
                        printf("[system] '%s'를 삭제하였습니다.\n", file_name[num-1]);
                        PRESSENTER
                    } else if (result== 1) {
                        printf("[system] '%s'를 삭제하는 데 실패했습니다.", file_name[num-1]);
                        PRESSENTER
                    }

                } else if ( strcmp(choice_msg, "q")==0 ) {                //'q'이면
                    
                    // send 2: int rk   ( form )
                    rk = 2;
                    send(sock, &rk, sizeof(rk), 0);
                    break;

                } else if ( strcmp(form, "r")==0 && count == 0 ) {
                    printf("메세지가 없습니다.\n");
                    // send 2: int rk   ( form )
                    rk = 4;
                    send(sock, &rk, sizeof(rk), 0);
                    CLEARBUFFER
                    break;
                } else if ( strcmp(form, "k")==0 && count == 0 ) {
                    // send 2: int rk   ( form )
                    rk = 5;
                    send(sock, &rk, sizeof(rk), 0);
                    CLEARBUFFER
                    break;
                } else {
                    
                    // send 2: int rk
                    rk = 3;
                    printf("잘못된 입력입니다.\n");
                    send(sock, &rk,sizeof(rk), 0);
                    
                    PRESSENTER
                    continue;
                }
            }
    
        } else if (user_choice == 3){           //[3] 돌아가기
            break;
        } else {
            printf("잘못된 입력입니다.\n");
            scanf(".");
            getchar();
            clearbuffer();
        }
    }
}

//메세지 - 클라이언트
void client_message(int sock, char *user_id) {
    
    // printf("void client_message() 실행 \n");                 //testprint
    while(1) {

        CLEAR

        int user_choice;
        int success;
        char title_msg[150];
        char script_msg[150];

        // printf("before sending char user_id\n");             //test print

        //send1: char user_id
        send(sock, user_id, sizeof(user_id), 0);                //파일생성을 위한 id 전송

        // printf("%s\n",user_id);                              //test print

        //ui 출력: choice 1
        printf("*****[%s/메세지]*****\n",user_id);
        printf("[1] 사서에게 메세지 보내기\n");
        printf("[2] 나의 메세지함 확인하기\n");
        printf("[3] 돌아가기\n");
        scanf("%d", &user_choice);

        //send2: int user_choice ( choice 1 )
        send(sock, &user_choice, sizeof(user_choice), 0);

        if(user_choice == 1) {                                  //[1] 사서에게 메세지 보내기
            
            CLEAR
            CLEARBUFFER
            int lib_read;
            int creatable = 0;

            //read 0: int creatable;
            read(sock, &creatable, sizeof(creatable));
            printf("creatable: %d\n", creatable);
            if (creatable == 1) {
                //read1: int success;
                read(sock, &success, sizeof(int));                      //사서 디렉토리의 user_id 파일이 열렸는지 확인
                if (success == 0) {
                printf("파일 열기 실패\n");
                return;
                } else {
                    printf("*****send message to librarian*****\n");
                }

                //파일 열어서 user_id.json가 존재하는지, 존재한다면 lib_read가 1인지, 0인지 체크.
                printf("제목을 입력해주세요:\n ");
                fgets(title_msg, sizeof(title_msg), stdin);
                title_msg[strlen(title_msg) -1] = '\0';

                printf("내용을 입력해주세요:\n ");
                fgets(script_msg, sizeof(script_msg), stdin);
                script_msg[strlen(script_msg) -1] = '\0';

                //send3: char title_msg & char script_msg
                send(sock, title_msg, sizeof(title_msg), 0);            //제목 전송
                send(sock, script_msg, sizeof(script_msg), 0);          //내용 전송

                printf("[system]: 메세지가 전달되었습니다...<Press Enter>\n");
                scanf(".");
                getchar();
            } else if (creatable == 0) {
                printf("사서가 메세지를 확인하지 않았습니다.\n");
                break;
            }
            
        //[2] 메세지함 확인하기
        } else if (user_choice == 2) {                          //[2] 사서로부터의 메세지 확인하기
            
            CLEAR
            //유저 폴더 열기

            //ui 출력: choice 2
            printf("[1] 받은 메세지함 확인\n");
            printf("[2] 보낸 메세지함 확인\n");
            clearbuffer();
            scanf("%d", &user_choice);

            //send3: int user_choice ( choice 2 )
            send(sock, &user_choice, sizeof(user_choice), 0);

            if (user_choice == 1) {                                 //[1] 받은 메세지함 확인하기.
                
                char file_name[50][50];
                int success;
                int count;

                // read 1: int success ( dir open에 대한 성공여부 )
                read(sock, &success, sizeof(success));

                if (success == 0) {
                    printf("[%s]의 메세지함 열기 실패(폴더가 존재하지 않음)\n", user_id);
                    break;
                } else {
                    CLEAR
                    printf("*****[%s]의 메세지함*****\n", user_id);
                }

                // read 2: int count ( 총 파일의 갯수 )
                read(sock, &count, sizeof(count));

                // read 3: char file_name ( 파일경로 )
                for (int i = 0; i < count; i++) {
                    read(sock, file_name[i], sizeof(file_name[i]));
                    printf("[%d]: %s\n", i+1, file_name[i]);
                }
                
                printf("***************************\n");
                printf("[번호+r]: 메세지 읽기\n");
                printf("[번호+k]: 메세지 삭제하기\n");
                printf("[  q  ]: 돌아가기\n");

                char choice_msg[10] = {0};                      //유저가 선택한 파일(경로)
                char file_path[50] = {0};                       //fopen에 사용할 경로

                clearbuffer();
                scanf("%s", choice_msg);

                // printf("choice_msg: %s\n", choice_msg);
                int num= atoi(choice_msg);                      //첫문자는 int형으로 바꾸기
                char form[1]= {0};                              //r/k는 form문자열에 저장.
                strcpy(form, &choice_msg[strlen(choice_msg)- 1]);

                printf("num: %d\n", num);
                printf("form: %s\n", form);

                int rk;

                // send 2: int rk ( 0(r): 읽기 / 1(k): 삭제 / 2(q): 돌아가기 )
                if ( strcmp(form, "r")==0) {                    //메세지 읽기(r)

                    CLEAR

                    rk = 0;
                    
                    send(sock, &rk, sizeof(rk), 0);

                    char title_msg[150];
                    char id[150];
                    char script_msg[150];

                    // send 3: char file_name[i]
                    send(sock, file_name[num-1], sizeof(file_name[num-1]), 0);

                    //read 4: char title_msg, char user_id, char script_msg
                    read(sock, title_msg, sizeof(title_msg));
                    read(sock, id, sizeof(id));
                    read(sock, script_msg, sizeof(script_msg));

                    usleep(150000);

                    CLEAR
                    printf("[제목]: %s\n", title_msg);
                    printf("[보낸이]: %s\n", id);
                    printf("[내용]: %s\n", script_msg);

                    printf("<Press Enter>\n");
                    scanf(".");
                    getchar();
                    break;
                        

                } else if (strcmp(form, "k")==0 ) {             //메세지 삭제하기
                    
                    CLEAR

                    // send 2: int rk
                    rk = 1;
                    send(sock, &rk, sizeof(rk), 0);
                            
                    send(sock, file_name[num-1], sizeof(file_name[num-1]), 0);

                    // read 4: int result
                    int result;
                    read(sock, &result, sizeof(result));

                    if (result== 0) {
                        printf("[%s]를 삭제하였습니다.\n", file_name[num-1]);
                        
                        scanf(".");
                        getchar();
                    } else if (result== 1) {
                        printf("[%s]를 삭제하는 데 실패했습니다.", file_name[num-1]);
                        scanf(".");
                        getchar();
                    }
                    break;
                } else if (strcmp(choice_msg, "q")==0) {        //'q'이면 뒤로가기
                    
                    // send 2: int rk   ( int form )
                    rk = 2;
                    send(sock, &rk, sizeof(rk), 0);
                    break;

                } else {
                    
                    // send 2: int rk
                    rk = 3;
                    send(sock, &rk, sizeof(rk), 0);
                    continue;
                }
            

            } else if (user_choice == 2) {                          //[2] 보낸 메세지함 확인하기
                
                char file_name[2][50];
                int success;

                send(sock, user_id, sizeof(user_id), 0);

                //read 1: int success
                read(sock, &success, sizeof(success));

                if (success== 0) {
                    printf("사서 디렉토리를 열 수 없습니다.\n");
                    break;
                } else {
                    printf("*****[%s의 보낸 메세지]*****\n", user_id);
                }
                
                int count;
                read(sock, &count, sizeof(count));

                for (int i = 0; i < count; i++) {
                    read(sock, file_name[i], sizeof(file_name[i]));
                    printf("[%d] %s\n", i+1, file_name[i]);
                }
                printf("***************************\n");
                printf("[번호+k]: 메세지 삭제하기\n");
                printf("[  q  ]: 돌아가기\n");

                char choice_msg[10] = {0};                      //유저가 선택한 파일(경로)
                char file_path[50] = {0};                       //fopen에 사용할 경로

                clearbuffer();
                scanf("%s", choice_msg);

                // printf("choice_msg: %s\n", choice_msg);
                int num= atoi(&choice_msg[0]);                  //첫문자는 int형으로 바꾸기
                char form[1]= {0};                              //r/k는 form문자열에 저장.
                strcpy(form, &choice_msg[strlen(choice_msg)- 1]);

                // printf("num: %d\n", num);
                // printf("form: %s\n", form);

                int rk;
                int result= 0;

                if (strcmp(form, "k")==0 ) {             //메세지 삭제하기
                    
                    // send 2: int rk
                    rk = 0;
                    send(sock, &rk, sizeof(rk), 0);

                    for ( int i=0; i<count; i++ ) {
                        if ( num == i+1 ) {
                            // send 3: char file_name[i] -> file_path
                            send(sock, file_name[i], sizeof(file_name[i]), 0);

                            // read 4: int result

                            read(sock, &result, sizeof(result));

                            if (result== 0) {
                                printf("[system] '%s'를 삭제하였습니다.\n", file_name[i]);
                                PRESSENTER
                            } else if (result== 1) {
                                printf("[system] '%s'를 삭제하는 데 실패했습니다.", file_name[i]);
                                PRESSENTER
                            }
                            break;
                        }
                    }

                } else if (strcmp(choice_msg, "q") == 0) {
                    rk = 1;
                    send(sock, &rk, sizeof(rk), 0);
                    break;
                } else {
                    // send 2: int rk
                    rk = 2;
                    send(sock, &rk, sizeof(rk), 0);
                    printf("잘못된 입력입니다.\n");
                    PRESSENTER
                    continue;
                }
                
            } else if (user_choice == 3) {                          //[3] 돌아가기
                break;
            } else {                                                //[4] 잘못된 입력
                continue;
            }

        } else if (user_choice == 3) {                          //[3] 돌아가기
            break;
        } else {                                                //[4] 유저의 제시되지 않은 입력
            continue;
        }

    }
}