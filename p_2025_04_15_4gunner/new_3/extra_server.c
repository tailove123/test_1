#include <time.h>

//대출관리 구조체 정의
typedef struct {
    char id[SIZE]; // 유저아이디
    char title[SIZE];  // 제목
    char author[SIZE]; // 저자 
    char publisher[SIZE]; // 출판사  
    int pub_year; // 출판년
    int num_books; //빌린 권수 
    char isbn[SIZE];  // ISBN
    int loan_time; //대출일
    char status[SIZE];
} Book2;

// 전역 도서 배열과 관련 변수 선언
Book2 loans[MAX_BOOKS];
int loan_count = 0;             // 현재 대출된 도서 수

//대출목록 구조체에 저장하는 함수
int load_loans(const char *filename) {
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
    cJSON *loan;
    cJSON_ArrayForEach(loan, root) {      // JSON 배열 순회
        strcpy(loans[count].id, cJSON_GetObjectItem(loan, "id")->valuestring);  
        strcpy(loans[count].title, cJSON_GetObjectItem(loan, "제목")->valuestring);  
        strcpy(loans[count].author, cJSON_GetObjectItem(loan, "저자")->valuestring);
        strcpy(loans[count].publisher, cJSON_GetObjectItem(loan, "출판사")->valuestring);
        loans[count].pub_year = cJSON_GetObjectItem(loan, "출판년")->valueint;
        loans[count].num_books = cJSON_GetObjectItem(loan, "권")->valueint;
        strcpy(loans[count].isbn, cJSON_GetObjectItem(loan, "ISBN")->valuestring);
        loans[count].loan_time = cJSON_GetObjectItem(loan, "대출 시간")->valueint;
        strcpy(loans[count].status, cJSON_GetObjectItem(loan, "상태")->valuestring);
        count++;                                                                // 도서 수 증가
    }

    cJSON_Delete(root);            // JSON 객체 해제
    free(data);                    // 메모리 해제
    loan_count = count;            // 총 도서 수 저장
    return count;                  // 로드된 도서 수 반환
}

// 대출 목록을 JSON 파일로 저장하는 함수
int save_loans(const char *filename) {
    cJSON *root = cJSON_CreateArray();     // 빈 JSON 배열 생성
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
        cJSON_AddStringToObject(a, "대출중", loans[i].status);
        cJSON_AddItemToArray(root, a);                                    // 배열에 추가
    }
    char *out = cJSON_Print(root);                // JSON 문자열로 변환
    FILE *fp = fopen(filename, "w");              // 파일 쓰기 모드로 열기
    fputs(out, fp);                               // 파일에 저장
    fclose(fp);                                   // 파일 닫기
    free(out);                                    // 문자열 메모리 해제
    cJSON_Delete(root);                           // JSON 배열 해제
    return 1;
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
            pthread_mutex_unlock(&book_mutex);
            return 1;
        }
    }
    pthread_mutex_unlock(&book_mutex);
    return 0;
}