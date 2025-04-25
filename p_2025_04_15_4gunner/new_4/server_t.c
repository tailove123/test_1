#include <stdio.h>              // 표준 입출력 함수 사용 (printf, fopen 등)
#include <stdlib.h>             // 동적 메모리 할당 (malloc, free 등)
#include <string.h>             // 문자열 처리 함수 (strcmp, strcpy 등)
#include <unistd.h>             // POSIX API (read, write, close 등)
#include <pthread.h>            // 쓰레드 사용을 위한 헤더
#include <netinet/in.h>         // 소켓 주소 구조체를 위한 헤더 (struct sockaddr_in 등)
#pragma pack(1)
#include "cJSON.h"              // JSON 처리 라이브러리 헤더
#include "cJSON.c"
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <dirent.h>

#define MAX_BOOKS 11000           // 도서 최대 등록 수
#define MAX_USERS 500
#define PORT 9000             // 서버가 열릴 포트 번호
#define SIZE 100

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

typedef struct {
    char id[SIZE]; // id
    char title[SIZE];            // 제목
    char author[SIZE];            // 저자
    char publisher[SIZE];              // 출판사
    int pub_year;              //출판년
    int num_books;            //권
    char isbn[SIZE];                 //ISBN
    int d_time;      // 대출시간
    char ready[SIZE];        // 승인상태
    char loan_addr[SIZE]; // 주소
} Book2;

typedef struct {
    char id[50];
    char pw[50];
    char name[50];
    int age;
    char phone[50];
    char addr[50];
    int msc; // 메세지카운트
    int bsc;
} User;

typedef struct {

    char date[100];
    char day_[50];
    int is_open;  // 1이면 영업 0이면 휴일
}bussiness_month;



// 전역 도서 배열과 관련 변수 선언
Book books[MAX_BOOKS];          // 도서 목록을 저장할 배열
User Users[MAX_USERS];
Book2 book_2s[MAX_BOOKS];
int book_count = 0;             // 현재 등록된 도서 수
int user_count = 0; // 현재 등록된 유저 수
int book_2_count = 0; //현재 대출한 유저의 수
pthread_mutex_t book_mutex = PTHREAD_MUTEX_INITIALIZER;  // 도서 데이터 접근 동기화를 위한 뮤텍스
pthread_mutex_t book2_mutex = PTHREAD_MUTEX_INITIALIZER;  // 대출정보 데이터 접근 동기화를 위한 뮤텍스

void librarian_message(char *id, int client_socket);
void librarian_sendmessage(char *id, int client_socket);
void librarian_checkmessage(char *id, int client_socket);
//클라이언트의 메세지 함수
void client_message(int client_socket);


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
int load_user(const char *filename) {
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

    int count_2 = 0;
    cJSON *User;
    cJSON_ArrayForEach(User, root) {      // JSON 배열 순회
        strcpy(Users[count_2].id, cJSON_GetObjectItem(User, "id")->valuestring);
        strcpy(Users[count_2].pw, cJSON_GetObjectItem(User, "password")->valuestring);  
        strcpy(Users[count_2].name, cJSON_GetObjectItem(User, "name")->valuestring);
        Users[count_2].age = cJSON_GetObjectItem(User, "age")->valueint;
        strcpy(Users[count_2].phone, cJSON_GetObjectItem(User, "phone")->valuestring);
        strcpy(Users[count_2].addr, cJSON_GetObjectItem(User, "addr")->valuestring);
        Users[count_2].msc = cJSON_GetObjectItem(User, "messagecount")->valueint;
        Users[count_2].msc = cJSON_GetObjectItem(User, "bullcount")->valueint;
        count_2++;                                                                // 도서 수 증가
    }
    
    cJSON_Delete(root);            // JSON 객체 해제
    free(data);                    // 메모리 해제
    user_count = count_2;            // 총 도서 수 저장
    return count_2;                  // 로드된 도서 수 반환
}

int load_book_2(const char *filename) { // 대출목록
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

    int count_3 = 0;
    cJSON *Book2;
    cJSON_ArrayForEach(Book2, root) {      // JSON 배열 순회
        strcpy(book_2s[count_3].id, cJSON_GetObjectItem(Book2, "id")->valuestring);
        strcpy(book_2s[count_3].title, cJSON_GetObjectItem(Book2, "제목")->valuestring);  
        strcpy(book_2s[count_3].author, cJSON_GetObjectItem(Book2, "저자")->valuestring);
        strcpy(book_2s[count_3].publisher, cJSON_GetObjectItem(Book2, "출판사")->valuestring);
        book_2s[count_3].pub_year = cJSON_GetObjectItem(Book2, "출판년")->valueint;
        book_2s[count_3].num_books = cJSON_GetObjectItem(Book2, "권")->valueint;
        strcpy(book_2s[count_3].isbn, cJSON_GetObjectItem(Book2, "ISBN")->valuestring);
        book_2s[count_3].d_time = cJSON_GetObjectItem(Book2, "대출 시간")->valueint;
        strcpy(book_2s[count_3].ready, cJSON_GetObjectItem(Book2, "상태")->valuestring);
        strcpy(book_2s[count_3].loan_addr, cJSON_GetObjectItem(Book2, "주소")->valuestring);

        count_3++;                                                                // 도서 수 증가
    }
    cJSON_Delete(root);            // JSON 객체 해제
    free(data);                    // 메모리 해제
    book_2_count = count_3;            // 총 도서 수 저장
    return count_3;                  // 로드된 도서 수 반환
}

int search_user_by_id(char *user_id)
{
    for(int i=0; i<user_count; i++)
    {
        if(strcmp(Users[i].id, user_id)==0)
        {
            return 1; // 찾으면 1 리턴
        }
    }
    return 0; //기본값은 못찾았는걸로
}

int save_user(const char *filename) {
    cJSON *root = cJSON_CreateArray();
    for (int i = 0; i < user_count; i++) {
        


        cJSON *u = cJSON_CreateObject();                                   
        cJSON_AddStringToObject(u, "id", Users[i].id);
        cJSON_AddStringToObject(u, "password", Users[i].pw);           
        cJSON_AddStringToObject(u, "name", Users[i].name);
        cJSON_AddNumberToObject(u, "age", Users[i].age);
        cJSON_AddStringToObject(u, "phone", Users[i].phone);
        cJSON_AddStringToObject(u, "addr", Users[i].addr);
        cJSON_AddNumberToObject(u, "messagecount", Users[i].msc);
        cJSON_AddNumberToObject(u, "bullcount", Users[i].bsc);
        cJSON_AddItemToArray(root, u);      
                                    // 배열에 추가
    }

    

    char *out = cJSON_Print(root);                // JSON 문자열로 변환
    FILE *fp = fopen(filename, "w");              // 파일 쓰기 모드로 열기
    fputs(out, fp);                               // 파일에 저장
    fclose(fp);                                   // 파일 닫기
    free(out);                                    // 문자열 메모리 해제
    cJSON_Delete(root);
                               // JSON 배열 해제
    return 1;
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

int save_book3s(const char *filename,int book,char *isbn) { // 대출목록 수정하는함수
    cJSON *root = cJSON_CreateArray();     // 빈 JSON 배열 생성
    for (int i = 0; i < book_2_count; i++) {
        if(strcmp(book_2s[i].isbn,isbn) == 0){
        cJSON *b2 = cJSON_CreateObject();                                   // 각 도서를 위한 객체 생성
        cJSON_AddStringToObject(b2, "id", book_2s[i].id);
        cJSON_AddStringToObject(b2, "제목", book_2s[i].title);            // 저자 추가
        cJSON_AddStringToObject(b2, "저자", book_2s[i].author);
        cJSON_AddStringToObject(b2, "출판사", book_2s[i].publisher);
        cJSON_AddNumberToObject(b2, "출판년", book_2s[i].pub_year);
        cJSON_AddNumberToObject(b2, "권", book_2s[i].num_books - book);
        cJSON_AddStringToObject(b2, "ISBN", book_2s[i].isbn);
        cJSON_AddNumberToObject(b2, "대출 시간", book_2s[i].d_time);
        cJSON_AddStringToObject(b2, "상태", book_2s[i].ready);
        cJSON_AddStringToObject(b2, "주소", book_2s[i].loan_addr);
        cJSON_AddItemToArray(root, b2);
        }                                    // 배열에 추가
    }
    char *out = cJSON_Print(root);                // JSON 문자열로 변환
    FILE *fp = fopen(filename, "w");              // 파일 쓰기 모드로 열기
    fputs(out, fp);                               // 파일에 저장
    fclose(fp);                                   // 파일 닫기
    free(out);                                    // 문자열 메모리 해제
    cJSON_Delete(root);                           // JSON 배열 해제
    return 1;
}

int save_book2s(const char *filename) { // 대출목록을 저장하는 함수
    cJSON *root = cJSON_CreateArray();     // 빈 JSON 배열 생성
    for (int i = 0; i < book_2_count; i++) {
        cJSON *b2 = cJSON_CreateObject();                                   // 각 도서를 위한 객체 생성
        cJSON_AddStringToObject(b2, "id", book_2s[i].id);
        cJSON_AddStringToObject(b2, "제목", book_2s[i].title);            // 저자 추가
        cJSON_AddStringToObject(b2, "저자", book_2s[i].author);
        cJSON_AddStringToObject(b2, "출판사", book_2s[i].publisher);
        cJSON_AddNumberToObject(b2, "출판년", book_2s[i].pub_year);
        cJSON_AddNumberToObject(b2, "권", book_2s[i].num_books);
        cJSON_AddStringToObject(b2, "ISBN", book_2s[i].isbn);
        cJSON_AddNumberToObject(b2, "대출 시간", book_2s[i].d_time);
        cJSON_AddStringToObject(b2, "상태", book_2s[i].ready);
        cJSON_AddStringToObject(b2, "주소", book_2s[i].loan_addr);
        cJSON_AddItemToArray(root, b2);                                    // 배열에 추가
    }
    char *out = cJSON_Print(root);                // JSON 문자열로 변환
    FILE *fp = fopen(filename, "w");              // 파일 쓰기 모드로 열기
    fputs(out, fp);                               // 파일에 저장
    fclose(fp);                                   // 파일 닫기
    free(out);                                    // 문자열 메모리 해제
    cJSON_Delete(root);                           // JSON 배열 해제
    return 1;
}

// 키워드로 도서 검색 후 갯수 반환
int search_books_count(const char *key, const char *value) {
    int found = 0;
    for (int i = 0; i < book_count; i++)
    {
        if ((strcmp(key, "title") == 0 && strstr(books[i].title, value)) || (strcmp(key, "author") == 0 && strstr(books[i].author, value))) 
        {
            found++;  // 검색 결과에 추가

        }
    }
    return found;  // 검색된 도서 수 반환
}
// 키워드로 도서 검색 후 반환
void search_books(const char *key, const char *value, Book *results) {
    int found = 0;
    for (int i = 0; i < book_count; i++)
    {
        if ((strcmp(key, "title") == 0 && strstr(books[i].title, value)) || (strcmp(key, "author") == 0 && strstr(books[i].author, value))) 
        {
            results[found++] = books[i];  // 검색 결과에 추가

        }
    }
}


// 도서 추가
int add_book(Book b) {
    pthread_mutex_lock(&book_mutex);          // 쓰레드 동기화를 위한 뮤텍스 잠금
    books[book_count++] = b;                  // 배열에 도서 추가
    save_books("DATA.json");             // 저장
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

int delete_book2(char *isbn) {
    pthread_mutex_lock(&book2_mutex);
    int found = 0;
    for (int i = 0; i < book_2_count; i++)
    {
        if (strcmp(book_2s[i].isbn, isbn) == 0)
        {
            for (int j = i; j < book_2_count - 1; j++) book_2s[j] = book_2s[j + 1];  // 뒤로 밀기
            book_2_count--;
            found = 1;
            break;
        }
    }
    if (found) save_book2s("DATA2.json");
    pthread_mutex_unlock(&book2_mutex);
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

int modify_book2s(Book2 b,int book,char *isbn) {
    pthread_mutex_lock(&book2_mutex);
    for (int i = 0; i < book_2_count; i++) {
        if (strcmp(book_2s[i].isbn, b.isbn) == 0)
        {
            book_2s[i] = b;
            save_book3s("DATA2.json",book,isbn);
            pthread_mutex_unlock(&book2_mutex);
            return 1;
        }
    }
    pthread_mutex_unlock(&book2_mutex);
    return 0;
}
void create_user_folder(const char *id) {
    char path[256];

    // 기본 경로 + 사용자명으로 폴더 경로 구성
    snprintf(path, sizeof(path), "/home/yang/C/project11/user/%s", id);

    // 폴더 생성 (0755 권한)
    if (mkdir(path, 0755) == -1) {
        if (errno == EEXIST) {
            printf("이미 폴더가 존재합니다: %s\n", path);
        } else {
            perror("폴더 생성 실패");
            exit(EXIT_FAILURE);
        }
    } else {
        printf("폴더가 생성되었습니다: %s\n", path);
    }
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
        if (strcmp(uid, "admin") == 0 && strcmp(id, "admin") == 0 && strcmp(pwd, pw) == 0){
            free(data);
            cJSON_Delete(root);
            return 2;
        }
        if (strcmp(uid, "saseo") == 0 && strcmp(id, "saseo") == 0 && strcmp(pwd, pw) == 0){
            free(data);
            cJSON_Delete(root);
            return 3;
        }
        else if (strcmp(uid, id) == 0 && strcmp(pwd, pw) == 0) {
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
int register_user(const char *id, const char *pw, const char *nickname, int year, const char *phone, const char *address, int messege_a, int bull_count) {
    FILE *fp = fopen("users.json", "r");
    cJSON *root = NULL;

    if (fp) {
        fseek(fp, 0, SEEK_END);
        long len = ftell(fp);
        rewind(fp);

        char *data = malloc(len + 1);
        fread(data, 1, len, fp);
        data[len] = '\0';
        fclose(fp);

        root = cJSON_Parse(data);
        free(data);
    }

    if (!root) {
        root = cJSON_CreateArray(); // 파일이 없거나 JSON 파싱 실패 시 새로 생성
    }

    // 중복 아이디 체크
    cJSON *user;
    cJSON_ArrayForEach(user, root) {
        char *uid = cJSON_GetObjectItem(user, "id")->valuestring;
        if (strcmp(uid, id) == 0) {
            cJSON_Delete(root);

            return 0; // 중복
        }
    }

    // 새 사용자 생성
    cJSON *new_user = cJSON_CreateObject();
    cJSON_AddStringToObject(new_user, "id", id);
    cJSON_AddStringToObject(new_user, "password", pw);
    cJSON_AddStringToObject(new_user, "name", nickname);
    cJSON_AddNumberToObject(new_user, "age", year);
    cJSON_AddStringToObject(new_user, "phone", phone);
    cJSON_AddStringToObject(new_user, "addr", address);
    cJSON_AddNumberToObject(new_user, "messagecount", messege_a);
    cJSON_AddNumberToObject(new_user, "bullcount", bull_count);
    cJSON_AddItemToArray(root, new_user);

    // 파일에 저장
    fp = fopen("users.json", "w");
    if (!fp) {
        cJSON_Delete(root);
        return 0;
    }

    char *out = cJSON_Print(root);
    fputs(out, fp);
    fclose(fp);

    free(out);
    cJSON_Delete(root);
    create_user_folder(id);
    return 1;
}

// 클라이언트 요청을 처리하는 쓰레드 함수
void *client_handler(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);  // 할당된 메모리 해제        send(client_socket,&user_count,sizeof(int),0);

    char cmd[16], id[50], pw[50], nickname[50], phone[50], address[100];
    int year = 0;
    int messege_a = 0;
    int bull_count = 0; // 불량자 구별카운트

    read(client_socket, cmd, sizeof(cmd));
    read(client_socket, id, sizeof(id));
    read(client_socket, pw, sizeof(pw));

    int result = 0;

    if (strcmp(cmd, "login") == 0) {
        result = login(id, pw);
    } else if (strcmp(cmd, "register") == 0) {
        // 추가 정보 읽기
        read(client_socket, nickname, sizeof(nickname));
        read(client_socket, &year, sizeof(int));
        read(client_socket, phone, sizeof(phone));
        read(client_socket, address, sizeof(address));
        read(client_socket, &messege_a, sizeof(int));

        result = register_user(id, pw, nickname, year, phone, address, messege_a,bull_count);
    }


    // 결과 전송
    write(client_socket, &result, sizeof(int));


    if (result == 1 && strcmp(cmd, "login") == 0) {
        while (1) {

            printf("[%s]: logged in\n", id);
            char action[16];
            read(client_socket, action, sizeof(action));  // 사용자 요청
            action[strcspn(action, "\n")] = '\0';
            if (strcmp(action, "1") == 0)
            {
                char id_[50];
                read(client_socket, id, sizeof(id));
                strcpy(id_, id);
                
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
                
                printf("%s\n", id_);

                cJSON_ArrayForEach(user, root) {
                    char *uid = cJSON_GetObjectItem(user, "id")->valuestring;
                    if(strcmp(uid,id_ )==0)
                    {
                        
                        char *name = cJSON_GetObjectItem(user, "name")->valuestring;
                        int age = cJSON_GetObjectItem(user, "age")->valueint;
                        char *phone = cJSON_GetObjectItem(user, "phone")->valuestring;
                        char *addr = cJSON_GetObjectItem(user, "addr")->valuestring;
                        User user_info;
                        

                        strcpy(user_info.name, name);
                        user_info.age = age;
                        strcpy(user_info.phone, phone);
                        strcpy(user_info.addr , addr);
                        

                        write(client_socket, user_info.id, sizeof(user_info.id));
                        write(client_socket, user_info.name, sizeof(user_info.name));
                        write(client_socket, &user_info.age, sizeof(user_info.age));
                        write(client_socket, user_info.phone, sizeof(user_info.phone));
                        write(client_socket, user_info.addr, sizeof(user_info.addr));

                        printf("%s\n",user_info.name);
                        printf("%d\n",user_info.age);
                        printf("%s\n",user_info.phone);
                        printf("%s\n",user_info.addr);

                    }
                    else{
                        printf("유저가 없거나 가져올수 없습니다.\n");
                    }
                 }
            }
            if (strcmp(action, "2") == 0) // 도서검색하기
            {
                char key[50], val[100];
                read(client_socket, key, sizeof(key));
                read(client_socket, val, sizeof(val));
                printf("%s %s\n",key, val);
                Book *p_found = NULL;
                int count = search_books_count(key, val);
                p_found = malloc(sizeof(Book)*count);
                write(client_socket, &count, sizeof(int));
                search_books(key, val, p_found);
                for (int i = 0; i < count; i++)
                {
                    write(client_socket, &p_found[i], sizeof(Book));
                }
                free(p_found);
            }
            else if (strcmp(action, "3") == 0)                 // 메세지
            {
                // printf("message\n");
                client_message(client_socket);
            }
            
            else if (strcmp(action, "4") == 0)
            {
                break;
            }                                                                                                                                                                                                                                                                                                                                                                                                                                              
        }
    }
    else if (result == 2 && strcmp(cmd, "login") == 0) { //관리자로 로그인했을때 기능
        while (1) {
            char action[16];
            read(client_socket, action, sizeof(action));  // 사용자 요청
            action[strcspn(action, "\n")] = '\0';
            if (strcmp(action, "1") == 0)
            {
                char somi[16];
                read(client_socket, somi, sizeof(somi));  // 사용자 요청
                somi[strcspn(somi, "\n")] = '\0';
                if (strcmp(somi, "1") == 0) //모든도서확인하기
                {

                    send(client_socket,&book_count,sizeof(int),0);
            
                    for (int i = 0; i < book_count; i++)
                    {
                        write(client_socket, &books[i], sizeof(Book));
                    }

                }
                if (strcmp(somi, "2") == 0) // 도서검색하기
                {
                    char key[50], val[100];
                    read(client_socket, key, sizeof(key));
                    read(client_socket, val, sizeof(val));
                    printf("%s %s\n",key, val);
                    Book *p_found = NULL;
                    int count = search_books_count(key, val);
                    p_found = malloc(sizeof(Book)*count);
                    write(client_socket, &count, sizeof(int));
                    search_books(key, val, p_found);
                    for (int i = 0; i < count; i++)
                    {
                        write(client_socket, &p_found[i], sizeof(Book));
                    }
                    free(p_found);
                }
                else if (strcmp(somi, "3") == 0) // 도서수정
                {
                    Book b;
                    read(client_socket, &b, sizeof(Book));
                    result = modify_book(b);
                    write(client_socket, &result, sizeof(int));
                }
                else if (strcmp(somi, "4") == 0) // 도서추가하기
                {
                    Book b;
                    read(client_socket, &b, sizeof(Book));
                    result = add_book(b);
                    write(client_socket, &result, sizeof(int));
                } 
                else if (strcmp(somi, "5") == 0) // 도서삭제
                {
                    char isbn[200];
                    read(client_socket, isbn, sizeof(char));
                    result = delete_book(isbn);
                    write(client_socket, &result, sizeof(int));
                }
                else if (strcmp(somi, "6") == 0)
                {
                    break;
                }
            }
            else if (strcmp(action, "2") == 0) // 모든계정관리
            {
                send(client_socket,&user_count,sizeof(int),0);

                for (int i = 0; i < user_count; i++)
                {
                    write(client_socket, &Users[i], sizeof(User));
                }
                char sub_action[16];
                read(client_socket, sub_action, sizeof(sub_action));

                if(strcmp(sub_action, "1")==0)
                {

                    
                    //여기서부터
                    char user_id_for_modify[16];
                    
                    // 수정을 원하는 계정의 아이디를 클라이언트로부터 받는다.
                    read(client_socket,user_id_for_modify, sizeof(user_id_for_modify));
                    
                    // 해당하는 유저가 있는지 확인한다.
                    // 해당 하는 유저가 있으면 1을 리턴 아니면 0을 리턴
                    int searched_user; 
                    searched_user = search_user_by_id(user_id_for_modify);
                    
                    send(client_socket, &searched_user, sizeof(int),0);
                    
                    /*
                    Users[user_count]반복문으로 ccnk 아이디의 인덱스를 찾아서
                    해당 인덱스의 id/pw를 따로 저장해둠 
                    그리고 해당 배열을 제거
                    그리고 받은 유저정보를 추가할때 idpw 넣자 
                    */
                    char id_[50];
                    char pw_[50];
                    int flag_for_delete =0;
                    int index_for_delete=0;
                    for (int i=0; i<user_count; i++)
                    {   
                        // user_id_for_modify 와 같은 index를 가져와라!
                        if(strcmp(Users[i].id, user_id_for_modify)==0)
                        {
                            strcpy(id_,Users[i].id);
                            strcpy(pw_,Users[i].pw);
                            index_for_delete = i;
                            flag_for_delete = 1;
                        }
                    }
                    // 클라이언트로부터 유저 정보를 받는ㄷㅏ
                    User modi_user;
                    memset(&modi_user, 0, sizeof(User));
                    
                    read(client_socket, &modi_user, sizeof(modi_user));

                    strcpy(modi_user.id,id_);
                    strcpy(modi_user.pw,pw_);
                    
                    // 해당 아이디를 찾았을때만배열에서 제거
                    if(flag_for_delete)
                    {
                        for (int j=index_for_delete; j<user_count-1; j++)
                        {   
                            Users[j]=Users[j+1];
                        }
                    }
                    user_count= user_count-1;
                    

                    // modi_user.id가 비어있으면 넣으면 안됨
                    if(strcmp(modi_user.id," ")!=0 || modi_user.id !=NULL)
                    {
                        memcpy(&Users[user_count],&modi_user, sizeof(User));
                    }
                    user_count = user_count +1; //억지로 최종카운트를 추가해준다.
            
                    save_user("users.json");

                    // 수정이 완료었다면 클라이언트에 응답으로 1을 넘긴다.
                    searched_user = 1;
                    send(client_socket,&searched_user, sizeof(searched_user),0);
                }//sub_action == 1 
                else if(strcmp(sub_action,"2")==0)
                {
                    
                    //아이디 추가
                    
                    User user_for_add;
                    memset(&user_for_add, 0, sizeof(User));
                    read(client_socket, &user_for_add, sizeof(user_for_add));

                    // 구조체 끝부분에 넣는다.
                    if((strlen(user_for_add.id)>=0) && (user_for_add.id !=NULL))
                    {
                        memcpy(&Users[user_count],&user_for_add, sizeof(User));
                    }
                    user_count = user_count +1;

                    int is_added;
                    // 파일 생성한다.
                    is_added = save_user("users.json");

                    // 클라이언트에 유저 추가 했다는 메시지 보낸다.
                    send(client_socket,&is_added, sizeof(is_added),0);
                }
            }
            else if (strcmp(action, "3") == 0) // 도서관오픈관리
            {
                break;
            }
            else if (strcmp(action, "4") == 0) // 대출자정보
            {
                break;
            }
            else if (strcmp(action, "5") == 0) // 로그아웃
            {
                break;
            }   
        }
    }
    else if (result == 3 && strcmp(cmd, "login") == 0) { //사서로 로그인했을때 기능
        while (1) {
            char action[16];
            read(client_socket, action, sizeof(action));  // 사용자 요청
            action[strcspn(action, "\n")] = '\0';

            if (strcmp(action, "1") == 0)
            {
                char somi[16];
                read(client_socket, somi, sizeof(somi));  // 사용자 요청
                somi[strcspn(somi, "\n")] = '\0';
                if (strcmp(somi, "1") == 0) // 도서검색하기
                {
                    char key[50], val[100];
                    read(client_socket, key, sizeof(key));
                    read(client_socket, val, sizeof(val));
                    printf("%s %s\n",key, val);
                    Book *p_found = NULL;
                    int count = search_books_count(key, val);
                    p_found = malloc(sizeof(Book)*count);
                    write(client_socket, &count, sizeof(int));
                    search_books(key, val, p_found);
                    for (int i = 0; i < count; i++)
                    {
                        write(client_socket, &p_found[i], sizeof(Book));
                    }
                    free(p_found);
                    
                }
                else if (strcmp(somi, "2") == 0) // 도서추가
                {
                    Book b;
                    read(client_socket, &b, sizeof(Book));
                    result = add_book(b);
                    write(client_socket, &result, sizeof(int));
                }
            }
            else if (strcmp(action,"2") == 0)
            {
                send(client_socket,&book_2_count,sizeof(int),0);
            
                    for (int i = 0; i < book_2_count; i++)
                    {
                        write(client_socket, &book_2s[i], sizeof(Book2));
                    }
                    while(1)
                    {
                        int received_choice = 0;
                        int recv_len = recv(client_socket, &received_choice, sizeof(int), 0);
                        if (recv_len <= 0) {
                            perror("recv 실패");
                            close(client_socket);
                        }
                        if(received_choice == 1)
                        {

                        }
                        if (received_choice == 2)
                        {
                            char isbn[50];
                            int found = 0;
                            read(client_socket, isbn, sizeof(isbn));
                            isbn[strcspn(isbn, "\n")] = '\0';  // 개행 제거

                            for (int i = 0; i < book_2_count; i++)
                            {
                                if (strcmp(isbn, book_2s[i].isbn) == 0)
                                {
                                    found = 1;

                                    printf("반납할 권수를 입력해주세요.\n");
                                    int return_count = 0;
                                    recv(client_socket, &return_count, sizeof(int), 0);

                                    if (return_count >= 0)
                                    {

                                        printf("반납처리가 완료되었습니다.\n");
                                        modify_book2s(book_2s[i],return_count,isbn);
                                        if ((book_2s[i].num_books - return_count) == 0)
                                        {
                                            delete_book2(isbn);
                                        }
                                    }
                                    else
                                    {
                                        printf("잘못된 입력입니다. (0 이하의 숫자)\n");
                                    }
                                    break;
                                }
                            }

                            if (!found) {
                                printf("해당 ISBN에 해당하는 도서를 찾을 수 없습니다.\n");
                            }
                        }
                    }
            }
            else if (strcmp(action, "3") == 0) // 모든계정관리
            {
                send(client_socket,&user_count,sizeof(int),0);
                
                for (int i = 0; i < user_count; i++)
                {
                    write(client_socket, &Users[i], sizeof(User));
                }
            }
            else if (strcmp(action, "4") == 0)      // 메세지 보내기
            {
                librarian_message(id, client_socket);
            }
            else if (strcmp(action, "5") == 0) // 로그아웃
            {
                break;
            }   
        }
    }

    close(client_socket);  // 소켓 종료
    return NULL;
}

// 메인 함수 - 서버 실행
int main() {
    load_books("DATA.json");  // 도서 데이터 불러오기
    load_user("users.json");  // 유저 데이터 불러오기
    load_book_2("DATA2.json");

    // for (int i=0;i<book_count;i++)
    // {
    // printf(" %d\n %s\n %s\n %s\n %d\n %d\n %s\n %s\n %s\n %s\n %d\n", books[i].no, books[i].title, books[i].author, books[i].publisher, books[i].pub_year,
    // books[i].num_books, books[i].isbn, books[i].extra_n, books[i].kdc, books[i].kdc_subject, books[i].loan_frequency);
    // }
    int server_fd = socket(PF_INET, SOCK_STREAM, 0);  // 소켓 생성
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
    close(server_fd);
    return 0;
}
void librarian_message(char *id, int client_socket) {

    while(1) {
        printf("now running: void librarian_message\n");
        int user_choice= 0;

        // read 1: int user_choice - choice 1
        read(client_socket, &user_choice, sizeof(user_choice));

        if (user_choice == 1) {                             //[1] 유저에게 메세지 보내기
            printf("[%s]: now running: void librarian_sendmessage\n", id);
            librarian_sendmessage(id, client_socket);
        } else if (user_choice == 2) {                      //[2] 사서의 메세지함 확인
            printf("[%s]: now running: void librarian_checkmessage\n", id);
            librarian_checkmessage(id, client_socket);
        } else if (user_choice == 3) {
            printf("[%s]: exit void librarian_message\n", id);
            break;
        } else {
            printf("[%s]: user's wrong input\n", id);
            continue;
            while(getchar()!='\n');
        }
    }
}
void librarian_sendmessage(char *id, int client_socket){
    
    printf("[%s]: 메세지 보내기 기능 사용중\n", id);
    char user_id[50];
    char all_users_id[50][150];
    int success = 0;

    // read 2: char user_id
    read(client_socket, user_id, sizeof(user_id));
    
    printf("[%s]: user_id로 '%s'를 전달받음\n", id, user_id);

    printf("[%s]: users.json파일 파싱 시작\n", id);
    FILE *fp = fopen("users.json", "r");
    if (!fp) return;                                    // 파일이 없으면 리턴

    fseek(fp, 0, SEEK_END);                             // 파일 끝으로 이동
    long len = ftell(fp);                               // 파일 길이 측정
    rewind(fp);                                         // 다시 파일 처음으로 이동

    char *data = malloc(len + 1);                       // JSON 데이터 저장할 메모리 할당
    fread(data, 1, len, fp);                            // 파일 데이터 읽기
    fclose(fp);                                         // 파일 닫기

    data[len] = '\0';                                   // 문자열 종료 문자
    cJSON *root = cJSON_Parse(data);                    // JSON 파싱
    if (!root) return;                                  // 파싱 실패 시 0 반환

    printf("user_id: %s\n",user_id);

    int count = 0;
    cJSON *users_info;
    cJSON_ArrayForEach(users_info, root) {      // JSON 배열 순회
        strcpy(all_users_id[count], cJSON_GetObjectItem(users_info, "id")->valuestring);
        printf("%s\n", all_users_id[count]);
        count++;                                                               
    }
    cJSON_Delete(root);            // JSON 객체 해제
    free(data);                    // 메모리 해제

    printf("[%s]: users.json파일 파싱 성공\n", id);


    for (int i = 0; i < count; i++) {
        if(strcmp(user_id, all_users_id[i])==0) {
            printf("유저가 존재함\n");
            success = 1;
            break;
        }
    }

    // send 1: int success;
    send(client_socket, &success, sizeof(success), 0);

    printf("[%s]: success 정보 전송\n", id);

    if (success == 1) {
        
        char title_msg[150];
        char script_msg[150];

        // read 3: char title_msg / char script_msg
        read(client_socket, title_msg, sizeof(title_msg));
        read(client_socket, script_msg, sizeof(script_msg));
        script_msg[strcspn(script_msg, "\n")] = '\0';

        //json구조로 메세지 담기
        cJSON *message = cJSON_CreateObject();
        cJSON_AddStringToObject(message, "제목", title_msg);
        cJSON_AddStringToObject(message, "id", "saseo");
        cJSON_AddStringToObject(message, "내용", script_msg);

        char *json_string = cJSON_Print(message);

        char file_path[300];
        sprintf(file_path, "./user/%s/%s.json", user_id, title_msg);
        
        FILE *fp = fopen(file_path, "w");

        if(fp == NULL) {
            printf("[%s]: %s: 파일 생성 실패\n", id, file_path);
        } else {
            printf("[%s]: %s: 파일 생성 성공\n", id, file_path);
            fprintf(fp, "%s", json_string);
            fclose(fp);
        }

        free(json_string);
        cJSON_Delete(message);
        
        send(client_socket, &success, sizeof(success), 0);
        printf("[%s]: %s의 파일에 메세지 저장완료\n", id, user_id);

        usleep(50000);
    }
}
void librarian_checkmessage(char *id, int client_socket){

    printf("[%s]: 메세지함 확인하기 기능 사용중\n", id);
                
    while(1) {
        // [1] 사서 dir 열기.
        DIR *dp = NULL;
        struct dirent* entry = NULL;
        struct stat buf;

        char dir_path[100] = {"./librarian"};
        int success= 1;

        // send 1: int success
        if ((dp=opendir(dir_path))== NULL){
            printf("%s를 열 수 없습니다.\n", dir_path);
            success = 0;
            // send 1: int success
            send(client_socket, &success, sizeof(success), 0);
            break;
        } else {
            printf("[%s]: %s 열기 완료\n", id, dir_path);
            // send 1: int success
            send (client_socket, &success, sizeof(success), 0);
        }


        // [2] 사서 dir에서 파일/디렉토리 정보 읽어들이기
        char file_name[50][50];
        int count = 0;
        int i= 0;
        while((entry = readdir(dp)) != NULL) { 
            lstat(entry->d_name, &buf);

        // [3] 사서 dir에서 읽어들인 파일/디렉토리를 배열에 저장하고, client로 전송.
            if (S_ISDIR(buf.st_mode) && strcmp(entry->d_name, ".")!=0 && strcmp(entry->d_name, "..")!=0) { 
                printf("[filelist]: [%d] %s\n", count+1, entry->d_name);
                strcpy(file_name[count], entry->d_name);
                count++;
            }
            i++;
        }

        //send 2: int count
        send(client_socket, &count, sizeof(count), 0);
        printf("[saseo]: count: %d\n",count);
        for (int j = 0; j < count; j++) {
            //send 3: char file_name[j]
            send(client_socket, file_name[j], sizeof(file_name[j]), 0);
        }

        // [4] 사서 dir 닫기.
        closedir(dp);

        int form = 0;
        //read 2: int form  ( int rk )
        read(client_socket, &form, sizeof(form));
        printf("[%s]: form: %d\n", id, form);

        char filename[50] = {0};
        char file_path[100] = {0};
        int result;

        if (form == 0) {                   //읽기(r)

            //filename을 받아, sprintf로 file_path를 지정 및 fopen.
            //
            char filename[50] = {0};
            char file_path[100] = {0};

            // read 3: filename (file_name[i])
            read(client_socket, filename, sizeof(filename));

            printf("filename: %s\n", filename);
            sprintf(file_path, "./librarian/%s", filename);
            printf("sprintfed file_path: %s\n", file_path);

            printf("[%s]: 메세지 읽기 기능 사용\n", id);

            FILE *fp = fopen(file_path, "r");

            if (fp == NULL) {
                printf("[%s]: 파일 열기 실패\n", id);
            } else {
                printf("[%s]: 파일열기 완료\n", id);
            }

            fseek(fp, 0, SEEK_END);                         // 파일 끝으로 이동
            long len = ftell(fp);                           // 파일 길이 측정
            rewind(fp);                                     // 다시 파일 처음으로 이동

            char *data = malloc(len + 1);                   // JSON 데이터 저장할 메모리 할당
            if (data == NULL) {
                printf("메모리 할당에 실패했습니다.\n");
                fclose(fp);
                return;
            }
            fread(data, 1, len, fp);                        // 파일 데이터 읽기
            fclose(fp);                                     // 파일 닫기
            data[len] = '\0';                               // 문자열 종료 문자

            cJSON *root = cJSON_Parse(data);                // JSON 파싱
            if (root == NULL) {
                printf("JSON 파싱에 실패했습니다.\n");
                free(data);
                return;
            }


            char title_msg[150];
            char user_id[50];
            char script_msg[150];

            printf("[%s]: %s - 파일 파싱 시작\n", id, file_path);
            cJSON *message;
            cJSON_ArrayForEach(message, root) {             // JSON 배열 순회
                strcpy(title_msg, cJSON_GetObjectItem(root, "제목")->valuestring);  
                strcpy(user_id, cJSON_GetObjectItem(root, "id")->valuestring);
                strcpy(script_msg, cJSON_GetObjectItem(root, "내용")->valuestring);     // 도서 수 증가
            }
            cJSON_Delete(root);                             // JSON 객체 해제
            free(data);                                     // 메모리 해제

            printf("[%s]: jSON파일 파싱 완료\n", id);


            int lib_read = 1;
            //json구조로 메세지 담기
            cJSON *message1 = cJSON_CreateObject();
            cJSON_AddStringToObject(message1, "제목", title_msg);
            cJSON_AddStringToObject(message1, "id", user_id);
            cJSON_AddStringToObject(message1, "내용", script_msg);
            cJSON_AddNumberToObject(message1, "사서확인", lib_read);
            char *json_string = cJSON_Print(message1);
            
            FILE *filepointer = fopen(file_path, "w");

            if(filepointer == NULL) {
                printf("[%s]: %s: 파일 생성 실패\n", id, file_path);
            } else {
                printf("[%s]: %s: 파일 생성 성공\n", id, file_path);
                fprintf(filepointer, "%s", json_string);
                fclose(filepointer);
            }
            free(json_string);
            cJSON_Delete(message);
            
            //send4
            // send(client_socket, &success, sizeof(success), 0);
            printf("[%s]: %s의 파일에 메세지 저장완료\n", id, user_id);

            //send 4: char title_msg / char user_id / char script_msg
            send(client_socket, title_msg, sizeof(title_msg), 0);
            send(client_socket, user_id, sizeof(user_id), 0);
            send(client_socket, script_msg, sizeof(script_msg), 0);

            usleep(50000);

            int go;
            read(client_socket, &go, sizeof(go));

        } else if (form == 1) {            //삭제하기(k)

            char filename[50] = {0};
            char file_path[100] = {0};
            // read 3: filename (file_name[i])
            read(client_socket, filename, sizeof(filename));
            printf("[%s]: file name: %s\n", id, filename);
    
            sprintf(file_path, "./librarian/%s", filename);
            printf("[%s]: file path: %s\n", id, file_path);

            printf("[%s]: 메세지 삭제 기능 사용", id);
            result = remove(file_path);                         //파일삭제 성공시 0반환
            if (result == 0) {
                //send 4: int result == 0
                send(client_socket, &result, sizeof(result), 0);
            } else {
                result= 1; 
                //send 4: int result == 1
                send(client_socket, &result, sizeof(result), 0);
            }
        } else if (form == 2) {            //돌아가기(q)
            printf("[%s]: void librarian_checkmessage() 종료\n", id);
            break;
        } else if (form == 3) {            //유효하지 않은 입력.
            printf("[%s]: 잘못된 입력\n", id);
            continue;
        } else if (form == 4) {
            continue;
        } else if (form == 5) {
            continue;
        } 
    }
}

//메세지 - 클라이언트
void client_message(int client_socket) {

    while(1) {
        printf("void client_message() 실행\n"); //testprint
        int user_choice = 0;
        char user_id[50] = {0};
        char title_msg[150] = {0};
        char script_msg[150] = {0};

        // printf("befor reading char user_id\n"); //test print
        //read1: char user_id
        read(client_socket, user_id, sizeof(user_id));    //id받기

        printf("[%s]: 일반 사용자 메세지 기능 사용\n", user_id);

        //read2: int user_choice ( choice 1 )
        read(client_socket, &user_choice, sizeof(user_choice));

        if(user_choice == 1) {                                          //[1] 사서에게 메세지 보내기

            //[1] 소켓으로 받은 유저가 보낸 메세지 받기
            printf("[%s]: 사서에게 메세지 보내기 중\n", user_id);
            
            //[2] id.json을 librarian폴더에 저장하기
            char file_path[100];
            sprintf(file_path, "./librarian/%s.json", user_id);         //sprintf로 경로에 접근해서

            int success=0;
            int creatable=0;
            int lib_read=0;

            FILE *filepointer = fopen(file_path, "r");
            printf("[%s]: 읽기모드로 사서확인이 0인지 1인지 확인하기\n", user_id);
            
            if (filepointer==NULL) {
                creatable=1;
            } else 
            {
                fseek(filepointer, 0, SEEK_END);                         // 파일 끝으로 이동
                long len = ftell(filepointer);                           // 파일 길이 측정
                rewind(filepointer);                                     // 다시 파일 처음으로 이동
    
                char *data = malloc(len + 1);                   // JSON 데이터 저장할 메모리 할당
                if (data == NULL) {
                    printf("메모리 할당에 실패했습니다.\n");
                    fclose(filepointer);
                    return;
                }
                fread(data, 1, len, filepointer);                        // 파일 데이터 읽기
                fclose(filepointer);                                     // 파일 닫기
                data[len] = '\0';                               // 문자열 종료 문자
    
                cJSON *root = cJSON_Parse(data);                // JSON 파싱
                if (root == NULL) {
                    printf("JSON 파싱에 실패했습니다.\n");
                    free(data);
                    return;
                }

                cJSON *lib_check = cJSON_GetObjectItemCaseSensitive(root, "사서확인");

                if (cJSON_IsNumber(lib_check)) {
                    printf("사서확인: %d\n", lib_check->valueint);
                }
                lib_read=lib_check->valueint;
                printf("lib_read: %d\n", lib_read);

                cJSON_Delete(root);                             // JSON 객체 해제
                free(data);        
                
                if (lib_read == 0) {
                    creatable = 0;
                } else if (lib_read == 1) {
                    creatable = 1;
                }
                
            }
            
            //send 0: int creatable
            printf("sending creatable: %d\n", creatable);
            send(client_socket, &creatable, sizeof(creatable), 0);
            
            if (creatable == 1) {

                FILE *fp = fopen(file_path, "w");                           //쓰기모드로 파일 열기
                printf("[%s]: 사서 디렉토리에 유저이름의 파일 생성\n", user_id);

                //send1: int success
                if (fp== NULL) {
                    success = 0;
                    send(client_socket, &success, sizeof(success), 0);
                    printf("[%s]: %s - 파일 열기 실패\n", user_id, file_path);
                } else {
                    success = 1;
                    send(client_socket, &success, sizeof(success), 0);
                    printf("[%s]: %s - 파일 열기 성공\n", user_id, file_path);
                }

                //read3: char title_msg & char script_msg
                read(client_socket, title_msg, sizeof(title_msg));
                title_msg[strcspn(title_msg, "\n")] = '\0';

                read(client_socket, script_msg, sizeof(script_msg));
                script_msg[strcspn(script_msg, "\n")] = '\0';

                // printf("titlemessage: %s\n", title_msg);
                // printf("scriptmessage: %s\n", script_msg);

                lib_read = 0;

                cJSON *root = cJSON_CreateObject();
                cJSON_AddStringToObject(root, "제목", title_msg);
                cJSON_AddStringToObject(root, "id", user_id);
                cJSON_AddStringToObject(root, "내용", script_msg);
                cJSON_AddNumberToObject(root, "사서확인", lib_read);
                char *json_string = cJSON_Print(root);

                if (fp) {
                    fprintf(fp, "%s", json_string);
                    fclose(fp);
                }
                free(json_string);
                cJSON_Delete(root);
                
                printf("[%s]: 사서에게 메세지 전송 완료\n", user_id);
            }
            else if (creatable == 0) {
                break;
            }
            

        } else if (user_choice == 2) {                                  //[2] 나의 메세지 확인하기

            printf("[%s]: 유저 메세지 확인 기능 사용중\n", user_id);

            //read3: int user_choice ( choice 2 )
            read(client_socket, &user_choice, sizeof(user_choice));

            if (user_choice == 1) {                                         //[1] 받은 메세지함
                
                char file_name[50][50];
                int count = 0;
                int i= 0;
                printf("[%s]: 받은 메세지함 확인하기 중\n", user_id);
                
                //[1] user_id에 해당하는 dir을 열고, 배열에 담아 파일목록 client에게 전송.
                // [2-1] 사서 dir 열기.
                DIR *dp = NULL;
                struct dirent* entry = NULL;
                struct stat buf;
                int success= 1;

                char dir_path[100] = {0};
                sprintf(dir_path, "./user/%s", user_id);                    //상대경로: ./user/'user_id'

                // send 1: int success ( dir open에 대한 성공여부 )
                if ((dp=opendir(dir_path))== NULL){
                    printf("%s를 열 수 없습니다.\n", dir_path);
                    success = 0;
                    send(client_socket, &success, sizeof(success), 0);
                    break;
                } else {
                    printf("[%s]: %s 열기 완료\n", user_id, dir_path);
                    send (client_socket, &success, sizeof(success), 0);
                }

                // [2-2] 유저 dir에서 파일/디렉토리 정보 읽어들이기
                while((entry = readdir(dp)) != NULL) { 
                    lstat(entry->d_name, &buf);

                // [2-3] 사서 dir에서 읽어들인 파일/디렉토리를 배열에 저장하고, client로 전송.
                    if (S_ISDIR(buf.st_mode) && strcmp(entry->d_name, ".")!=0 && strcmp(entry->d_name, "..")!=0) { 
                        printf("[filelist]: [%d] %s\n", count+1, entry->d_name);
                        strcpy(file_name[count], entry->d_name);
                        count++;
                    }
                    i++;
                }

                //send 2: int count ( 총 파일의 갯수 )
                send(client_socket, &count, sizeof(count), 0);
                printf("[%s]: count: %d\n", user_id, count);
                
                //send 3: char file_name[j] ( 파일경로 )
                for (int j = 0; j < count; j++) {
                    send(client_socket, file_name[j], sizeof(file_name[j]), 0);
                }

                // [2-4] 사서 dir 닫기.
                closedir(dp);

                /************************************************************************************* */

                int form;
                //read 2: int form
                read(client_socket, &form, sizeof(form));
                printf("[%s]: form: %d\n", user_id, form);

                int result;

                //read 2: int form ( 0(r): 읽기 / 1(k): 삭제 / 2(q): 돌아가기 )
                if (form == 0) {                                                //메세지 읽기(r)
                            
                    char filename[50] = {0};
                    char file_path[150] = {0};
                    // read 3: filename (file_name[i])
                    read(client_socket, filename, sizeof(filename));
                    printf("[%s]: file name: %s\n", user_id, filename);
    
                    sprintf(file_path, "./user/%s/%s", user_id, filename);
                    printf("[%s]: file path: %s\n", user_id, file_path);

                    printf("[%s]: 메세지 읽기 기능 사용\n", user_id);

                    FILE *fp = fopen(file_path, "r");

                    if (fp == NULL) printf("[%s]: 파일 열기 실패\n", user_id);
                    else printf("[%s]: 파일열기 완료\n", user_id);

                    fseek(fp, 0, SEEK_END);                                         // 파일 끝으로 이동
                    long len = ftell(fp);                                           // 파일 길이 측정
                    rewind(fp);                                                     // 다시 파일 처음으로 이동

                    char *data = malloc(len + 1);                                   // JSON 데이터 저장할 메모리 할당
                    if (data == NULL) {
                        printf("메모리 할당에 실패했습니다.\n");
                        fclose(fp);
                        return;
                    }
                    fread(data, 1, len, fp);                                        // 파일 데이터 읽기
                    fclose(fp);                                                     // 파일 닫기
                    data[len] = '\0';                                               // 문자열 종료 문자

                    cJSON *root = cJSON_Parse(data);                                // JSON 파싱
                    if (root == NULL) {
                        printf("[%s]: '%s' 파싱에 실패했습니다.\n", user_id, file_path);
                        free(data);
                        return;
                    }

                    char title_msg[150];
                    char id[50];
                    char script_msg[150];

                    printf("[%s]: %s - 파일 파싱 시작\n", user_id, file_path);

                    cJSON *message;
                    cJSON_ArrayForEach(message, root) {                             // JSON 배열 순회
                        strcpy(title_msg, cJSON_GetObjectItem(root, "제목")->valuestring);  
                        strcpy(id, cJSON_GetObjectItem(root, "id")->valuestring);
                        strcpy(script_msg, cJSON_GetObjectItem(root, "내용")->valuestring);                                                             // 도서 수 증가
                    }
                    cJSON_Delete(root);                                             // JSON 객체 해제
                    free(data);                                                     // 메모리 해제

                    printf("[%s]: jSON파일 파싱 완료\n", user_id);

                    printf("title_msg: %s\n", title_msg);
                    printf("script_msg: %s\n", script_msg);
                    printf("[%s]: %s - 파일 파싱 완료\n", user_id, file_path);

                    //send 4: char title_msg / char user_id / char script_msg
                    send(client_socket, title_msg, sizeof(title_msg), 0);
                    send(client_socket, id, sizeof(user_id), 0);
                    send(client_socket, script_msg, sizeof(script_msg), 0);

                    usleep(200000);

                } else if (form == 1) {                                         //삭제하기(k)

                    char filename[50] = {0};
                    char file_path[150] = {0};
                    // read 3: filename (file_name[i])
                    read(client_socket, filename, sizeof(filename));
                    printf("[%s]: file name: %s\n", user_id, filename);
    
                    sprintf(file_path, "./user/%s/%s", user_id, filename);
                    printf("[%s]: file path: %s\n", user_id, file_path);

                    printf("[%s]: 메세지 삭제 기능 사용", user_id);
                    result = remove(file_path);                                     //파일삭제 성공시 0반환
                    if (result == 0) {
                        //send 4: int result == 0
                        send(client_socket, &result, sizeof(result), 0);
                    } else {
                        result= 1; 
                        //send 4: int result == 1
                        send(client_socket, &result, sizeof(result), 0);
                    }
                } else if (form == 2) {                                         //돌아가기(q)
                    break;
                } else if (form == 3) {                                         //유효하지 않은 입력.
                    printf("[%s]: 잘못된 입력\n", user_id);
                    continue;
                }
            } else if (user_choice == 2){                                   //[2] 보낸 메세지함 확인하기
                //사서dir open 이후 해당 유저 file이 있는지 확인
                
                // [2-1] 사서 dir 열기.
                DIR *dp = NULL;
                struct dirent* entry = NULL;
                struct stat buf;

                char dir_path[100] = {"./librarian"};
                int success= 1;

                read(client_socket, user_id, sizeof(user_id));
                printf("user_id: %s\n", user_id);

                // send 1: int success
                if ((dp=opendir(dir_path))== NULL){
                    printf("%s를 열 수 없습니다.\n", dir_path);
                    success = 0;
                    // send 1: int success
                    send(client_socket, &success, sizeof(success), 0);
                    break;
                } else {
                    printf("[%s]: %s 열기 완료\n", user_id, dir_path);
                    // send 1: int success
                    send (client_socket, &success, sizeof(success), 0);
                }

                // [2-2] 사서 dir에서 파일/디렉토리 정보 읽어들이기
                char file_name[50][50];
                int count = 0;
                int i= 0;
                char user_file[150];
                sprintf(user_file, "%s.json", user_id);

                printf("user_id: %s\n", user_id);
                printf("user_file: %s\n", user_file);

                while((entry = readdir(dp)) != NULL) {
                    lstat(entry->d_name, &buf);

                // [2-3] 사서 dir에서 읽어들인 파일/디렉토리를 배열에 저장하고, client로 전송.
                    if (S_ISDIR(buf.st_mode) && strcmp(entry->d_name, ".")!=0 && strcmp(entry->d_name, "..")!=0) { 
                        printf("[filelist]: [%d] %s\n", count+1, entry->d_name);
                        if (strcmp(entry->d_name, user_file)==0) {
                            strcpy(file_name[count], entry->d_name);
                            count++;
                        }               
                    }
                    i++;
                }

                if (strcmp(file_name[count], user_file)==0){

                }
                //send 2: int count
                send(client_socket, &count, sizeof(count), 0);
                printf("[%s]: count: %d\n",user_id, count);
                for (int j = 0; j < count; j++) {
                    //send 3: char file_name[j]
                    send(client_socket, file_name[j], sizeof(file_name[j]), 0);
                }

                // [2-4] 사서 dir 닫기.
                closedir(dp);


                int form;
                //read 2: int form  ( rk )
                read(client_socket, &form, sizeof(form));
                printf("[%s]: form: %d\n", user_id, form);


                if (form == 0) {             

                    char filename[50] = {0};
                    char file_path[100] = {0};
                    // read 3: filename (file_name[i])
                    read(client_socket, filename, sizeof(filename));
                    printf("[%s]: file name: %s\n", user_id, filename);
    
                    sprintf(file_path, "./librarian/%s", filename);
                    printf("[%s]: file path: %s\n", user_id, file_path);

                    int result;
                    printf("[%s]: 메세지 삭제 기능 사용\n", user_id);
                    result = remove(file_path);                         //파일삭제 성공시 0반환

                    if (result == 0) {
                        //send 4: int result == 0
                        send(client_socket, &result, sizeof(result), 0);
                        printf("[%s]: 메세지 삭제 완료\n", user_id);
                    } else {
                        result= 1; 
                        //send 4: int result == 1
                        send(client_socket, &result, sizeof(result), 0);
                    }

                } else if (form == 1) {
                    break;
                } else {
                    continue;
                }

            } else if (user_choice == 3) {                                  //[3] 돌아가기
                printf("[%s]: 돌아가기\n", user_id);
                break;
            } else {                                                        //[4] 잘못된 입력
                printf("[%s]: user's wrong input - client_message/checking message user_sent\n", user_id);
                continue;
            }


        } else if (user_choice == 3) {                                  //[3] 돌아가기
            printf("[%s]: client 메뉴로 돌아가기\n", user_id);
            break;
        } else {                                                        //[4] 유저의 제시되지 않은 입력
            printf("[%s]: user's wrong input - client_message", user_id);
            continue;
            
        }
    }
}
