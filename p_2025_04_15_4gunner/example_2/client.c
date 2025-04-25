#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct {
    int id;
    char title[100];
    char author[50];
    int year;
} Book;

#define PORT 8080

int main() {
    int sock;
    struct sockaddr_in serv_addr = {0};
    char choice[10], id[50], pw[50];

    printf("1. 로그인\n2. 회원가입\n>> ");
    fgets(choice, sizeof(choice), stdin);

    printf("ID: "); fgets(id, sizeof(id), stdin); id[strcspn(id, "\n")] = 0;
    printf("PW: "); fgets(pw, sizeof(pw), stdin); pw[strcspn(pw, "\n")] = 0;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    if (choice[0] == '1') send(sock, "login", 16, 0);
    else send(sock, "register", 16, 0);
    send(sock, id, sizeof(id), 0);
    send(sock, pw, sizeof(pw), 0);

    int result;
    read(sock, &result, sizeof(int));
    if (result != 1) {
        printf("실패했습니다.\n");
        close(sock);
        return 0;
    }

    printf("성공적으로 로그인/회원가입되었습니다.\n");

    while (1) {
        char cmd[16];
        printf("\n명령어 입력 (search/add/delete/modify/exit): ");
        fgets(cmd, sizeof(cmd), stdin); cmd[strcspn(cmd, "\n")] = 0;
        send(sock, cmd, sizeof(cmd), 0);

        if (strcmp(cmd, "search") == 0) {
            char key[16], val[100];
            printf("검색 기준(title/author/year): "); fgets(key, sizeof(key), stdin); key[strcspn(key, "\n")] = 0;
            printf("검색어 입력: "); fgets(val, sizeof(val), stdin); val[strcspn(val, "\n")] = 0;
            send(sock, key, sizeof(key), 0);
            send(sock, val, sizeof(val), 0);

            int count;
            Book results[100];
            read(sock, &count, sizeof(int));
            read(sock, results, sizeof(Book) * count);
            for (int i = 0; i < count; i++) {
                printf("[%d] %s - %s (%d)\n", results[i].id, results[i].title, results[i].author, results[i].year);
            }
        } else if (strcmp(cmd, "add") == 0) {
            Book b;
            printf("ID: "); scanf("%d", &b.id); getchar();
            printf("제목: "); fgets(b.title, sizeof(b.title), stdin); b.title[strcspn(b.title, "\n")] = 0;
            printf("저자: "); fgets(b.author, sizeof(b.author), stdin); b.author[strcspn(b.author, "\n")] = 0;
            printf("연도: "); scanf("%d", &b.year); getchar();
            send(sock, &b, sizeof(Book), 0);
            read(sock, &result, sizeof(int));
            printf(result ? "도서 추가 성공\n" : "도서 추가 실패\n");
        } else if (strcmp(cmd, "delete") == 0) {
            int id;
            printf("삭제할 도서 ID: "); scanf("%d", &id); getchar();
            send(sock, &id, sizeof(int), 0);
            read(sock, &result, sizeof(int));
            printf(result ? "삭제 성공\n" : "삭제 실패\n");
        } else if (strcmp(cmd, "modify") == 0) {
            Book b;
            printf("수정할 도서 ID: "); scanf("%d", &b.id); getchar();
            printf("새 제목: "); fgets(b.title, sizeof(b.title), stdin); b.title[strcspn(b.title, "\n")] = 0;
            printf("새 저자: "); fgets(b.author, sizeof(b.author), stdin); b.author[strcspn(b.author, "\n")] = 0;
            printf("새 연도: "); scanf("%d", &b.year); getchar();
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