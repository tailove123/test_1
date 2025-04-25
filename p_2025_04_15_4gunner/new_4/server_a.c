#include <stdio.h>              // 표준 입출력 함수 사용 (printf, fopen 등)
#include <stdlib.h>             // 동적 메모리 할당 (malloc, free 등)
#include <string.h>             // 문자열 처리 함수 (strcmp, strcpy 등)
#include <unistd.h>             // POSIX API (read, write, close 등)
#include <pthread.h>            // 쓰레드 사용을 위한 헤더
#include <netinet/in.h>         // 소켓 주소 구조체를 위한 헤더 (struct sockaddr_in 등)
#include "cJSON.h"   // JSON 처리 라이브러리 헤더
#include "cJSON.c"
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <stdint.h> 
#pragma pack(1)
#define MAX_BOOKS 11000           // 도서 최대 등록 수
#define MAX_USERS 500
#define PORT 10001             // 서버가 열릴 포트 번호
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

//대출관리 구조체 정의
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
Book2 loans[MAX_BOOKS];
User Users[MAX_USERS];
int book_count = 0;             // 현재 등록된 도서 수
int loan_count = 0;             // 현재 대출된 도서 수
int user_count = 0; // 현재 등록된 유저 수

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
    return count;
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
int register_user(const char *id, const char *pw, const char *nickname, int year, const char *phone, const char *address, int messege_a) {
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



// 클라이언트 요청을 처리하는 쓰레드 함수
void *client_handler(void *arg) {
    time_t timer = time(NULL);
    struct tm* t = localtime(&timer);
    int client_socket = *(int *)arg;
    free(arg);  // 할당된 메모리 해제
    make_bull(Users, timer);
    while(1)
    {
        char cmd[16] = {0};
        char id[50] = {0};
        char pw[50] = {0};
        char nickname[50] = {0};
        char phone[50] = {0};
        char address[100]= {0};
        int year = 0;
        int messege_a = 1;
        int plag_exit = 0;
        int result = 0;

        read(client_socket, cmd, sizeof(cmd));
        read(client_socket, id, sizeof(id));
        read(client_socket, pw, sizeof(pw));



        if (strcmp(cmd, "login") == 0) {
            result = login(id, pw);
        } else if (strcmp(cmd, "register") == 0) {
            // 추가 정보 읽기
            read(client_socket, nickname, sizeof(nickname));
            read(client_socket, &year, sizeof(int));
            read(client_socket, phone, sizeof(phone));
            read(client_socket, address, sizeof(address));
            read(client_socket, &messege_a, sizeof(int));

            result = register_user(id, pw, nickname, year, phone, address, messege_a);
        }

        // 결과 전송
        write(client_socket, &result, sizeof(int));

        // 일반 사용자
        if (result == 1 && strcmp(cmd, "login") == 0) {
            while (1)
            {
                printf("나는 일반 사용자입니다\n");
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
                if (strcmp(action, "1") == 0)   //1. 내 정보 확인하기
                {
                    printf("내정보 확인\n");
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
                
                else if (strcmp(action, "5") == 0)
                {
                    d_found = NULL;
                    free(d_found);
                    d_found = NULL;
                    break;
                }
                d_found = NULL; 
                free(d_found);
                d_found = NULL;                                                                                                                                                                                                                                                                                                                                                                                                                                              
            }
        }
        else if (result == 2 && strcmp(cmd, "login") == 0) { //관리자로 로그인했을때 기능
            while (1) {
                printf("나는 관리잡니다.\n");
                char action[16] = {0};
                read(client_socket, action, sizeof(action));  // 사용자 요청
                if (strcmp(action, "1") == 0)
                {
                    char somi[16] = {0};
                    read(client_socket, somi, sizeof(somi));  // 사용자 요청
                    if (strcmp(somi, "1") == 0) //모든도서확인하기
                    {
                        break;
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
                    else if (strcmp(somi, "3") == 0) // 도서수정하기
                    {
                        Book b;
                        read(client_socket, &b, sizeof(Book));
                        result = modify_book(b);
                        write(client_socket, &result, sizeof(int));
                    } 
                    else if (strcmp(somi, "4") == 0)
                    {
                        char isbn[200];
                        read(client_socket, isbn, sizeof(char));
                        result = delete_book(isbn);
                        write(client_socket, &result, sizeof(int));
                    } 
                    else if (strcmp(somi, "5") == 0)
                    {
                        Book b;
                        read(client_socket, &b, sizeof(Book));
                        result = modify_book(b);
                        write(client_socket, &result, sizeof(int));
                    }
                    else if (strcmp(somi, "6") == 0)
                    {
                        continue;
                    }
                }
                else if (strcmp(action, "2") == 0) // 모든계정관리
                {
                    break;
                }
                else if (strcmp(action, "3") == 0) // 도서관오픈관리
                {
                    break;
                }
                else if (strcmp(action, "4") == 0) // 대출관리
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
        else if (result == 3 && strcmp(cmd, "login") == 0) { //사서로 로그인했을때 기능
            while (1) {
                printf("나는 사서입니다.\n");
                char action[16];
                read(client_socket, action, sizeof(action));  // 사용자 요청
                action[strcspn(action, "\n")] = '\0';

                if (strcmp(action, "5") == 0) // 로그아웃
                {
                    break;
                }   
            }
        }
    }

    close(client_socket);  // 소켓 종료
    return NULL;
    printf("소켓종료\n");
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

void utot(int unixtime)
{

}

// 메인 함수 - 서버 실행
int main() {
    load_books("DATA.json");  // 도서 데이터 불러오기
    load_loans("DATA2.json");
    load_user("users.json");  // 유저 데이터 불러오기

    time_t timer;
    struct tm* t = localtime(&timer);

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