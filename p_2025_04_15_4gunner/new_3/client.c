#include <stdio.h>              // 표준 입출력
#include <stdlib.h>             // 메모리/문자열 함수
#include <string.h>             // 문자열 처리 함수
#include <unistd.h>             // read(), write(), close() 등
#include <netinet/in.h>         // sockaddr_in 구조체 정의
#include <arpa/inet.h>          // inet_pton() 함수
#pragma pack(1)
#define SIZE 100
#define MAX_BOOKS 11000
#define PORT 7000  // 서버 포트
#define MAX_LOANS 100  // 대출 가능한 최대 도서 수

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
    int pubyear; // 출판년
    int num_books; //빌린 권수 
    char isbn[SIZE];  // ISBN
    int loan_time; //대출일
    char status[SIZE];
} Book2;


void clear_newline(char *str) {
    str[strcspn(str, "\n")] = 0; // Remove the newline character
}

int main() {
    int sock;
    struct sockaddr_in serv_addr = {0};
    char choice[10], id[50], pw[50];
    char nickname[50], phone[50], address[100];
    int year;
    int message_a = 1;
    int plag_exit = 0;
    int d_count = 0;
    Book2 d_results[MAX_LOANS];  // 대출한 책 저장용
    
    while(1)
    {
        printf("1.로그인\n2.회원가입\n3.대출하기\n4.반납하기>> ");
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

            printf("성공적으로 로그인이 완료되었습니다.%d\n",result);

    // 명령어 루프
            if (result == 1)
            {
                while (plag_exit == 0)
                {
                    char cmd[16];
                    int num = 0;
                    printf("\n명령어 입력 (search/add/delete/modify/exit): ");
                    fgets(cmd, sizeof(cmd), stdin);
                    cmd[strcspn(cmd, "\n")] = 0;
                    send(sock, cmd, sizeof(cmd), 0);
            
                    if (strcmp(cmd, "search") == 0)
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
                            printf("대출할 번호를 입력해주세요: ");
                            scanf("%d", &num); 
                            getchar();
                            strcpy(d_results[d_count].title, p_results[num].title);
                            strcpy(d_results[d_count].author, p_results[num].author);
                            strcpy(d_results[d_count].publisher, p_results[num].publisher);
                            d_results[d_count].pubyear = p_results[num].pub_year;
                            strcpy(d_results[d_count].isbn, p_results[num].isbn);
                            strcpy(d_results[d_count].id, id); // 유저 ID 저장
                            printf("권수를 입력해주세요: ");
                            int num_2;
                            scanf("%d", &num_2); 
                            getchar();
                            printf(">> 대출 내역:\n");
                            for (int i = 0; i <= d_count; i++) {
                                printf("ID: %s | 제목: %s | 저자: %s | 출판사: %s | 출판년: %d | ISBN: %s | 권수: %d\n",
                                    d_results[i].id, d_results[i].title, d_results[i].author,
                                    d_results[i].publisher, d_results[i].pubyear,
                                    d_results[i].isbn, num_2);
                            }
                            d_count++;
                        }
                        else
                        printf("검색 결과가 없습니다.\n");
                        free(p_results);
                    } 
                    else if (strcmp(cmd, "add") == 0)
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
                    else if (strcmp(cmd, "delete") == 0)
                    {
                        char isbn[100];
                        printf("삭제할 도서 ISBN: "); fgets(isbn, sizeof(isbn), stdin); isbn[strcspn(isbn, "\n")] = 0;
                        write(sock, isbn, sizeof(isbn));
                        read(sock, &result, sizeof(int));
                        printf(result ? "삭제 성공\n" : "삭제 실패\n");

                    }
                    else if (strcmp(cmd, "modify") == 0)
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
                        if (plag_fail == 1)
                        {
                            printf("도서 수정 실패(숫자만!! : 붙여진 숫자만 입력해 주세요.\n");
                            plag_fail = 0;
                        }
                    }
                    else if (strcmp(cmd, "exit") == 0)
                    {
                        plag_exit = 1;
                        break;
                    }
                }
            }
            else if (result == 2)
            {
                while(1){
                    printf("관리자님 환영합니다.\n");
                    printf("1.도서관리\n2.모든계정관리\n3.도서관오픈관리\n4.대출정보\n5.로그아웃\n");
                    char cmd[16];
                    fgets(cmd, sizeof(cmd), stdin);
                    cmd[strcspn(cmd, "\n")] = 0;
                    send(sock, cmd, sizeof(cmd), 0);
                    if (strcmp(cmd, "5") == 0)
                    {
                        plag_exit = 1;
                        break;
                    }
                }
            }
            else if (result == 3){
                printf("사서님 환영합니다.\n");
                printf("1.도서관리\n2.대출정보\n3.모든계정관리\n4.메세지\n5.로그아웃\n"); 
                char cmd[16];
                fgets(cmd, sizeof(cmd), stdin);
                cmd[strcspn(cmd, "\n")] = 0;
                send(sock, cmd, sizeof(cmd), 0);
                if (strcmp(cmd, "5") == 0)
                {
                    plag_exit = 1;
                    break;
                }
            }
            close(sock);
        }
    }
    
}