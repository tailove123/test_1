// server.c
// 도서관리 서버 - 멀티 클라이언트, JSON 기반 도서 처리, 로그인/회원가입 기능 포함

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include "/usr/local/include/cjson/cJSON.h"

#define MAX_BOOKS 100
#define PORT 8080

// 도서 구조체 정의
typedef struct {
    int id;
    char title[100];
    char author[50];
    int year;
} Book;

// 전역 변수
Book books[MAX_BOOKS];
int book_count = 0;
pthread_mutex_t book_mutex = PTHREAD_MUTEX_INITIALIZER;

// 도서 파일에서 불러오기
int load_books(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    rewind(fp);

    char *data = malloc(len + 1);
    fread(data, 1, len, fp);
    fclose(fp);
    data[len] = '\0';

    cJSON *root = cJSON_Parse(data);
    if (!root) return 0;

    int count = 0;
    cJSON *book;
    cJSON_ArrayForEach(book, root) {
        books[count].id = cJSON_GetObjectItem(book, "id")->valueint;
        strcpy(books[count].title, cJSON_GetObjectItem(book, "title")->valuestring);
        strcpy(books[count].author, cJSON_GetObjectItem(book, "author")->valuestring);
        books[count].year = cJSON_GetObjectItem(book, "year")->valueint;
        count++;
    }
    cJSON_Delete(root);
    free(data);
    book_count = count;
    return count;
}

// 도서 파일 저장
int save_books(const char *filename) {
    cJSON *root = cJSON_CreateArray();
    for (int i = 0; i < book_count; i++) {
        cJSON *b = cJSON_CreateObject();
        cJSON_AddNumberToObject(b, "id", books[i].id);
        cJSON_AddStringToObject(b, "title", books[i].title);
        cJSON_AddStringToObject(b, "author", books[i].author);
        cJSON_AddNumberToObject(b, "year", books[i].year);
        cJSON_AddItemToArray(root, b);
    }
    char *out = cJSON_Print(root);
    FILE *fp = fopen(filename, "w");
    fputs(out, fp);
    fclose(fp);
    free(out);
    cJSON_Delete(root);
    return 1;
}

// 검색 기능 (제목, 저자, 연도)
int search_books(const char *key, const char *value, Book results[]) {
    int found = 0;
    for (int i = 0; i < book_count; i++) {
        if ((strcmp(key, "title") == 0 && strstr(books[i].title, value)) ||
            (strcmp(key, "author") == 0 && strstr(books[i].author, value)) ||
            (strcmp(key, "year") == 0 && books[i].year == atoi(value))) {
            results[found++] = books[i];
        }
    }
    return found;
}

// 도서 추가
int add_book(Book b) {
    pthread_mutex_lock(&book_mutex);
    books[book_count++] = b;
    save_books("book_data.json");
    pthread_mutex_unlock(&book_mutex);
    return 1;
}

// 도서 삭제
int delete_book(int id) {
    pthread_mutex_lock(&book_mutex);
    int found = 0;
    for (int i = 0; i < book_count; i++) {
        if (books[i].id == id) {
            for (int j = i; j < book_count - 1; j++) books[j] = books[j + 1];
            book_count--;
            found = 1;
            break;
        }
    }
    if (found) save_books("book_data.json");
    pthread_mutex_unlock(&book_mutex);
    return found;
}

// 도서 수정
int modify_book(Book b) {
    pthread_mutex_lock(&book_mutex);
    for (int i = 0; i < book_count; i++) {
        if (books[i].id == b.id) {
            books[i] = b;
            save_books("book_data.json");
            pthread_mutex_unlock(&book_mutex);
            return 1;
        }
    }
    pthread_mutex_unlock(&book_mutex);
    return 0;
}

// 로그인 기능
int login(const char *id, const char *pw) {
    FILE *fp = fopen("users.json", "r");
    if (!fp) return 0;
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    rewind(fp);

    char *data = malloc(len + 1);
    fread(data, 1, len, fp);
    data[len] = '\0';
    fclose(fp);

    cJSON *root = cJSON_Parse(data);
    cJSON *user;
    cJSON_ArrayForEach(user, root) {
        char *uid = cJSON_GetObjectItem(user, "id")->valuestring;
        char *pwd = cJSON_GetObjectItem(user, "password")->valuestring;
        if (strcmp(uid, id) == 0 && strcmp(pwd, pw) == 0) {
            free(data);
            cJSON_Delete(root);
            return 1;
        }
    }
    free(data);
    cJSON_Delete(root);
    return 0;
}

// 회원가입 기능
int register_user(const char *id, const char *pw) {
    FILE *fp = fopen("users.json", "r+");
    if (!fp) return 0;

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    rewind(fp);

    char *data = malloc(len + 1);
    fread(data, 1, len, fp);
    data[len] = '\0';

    cJSON *root = cJSON_Parse(data);
    if (!root) {
        root = cJSON_CreateArray(); // 비어있을 경우
    }

    cJSON *user;
    cJSON_ArrayForEach(user, root) {
        char *uid = cJSON_GetObjectItem(user, "id")->valuestring;
        if (strcmp(uid, id) == 0) {
            free(data);
            cJSON_Delete(root);
            return 0;
        }
    }

    cJSON *new_user = cJSON_CreateObject();
    cJSON_AddStringToObject(new_user, "id", id);
    cJSON_AddStringToObject(new_user, "password", pw);
    cJSON_AddItemToArray(root, new_user);

    char *out = cJSON_Print(root);
    freopen("users.json", "w", fp);
    fputs(out, fp);
    fclose(fp);

    free(data);
    free(out);
    cJSON_Delete(root);
    return 1;
}

// 클라이언트 요청 처리 쓰레드 함수
void *client_handler(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);

    char cmd[16], id[50], pw[50];
    read(client_socket, cmd, sizeof(cmd));
    read(client_socket, id, sizeof(id));
    read(client_socket, pw, sizeof(pw));

    int result = 0;
    if (strcmp(cmd, "login") == 0) {
        result = login(id, pw);
    } else if (strcmp(cmd, "register") == 0) {
        result = register_user(id, pw);
    }
    write(client_socket, &result, sizeof(int));

    if (result == 1 && strcmp(cmd, "login") == 0) {
        while (1) {
            char action[16];
            read(client_socket, action, sizeof(action));

            if (strcmp(action, "search") == 0) {
                char key[16], val[100];
                Book found[MAX_BOOKS];
                read(client_socket, key, sizeof(key));
                read(client_socket, val, sizeof(val));
                int count = search_books(key, val, found);
                write(client_socket, &count, sizeof(int));
                write(client_socket, found, sizeof(Book) * count);
            } else if (strcmp(action, "add") == 0) {
                Book b;
                read(client_socket, &b, sizeof(Book));
                result = add_book(b);
                write(client_socket, &result, sizeof(int));
            } else if (strcmp(action, "delete") == 0) {
                int id;
                read(client_socket, &id, sizeof(int));
                result = delete_book(id);
                write(client_socket, &result, sizeof(int));
            } else if (strcmp(action, "modify") == 0) {
                Book b;
                read(client_socket, &b, sizeof(Book));
                result = modify_book(b);
                write(client_socket, &result, sizeof(int));
            } else if (strcmp(action, "exit") == 0) {
                break;
            }
        }
    }

    close(client_socket);
    return NULL;
}

// 메인 함수: 서버 시작 및 연결 대기
int main() {
    load_books("book_data.json");

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 5);
    printf("서버 실행 중...\n");

    while (1) {
        int *client_socket = malloc(sizeof(int));
        socklen_t addrlen = sizeof(address);
        *client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, client_socket);
        pthread_detach(tid);
    }

    return 0;
}