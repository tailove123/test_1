#include <stdio.h>              // 표준 입출력 함수 사용 (printf, fopen 등)
#include <stdlib.h>             // 동적 메모리 할당 (malloc, free 등)
#include <string.h>             // 문자열 처리 함수 (strcmp, strcpy 등)
#include <unistd.h>             // POSIX API (read, write, close 등)
#include <pthread.h>            // 쓰레드 사용을 위한 헤더
#include <netinet/in.h>         // 소켓 주소 구조체를 위한 헤더 (struct sockaddr_in 등)
#pragma pack(1)
#include "cJSON.h"   // JSON 처리 라이브러리 헤더
#include "cJSON.c"
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define MAX_BOOKS 11000           // 도서 최대 등록 수
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
} Book2;

typedef struct {
    char id[50];
    char pw[50];
    char name[50];
    int age;
    char phone[50];
    char addr[50];
}User;

typedef struct {

    char date[100];
    char day_[50];
    int is_open;  // 1이면 영업 0이면 휴일
}bussiness_month;



// 전역 도서 배열과 관련 변수 선언
Book books[MAX_BOOKS];          // 도서 목록을 저장할 배열
Book2 loans[MAX_BOOKS];
int book_count = 0;             // 현재 등록된 도서 수
int loan_count = 0;             // 현재 대출된 도서 수
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
void search_plus_loans(Book2 *results, int num_loan, long timer) {
    for (int i = 0; i < book_count; i++)
    {
        if ((strcmp(books[i].isbn, results->isbn) == 0)) 
        {
            strcpy(results->title, books[i].title);
            strcpy(results->author, books[i].author);
            strcpy(results->publisher, books[i].publisher);
            results->pub_year = books[i].pub_year;
            strcpy(results->isbn, books[i].isbn);
            strcpy(results->status, "승인대기");
            results->loan_time = timer;
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

// 클라이언트 요청을 처리하는 쓰레드 함수
void *client_handler(void *arg) {
    time_t timer = time(NULL);
    struct tm* t = localtime(&timer);
    int client_socket = *(int *)arg;
    free(arg);  // 할당된 메모리 해제
    
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
                            int num_1 = 0;
                            int num_2 = 0;
                            read(client_socket, &num_1, sizeof(int));
                            if (num_1== 0)
                            {
                                free(p_found);
                                p_found = NULL;
                                continue;
                            }
                            int num = 0;  // 클라이언트 명령
                            int num_loan = 0;  //빌릴 책 갯수
                            read(client_socket, &num, sizeof(int));      //7.대출,다시,처음
                            if(num ==1)    //  대출이면
                            {
                                read(client_socket, &num_1, sizeof(int));
                                read(client_socket, &num_2, sizeof(int));
                                if (num_1== 0 || num_2>count || num_2 <0)
                                {
                                    free(p_found);
                                    p_found = NULL;
                                    continue;
                                }
                                // 대출 저장 위한 임시 구조체
                                Book2 *tmp = malloc(sizeof(Book2));
                                memset(tmp, 0, sizeof(Book2));
                                read(client_socket, tmp, sizeof(Book2));
                                read(client_socket, &num_loan, sizeof(int));
                                search_plus_loans(tmp, num_loan, timer);
                                tmp->num_books = num_loan;
                                tmp->loan_time = timer;
                                strcpy(tmp->id, id); // 유저 ID 저장
                                add_loan(*tmp);  //임시 구조체값을 전체 대출목록에 저장
                                free(tmp);
                                tmp = NULL;
                                free(p_found);
                                p_found = NULL;
                            }
                            else if(num ==2)
                            {
                                free(p_found);
                                p_found = NULL;
                                continue;
                            }
                            else if(num ==3)
                            {
                                free(p_found);
                                p_found = NULL;
                                break;
                            }
                            else
                            {
                                free(p_found);
                                p_found = NULL;
                                break;
                            }
                        }
                    }
                }
                
                else if (strcmp(action, "5") == 0)
                {
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
                        result = add_book(b);
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
                        break;
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

void utot(int unixtime)
{

}

// 메인 함수 - 서버 실행
int main() {
    load_books("DATA.json");  // 도서 데이터 불러오기
    load_loans("DATA2.json");

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