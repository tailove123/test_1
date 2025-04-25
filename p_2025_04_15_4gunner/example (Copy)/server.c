#include <stdio.h>              // 표준 입출력 함수 사용 (printf, fopen 등)
#include <stdlib.h>             // 동적 메모리 할당 (malloc, free 등)
#include <string.h>             // 문자열 처리 함수 (strcmp, strcpy 등)
#include <unistd.h>             // POSIX API (read, write, close 등)
#include <pthread.h>            // 쓰레드 사용을 위한 헤더
#include <netinet/in.h>         // 소켓 주소 구조체를 위한 헤더 (struct sockaddr_in 등)
// #include "/usr/local/include/cjson/cJSON.h"   // JSON 처리 라이브러리 헤더
#include "cJSON.h"
#include "cJSON.c"

#define MAX_BOOKS 20000           // 도서 최대 등록 수
#define PORT 8080               // 서버가 열릴 포트 번호
#define SIZE 200
// 도서 구조체 정의
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

// 전역 도서 배열과 관련 변수 선언
Book books[MAX_BOOKS];          // 도서 목록을 저장할 배열
int book_count = 0;             // 현재 등록된 도서 수
pthread_mutex_t book_mutex = PTHREAD_MUTEX_INITIALIZER;  // 도서 데이터 접근 동기화를 위한 뮤텍스

// 도서 목록을 JSON 파일에서 불러오는 함수
int load_books(const char *filename) {
    FILE *fp = fopen(filename, "r");      // 파일 열기
    if (!fp) return 0;                    // 파일이 없으면 0 리턴

    fseek(fp, 0, SEEK_END);               // 파일 끝으로 이동
    long len = ftell(fp);                // 파일 길이 측정
    rewind(fp);                           // 다시 파일 처음으로 이동

    char *data = malloc(len + 1);         // JSON 데이터 저장할 메모리 할당
    fread(data, 1, len, fp);              // 파일 데이터 읽기
    fclose(fp);                           // 파일 닫기
    data[len] = '\0';                    // 문자열 종료 문자

    cJSON *root = cJSON_Parse(data);      // JSON 파싱
    if (!root) return 0;                  // 파싱 실패 시 0 반환

    int count = 0;
    cJSON *book;
    cJSON_ArrayForEach(book, root) {      // JSON 배열 순회
        books[count].no = cJSON_GetObjectItem(book, "No")->valueint;
        strcpy(books[count].title, cJSON_GetObjectItem(book, "제목")->valuestring);  
        strcpy(books[count].author, cJSON_GetObjectItem(book, "저자")->valuestring);
        strcpy(books[count].publisher, cJSON_GetObjectItem(book, "출판사")->valuestring);
        books[count].pub_year = cJSON_GetObjectItem(book, "출판년")->valueint;
        books[count].num_books = cJSON_GetObjectItem(book, "권")->valueint;
        strcpy(books[count].isbn, cJSON_GetObjectItem(book, "ISBN")->valuestring);
        strcpy(books[count].extra_n, cJSON_GetObjectItem(book, "부가기호")->valuestring);
        strcpy(books[count].kdc, cJSON_GetObjectItem(book, "KDC")->valuestring);
        strcpy(books[count].kdc_subject, cJSON_GetObjectItem(book, "KDC 주제명")->valuestring);
        books[count].loan_frequency = cJSON_GetObjectItem(book, "대출 빈도")->valueint;
        
        count++;                                                                // 도서 수 증가
    }

    cJSON_Delete(root);            // JSON 객체 해제
    free(data);                    // 메모리 해제
    book_count = count;            // 총 도서 수 저장
    return count;                  // 로드된 도서 수 반환
}

// 도서 목록을 JSON 파일로 저장하는 함수
int save_books(const char *filename) {
    cJSON *root = cJSON_CreateArray();     // 빈 JSON 배열 생성
    for (int i = 0; i < book_count; i++) {
        cJSON *b = cJSON_CreateObject();                                   // 각 도서를 위한 객체 생성
        cJSON_AddNumberToObject(b, "No", books[i].no);
        cJSON_AddStringToObject(b, "제목", books[i].title);            // 저자 추가
        cJSON_AddStringToObject(b, "저자", books[i].author);
        cJSON_AddStringToObject(b, "출판사", books[i].publisher);
        cJSON_AddNumberToObject(b, "출판년", books[i].pub_year);
        cJSON_AddNumberToObject(b, "권", books[i].num_books);
        cJSON_AddStringToObject(b, "ISBN", books[i].isbn);
        cJSON_AddStringToObject(b, "부가기호", books[i].extra_n);
        cJSON_AddStringToObject(b, "KDC", books[i].kdc);
        cJSON_AddStringToObject(b, "KDC 주제명", books[i].kdc_subject);
        cJSON_AddNumberToObject(b, "대출 빈도", books[i].loan_frequency);
        cJSON_AddItemToArray(root, b);                                    // 배열에 추가
    }
    char *out = cJSON_Print(root);                // JSON 문자열로 변환
    FILE *fp = fopen(filename, "w");              // 파일 쓰기 모드로 열기
    fputs(out, fp);                               // 파일에 저장
    fclose(fp);                                   // 파일 닫기
    free(out);                                    // 문자열 메모리 해제
    cJSON_Delete(root);                           // JSON 배열 해제
    return 1;
}

// 키워드로 도서 검색
int search_books(const char *key, const char *value, Book results[]) {
    int found = 0;
    for (int i = 0; i < book_count; i++) {
        if ((strcmp(key, "title") == 0 && strstr(books[i].title, value)) ||
            (strcmp(key, "author") == 0 && strstr(books[i].author, value))) 
        {
            results[found++] = books[i];  // 검색 결과에 추가
        }
    }
    return found;  // 검색된 도서 수 반환
}

// 도서 추가
int add_book(Book b) {
    pthread_mutex_lock(&book_mutex);          // 쓰레드 동기화를 위한 뮤텍스 잠금
    books[book_count++] = b;                  // 배열에 도서 추가
    save_books("book_data.json");             // 저장
    pthread_mutex_unlock(&book_mutex);        // 뮤텍스 해제
    return 1;
}

// 도서 삭제
int delete_book(char *isbn) {
    pthread_mutex_lock(&book_mutex);
    int found = 0;
    for (int i = 0; i < book_count; i++)
    {
        if (strcmp(books[i].isbn, isbn) ==0)
        {
            for (int j = i; j < book_count - 1; j++) books[j] = books[j + 1];  // 뒤로 밀기
            book_count--;
            found = 1;
            break;
        }
    }
    if (found) save_books("DATA.json");
    pthread_mutex_unlock(&book_mutex);
    return found;
}

// 도서 수정
int modify_book(Book b) {
    pthread_mutex_lock(&book_mutex);
    for (int i = 0; i < book_count; i++) {
        if (strcmp(books[i].isbn, b.isbn) == 0)
        {
            books[i] = b;
            save_books("DATA.json");
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
    if (!root) root = cJSON_CreateArray();

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

// 클라이언트 요청을 처리하는 쓰레드 함수
void *client_handler(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);  // 할당된 메모리 해제

    char cmd[16], id[50], pw[50];
    read(client_socket, cmd, sizeof(cmd));  // 명령: login / register
    read(client_socket, id, sizeof(id));    // 사용자 ID
    read(client_socket, pw, sizeof(pw));    // 비밀번호

    int result = 0;
    if (strcmp(cmd, "login") == 0) result = login(id, pw);
    else if (strcmp(cmd, "register") == 0) result = register_user(id, pw);
    write(client_socket, &result, sizeof(int));

    if (result == 1 && strcmp(cmd, "login") == 0) {
        while (1) {
            char action[16];
            read(client_socket, action, sizeof(action));  // 사용자 요청

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
                char isbn[200];
                read(client_socket, isbn, sizeof(char));
                result = delete_book(isbn);
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

    close(client_socket);  // 소켓 종료
    return NULL;
}

// 메인 함수 - 서버 실행
int main() {
    load_books("book_data.json");  // 도서 데이터 불러오기

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);  // 소켓 생성
    struct sockaddr_in address = {0};                 // 주소 구조체 초기화
    address.sin_family = AF_INET;                     // IPv4 사용
    address.sin_addr.s_addr = INADDR_ANY;             // 모든 IP 허용
    address.sin_port = htons(PORT);                   // 포트 설정

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));  // 바인딩
    listen(server_fd, 5);                            // 연결 대기
    printf("서버 실행 중...\n");

    while (1) {
        int *client_socket = malloc(sizeof(int));  // 동적 할당된 소켓 저장용
        socklen_t addrlen = sizeof(address);
        *client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen); // 연결 수락

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, client_socket);  // 쓰레드 생성
        pthread_detach(tid);  // 쓰레드 자원 자동 해제
    }

    return 0;
}