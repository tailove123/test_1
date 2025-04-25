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
#define PORT 20000             // 서버가 열릴 포트 번호
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
    long loan_time;      // 대출시간
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
Book2 loans[MAX_BOOKS];
int book_count = 0;             // 현재 등록된 도서 수
int user_count = 0; // 현재 등록된 유저 수
int loan_count = 0;   
int book_2_count = 0; //현재 대출한 유저의 수
pthread_mutex_t book_mutex = PTHREAD_MUTEX_INITIALIZER;  // 도서 데이터 접근 동기화를 위한 뮤텍스
pthread_mutex_t book2_mutex = PTHREAD_MUTEX_INITIALIZER;  // 대출정보 데이터 접근 동기화를 위한 뮤텍스


int search_user_by_id(char *user_id);
void set_calendar(bussiness_month *m, int mon);
void make_json_file_for_bussiness(bussiness_month *m, int mon);
void solution(int a, int b, char *c);
int set_work_day(int month, int day, bussiness_month *m4);
int is_open_for_business(bussiness_month *m, int mon, int day);
int set_holiday(int mon, int day,bussiness_month *ms);
void parsing_json_to_struct_for_bussiness(bussiness_month *m, int mon);
char *read_json_file(const char *filename);



void librarian_message(char *id, int client_socket);
void librarian_sendmessage(char *id, int client_socket);
void librarian_checkmessage(char *id, int client_socket);
//클라이언트의 메세지 함수
void client_message(int client_socket);

void remove_hyphens(char *str) {
    char *src = str, *dst = str;
    while (*src) {
        if (*src != '-') {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
}

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
        book_2s[count_3].loan_time = cJSON_GetObjectItem(Book2, "대출 시간")->valueint;
        strcpy(book_2s[count_3].status, cJSON_GetObjectItem(Book2, "상태")->valuestring);
        strcpy(book_2s[count_3].loan_addr, cJSON_GetObjectItem(Book2, "주소")->valuestring);

        count_3++;                                                                // 도서 수 증가
    }
    cJSON_Delete(root);              // JSON 객체 해제
    free(data);                      // 메모리 해제
    book_2_count = count_3;          // 총 도서 수 저장
    return count_3;                  // 로드된 도서 수 반환
}

/// @brief Users구조체에서 user_id를 검색후 결과 리턴
/// @param user_id 유저 아이디
/// @return 찾으면 1 못찾으면 0
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

// (0,1) 유저등록하는 함수 (파일명)
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
        cJSON_AddNumberToObject(u, "messagecount", Users[i].bsc);
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

int save_book3s(char *filename, char *isbn) { // 대출목록 저장하는애
    cJSON *root = cJSON_CreateArray();

    for (int i = 0; i < book_2_count; i++) {

        cJSON *b2 = cJSON_CreateObject();
        cJSON_AddStringToObject(b2, "id", book_2s[i].id);
        cJSON_AddStringToObject(b2, "제목", book_2s[i].title);
        cJSON_AddStringToObject(b2, "저자", book_2s[i].author);
        cJSON_AddStringToObject(b2, "출판사", book_2s[i].publisher);
        cJSON_AddNumberToObject(b2, "출판년", book_2s[i].pub_year);
        cJSON_AddNumberToObject(b2, "권", book_2s[i].num_books); // 수량 계산 X
        cJSON_AddStringToObject(b2, "ISBN", book_2s[i].isbn);
        cJSON_AddNumberToObject(b2, "대출 시간", book_2s[i].loan_time);
        cJSON_AddStringToObject(b2, "상태", book_2s[i].status);
        cJSON_AddStringToObject(b2, "주소", book_2s[i].loan_addr);
        cJSON_AddItemToArray(root, b2);

    }

    char *out = cJSON_Print(root);
    FILE *fp = fopen(filename, "w");
    fputs(out, fp);
    fclose(fp);
    free(out);
    cJSON_Delete(root);
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
        cJSON_AddNumberToObject(b2, "대출 시간", book_2s[i].loan_time);
        cJSON_AddStringToObject(b2, "상태", book_2s[i].status);
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

int modify_book2s(int return_count, char *isbn) { // 대출목록 수정해주는애
    pthread_mutex_lock(&book2_mutex);
    for (int i = 0; i < book_2_count; i++) {
        if (strcmp(book_2s[i].isbn, isbn) == 0) {
            book_2s[i].num_books = return_count; // 반납 시 권 수 증가
            save_book3s("DATA2.json", isbn); // 수정된 데이터 저장
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
    snprintf(path, sizeof(path), "./user/%s", id);

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

//대출목록 구조체에 저장하는 함수
int load_loans(const char *filename) {
    FILE *fp_1 = fopen(filename, "r");      // 파일 열기
    if (!fp_1) return 0;                    // 파일이 없으면 0 리턴

    fseek(fp_1, 0, SEEK_END);               // 파일 끝으로 이동
    long len_1 = ftell(fp_1);                // 파일 길이 측정
    rewind(fp_1);                           // 다시 파일 처음으로 이동

    char *data_1 = malloc(len_1 + 1);         // JSON 데이터 저장할 메모리 할당
    fread(data_1, 1, len_1, fp_1);              // 파일 데이터 읽기
    fclose(fp_1);                           // 파일 닫기
    data_1[len_1] = '\0';                    // 문자열 종료 문자

    cJSON *root_1 = cJSON_Parse(data_1);      // JSON 파싱
    if (!root_1) return 0;                  // 파싱 실패 시 0 반환

    int count_1 = 0;
    cJSON *loan;
    cJSON_ArrayForEach(loan, root_1) {      // JSON 배열 순회
        strcpy(loans[count_1].id, cJSON_GetObjectItem(loan, "id")->valuestring);  
        strcpy(loans[count_1].title, cJSON_GetObjectItem(loan, "제목")->valuestring);  
        strcpy(loans[count_1].author, cJSON_GetObjectItem(loan, "저자")->valuestring);
        strcpy(loans[count_1].publisher, cJSON_GetObjectItem(loan, "출판사")->valuestring);
        loans[count_1].pub_year = cJSON_GetObjectItem(loan, "출판년")->valueint;
        loans[count_1].num_books = cJSON_GetObjectItem(loan, "권")->valueint;
        strcpy(loans[count_1].isbn, cJSON_GetObjectItem(loan, "ISBN")->valuestring);
        loans[count_1].loan_time = cJSON_GetObjectItem(loan, "대출 시간")->valueint;
        strcpy(loans[count_1].status, cJSON_GetObjectItem(loan, "상태")->valuestring);
        strcpy(loans[count_1].loan_addr, cJSON_GetObjectItem(loan, "배송지")->valuestring);
        count_1++;                                                                // 도서 수 증가
    }
    cJSON_Delete(root_1);            // JSON 객체 해제
    free(data_1);                    // 메모리 해제
    loan_count = count_1;            // 총 도서 수 저장
    return count_1;
}

// 대출 목록을 JSON 파일로 저장하는 함수
int save_loans(const char *filename) {
    cJSON *root_1 = cJSON_CreateArray();     // 빈 JSON 배열 생성
    for (int i = 0; i < loan_count; i++) {
        cJSON *a = cJSON_CreateObject();                                   // 각 도서를 위한 객체 생성
        cJSON_AddStringToObject(a, "id", loans[i].id);
        cJSON_AddStringToObject(a, "제목", loans[i].title);            // 저자 추가
        cJSON_AddStringToObject(a, "저자", loans[i].author);
        cJSON_AddStringToObject(a, "출판사", loans[i].publisher);
        cJSON_AddNumberToObject(a, "출판년", loans[i].pub_year);
        cJSON_AddNumberToObject(a, "권", loans[i].num_books);
        cJSON_AddStringToObject(a, "ISBN", loans[i].isbn);
        cJSON_AddNumberToObject(a, "대출 시간", loans[i].loan_time);
        cJSON_AddStringToObject(a, "상태", loans[i].status);
        cJSON_AddStringToObject(a, "배송지", loans[i].loan_addr);
        cJSON_AddItemToArray(root_1, a);                                    // 배열에 추가
    }
    char *out = cJSON_Print(root_1);                // JSON 문자열로 변환
    FILE *fp_1 = fopen(filename, "w");              // 파일 쓰기 모드로 열기
    fputs(out, fp_1);                               // 파일에 저장
    fclose(fp_1);                                   // 파일 닫기
    free(out);                                    // 문자열 메모리 해제
    cJSON_Delete(root_1);                           // JSON 배열 해제
    return 1;
}

// 유저 아이디로 대출 검색 후 갯수 반환
int search_user_loans_count(const char *id) {
    int found = 0;
    for (int i = 0; i < loan_count; i++)
    {
        if ((strcmp(loans[i].id, id) == 0)) 
        {
            found++;  // 검색 결과에 추가
        }
    }
    return found;  // 검색된 도서 수 반환
}
// 유저 아이디로 도서 검색 후 구조체에 채움
void search_user_loans(const char *id, Book2 *results) {
    int found = 0;
    for (int i = 0; i < loan_count; i++)
    {
        if ((strcmp(loans[i].id, id) == 0)) 
        {
            results[found++] = loans[i];  // 검색 결과에 추가

        }
    }
}
// ISBN으로 도서 검색 후 구조체에 채움
void search_plus_loans(Book2 *results, int num_loan) {
    for (int i = 0; i < book_count; i++)
    {
        if ((strcmp(books[i].isbn, results->isbn) == 0)) 
        {
            // strcpy(results->title, books[i].title);
            // strcpy(results->author, books[i].author);
            // strcpy(results->publisher, books[i].publisher);
            // results->pub_year = books[i].pub_year;
            // strcpy(results->isbn, books[i].isbn);
            // strcpy(results->status, "승인대기");
            // results->loan_time = timer;
            pthread_mutex_lock(&book_mutex);
            books[i].num_books -= num_loan;
            pthread_mutex_unlock(&book_mutex);
            break;
        }
    }
}

// 키워드로 대출 검색 후 갯수 반환
int search_loans_count(const char *key, const char *value) {
    int found = 0;
    for (int i = 0; i < loan_count; i++)
    {
        if ((strcmp(key, "title") == 0 && strstr(loans[i].title, value)) || (strcmp(key, "author") == 0 && strstr(loans[i].author, value)) || (strcmp(key, "isbn") == 0 && strstr(loans[i].isbn, value))) 
        {
            found++;  // 검색 결과에 추가

        }
    }
    return found;  // 검색된 도서 수 반환
}
// 키워드로 도서 검색 후 반환
void search_loans(const char *key, const char *value, Book2 *results) {
    int found = 0;
    for (int i = 0; i < loan_count; i++)
    {
        if ((strcmp(key, "title") == 0 && strstr(loans[i].title, value)) || (strcmp(key, "author") == 0 && strstr(loans[i].author, value)) || (strcmp(key, "isbn") == 0 && strstr(loans[i].isbn, value))) 
        {
            results[found++] = loans[i];  // 검색 결과에 추가

        }
    }
}

// 도서 대출시 책수량 -
void modify_book_byloan(Book2 b) {
    pthread_mutex_lock(&book_mutex);
    for (int i = 0; i < book_count; i++) {
        if (strcmp(books[i].isbn, b.isbn) == 0)
        {
            books[i].num_books -= b.num_books;
            save_books("DATA.json");
            pthread_mutex_unlock(&book_mutex);
        }
    }
    pthread_mutex_unlock(&book_mutex);
}

// 대출 추가
int add_loan(Book2 b) {
    pthread_mutex_lock(&book_mutex);          // 쓰레드 동기화를 위한 뮤텍스 잠금
    loans[loan_count++] = b;                  // 배열에 도서 추가
    save_loans("DATA2.json");             // 저장
    pthread_mutex_unlock(&book_mutex);        // 뮤텍스 해제
    return 1;
}

// 대출 삭제
int delete_loan(char *isbn) {
    pthread_mutex_lock(&book_mutex);
    int found = 0;
    for (int i = 0; i < loan_count; i++)
    {
        if (strcmp(loans[i].isbn, isbn) ==0)
        {
            for (int j = i; j < loan_count - 1; j++) loans[j] = loans[j + 1];  // 뒤로 밀기
            loan_count--;
            found = 1;
            break;
        }
    }
    if (found) save_loans("DATA2.json");
    pthread_mutex_unlock(&book_mutex);
    return found;
}

// 대출 수정
int modify_loan(Book2 b) {
    pthread_mutex_lock(&book_mutex);
    for (int i = 0; i < loan_count; i++) {
        if (strcmp(loans[i].isbn, b.isbn) == 0)
        {
            loans[i] = b;
            save_loans("DATA2.json");
            return 1;
        }
    }
    pthread_mutex_unlock(&book_mutex);
    return 0;
}

// 현장대출은 6일+28일 지나면 온라인대출은 10일+28일 지나면 불량대출자로 만드는 함수
void make_bull(User *Users, time_t timer)
{
    for(int i = 0;i<loan_count;i++)
    {
        if(((loans[i].loan_time +2937600)> timer) && (strcmp(loans[i].status, "대출중")==0))
        {
            for(int j = 0;j<user_count;j++)
            {
                if(strcmp(loans[i].id, Users[j].id)==0)
                {
                    pthread_mutex_lock(&book_mutex);
                    Users[j].bsc = 1;
                    save_user("users.json");
                    pthread_mutex_unlock(&book_mutex);
                }
            }
        }
        else if(((loans[i].loan_time +3283200)> timer) && (strcmp(loans[i].status, "온라인대출중")==0))
        {
            for(int j = 0;j<user_count;j++)
            {
                if(strcmp(loans[i].id, Users[j].id)==0)
                {
                    pthread_mutex_lock(&book_mutex);
                    Users[j].bsc = 1;
                    save_user("users.json");
                    pthread_mutex_unlock(&book_mutex);
                }
            }
        }
    }
}

// 불량대출자 구제하는 함수
void save_bull(User *Users)
{
    for(int i = 0;i<user_count;i++)
    {
        if(Users[i].bsc=1)
        {
            pthread_mutex_lock(&book_mutex);
            Users[i].bsc = 0;
            save_user("users.json");
            pthread_mutex_unlock(&book_mutex);
        }
    }
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
// 클라이언트 요청을 처리하는 쓰레드 함수
void *client_handler(void *arg) {
    time_t timer = time(NULL);
    struct tm* t = localtime(&timer);
    int client_socket = *(int *)arg;
    free(arg);  // 할당된 메모리 해제        send(client_socket,&user_count,sizeof(int),0);
    make_bull(Users, timer);
    char cmd[16], id[50], pw[50], nickname[50], phone[50], address[100];
    int year = 0;
    int messege_a = 0;
    int bull_count = 0; // 불량자 구별카운트

    /* 달력정보를 초기화하는 내용 */
    bussiness_month m4[31];
    memset(&m4, 0, sizeof(bussiness_month)*31);
    parsing_json_to_struct_for_bussiness(m4, 4);



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

    int helloworld;
    read(client_socket, &helloworld, sizeof(helloworld));

    if (result == 1 && strcmp(cmd, "login") == 0 && helloworld == 0) {      //일반사용자로 로그인했을때
        printf("[%s]: logged in\n", id);
        while (1) {
            char id_[50] = {0};
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
                    
                    strcpy(user_info.id, id_);
                    strcpy(user_info.name, name);
                    user_info.age = age;
                    strcpy(user_info.phone, phone);
                    strcpy(user_info.addr , addr);
                    
                    write(client_socket, user_info.id, sizeof(user_info.id));
                    write(client_socket, user_info.name, sizeof(user_info.name));
                    write(client_socket, &user_info.age, sizeof(user_info.age));
                    write(client_socket, user_info.phone, sizeof(user_info.phone));
                    write(client_socket, user_info.addr, sizeof(user_info.addr));
                    // printf("%s\n",user_info.name);
                    // printf("%d\n",user_info.age);
                    // printf("%s\n",user_info.phone);
                    // printf("%s\n",user_info.addr);
                }
                else{
                    printf("유저가 없거나 가져올수 없습니다.\n");
                }
            }
            // for(int i = 0;i<loan_count;i++)
            // {
            //     printf("%s\n %s\n %s\n %s\n %d\n %d\n %s\n %d\n %s\n"
            //         , loans[i].id, loans[i].title, loans[i].author, loans[i].publisher, loans[i].pub_year, loans[i].num_books, loans[i].isbn, loans[i].loan_time, loans[i].status);
            // }
            int d_count = search_user_loans_count(id);
            // printf("%d\n", d_count);
            Book2 *d_found = malloc(sizeof(Book2)*d_count);
            memset(d_found, 0, sizeof(Book2)*d_count);
            write(client_socket, &d_count, sizeof(int));         //1.멀록카운트
            if(d_count>0)
            {
                search_user_loans(id, d_found);
                for (int i = 0; i < d_count; i++)
                {
                    write(client_socket, &d_found[i], sizeof(Book2)); //2.대출목록
                }
            }
            // for(int i = 0;i<loan_count;i++)
            // {
            //     printf("%s\n %s\n %s\n %s\n %d\n %d\n %s\n %ld\n %s\n"
            //         , d_found[i].id, d_found[i].title, d_found[i].author, d_found[i].publisher, d_found[i].pub_year, d_found[i].num_books, d_found[i].isbn, d_found[i].loan_time, d_found[i].status);
            // }
            char action[16] = {0};
            read(client_socket, action, sizeof(action));  // 3.사용자 요청
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
                while(1)
                {
                    char key[50] = {0};
                    char val[100] = {0};
                    char tmp_addr[100] = {0};
                    read(client_socket, key, sizeof(key));      //4.키
                    if (strcmp(key, "back")==0)
                    {
                        break;
                    }
                    read(client_socket, val, sizeof(val));      //5.밸류
                    printf("%s %s\n",key, val);
                    int count = search_books_count(key, val);
                    write(client_socket, &count, sizeof(int));        //6. 검색카운트
                    if (count>0)
                    {

                        Book *p_found = malloc(sizeof(Book)*count);
                        memset(p_found, 0, sizeof(Book)*count);
                        search_books(key, val, p_found);
                        // for(int i = 0; i<count;i++)
                        // {
                        //     printf("[%d\n] %s\n %s\n %s\n %d\n %d\n %s\n"
                        //         ,i, p_found[i].title, p_found[i].author, p_found[i].publisher,p_found[i].pub_year, p_found[i].num_books, p_found[i].isbn);
                        // }
                        for (int i = 0; i < count; i++)
                        {
                            write(client_socket, &p_found[i], sizeof(Book));
                        }
                        int num_1 = 0;  //오류 분석
                        read(client_socket, &num_1, sizeof(int));
                        if (num_1== 0)
                        {
                            p_found = NULL;
                            free(p_found);
                            p_found = NULL;
                            continue;
                        }
                        int num = 0;  // 클라이언트 명령
                        int num_loan = 0;  //빌릴 책 갯수
                        read(client_socket, &num, sizeof(int));      //7.대출,다시,처음
                        if(num ==1)    //  대출이면
                        {
                            read(client_socket, &num_1, sizeof(int));  //오류분석
                            read(client_socket, &num, sizeof(int));  //대출 번호
                            
                            if (num_1== 0 || num>count || num <0)
                            {
                                p_found = NULL;
                                free(p_found);
                                p_found = NULL;
                                continue;
                            }
                            read(client_socket, &num_1, sizeof(int));   //오류분석
                            read(client_socket, &num_loan, sizeof(int));   //책권수
                            if(num_1 ==0)
                            {
                                p_found = NULL;
                                free(p_found);
                                p_found = NULL;
                                continue;
                            }
                            printf("num : %d\n", num);

                            int error_msg = 0;         //0이면 성공, 1이면 수량부족, 2면 같은책 2권 넘어감, 3이면 10권 넘음
                            for(int i = 0;i<book_count;i++)
                            {
                                if(strcmp(books[i].isbn, p_found[num-1].isbn)==0) //고른책과 전체 책목록 비교
                                {
                                    if((books[i].num_books-num_loan)<0) // (해당 책남은 수량 - 빌릴 책수량)이 음수면
                                    {
                                        error_msg = 1;   //수량 부족 에러코드
                                        break;
                                    }
                                    else
                                    {
                                        for(int j = 0;j<d_count;j++)
                                        {
                                            if(strcmp(d_found[j].isbn, p_found[num-1].isbn)==0)  // (대출목록의 isbn과 고른책의 isbn 중복검사)
                                            {
                                                printf("대출목록 : %s\n  고른책 : %s\n",d_found[j].isbn, p_found[num-1].isbn);
                                                if((d_found[j].num_books+num_loan)>2)
                                                {
                                                    error_msg = 2;   //같은책 2권 넘어감
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    break;  //비교해서 하나라도 나오면 반복문 종료
                                }
                            }
                            if(num_loan>2)
                            {
                                error_msg = 2;
                            }
                            int tmp_error_check = 0 + num_loan;     //체크용 변수
                            for(int i =0;i<d_count;i++)
                            {
                                tmp_error_check += d_found[i].num_books;
                            }
                            if(tmp_error_check >10) error_msg = 3;    //빌리려는 책이 10권이 넘어가면
                            if(num_loan == 0) error_msg = 4;    //0권을 입력하면
                            for(int i = 0;i<user_count;i++)
                            {
                                if((strcmp(Users[i].id, id)==0) && Users[i].bsc ==1)
                                    error_msg = 5;
                            }
                            for(int i = 0;i<loan_count;i++)
                            {
                                if((strcmp(loans[i].id, id)==0) && (loans[i].loan_time+864000 < timer))
                                    error_msg = 6;
                            }
                            write(client_socket, &error_msg, sizeof(int));
                            printf("빌릴 책 갯수:%d\n",num_loan);
                            printf("에러메시지:%d\n",error_msg);
                            if(error_msg ==1)
                            {
                                p_found = NULL;
                                free(p_found);
                                p_found = NULL;
                                continue;
                            }
                            else if(error_msg ==2)
                            {
                                p_found = NULL;
                                free(p_found);
                                p_found = NULL;
                                continue;
                            }
                            else if(error_msg ==3)
                            {
                                p_found = NULL;
                                free(p_found);
                                p_found = NULL;
                                continue;
                            }
                            else if(error_msg ==4)
                            {
                                p_found = NULL;
                                free(p_found);
                                p_found = NULL;
                                continue;
                            }
                            else if(error_msg ==5)
                            {
                                p_found = NULL;
                                free(p_found);
                                p_found = NULL;
                                continue;
                            }
                            else if(error_msg ==6)
                            {
                                p_found = NULL;
                                free(p_found);
                                p_found = NULL;
                                continue;
                            }

                            // 대출 저장 위한 임시 구조체
                            Book2 *tmp = malloc(sizeof(Book2));
                            memset(tmp, 0, sizeof(Book2));
                            read(client_socket, tmp, sizeof(Book2));
                            search_plus_loans(tmp, num_loan);      //전체 책 목록에서 -num_loan
                            tmp->loan_time = timer;
                            add_loan(*tmp);  //임시 구조체값을 전체 대출목록에 저장
                            d_found[d_count++] = *tmp;
                            pthread_mutex_lock(&book_mutex);
                            save_books("DATA.json");
                            pthread_mutex_unlock(&book_mutex);
                            tmp = NULL;
                            free(tmp);
                            tmp = NULL;
                            p_found = NULL;
                            free(p_found);
                            p_found = NULL;
                        }
                        else if(num ==2)
                        {
                            p_found = NULL;
                            free(p_found);
                            p_found = NULL;
                            continue;
                        }
                        else if(num ==3)
                        {
                            p_found = NULL;
                            free(p_found);
                            p_found = NULL;
                            break;
                        }
                        else
                        {
                            p_found = NULL;
                            free(p_found);
                            p_found = NULL;
                            break;
                        }
                    }
                }
            }
            else if (strcmp(action, "3") == 0)                 // 메세지
            {
                // printf("message\n");
                client_message(client_socket);
            }
            
            else if (strcmp(action, "4") == 0)
            {
                d_found = NULL; 
                free(d_found);
                d_found = NULL; 
                break;
            }                                                                                                                                                                                                                                                                                                                                                                                                                                              
        }
    }
    else if (result == 2 && strcmp(cmd, "login") == 0 && helloworld == 1) { //관리자로 로그인했을때 기능
        printf("[%s]:logged in\n", id);
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
                    memset(p_found, 0, sizeof(Book)*count);
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

                

                char sub_action[16]="\0";

                //read cmd
                read(client_socket, cmd, sizeof(cmd));
                int mon_and_day[2] = {0,};

               
                

                if(strcmp(cmd,"1")==0) //영업일 설정
                {
                    
                    // read mon_and_day
                    read(client_socket, mon_and_day, sizeof(mon_and_day));
                    // printf("log 827     :%d %d\n",mon_and_day[0], mon_and_day[1] );
                    //영업일 설정 리턴 0 or 1 잘설정되면 1
                    int set_work_result =0;
                    
                    
                    //1 파싱함
                    parsing_json_to_struct_for_bussiness(m4, 4);

                    //2. 설정함
                    set_work_result = set_work_day( mon_and_day[0],  mon_and_day[1], m4);
                    
                    //3. 저장함
                    make_json_file_for_bussiness(m4, 4);

                    

                    
                    printf("log server 1 send set_work_result : %d\n", set_work_result);
                    // send set_work_result
                    send(client_socket, &set_work_result, sizeof(set_work_result),0);

                    // check
                    if(is_open_for_business(m4,mon_and_day[0],  mon_and_day[1]))
                    {
                        printf("%d월 %d일은 영업일 입니다.\n",mon_and_day[0],  mon_and_day[1]);
                    }else{
                        printf("%d월 %d일은 영업이 아닙니다.\n",mon_and_day[0],  mon_and_day[1]);
                    }
                    
                }else if(strcmp(cmd,"2")==0)
                {
                    //휴업일 설정 리턴 0 or 1   
                    

                    // read mon_and_day
                    read(client_socket, mon_and_day, sizeof(mon_and_day));
                    memset(&m4, 0, sizeof(bussiness_month)*31);
                    parsing_json_to_struct_for_bussiness(m4, 4);

                    int set_holy_result =0;


                    //1 파싱함
                    parsing_json_to_struct_for_bussiness(m4, 4);

                    //2. 설정함
                    set_holy_result = set_holiday( mon_and_day[0],  mon_and_day[1], m4);
                    
                    //3. 저장함
                    make_json_file_for_bussiness(m4, 4);

                    send(client_socket, &set_holy_result, sizeof(set_holy_result),0);

                    if(is_open_for_business(m4,mon_and_day[0],  mon_and_day[1]))
                    {
                        printf("%d월 %d일은 영업일 입니다.\n",mon_and_day[0],  mon_and_day[1]);
                        
                    }else{
                        printf("%d월 %d일은 영업이 아닙니다.\n",mon_and_day[0],  mon_and_day[1]);
                        

                    }

                }else if(strcmp(cmd,"3")==0)
                {
                    //해당일이 영업일인지 아닌지 체크

                    // read mon_and_day
                    read(client_socket, mon_and_day, sizeof(mon_and_day));

                    memset(&m4, 0, sizeof(bussiness_month)*31);
                    parsing_json_to_struct_for_bussiness(m4, 4);
                    
                    for(int i=0; i<31; i++)
                    {
                        printf("logs 1026 for %s\n", m4[i].date);
                    }

                    int result;
                    if(is_open_for_business(m4,mon_and_day[0],  mon_and_day[1]))
                    {
                        printf("%d월 %d일은 영업일 입니다.\n",mon_and_day[0],  mon_and_day[1]);
                        result = 1;
                        
                    }else{
                        printf("%d월 %d일은 영업이 아닙니다.\n",mon_and_day[0],  mon_and_day[1]);
                        result = 0;

                    }
                    printf("logs 1040 : %d\n",result);

                    send(client_socket, &result, sizeof(result),0);


                }

               
            }
            else if (strcmp(action, "4") == 0) // 대출자정보
            {
                while(1)
                {
                    char loan_id[50] = {0};
                    int num_1 = 0;  //오류 분석용
                    int d_num = 0;  //대출 목록 번호
                    write(client_socket,&loan_count,sizeof(int));
                        
                    for(int i=0;i<loan_count;i++)
                    {
                        if(strcmp(loans[i].status, "온라인승인대기")==0)         //대기먼저 구조체에 채움
                            write(client_socket,&loans[i],sizeof(Book2));
                        else
                            write(client_socket,&loans[i],sizeof(Book2));
                    }
                    read(client_socket, action, sizeof(action));

                    if(strcmp(action, "1")==0)
                    {
                        read(client_socket, action, sizeof(action));
                        if(strcmp(action, "1") == 0)             //온라인 대출승인
                        {
                        //     read(client_socket, &num_1, sizeof(int));
                        //     read(client_socket, &action, sizeof(int));
                        //     if (num_1== 0 || d_num >loan_count || d_num < 0)
                        //     {
                        //         continue;
                        //     }
                            read(client_socket, loan_id, sizeof(loan_id));   //승인할 아이디 받음
                            int tmp_num=0;
                            int d_msg=0;    //오류 메시지 확인용 0이면 실패 1이면 성공
                            
                            for(int i = 0;i<loan_count;i++)  //대출목록에서 아이디와 온라인승인대기인걸 검색
                            {
                                if((strcmp(loan_id, loans[i].id)==0) && (strcmp(loans[i].status,"온라인승인대기")==0))
                                    ++tmp_num;
                            }
                            if (tmp_num>2)
                            {
                                for(int i = 0;i<loan_count;i++)   //3이상이면 온라인 승인 대기를 대출중으로 바꿈
                                {
                                    if((strcmp(loan_id, loans[i].id)==0) && (strcmp(loans[i].status,"온라인승인대기")==0))
                                    {
                                        strcpy(loans[i].status, "온라인대출중");
                                        loans[i].loan_time = timer;
                                        ///////////유닉스 시간값 가게 해야함///////////////////
                                        pthread_mutex_lock(&book_mutex);
                                        save_loans("DATA2.json");
                                        pthread_mutex_unlock(&book_mutex);
                                    }
                                }
                                d_msg = 1;
                            }
                            else
                                d_msg = 0;
                            write(client_socket,&d_msg, sizeof(int));
                            break;

                        }
                        else if(strcmp(action, "2") == 0)        //현장대출승인
                        {
                            int d_msg = 0; //오류 확인용 임시 0이면 아이디 없음 1이면 있음
                            int tmp_usercount = 0;  //임시 유저 위치 확인용
                            read(client_socket, loan_id, sizeof(loan_id));
                            for(int i=0;i<user_count;i++)
                            {
                                if(strcmp(Users[i].id, loan_id)==0)
                                {
                                    tmp_usercount = i;
                                    d_msg = 1;
                                    break;
                                }
                            }
                            write(client_socket, &d_msg, sizeof(int));
                            if(d_msg ==0) break;
                            int d_count = search_user_loans_count(loan_id);
                            // printf("%d\n", d_count);
                            Book2 *d_found = malloc(sizeof(Book2)*d_count);
                            memset(d_found, 0, sizeof(Book2)*d_count);
                            if(d_count>0) search_user_loans(loan_id, d_found); //유저 아이디로 검색후 도서구조체에 채움

                            char loan_isbn[100] = {0};
                            read(client_socket, loan_isbn,sizeof(loan_isbn));
                            int error_msg = 0;         //0이면 성공, 1이면 수량부족, 2면 같은책 2권 넘어감, 3이면 10권 넘음 4면 검색값없음
                            int tmp_i = 0; // 고른책 전체 책에서 몇번째 isbn인지 저장
                            for(int i = 0;i<book_count;i++)
                            {
                                if(strcmp(books[i].isbn, loan_isbn)==0) //고른책과 전체 책목록 비교
                                {
                                    tmp_i = i;
                                    if((books[i].num_books-1)<0) // (해당 책남은 수량 - 빌릴 책수량)이 음수면
                                    {
                                        error_msg = 1;   //수량 부족 에러코드
                                        break;
                                    }
                                    else
                                    {
                                        for(int j = 0;j<d_count;j++)
                                        {
                                            if(strcmp(d_found[j].isbn, loan_isbn)==0)  // (대출목록의 isbn과 고른책의 isbn 중복검사)
                                            {
                                                printf("대출목록 : %s\n  고른책 : %s\n",d_found[j].isbn, loan_isbn);
                                                if((d_found[j].num_books+1)>2)
                                                {
                                                    error_msg = 2;   //같은책 2권 넘어감
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    break;  //비교해서 하나라도 나오면 반복문 종료
                                }
                            }
                            int tmp_error_check = 0 + 1;     //체크용 변수
                            for(int i =0;i<d_count;i++)
                            {
                                tmp_error_check += d_found[i].num_books;
                            }
                            if(tmp_error_check >10) error_msg = 3;    //빌리려는 책이 10권이 넘어가면
                            if(strcmp(books[tmp_i].isbn, loan_isbn)!=0) error_msg = 4;
                            for(int i = 0;i<user_count;i++)
                            {
                                if((strcmp(Users[i].id, loan_id)==0) && Users[i].bsc ==1)
                                    error_msg = 5;
                            }
                            write(client_socket, &error_msg, sizeof(int));
                            if(error_msg ==0)
                            {
                                    // 대출 저장 위한 임시 구조체
                                Book2 *tmp = malloc(sizeof(Book2));
                                memset(tmp, 0, sizeof(Book2));
                                
                                strcpy(tmp->loan_addr,Users[tmp_usercount].addr);
                                strcpy(tmp->title, books[tmp_i].title);
                                strcpy(tmp->author, books[tmp_i].author);
                                strcpy(tmp->publisher, books[tmp_i].publisher);
                                tmp->pub_year = books[tmp_i].pub_year;
                                strcpy(tmp->isbn, books[tmp_i].isbn);
                                strcpy(tmp->id, Users[tmp_usercount].id); // 유저 ID 저장
                                tmp->num_books = 1;
                                strcpy(tmp->status, "대출중");
                                tmp->loan_time = timer;
                                search_plus_loans(tmp, 1);      //전체 책 목록에서 -num_loan
                                add_loan(*tmp);  //임시 구조체값을 전체 대출목록에 저장
                                pthread_mutex_lock(&book_mutex);
                                save_books("DATA.json");
                                pthread_mutex_unlock(&book_mutex);
                                tmp = NULL;
                                free(tmp);
                                tmp = NULL;
                                printf("대출 완료 되었습니다!\n");
                                d_found = NULL;
                                free(d_found);
                                d_found = NULL;
                                break;
                            }
                            else if(error_msg ==1)
                            {
                                printf("수량이 부족합니다.\n");
                                d_found = NULL;
                                free(d_found);
                                d_found = NULL;
                                break;
                            }
                            else if(error_msg ==2)
                            {
                                printf("같은 책을 2권 초과하였습니다.\n");
                                d_found = NULL;
                                free(d_found);
                                d_found = NULL;
                                break;
                            }
                            else if(error_msg ==3)
                            {
                                printf("10권 넘게 빌릴 수 없습니다.\n");
                                d_found = NULL;
                                free(d_found);
                                d_found = NULL;
                                break;
                            }
                            else if(error_msg ==4)
                            {
                                printf("ISBN을 다시 확인하여 주십시오.\n");
                                d_found = NULL;
                                free(d_found);
                                d_found = NULL;
                                break;
                            }
                            else if(error_msg ==5)
                            {
                                printf("불량대출자입니다(대출불가)\n");
                                d_found = NULL;
                                free(d_found);
                                d_found = NULL;
                                break;
                            }
                        }
                    }
                    else if(strcmp(action, "2") == 0) //도서반납
                    {
                        Book bannam;
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

                                if (return_count > 0)
                                {
                                    int current_books = book_2s[i].num_books;

                                    if (modify_book2s(return_count, isbn))
                                    {
                                        printf("반납처리가 완료되었습니다.\n");
                                        for(int i = 0;i < book_count;i++){
                                            if(strcmp(books[i].isbn,isbn) == 0)
                                            {
                                                strcpy(bannam.author,books[i].author);
                                                strcpy(bannam.extra_n,books[i].extra_n);
                                                strcpy(bannam.isbn,books[i].isbn);
                                                strcpy(bannam.kdc,books[i].kdc);
                                                strcpy(bannam.kdc_subject,books[i].kdc_subject);
                                                bannam.loan_frequency = books[i].loan_frequency;
                                                bannam.no = books[i].no;
                                                bannam.pub_year = books[i].pub_year;
                                                strcpy(bannam.publisher,books[i].publisher);
                                                strcpy(bannam.title,books[i].title);
                                                if(books[i].num_books < 0)
                                                {
                                                    bannam.num_books = (books[i].num_books - return_count);
                                                }
                                                else 
                                                    bannam.num_books = (books[i].num_books + return_count);
                                                    printf("여기까지와지니?\n");
                                                    printf("%d\n",bannam.num_books);
                                                
                                            }
                                        }
                                        modify_book(bannam);

                                        if ((current_books - return_count) <= 0)
                                        {
                                            delete_book2(isbn);
                                            printf("대출 목록에서 삭제되었습니다.\n");
                                        }
                                    }
                                    else
                                    {
                                        printf("도서 정보 수정 중 오류가 발생했습니다.\n");
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
                    else if(strcmp(action, "3") == 0)  //불량 대출자 구제
                    {
                        save_bull(Users);
                        break;
                    }
                    else if(strcmp(action, "4") == 0)  //대출목록수정
                    {
                        Book2 b;
                        int num_1 = 0;    //오류 측정용 변수
                        int num_2 = 0;
                        int num_3 = 0;
                        read(client_socket, &num_1, sizeof(int));
                        if (num_1== 0) break;
                        read(client_socket, &num_2, sizeof(int));
                        if (num_2== 0) break;
                        read(client_socket, &num_3, sizeof(int));
                        if (num_3== 0) break;
                        read(client_socket, &b, sizeof(Book2));
                        result = modify_loan(b);
                        write(client_socket, &result, sizeof(int));
                        break;
                    }
                }
            }
            else if (strcmp(action, "5") == 0) // 로그아웃
            {
                break;
            }   
        }
    }
    else if (result == 3 && strcmp(cmd, "login") == 0 && helloworld == 2) { //사서로 로그인했을때 기능
        printf("[%s]:logged in\n", id);
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
                        break;
                    }
                    char loan_id[50] = {0};
                    int num_1 = 0;  //오류 분석용
                    int d_num = 0;  //대출 목록 번호
                    write(client_socket,&loan_count,sizeof(int));
                        
                    for(int i=0;i<loan_count;i++)
                    {
                        if(strcmp(loans[i].status, "온라인승인대기")==0)         //대기먼저 구조체에 채움
                            write(client_socket,&loans[i],sizeof(Book2));
                        else
                            write(client_socket,&loans[i],sizeof(Book2));
                    }
                    read(client_socket, action, sizeof(action));

                    if(strcmp(action, "1")==0)
                    {
                        read(client_socket, action, sizeof(action));
                        if(strcmp(action, "1") == 0)             //온라인 대출승인
                        {
                        //     read(client_socket, &num_1, sizeof(int));
                        //     read(client_socket, &action, sizeof(int));
                        //     if (num_1== 0 || d_num >loan_count || d_num < 0)
                        //     {
                        //         continue;
                        //     }
                            read(client_socket, loan_id, sizeof(loan_id));   //승인할 아이디 받음
                            int tmp_num=0;
                            int d_msg=0;    //오류 메시지 확인용 0이면 실패 1이면 성공
                            for(int i = 0;i<loan_count;i++)  //대출목록에서 아이디와 온라인승인대기인걸 검색
                            {
                                if((strcmp(loan_id, loans[i].id)==0) && (strcmp(loans[i].status,"온라인승인대기")==0))
                                    ++tmp_num;
                            }
                            if (tmp_num>2)
                            {
                                for(int i = 0;i<loan_count;i++)   //3이상이면 온라인 승인 대기를 대출중으로 바꿈
                                {
                                    if((strcmp(loan_id, loans[i].id)==0) && (strcmp(loans[i].status,"온라인승인대기")==0))
                                    {
                                        strcpy(loans[i].status, "온라인대출중");
                                        loans[i].loan_time = timer;
                                        ///////////유닉스 시간값 가게 해야함///////////////////
                                        pthread_mutex_lock(&book_mutex);
                                        save_loans("DATA2.json");
                                        pthread_mutex_unlock(&book_mutex);
                                    }
                                }
                                d_msg = 1;
                            }
                            else
                                d_msg = 0;
                            write(client_socket,&d_msg, sizeof(int));
                            break;

                        }
                        else if(strcmp(action, "2") == 0)        //현장대출승인
                        {
                            int d_msg = 0; //오류 확인용 임시 0이면 아이디 없음 1이면 있음
                            int tmp_usercount = 0;  //임시 유저 위치 확인용
                            read(client_socket, loan_id, sizeof(loan_id));
                            for(int i=0;i<user_count;i++)
                            {
                                if(strcmp(Users[i].id, loan_id)==0)
                                {
                                    tmp_usercount = i;
                                    d_msg = 1;
                                    break;
                                }
                            }
                            write(client_socket, &d_msg, sizeof(int));
                            if(d_msg ==0) break;
                            int d_count = search_user_loans_count(loan_id);
                            // printf("%d\n", d_count);
                            Book2 *d_found = malloc(sizeof(Book2)*d_count);
                            memset(d_found, 0, sizeof(Book2)*d_count);
                            if(d_count>0) search_user_loans(loan_id, d_found); //유저 아이디로 검색후 도서구조체에 채움

                            char loan_isbn[100] = {0};
                            read(client_socket, loan_isbn,sizeof(loan_isbn));
                            int error_msg = 0;         //0이면 성공, 1이면 수량부족, 2면 같은책 2권 넘어감, 3이면 10권 넘음 4면 검색값없음
                            int tmp_i = 0; // 고른책 전체 책에서 몇번째 isbn인지 저장
                            for(int i = 0;i<book_count;i++)
                            {
                                if(strcmp(books[i].isbn, loan_isbn)==0) //고른책과 전체 책목록 비교
                                {
                                    tmp_i = i;
                                    if((books[i].num_books-1)<0) // (해당 책남은 수량 - 빌릴 책수량)이 음수면
                                    {
                                        error_msg = 1;   //수량 부족 에러코드
                                        break;
                                    }
                                    else
                                    {
                                        for(int j = 0;j<d_count;j++)
                                        {
                                            if(strcmp(d_found[j].isbn, loan_isbn)==0)  // (대출목록의 isbn과 고른책의 isbn 중복검사)
                                            {
                                                printf("대출목록 : %s\n  고른책 : %s\n",d_found[j].isbn, loan_isbn);
                                                if((d_found[j].num_books+1)>2)
                                                {
                                                    error_msg = 2;   //같은책 2권 넘어감
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    break;  //비교해서 하나라도 나오면 반복문 종료
                                }
                            }
                            int tmp_error_check = 0 + 1;     //체크용 변수
                            for(int i =0;i<d_count;i++)
                            {
                                tmp_error_check += d_found[i].num_books;
                            }
                            if(tmp_error_check >10) error_msg = 3;    //빌리려는 책이 10권이 넘어가면
                            if(strcmp(books[tmp_i].isbn, loan_isbn)!=0) error_msg = 4;
                            for(int i = 0;i<user_count;i++)
                            {
                                if((strcmp(Users[i].id, loan_id)==0) && Users[i].bsc ==1)
                                    error_msg = 5;
                            }
                            write(client_socket, &error_msg, sizeof(int));
                            if(error_msg ==0)
                            {
                                    // 대출 저장 위한 임시 구조체
                                Book2 *tmp = malloc(sizeof(Book2));
                                memset(tmp, 0, sizeof(Book2));
                                
                                strcpy(tmp->loan_addr,Users[tmp_usercount].addr);
                                strcpy(tmp->title, books[tmp_i].title);
                                strcpy(tmp->author, books[tmp_i].author);
                                strcpy(tmp->publisher, books[tmp_i].publisher);
                                tmp->pub_year = books[tmp_i].pub_year;
                                strcpy(tmp->isbn, books[tmp_i].isbn);
                                strcpy(tmp->id, Users[tmp_usercount].id); // 유저 ID 저장
                                tmp->num_books = 1;
                                strcpy(tmp->status, "대출중");
                                tmp->loan_time = timer;
                                search_plus_loans(tmp, 1);      //전체 책 목록에서 -num_loan
                                add_loan(*tmp);  //임시 구조체값을 전체 대출목록에 저장
                                pthread_mutex_lock(&book_mutex);
                                save_books("DATA.json");
                                pthread_mutex_unlock(&book_mutex);
                                tmp = NULL;
                                free(tmp);
                                tmp = NULL;
                                printf("대출 완료 되었습니다!\n");
                                d_found = NULL;
                                free(d_found);
                                d_found = NULL;
                                break;
                            }
                            else if(error_msg ==1)
                            {
                                printf("수량이 부족합니다.\n");
                                d_found = NULL;
                                free(d_found);
                                d_found = NULL;
                                break;
                            }
                            else if(error_msg ==2)
                            {
                                printf("같은 책을 2권 초과하였습니다.\n");
                                d_found = NULL;
                                free(d_found);
                                d_found = NULL;
                                break;
                            }
                            else if(error_msg ==3)
                            {
                                printf("10권 넘게 빌릴 수 없습니다.\n");
                                d_found = NULL;
                                free(d_found);
                                d_found = NULL;
                                break;
                            }
                            else if(error_msg ==4)
                            {
                                printf("ISBN을 다시 확인하여 주십시오.\n");
                                d_found = NULL;
                                free(d_found);
                                d_found = NULL;
                                break;
                            }
                            else if(error_msg ==5)
                            {
                                printf("불량대출자입니다(대출불가)\n");
                                d_found = NULL;
                                free(d_found);
                                d_found = NULL;
                                break;
                            }
                        }
                    }
                    else if(strcmp(action, "2") == 0) //도서반납
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
                                Book bannam;
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

                                        if (return_count > 0)
                                        {
                                            int current_books = book_2s[i].num_books;

                                            if (modify_book2s(return_count, isbn))
                                            {
                                                printf("반납처리가 완료되었습니다.\n");
                                                for(int i = 0;i < book_count;i++){
                                                    if(strcmp(books[i].isbn,isbn) == 0)
                                                    {
                                                        strcpy(bannam.author,books[i].author);
                                                        strcpy(bannam.extra_n,books[i].extra_n);
                                                        strcpy(bannam.isbn,books[i].isbn);
                                                        strcpy(bannam.kdc,books[i].kdc);
                                                        strcpy(bannam.kdc_subject,books[i].kdc_subject);
                                                        bannam.loan_frequency = books[i].loan_frequency;
                                                        bannam.no = books[i].no;
                                                        bannam.pub_year = books[i].pub_year;
                                                        strcpy(bannam.publisher,books[i].publisher);
                                                        strcpy(bannam.title,books[i].title);
                                                        if(books[i].num_books < 0)
                                                        {
                                                            bannam.num_books = (books[i].num_books - return_count);
                                                        }
                                                        else 
                                                            bannam.num_books = (books[i].num_books + return_count);
                                                            printf("여기까지와지니?\n");
                                                            printf("%d\n",bannam.num_books);
                                                        
                                                    }
                                                }
                                                modify_book(bannam);

                                                if ((current_books - return_count) <= 0)
                                                {
                                                    delete_book2(isbn);
                                                    printf("대출 목록에서 삭제되었습니다.\n");
                                                }
                                            }
                                            else
                                            {
                                                printf("도서 정보 수정 중 오류가 발생했습니다.\n");
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
                            if (received_choice == 4)
                            {
                                break;
                            }
                        }
                    }
                    else if(strcmp(action, "3") == 0)  //불량 대출자 구제
                    {
                        save_bull(Users);
                        break;
                    }
                    else if(strcmp(action, "4") == 0)  //대출목록수정
                    {
                        Book2 b;
                        int num_1 = 0;    //오류 측정용 변수
                        int num_2 = 0;
                        int num_3 = 0;
                        read(client_socket, &num_1, sizeof(int));
                        if (num_1== 0) break;
                        read(client_socket, &num_2, sizeof(int));
                        if (num_2== 0) break;
                        read(client_socket, &num_3, sizeof(int));
                        if (num_3== 0) break;
                        read(client_socket, &b, sizeof(Book2));
                        result = modify_loan(b);
                        write(client_socket, &result, sizeof(int));
                        break;
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
            else if (strcmp(action, "5") == 0) // 영업일 확인
            {

                //사서 오픈관리
                while(1)
                {
                    printf("사서가 영업일 확인 기능을 선택했습니다. \n");
                
                    bussiness_month m4[31];
                    set_calendar(m4, 4);
                    make_json_file_for_bussiness(m4,4); //이 명령을 하면 json파일이 리셋됨
    
                    time_t timer = time(NULL);
                    struct tm* t = localtime(&timer);

                     //오늘 날짜를 문자열로 바꿈
                     char day[10];
                     sprintf(day, "%d", t->tm_mday);
                     char holiday[50];

                    for(int i=1;i<31;i++){


                        if(strstr(m4[i].date,day))
                        {
                            // printf("인덱스는? %d %s일 쉬는지?%d \n", i, m4[i].date,  m4[i].is_open);
                            if(m4[i].is_open)
                            {
                                strcpy(holiday, "영업일 입니다.");
                            }else
                            {   
                                strcpy(holiday, "휴일 입니다.");
                            }
                        }
                    
                    }
                    printf("금일은  %s \n",holiday);

                    send(client_socket,holiday,sizeof(holiday),0);
                    break;
                }//while(1)문끝

                break;
            }   
            else if (strcmp(action, "6") == 0) // 로그아웃
            {
                break;
            }   
        }
    }
    else if (result == 2 && strcmp(cmd, "login") == 0 && helloworld == 3) {
        close(client_socket);
        return NULL;
    }

    close(client_socket);  // 소켓 종료
    return NULL;
}

// 메인 함수 - 서버 실행
int main(int argc, char *argv[]) {
    load_books("DATA.json");  // 도서 데이터 불러오기
    load_user("users.json");  // 유저 데이터 불러오기
    load_book_2("DATA2.json");
    load_loans("DATA2.json");

    // for (int i=0;i<book_count;i++)
    // {
    // printf(" %d\n %s\n %s\n %s\n %d\n %d\n %s\n %s\n %s\n %s\n %d\n", books[i].no, books[i].title, books[i].author, books[i].publisher, books[i].pub_year,
    // books[i].num_books, books[i].isbn, books[i].extra_n, books[i].kdc, books[i].kdc_subject, books[i].loan_frequency);
    // }
    int server_fd = socket(PF_INET, SOCK_STREAM, 0);  // 소켓 생성
    struct sockaddr_in address = {0};                 // 주소 구조체 초기화
    address.sin_family = AF_INET;                     // IPv4 사용
    address.sin_addr.s_addr = INADDR_ANY;             // 모든 IP 허용
    address.sin_port = htons(atoi(argv[1]));                // 포트 설정

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

//메세지 - 사서
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

        printf("[%s]: title_msg: %s\n", id, title_msg);
        printf("[%s]: script_msg: %s\n", id, title_msg);
        usleep(50000);

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
        int go;

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
            int go;

            read (client_socket, &go, sizeof(go));

        } else if (form == 2) {            //돌아가기(q)
            printf("[%s]: void librarian_checkmessage() 종료\n", id);
            read(client_socket, &go, sizeof(go));
            break;
        } else if (form == 3) {            //유효하지 않은 입력.
            printf("[%s]: 잘못된 입력\n", id);
            read(client_socket, &go, sizeof(go));
            continue;
        } else if (form == 4) {            //읽기(r)-메세지가 없을 때
            read(client_socket, &go, sizeof(go));
            continue;
        } else if (form == 5) {            //삭제(k)-메세지가 없을 때
            read(client_socket, &go, sizeof(go));
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
                int go=0;

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

                    read(client_socket, &go, sizeof(go));

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
                        read(client_socket, &go, sizeof(go));
                    } else {
                        result= 1; 
                        //send 4: int result == 1
                        send(client_socket, &result, sizeof(result), 0);
                        read(client_socket, &go, sizeof(go));
                    }
                } else if (form == 2) {                                         //돌아가기(q)
                    read(client_socket, &go, sizeof(go));
                    break;
                } else if (form == 3) {                                         //유효하지 않은 입력.
                    printf("[%s]: 잘못된 입력\n", user_id);
                    read(client_socket, &go, sizeof(go));
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
// 영업일을 지정하는 함수 (월, 일, 달력객체)
int set_work_day(int month, int day, bussiness_month *m4)
{

    // int a,b;
    // printf("업무일로 설정할 날짜정보를 입력하세요 예 4월 24일: 4 24       \n");
    // scanf("%d %d", &a, &b);

    char date_[50];
    sprintf(date_, "2025-%d-%d",month,day);
    int result =0;
    for(int i=0; i< 30; i++)
    {
        
        // printf("%s\n", m4[i].date);
        if(strcmp(m4[i].date, date_)==0)
        {
            // printf("%s %s\n",m4[i].date, date_);
            m4[i].is_open = 1;
            printf("%s를 업무일로 설정 완료!\n", m4[i].date);
            
            return 1;

        }else{
            result = 0;
        }

       
        
    }
    return result;
}
// example_4.json파일을 파싱하여 구조체로 만드는 함수(달력객체, 월)
void parsing_json_to_struct_for_bussiness(bussiness_month *m, int mon)
{
    char *json_string = read_json_file("example_4.json");
    if(!json_string){
        printf("jsonfile 읽다가 문제생김");
    }
    cJSON * json_array = cJSON_Parse(json_string);
    
    if(!json_array ||!cJSON_IsArray(json_array)){
        printf("JSON 파싱 실패 또는 배열이아님\n");
        exit(1);
    }

    // int array_size = cJSON_GetArraySize(json_array);

    for(int i=0; i<31; i++)
    {
        

        cJSON *day = cJSON_GetArrayItem(json_array,i);
        if(!cJSON_IsObject(day)) continue;

        /*
        "date" : "2025-4-1",
        "day_" : "화",
        "is_open" : 1
        */
        cJSON *d1 = cJSON_GetObjectItemCaseSensitive(day,"date");
        cJSON *d2 = cJSON_GetObjectItemCaseSensitive(day,"day_");
        cJSON *d3 = cJSON_GetObjectItemCaseSensitive(day,"is_open");
        strcpy(m[i].date, d1->valuestring);
        strcpy(m[i].day_, d2->valuestring);
        m[i].is_open=d3->valueint;

        
    
        

    }

}
// 특정 json파일을 읽는 기능을 하는 함수 (파일명)
char *read_json_file(const char *filename)
{
    FILE * file = fopen(filename, "r");
    if(!file){
        printf("파일을 열수 없습니다. %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *json_data = (char *)malloc(file_size+1);
    if(!json_data){
        printf("메모리 할당 실패\n");
        fclose(file);
        return NULL;
    }

    fread(json_data, 1,file_size, file);
    json_data[file_size] = '\0';

    fclose(file);
    return json_data;
}
// 휴일을 지정하는 함수( 월 , 일 , 달력객체)
int set_holiday(int mon, int day,bussiness_month *m)
{

 
    int result;
    char date_[50];
    sprintf(date_, "2025-%d-%d",mon,day);
    
    for(int i=1; i< 31; i++)
    {
        if(i!=0)
        {
            // printf("%s\n", m4[i].date);
            if(strcmp(m[i].date, date_)==0)
            {
                // printf("%s %s\n",m4[i].date, date_);
                m[i].is_open = 0;
                printf("%s를 휴일로 설정 완료!\n", m[i].date);
                
                return 1;
            }
            else {
                result =0;
            }

        }
        
        
    }
    return result;
}
// (0,1) 지정일이 영업일인지 체크하는 함수 (달력객체, 월, 일)
int is_open_for_business(bussiness_month *m, int mon, int day)
{
    

    char date_[50];
    sprintf(date_, "2025-%d-%d",mon,day);

   
    for(int i=0; i< 31; i++)
    {
        
        // printf("%s\n", m4[i].date);
        if(strcmp(m[i].date, date_)==0)
        {
            
            if(m[i].is_open ==1)
            {
                return 1;
            }
            else{
                return 0;

            }
            

        }
    }
}

/// @brief 영업일,휴일 정보가 있는 구조체를 참조하여 example_4.json파일을 생성하는 함수
/// @param m 달력객체
/// @param mon 해당월 (4)
void make_json_file_for_bussiness(bussiness_month *m, int mon)
{
    int DayOfMonth[12] =  {31,28,31,30,31,30,31,31,30,31,30,31};
    char file_name[50];
    sprintf(file_name, "example_%d.json", mon);
    FILE *fp = fopen(file_name, "w");
    fprintf(fp, "[");
    
    for(int i=0; i<29;i++) 
    {
        if(strlen(m[i].day_)!=0)
        {
            fprintf(fp,"{\n");
            fprintf(fp, "\"date\" : \"%s\",\n",m[i].date);
            fprintf(fp, "\"day_\" : \"%s\",\n",m[i].day_);
            fprintf(fp, "\"is_open\" : %d\n",m[i].is_open);
    
            // printf("loooog  make_json_file      %d\n",i);
            if (i<28) //5월일땐 30으로 변경해야함
            {
                /* code */
                fprintf(fp, "},\n");
            }else{
                fprintf(fp, "}\n");
            }  

        }
            
    }

    fprintf(fp, "]\n"); //전체닫기
    fclose(fp);
}

/// @brief 달력 구조체를 생성하는 함수
/// @param m 달력구조체 변수
/// @param mon 몇월인지? 4
void set_calendar(bussiness_month *m, int mon)
{

    // bussiness_month_4 m4[32];
    //달력 배열은 30개  날짜별로  날짜 , 요일, 영업유무를 담는다.

    //날짜와 요일을 만들고 영업유무에 값을 넣는 함수를 만든다. 
    char date_[50];
    char day_[10];
    int DayOfMonth[12] =  {31,28,31,30,31,30,31,31,30,31,30,31};

    for(int i=0; i<DayOfMonth[mon-1]+1; i++)
    {
        if(i!=0)
        {
            sprintf(date_ ,"2025-%d-%d",mon,i);
            // printf("%s  ",date_);
            strcpy(m[i].date, date_);
            solution(mon,i, day_);  //날짜를 넣어서 
            // printf("%s\n", day_);        
            strcpy(m[i].day_,day_);        


            /**set_calendar 함수에서의 내용 */
            // printf("set_calendar    >%s\n",date_);

            if((strcmp(m[i].day_,"토")==0)||(strcmp(m[i].day_,"일")==0))
            {
                m[i].is_open = 0;
                // printf("휴일입니다.\n");
            }else{
                m[i].is_open = 1;
                // printf("영업일입니다.\n");
            }

        }
        
    }
    // for(int i=1; i<31; i++)
    // {
    //     printf("날짜: %s  ",m4[i].date);
    //     printf("%s 요일:",m4[i].day_);
    //     printf("영업 유무 %d \n",m4[i].is_open);
    // }

}


/// @brief 월, 일, 문자열을 매개변수로 주면 문자열매개변로 요일을 저장하는 함수
/// @param a 월
/// @param b 일
/// @param c 요일
void solution(int a, int b, char *c) {
    char *DayOfTheWeek[] = {"일","월","화","수","목", "금","토"};

    int DayOfMonth[12] =  {31,28,31,30,31,30,31,31,30,31,30,31};
    int Total = 0;
    // printf("%d\n", a);
   //    a--;
   char the_day[10];
   while(a>0){
    Total +=DayOfMonth[--a];
   }   
   Total += b;
   
   strcpy(c,DayOfTheWeek[Total%7] );

}
