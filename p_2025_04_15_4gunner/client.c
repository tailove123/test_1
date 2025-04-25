#include <stdio.h>              // 표준 입출력
#include <stdlib.h>             // 메모리/문자열 함수
#include <string.h>             // 문자열 처리 함수
#include <unistd.h>             // read(), write(), close() 등
#include <netinet/in.h>         // sockaddr_in 구조체 정의
#include <arpa/inet.h>          // inet_pton() 함수
#define SIZE 200

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


#define PORT 8080  // 서버 포트

int main() {
    int sock;
    struct sockaddr_in serv_addr = {0};
    char choice[10], id[50], pw[50];

    // 로그인 또는 회원가입 메뉴
    printf("1. 로그인\n2. 회원가입\n>> ");
    fgets(choice, sizeof(choice), stdin);

    // 사용자 ID 입력
    printf("ID: ");
    fgets(id, sizeof(id), stdin);
    id[strcspn(id, "\n")] = 0;  // 개행 제거

    // 비밀번호 입력
    printf("PW: ");
    fgets(pw, sizeof(pw), stdin);
    pw[strcspn(pw, "\n")] = 0;

    // 소켓 생성
    sock = socket(AF_INET, SOCK_STREAM, 0);

    // 서버 주소 설정
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    // 서버에 연결
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    // 로그인 or 회원가입 명령 전송
    if (choice[0] == '1') send(sock, "login", 16, 0);
    else send(sock, "register", 16, 0);
    send(sock, id, sizeof(id), 0);
    send(sock, pw, sizeof(pw), 0);

    // 결과 수신
    int result;
    read(sock, &result, sizeof(int));
    if (result != 1) {
        printf("실패했습니다.\n");
        close(sock);
        return 0;
    }

    printf("성공적으로 로그인/회원가입되었습니다.\n");

    // 명령어 루프
    while (1) {
        char cmd[16];
        printf("\n명령어 입력 (search/add/delete/modify/exit): ");
        fgets(cmd, sizeof(cmd), stdin);
        cmd[strcspn(cmd, "\n")] = 0;
        send(sock, cmd, sizeof(cmd), 0);

        if (strcmp(cmd, "search") == 0) {
            char key[16], val[100];
            printf("검색 기준(title/author): ");
            fgets(key, sizeof(key), stdin);
            key[strcspn(key, "\n")] = 0;

            printf("검색어 입력: ");
            fgets(val, sizeof(val), stdin);
            val[strcspn(val, "\n")] = 0;

            send(sock, key, sizeof(key), 0);
            send(sock, val, sizeof(val), 0);

            int count;
            Book results[100];
            read(sock, &count, sizeof(int));
            read(sock, results, sizeof(Book) * count);

            for (int i = 0; i < count; i++) {
                printf("[%d] 제목 : %s 글쓴이 : %s 출판사 : %s 출판년 : %d\n", i, results[i].title, results[i].author, results[i].publisher, results[i].pub_year);
            }

        } else if (strcmp(cmd, "add") == 0) {
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

        } else if (strcmp(cmd, "delete") == 0) {
            int id;
            printf("삭제할 도서 ISBN: "); scanf("%d", &id); getchar();
            send(sock, &id, sizeof(int), 0);
            read(sock, &result, sizeof(int));
            printf(result ? "삭제 성공\n" : "삭제 실패\n");

        } else if (strcmp(cmd, "modify") == 0) {
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

        } else if (strcmp(cmd, "exit") == 0) {
            break;
        }
    }

    close(sock);
    return 0;
}
