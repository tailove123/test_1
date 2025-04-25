#include <stdio.h>              // 표준 입출력
#include <stdlib.h>             // 메모리/문자열 함수
#include <string.h>             // 문자열 처리 함수
#include <unistd.h>             // read(), write(), close() 등
#include <netinet/in.h>         // sockaddr_in 구조체 정의
#include <arpa/inet.h>          // inet_pton() 함수
#include <time.h>
#include <stdint.h> 
#pragma pack(1)
#define SIZE 100
#define MAX_BOOKS 11000
#define PORT 10001  // 서버 포트
#define MAX_LOANS 100  // 대출 가능한 최대 도서 수
#define MAX_USERS 500

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
    char id[SIZE]; // 유저아이디
    char title[SIZE];  // 제목
    char author[SIZE]; // 저자 
    char publisher[SIZE]; // 출판사  
    int pub_year; // 출판년
    int num_books; //빌린 권수 
    char isbn[SIZE];  // ISBN
    long loan_time; //대출일
    char status[SIZE];
    char loan_addr[SIZE];
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

typedef struct {

    char date[100];
    char day_[50];
    int is_open;  // 1이면 영업 0이면 휴일
}bussiness_month;

void clearbuffer(){
    while (getchar() !='\n');
}

void clear_newline(char *str) {
    str[strcspn(str, "\n")] = 0; // Remove the newline character
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr = {0};

    sock = socket(PF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    while(1)
    {
        char choice[10] = {0};
        char id[50] = {0};
        char pw[50] = {0};
        char nickname[50] = {0};
        char phone[50] = {0};
        char address[100] = {0};
        int year = 0;
        int message_a = 1;
        int plag_exit = 0;
    
        printf("1.로그인\n2.회원가입\n3.대출하기\n4.반납하기>> ");
        fgets(choice, sizeof(choice), stdin);
        clear_newline(choice); // Remove the newline from choice input

        printf("ID: "); fgets(id, sizeof(id), stdin); clear_newline(id);
        printf("PW: "); fgets(pw, sizeof(pw), stdin); clear_newline(pw);

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
        if (result == 1)  //일반
        {
            while (1)
            {
                User user_info = {0};
                
                send(sock, id, sizeof(id), 0);
                
                //서버로부터 응답을 받음
                read(sock, user_info.id, sizeof(user_info.id));
                read(sock, user_info.name, sizeof(user_info.name));
                read(sock, &user_info.age, sizeof(int));
                read(sock, user_info.phone, sizeof(user_info.phone));
                read(sock, user_info.addr, sizeof(user_info.addr));
                printf("%s, %s, %d, %s, %s", user_info.id, user_info.name, user_info.age, user_info.phone, user_info.addr);
                int d_count = 0;
                read(sock, &d_count, sizeof(int));             //1. 멀록카운트
                Book2 *d_results = malloc(sizeof(Book2)*d_count);
                memset(d_results, 0, sizeof(Book2)*d_count);
                if(d_count>0)     //대출 목록 불러오기
                {
                    for (int i=0;i<d_count;i++)                     
                    {
                        read(sock, &d_results[i], sizeof(Book2));  //2.대출목록
                        printf("%s\n", d_results[i].title);
                    }

                }
                char cmd[16] = {0};
                printf("\n1.내정보확인하기\n2.온라인대출하기\n3.메세지보내기\n4.메세지삭제하기\n5.로그아웃\n");
                fgets(cmd, sizeof(cmd), stdin);
                cmd[strlen(cmd) - 1] = '\0';
                send(sock, cmd, sizeof(cmd), 0);               //3 사용자 요청
                if (strcmp(cmd,"1") == 0)  //1. 내 정보 확인하기
                {
                    printf("id :%s\n",id);
                    printf("유저명 :%s\n",user_info.name);
                    printf("나이 :%d\n",user_info.age);
                    printf("전화번호 :%s\n",user_info.phone);
                    printf("주소 :%s\n",user_info.addr);
                    if(user_info.bsc == 1)
                        printf("불량대출자");

                    //내 정보를 화면에 출력 + 내 대출목록 

                }
                else if (strcmp(cmd, "2") == 0)  //온라인 대출하기
                {
                    while(1)
                    {
                        int num = 0;
                        int num_1 = 0;
                        char key[50] = {0};
                        char val[100] = {0};
                        char tmp_addr[100] = {0};
                        printf("검색 기준(title/author/back): ");
                        fgets(key, sizeof(key), stdin);
                        key[strlen(key) - 1] = '\0';
                        // key[strcspn(key, "\n")] = 0;
                        send(sock, key, sizeof(key), 0);         //4.키
                        if (strcmp(key, "back")==0)
                        {
                            break;
                        } 
                        printf("검색어 입력: ");
                        fgets(val, sizeof(val), stdin);
                        val[strlen(val) - 1] = '\0';
                        // val[strcspn(val, "\n")] = 0;

                        send(sock, val, sizeof(val), 0);         //5. 밸류
                        int count = 0;
                        // memset(p_results, 0, sizeof(Book));
                        read(sock, &count, sizeof(int));          //6.검색카운트
                        if (count>0)
                        {
                            Book *p_results = malloc(sizeof(Book)*count);
                            memset(p_results, 0, sizeof(Book)*count);
                            for (int i = 0; i < count; i++)
                            {
                                read(sock, &p_results[i], sizeof(Book));
                            }
                            for (int i = 0; i < count; i++)
                            {
                                printf("[%d] 제목 : %s 글쓴이 : %s 출판사 : %s 출판년 : %d ISBN : %s 권수 : %d\n", i+1, p_results[i].title, p_results[i].author, p_results[i].publisher, p_results[i].pub_year,p_results[i].isbn,p_results[i].num_books);
            
                            }
                            printf("\n1.대출\n2.다시검색\n3.처음으로\n");
                            num_1 = scanf("%d", &num); 
                            getchar();
                            write(sock, &num_1, sizeof(int));
                            if (num_1== 0)
                            {
                                printf("메뉴에 적힌 숫자를 입력해 주세요.\n");
                                getchar();
                                p_results = NULL;
                                free(p_results);
                                p_results = NULL;
                                continue;
                            }
                            write(sock, &num, sizeof(int));     //7.대출,다시,처음
                            if(num == 1)   //1. 대출
                            {
                                printf("대출할 번호를 입력해주세요: \n");
                                num_1 = scanf("%d", &num); 
                                getchar();

                                write(sock, &num_1, sizeof(int));  //오류분석
                                write(sock, &num, sizeof(int));    //대출 번호
                                
                                
                                if (num_1== 0 || num >count || num < 0)
                                {
                                    printf("목록에서 해당 도서의 []안의 숫자를 입력해 주세요.\n");
                                    getchar();
                                    p_results = NULL;
                                    free(p_results);
                                    p_results = NULL;
                                    continue;
                                }
                                // else if (num>p_results[num-1].num_books)
                                // {
                                    
                                // }
                                printf("권수를 입력해주세요: ");
                                int num_loan = 0;  //권수
                                num_1 = scanf("%d", &num_loan);
                                getchar();
                                write(sock, &num_1, sizeof(int));  //오류분석
                                write(sock, &num_loan, sizeof(int)); //책권수
                                if (num_1== 0)
                                {
                                    printf("숫자를 입력해 주세요..\n");
                                    getchar();
                                    p_results = NULL;
                                    free(p_results);
                                    p_results = NULL;
                                    continue;
                                }
                                int error_msg = 0;
                                read(sock, &error_msg, sizeof(int));
                                if(error_msg ==1)
                                {
                                    printf("수량이 부족합니다.\n");
                                    getchar();
                                    p_results = NULL;
                                    free(p_results);
                                    p_results = NULL;
                                    continue;
                                }
                                else if(error_msg ==2)
                                {
                                    printf("같은 책을 2권 초과하였습니다.\n");
                                    getchar();
                                    p_results = NULL;
                                    free(p_results);
                                    p_results = NULL;
                                    continue;
                                }
                                else if(error_msg ==3)
                                {
                                    printf("10권 넘게 빌릴 수 없습니다.\n");
                                    getchar();
                                    p_results = NULL;
                                    free(p_results);
                                    p_results = NULL;
                                    continue;
                                }
                                else if(error_msg ==4)
                                {
                                    printf("1 이상을 입력 해 주십시오.\n");
                                    getchar();
                                    p_results = NULL;
                                    free(p_results);
                                    p_results = NULL;
                                    continue;
                                }
                                else if(error_msg ==5)
                                {
                                    printf("불량대출자입니다(대출불가)\n");
                                    getchar();
                                    p_results = NULL;
                                    free(p_results);
                                    p_results = NULL;
                                    continue;
                                }
                                printf("받을 주소를 입력해주세요 (입력하지 않고 엔터를 입력하면 기본 주소로 보내집니다.)");
                                printf("%s\n",user_info.addr);
                                fgets(tmp_addr, sizeof(tmp_addr), stdin);
                                tmp_addr[strlen(tmp_addr) - 1] = '\0';
                                if(tmp_addr[0]=='\0')
                                {
                                    strcpy(d_results[d_count].loan_addr,user_info.addr);
                                }
                                else strcpy(d_results[d_count].loan_addr,tmp_addr);

                                printf("%s\n",d_results[d_count].loan_addr);

                                strcpy(d_results[d_count].title, p_results[num-1].title);
                                strcpy(d_results[d_count].author, p_results[num-1].author);
                                strcpy(d_results[d_count].publisher, p_results[num-1].publisher);
                                d_results[d_count].pub_year = p_results[num-1].pub_year;
                                strcpy(d_results[d_count].isbn, p_results[num-1].isbn);
                                strcpy(d_results[d_count].id, user_info.id); // 유저 ID 저장
                                d_results[d_count].num_books = num_loan;
                                strcpy(d_results[d_count].status, "온라인승인대기");
                                p_results = NULL;
                                free(p_results);
                                p_results = NULL;

                                printf(">> 대출 내역:\n");
                                for (int i = 0; i <= d_count; i++) {
                                    printf("ID: %s | 제목: %s | 저자: %s | 출판사: %s | 출판년: %d | ISBN: %s | 권수: %d\n | 배송지: %s\n | 상태: %s\n",
                                        d_results[i].id, d_results[i].title, d_results[i].author,
                                        d_results[i].publisher, d_results[i].pub_year,
                                        d_results[i].isbn, d_results[i].num_books, d_results[i].loan_addr, d_results[i].status);
                                }
                                write(sock,&d_results[d_count],sizeof(Book2));
                                d_count++;  
                            }
                            else if(num == 2)   //2. 다시검색
                            {
                                p_results = NULL;
                                free(p_results);
                                p_results = NULL;
                                continue;
                            }
                            else if(num == 3)   //3. 처음으로
                            {
                                p_results = NULL;
                                free(p_results);
                                p_results = NULL;
                                break;
                            }
                            else
                            {
                                p_results = NULL;
                                free(p_results);
                                p_results = NULL;
                                break;
                            }
                        }
                        else
                        {
                            printf("검색 결과가 없습니다.\n");
                            getchar();
                        }
                    }
                } 
                else if (strcmp(cmd, "5") == 0)
                {
                    d_results = NULL;
                    free(d_results);
                    d_results = NULL;
                    break;
                }
                d_results = NULL;
                free(d_results);
                d_results = NULL;
            }
        }
        else if (result == 2)           // 관리자
        {
            while(1){
                printf("관리자님 환영합니다.\n");
                printf("1.도서관리\n2.모든계정관리\n3.도서관오픈관리\n4.대출관리\n5.로그아웃\n");
                char cmd[16] = {0};
                fgets(cmd, sizeof(cmd), stdin);
                cmd[strlen(cmd) - 1] = '\0';
                send(sock, cmd, sizeof(cmd), 0);
                if (strcmp(cmd, "1") == 0) // 도서관리
                {   
                    char cmp[16] = {0};
                    printf("1.도서목록\n2.도서검색\n3.도서수정\n4.도서추가\n5.도서삭제\n6.돌아가기");
                    fgets(cmp, sizeof(cmp), stdin);
                    cmp[strlen(cmp) - 1] = '\0';
                    send(sock, cmp, sizeof(cmp), 0);
                    if(strcmp(cmp, "1") == 0)
                    {
                        break;
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
                        memset(p_results, 0, sizeof(Book)*count);
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
                        else printf("검색 결과가 없습니다.\n");
                        p_results = NULL;
                        free(p_results);
                    }
                    else if (strcmp(cmp, "3") == 0) //도서수정
                    {
                        int plag_fail = 0;
                        Book b;
                        while(1)
                        {
                            int book_count = 0;
                            printf("수정할 도서 ISBN: "); fgets(b.isbn, sizeof(b.isbn), stdin); b.isbn[strcspn(b.isbn, "\n")] = 0;
                            printf("새 No: "); scanf("%d", &b.no); getchar();
                            printf("새 제목: "); fgets(b.title, sizeof(b.title), stdin); b.title[strcspn(b.title, "\n")] = 0;
                            printf("새 저자: "); fgets(b.author, sizeof(b.author), stdin); b.author[strcspn(b.author, "\n")] = 0;
                            printf("새 출판사: "); fgets(b.publisher, sizeof(b.publisher), stdin); b.publisher[strcspn(b.publisher, "\n")] = 0;
                            int num_1 = 0;
                            printf("(숫자만!!)출판년: "); num_1 = scanf("%d", &b.pub_year); while(getchar() != '\n'); 
                            if (num_1== 0)
                            {
                                plag_fail = 1;
                                break;
                            }
                            int num_2 = 0;
                            printf("(숫자만!!)권 수: "); num_2 = scanf("%d", &b.num_books); while(getchar() != '\n'); 
                            if (num_2== 0)
                            {
                                plag_fail = 1;
                                break;
                            }
                            printf("새 부가기호: "); fgets(b.extra_n, sizeof(b.extra_n), stdin); b.extra_n[strcspn(b.extra_n, "\n")] = 0;
                            printf("새 KDC: "); fgets(b.kdc, sizeof(b.kdc), stdin); b.kdc[strcspn(b.kdc, "\n")] = 0;
                            printf("새 KDC 주제: "); fgets(b.kdc_subject, sizeof(b.kdc_subject), stdin); b.kdc_subject[strcspn(b.kdc_subject, "\n")] = 0;
                            printf("새 대출빈도: "); scanf("%d", &b.loan_frequency); getchar();

                            send(sock, &b, sizeof(Book), 0);
                            read(sock, &result, sizeof(int));
                            printf(result ? "수정 성공\n" : "수정 실패\n");
                            break;
                        }

                    }
                    else if (strcmp(cmp, "4") == 0) // 도서추가
                    {
                        Book b;
                        int plag_fail = 0;
                        while(1)
                        {

                            int book_count = 0;
                            int plag_isbn_duplication = 0;
                            read(sock, &book_count, sizeof(int));
                            b.no = book_count+1;
                            printf("제목: "); fgets(b.title, sizeof(b.title), stdin); b.title[strcspn(b.title, "\n")] = 0;
                            printf("저자: "); fgets(b.author, sizeof(b.author), stdin); b.author[strcspn(b.author, "\n")] = 0;
                            printf("출판사: "); fgets(b.publisher, sizeof(b.publisher), stdin); b.publisher[strcspn(b.publisher, "\n")] = 0;
                            int num_1 = 0;
                            printf("(숫자만!!)출판년: "); num_1 = scanf("%d", &b.pub_year); while(getchar() != '\n'); 
                            if (num_1== 0)
                            {
                                plag_fail = 1;
                                break;
                            }
                            int num_2 = 0;
                            printf("(숫자만!!)권 수: "); num_2 = scanf("%d", &b.num_books); while(getchar() != '\n'); 
                            if (num_2== 0)
                            {
                                plag_fail = 1;
                                break;
                            }
                            printf("ISBN: "); fgets(b.isbn, sizeof(b.isbn), stdin); b.isbn[strcspn(b.isbn, "\n")] = 0;
                            char pre_isbn[100];
                            strcpy(pre_isbn, b.isbn);
                            printf("%s\n", pre_isbn);
                            write(sock, pre_isbn, sizeof(pre_isbn));
                            read(sock, &plag_isbn_duplication, sizeof(int));
                            if(plag_isbn_duplication == 1)
                            {
                                plag_fail = 2;
                                break;
                            }
                            printf("부가기호: "); fgets(b.extra_n, sizeof(b.extra_n), stdin); b.extra_n[strcspn(b.extra_n, "\n")] = 0;
                            printf("KDC: "); fgets(b.kdc, sizeof(b.kdc), stdin); b.kdc[strcspn(b.kdc, "\n")] = 0;
                            printf("KDC 주제: "); fgets(b.kdc_subject, sizeof(b.kdc_subject), stdin); b.kdc_subject[strcspn(b.kdc_subject, "\n")] = 0;
                            b.loan_frequency = 0;
                        
                            send(sock, &b, sizeof(Book), 0);
                            read(sock, &result, sizeof(int));
                            printf(result ? "도서 추가 성공\n" : "도서 추가 실패\n");
                            break;
                        }
                        if (plag_fail == 1)
                        {
                            printf("도서 추가 실패(숫자만!! : 붙여진 숫자만 입력해 주세요.\n");
                            plag_fail = 0;
                        }
                        else if (plag_fail == 2)
                        {
                            printf("도서 추가 실패(ISBN이 중복되었습니다.\n");
                            plag_fail = 0;
                        }
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
                        continue;
                    }
                }
                if (strcmp(cmd, "2") == 0) // 모든계정관리
                {
                    
                }
                if (strcmp(cmd, "3") == 0) // 도서관오픈관리
                {
                    
                }
                if (strcmp(cmd, "4") == 0) // 대출관리
                {
                    while(1)
                    {
                        char loan_id[50] = {0};
                        int num_1 = 0;  //오류 분석용
                        int d_num = 0;  //대출 목록 번호
                        int d_count = 0;
                        read(sock, &d_count, sizeof(int));             //1. 멀록카운트
                        Book2 *d_results = malloc(sizeof(Book2)*d_count);
                        memset(d_results, 0, sizeof(Book2)*d_count);
                        for (int i=0;i<d_count;i++)                     
                        {
                            read(sock, &d_results[i], sizeof(Book2));  //2.대출목록
                            printf("[%d] 대출일:%ld | 상태:%s | 대출자ID:%s | 제목: %s | 권수: %d | ISBN: %s | 주소: %s\n" 
                                ,i+1, d_results[i].loan_time, d_results[i].status, d_results[i].id, d_results[i].title, d_results[i].num_books, d_results[i].isbn, d_results[i].loan_addr);
                        }

                        printf("[1]대출승인\n[2]도서반납\n[3]불량대출자구제\n[4]대출목록수정\n");
                        fgets(cmd, sizeof(cmd), stdin);
                        cmd[strlen(cmd) - 1] = '\0';
                        write(sock, cmd, sizeof(cmd));

                        if(strcmp(cmd, "1") == 0) //대출승인
                        {
                            printf("[1]온라인대출승인\n[2]현장대출승인\n");
                            fgets(cmd, sizeof(cmd), stdin);
                            cmd[strlen(cmd) - 1] = '\0';
                            write(sock, cmd, sizeof(cmd));
                            if(strcmp(cmd, "1") == 0)             //온라인 대출승인
                            {
                                // printf("대출 승인할 번호를 입력해주세요: \n");
                                // num_1 = scanf("%d", &d_num); 
                                // getchar();
                                // write(sock, &num_1, sizeof(int));  //오류분석
                                // write(sock, &d_num, sizeof(int));    //대출 목록 번호
                                // if (num_1== 0 || d_num >d_count || d_num < 0)
                                // {
                                //     printf("목록에서 해당 도서의 []안의 숫자를 입력해 주세요.\n");
                                //     getchar();
                                //     d_results = NULL;
                                //     free(d_results);
                                //     d_results = NULL;
                                //     continue;
                                // }
                                printf("대출 승인할 ID를 입력해 주세요.\n");
                                fgets(loan_id, sizeof(loan_id), stdin);
                                loan_id[strlen(loan_id) - 1] = '\0';
                                write(sock, loan_id, sizeof(loan_id));
                                int d_msg=0; //오류 메시지 확인용
                                read(sock, &d_msg, sizeof(int));
                                if(d_msg == 1)
                                {
                                    printf("대출승인 성공하였습니다.\n");
                                    getchar();
                                }
                                else if(d_msg == 0)
                                {
                                    printf("온라인 대출신청이 3개가 넘지 않아 대출승인 실패하였습니다.\n");
                                    getchar();
                                }
                                d_results = NULL;
                                free(d_results);
                                d_results = NULL;
                                break;
                        
                            }
                            else if(strcmp(cmd, "2") == 0)        //현장대출승인
                            {
                                int d_msg=0; //오류 메시지 확인용
                                printf("대출 승인할 ID를 입력해 주세요.\n");
                                fgets(loan_id, sizeof(loan_id), stdin);
                                loan_id[strlen(loan_id) - 1] = '\0';
                                write(sock, loan_id, sizeof(loan_id));
                                read(sock, &d_msg, sizeof(int));
                                if(d_msg == 0)
                                {
                                    printf("회원이 아닙니다. 아이디를 확인해주세요\n");
                                    getchar();
                                    d_results = NULL;
                                    free(d_results);
                                    d_results = NULL;
                                    break;

                                }
                                else if (d_msg ==1)
                                {
                                    printf("ISBN을 입력해 주십시오.\n");
                                }
                                char loan_isbn[100] = {0};
                                fgets(loan_isbn, sizeof(loan_isbn), stdin);
                                loan_isbn[strlen(loan_isbn) - 1] = '\0';
                                write(sock, loan_isbn, sizeof(loan_isbn));
                                int error_msg = 0;      //0이면 성공, 1이면 수량부족, 2면 같은책 2권 넘어감, 3이면 10권 넘음
                                read(sock, &error_msg, sizeof(int));
                                if(error_msg ==0)
                                {
                                    printf("대출 완료 되었습니다!\n");
                                    getchar();
                                }
                                else if(error_msg ==1)
                                {
                                    printf("수량이 부족합니다.\n");
                                    getchar();
                                }
                                else if(error_msg ==2)
                                {
                                    printf("같은 책을 2권 초과하였습니다.\n");
                                    getchar();
                                }
                                else if(error_msg ==3)
                                {
                                    printf("10권 넘게 빌릴 수 없습니다.\n");
                                    getchar();
                                }
                                else if(error_msg ==4)
                                {
                                    printf("ISBN을 다시 확인하여 주십시오.\n");
                                    getchar();
                                }
                                else if(error_msg ==5)
                                {
                                    printf("불량대출자입니다(대출불가)\n");
                                    getchar();
                                }
                                break;
                            }

                        }
                        else if(strcmp(cmd, "2") == 0) //도서반납
                        {
                            
                        }
                        else if(strcmp(cmd, "3") == 0)  //불량 대출자 구제
                        {
                            printf("불량대출자들이 구제 되었습니다.\n");
                            getchar();
                            break;
                        }
                        else if(strcmp(cmd, "4") == 0)  //대출목록수정
                        {
                            int plag_fail = 0;
                            Book2 b;
                            int book_count = 0;
                            int num_1 = 0;
                            int num_2 = 0;
                            int num_3 = 0;
                            printf("수정할 도서 ISBN: "); fgets(b.isbn, sizeof(b.isbn), stdin); b.isbn[strcspn(b.isbn, "\n")] = 0;
                            printf("새 제목: "); fgets(b.title, sizeof(b.title), stdin); b.title[strcspn(b.title, "\n")] = 0;
                            printf("새 대출자: "); fgets(b.id, sizeof(b.id), stdin); b.id[strcspn(b.id, "\n")] = 0;
                            printf("새 저자: "); fgets(b.author, sizeof(b.author), stdin); b.author[strcspn(b.author, "\n")] = 0;
                            printf("새 출판사: "); fgets(b.publisher, sizeof(b.publisher), stdin); b.publisher[strcspn(b.publisher, "\n")] = 0;
                            
                            printf("(숫자만!!)출판년: "); num_1 = scanf("%d", &b.pub_year); clearbuffer();
                            write(sock, &num_1, sizeof(int));
                            if (num_1== 0)
                            {
                                plag_fail = 1;
                                d_results = NULL;
                                free(d_results);
                                d_results = NULL;
                                break;
                            }
                            
                            printf("(숫자만!!)권 수: "); num_2 = scanf("%d", &b.num_books); clearbuffer();
                            write(sock, &num_1, sizeof(int));
                            if (num_2== 0)
                            {
                                plag_fail = 1;
                                d_results = NULL;
                                free(d_results);
                                d_results = NULL;
                                break;
                            }
                            
                            printf("(숫자만!!)대출시간: "); num_3 = scanf("%ld", &b.loan_time); clearbuffer();
                            write(sock, &num_1, sizeof(int));
                            if (num_3== 0)
                            {
                                plag_fail = 1;
                                d_results = NULL;
                                free(d_results);
                                d_results = NULL;
                                break;
                            }
                            printf("새 승인상태: "); fgets(b.status, sizeof(b.status), stdin); b.status[strcspn(b.status, "\n")] = 0;
                            printf("새 주소: "); fgets(b.loan_addr, sizeof(b.loan_addr), stdin); b.loan_addr[strcspn(b.loan_addr, "\n")] = 0;

                            send(sock, &b, sizeof(Book), 0);
                            read(sock, &result, sizeof(int));
                            printf(result ? "수정 성공\n" : "수정 실패(ISBN을 확인하여 주십시오)\n");
                            getchar();
                            d_results = NULL;
                            free(d_results);
                            d_results = NULL;
                            break;
                            
                        }
                    }
                }
                if (strcmp(cmd, "5") == 0) // 로그아웃
                {
                    plag_exit = 1;
                    break;
                }
            }
        }
        else if (result == 3)       //사서
        {
            printf("사서님 환영합니다.\n");
            printf("1.도서관리\n2.대출정보\n3.모든계정관리\n4.메세지\n5.로그아웃\n"); 
            char cmd[16];
            fgets(cmd, sizeof(cmd), stdin);
            cmd[strlen(cmd) - 1] = '\0';
            send(sock, cmd, sizeof(cmd), 0);
            if (strcmp(cmd, "1") == 0) // 도서관리
            {
                break;
            }
            else if (strcmp(cmd, "2") == 0) // 대출정보
            {
                break;
            }
            else if (strcmp(cmd, "3") == 0) // 모든계정관리
            {
                break;
            }
            else if (strcmp(cmd, "4") == 0) // 메세지
            {
                break;
            }
            else if (strcmp(cmd, "5") == 0) // 로그아웃
            {
                break;
            }
        }
    }
    close(sock);
}