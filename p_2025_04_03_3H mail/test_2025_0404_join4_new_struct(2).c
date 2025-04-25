#include <stdio.h>
#include <stdlib.h> // 문자열을 숫자로 변환하는atoi()함수
#include <string.h> // 문자열 조작을 위한strtok(),strcpy()등
#include <sys/stat.h> // mkdir()
#include <unistd.h> // sleep()
#include <dirent.h> // 디렉터리의 구조를 구조체 dirent로 정의
#include <time.h>

#define MAX_LINE 100  // 최대 줄 길이
#define MAX_USERS 100  // 최대 저장할 회원 수
#define SIZE 100
#define PAGE 10
#define MAX_MAIL_NUM 100
#define CLEAR system("clear");

typedef struct{
    int u_num;
    char u_ID[SIZE];
    char u_PW[SIZE];
    char u_name[SIZE];
    int u_age;
    char u_phone_1[SIZE];
    char u_phone_2[SIZE];
    char u_phone_3[SIZE];
} users;

typedef struct{
    char new_id[SIZE];
    char new_pw[SIZE];
    char new_pw_check[SIZE];
    char new_name[SIZE];
    char new_phone[SIZE];
    int new_age;
    int char_cnt[4]; // 아스크 코드값이 영어, 숫자, 특수문자가 아닌경우

    char new_phone_1[SIZE];
    char new_phone_2[SIZE];
    char new_phone_3[SIZE];
} new_user;

// typedef struct{
//     char file_name[SIZE];
//     char file_contents[SIZE];
//     char send_id[SIZE];
// } user_dir_info;

void login(users *, int *, char *, char *);
void load_csv(FILE *, users *, int *, char *, char *);
void join_func(int , new_user , users *, char *, char *, char *);
void find_user_func(users *, int , char *);
void send_message(char* , char *);
int user_exists(const char* , char *);
void mk_txt(FILE *, char *, char *, struct tm *);
void text_list_func(char *, char *);
int compare(const void *, const void *);
void myinfo(users *, int *, char *, char *);

int main() 
{
    FILE *fp;
    users users_info[SIZE];  // 구조체 배열선언
    int count = 0; // 현재 저장된 회원 수(처음엔 0)

    char login_u_ID[SIZE];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    struct tm *ptr_tm = &tm;
    int user_input;

    //현재 유저 작업 디렉토리 경로
    char cwd_path[SIZE];
    char csv_path[SIZE];                        
    if ( getcwd(cwd_path, SIZE) == NULL )
    {
        fprintf(stderr, "Error: getcwd() cannot execute\n") ;
        exit(1); 
    }
    strcpy(csv_path,cwd_path);  //cwd주소를 scv주소에 복사
    strcat(csv_path,"/DB/test_u_DB.csv");   //csv주소 지정  "DB파일명 바뀌면 이걸 건들것!"
    // 구조체에 저장된 데이터 출력
    // printf("[ 회원 정보 ]\n");
    // for (int i = 0; i < count; i++) 
    // {
    //     printf("user_num : %d, user_id : %s, user_pw : %s, user_name : %s, user_age : %d, user_phone : %s-%s-%s\n", users_info[i].u_num, users_info[i].u_ID, users_info[i].u_PW, users_info[i].u_name, users_info[i].u_age, users_info[i].u_phone_1, users_info[i].u_phone_2, users_info[i].u_phone_3);
    // }
    char folder[SIZE] = "/ID";
    load_csv(fp, users_info, &count, csv_path, cwd_path);

    int main_switch = 1;
    while (main_switch)
    {
        CLEAR;
        
        printf("███████╗██╗    ██╗    ███╗   ███╗ █████╗ ██╗██╗     \n");
        printf("╚════██║██║    ██║    ████╗ ████║██╔══██╗██║██║     \n");
        printf("███████║█████████║    ██╔████╔██║███████║██║██║     \n");
        printf("╚════██║██╔════██║    ██║╚██╔╝██║██╔══██║██║██║     \n");
        printf("███████║██║    ██║    ██║ ╚═╝ ██║██║  ██║██║███████╗\n");
        printf("╚══════╝╚═╝    ╚═╝    ╚═╝     ╚═╝╚═╝  ╚═╝╚═╝╚══════╝\n\n\n");
        printf("             1. 로그인     2. 회원가입\n");
        scanf("%d",&user_input);
        if (user_input == 1)
        {
            // 로그인 코드
            login(users_info, &count, login_u_ID, cwd_path);

            char c_input;
            int i_input;
            printf("w.작성   a.목록   t.전송   f.유저찾기   c.정보확인   z.종료\n");
            scanf("%s",&c_input);       
            if (c_input == 'w')
            {
                //w.txt 만들기
                mk_txt(fp, login_u_ID, cwd_path, ptr_tm);
                continue;
            }
            else if (c_input == 'a')
            {
                //a.txt 목록
                // if ()
                // {
                //     printf("Warning 메일함에 메일이 90개이상입니다.");
                // }
                
                text_list_func(login_u_ID, cwd_path);
                continue;          
            }
            else if (c_input == 't')
            {
                //t.txt 전송하기
                printf("1.작성 후 전송   2.목록 불러오기\n");
                scanf("%d",&i_input);
                if (i_input == 1)
                {
                    mk_txt(fp, login_u_ID, cwd_path, ptr_tm);
                }
                else if (i_input == 2)
                {
                    text_list_func(login_u_ID, cwd_path);
                }
                else
                {
                    printf("잘못된입력, txt목록출력\n");
                }
                send_message(login_u_ID, cwd_path);
                continue;          
            }
            else if (c_input == 'f')
            {
                //f.유저 찾기 코드
                find_user_func(users_info, count, cwd_path);
                continue;         
            }
            else if (c_input == 'c')
            {
                //c.내정보확인
                myinfo(users_info, &count, login_u_ID, cwd_path);
                continue;
            }
            else if (c_input == 'z')
            {
                printf("1.로그아웃  2.종료하기");
                scanf("%d",&i_input);
                if (i_input == 1)
                {
                    break;
                }
                else if (i_input == 2)
                {
                    printf("종료합니다\n");
                    return 0;
                }           
            }
        }
        if (user_input == 2)
        {
        // 회원가입 코드
        new_user new_user_info;
        join_func(count, new_user_info, users_info, folder, csv_path, cwd_path);
        }
    }               
}

// csv파일 불러와서 구조체에 저장
void load_csv(FILE *fp, users *users_info, int *count, char *csv_path, char *cwd_path)
{
    char buffer[MAX_LINE];

    // CSV 파일 열기
    // fp = fopen("/home/iot122/Desktop/3H_mail/DB/test_u_DB.csv", "r");
    // if (fp == NULL) {
    //     perror("파일을 열 수 없습니다");
    //     // return 1;
    // }
    // CSV 파일 열기
    fp = fopen(csv_path, "r");
    if (fp == NULL) {
        perror("파일을 열 수 없습니다");
        return 1;
    }

    // 한 줄씩 읽어서 구조체에 저장
    //fgets() 한줄씩읽기
    //strtok() → 쉼표 , 기준으로 데이터 분리
    //atoi(), strcpy() → 데이터 변환 및 저장
    while (fgets(buffer, sizeof(buffer), fp) != NULL && (*count) < MAX_USERS) // 100명이 넘으면 중지
    {
        // 쉼표(,)를 기준으로 데이터 분리
        char *token = strtok(buffer, ","); 

        users_info[*count].u_num = atoi(token); // 첫 번째 값 : 사용자 번호

        token = strtok(NULL, ",");
        strcpy(users_info[*count].u_ID, token);  // 두 번째 값 : ID
        
        token = strtok(NULL, ",");
        strcpy(users_info[*count].u_PW, token);  // 세 번째 값 : PW

        token = strtok(NULL, ",");
        strcpy(users_info[*count].u_name, token);  // 네 번째 값 : 이름

        token = strtok(NULL, ",");
        users_info[*count].u_age = atoi(token); // 다섯번째 값 : 나이

        token = strtok(NULL, ",");
        strcpy(users_info[*count].u_phone_1, token); // 여섯번째 값 : 핸드폰 번호1

        token = strtok(NULL, ",");
        strcpy(users_info[*count].u_phone_2, token); // 일곱섯번째 값 : 핸드폰 번호2

        token = strtok(NULL, ",");
        strcpy(users_info[*count].u_phone_3, token); // 여덟번째 값 : 핸드폰 번호3


        if (token != NULL) 
        {
            token[strcspn(token, "\n")] = '\0'; 
            //    strcspn(token, "\n") → \n(개행 문자) 위치 찾기
            //    token[strcspn(token, "\n")] = '\0'; → 개행 문자를 \0(문자열 끝)로 바꿈
            strcpy(users_info[*count].u_phone_3, token);  // 여덟번째 값 (전화번호3)
        }
        (*count)++;  // 회원 수 증가
    }
    fclose(fp);  // 파일 닫기
}

// 로그인 함수
void login(users *users_info, int *count, char *login_u_ID,  char *cwd_path)
{
    char user_id[SIZE];
    char user_pw[SIZE];
    int plag_login=0;
    int user_input;
    while(1)
    {
        printf("아이디를 입력해 주세요 : \n");
        scanf("%s",user_id);
        while (getchar() != '\n');
        for (int i = 0 ; i < *count ; i++) // ID 중복 검사 코드
        {
            if (strcmp(user_id, users_info[i].u_ID) == 0)
            {
                printf("비밀번호를 입력해 주세요. : \n");
                scanf("%s",user_pw);
                if (strcmp(user_pw, users_info[i].u_PW) == 0)
                {
                    printf("반갑습니다. %s 님.\n",user_id);
                    login_u_ID = user_id;
                    printf("%s",login_u_ID);
                    plag_login = 1;
                    getchar();
                    break;
                }
                else
                {
                    i--;
                    continue;
                }
            }
        }
        if (plag_login == 0)
        {
            printf("아이디를 확인해 주세요.\n");
            getchar();
            continue;
        }
        break;    
    }
}

// 회원 가입 함수
void join_func(int count, new_user new_user_info, users *users_info, char *folder,  char *csv_path, char *cwd_path)
{
    
    // 회원가입하는 코드
    FILE *fp;
    printf("[회원가입]\n");
    printf("이름 : ");
    scanf("%s", new_user_info.new_name);
    printf("나이 : ");
    scanf("%d", &new_user_info.new_age);

    while(1) // 회원가입 핸드폰 번호 입력
    {    
        printf("핸드폰 번호 : ");
        scanf("%s", new_user_info.new_phone);
        if (strlen(new_user_info.new_phone) == 11) // '-' 없이 숫자만 입력한 경우
        {
            for (int i = 0 ; i < 11 ; i++)
            {
                if (i <= 2)
                {
                    new_user_info.new_phone_1[i] = new_user_info.new_phone[i];
                }
                else if (i >= 3 && i <= 6)
                {
                    new_user_info.new_phone_2[i-3] = new_user_info.new_phone[i];
                }
                else if (i >= 7 && i <= 10)
                {
                    new_user_info.new_phone_3[i-7] = new_user_info.new_phone[i];
                }
            }
            break;
        }
        else if (strlen(new_user_info.new_phone) == 13) // '-' 포함해서 입력한 경우
        {
            for (int i = 0 ; i < 13 ; i++)
            {
                if (i <= 2)
                {
                    new_user_info.new_phone_1[i] = new_user_info.new_phone[i];
                }
                else if (i >= 4 && i <= 7)
                {
                    new_user_info.new_phone_2[i-4] = new_user_info.new_phone[i];
                }
                else if (i >= 9 && i <= 12)
                {
                    new_user_info.new_phone_3[i-9] = new_user_info.new_phone[i];
                }
            }
            break;
        }
        else
        {
            printf("핸드폰 번호를 다시 입력해주세요.\n");
            continue;
        }
    }

    while(1) // 회원가입 ID 입력
    {
        int same_switch = 0; // ID 중복 스위치 (0이면 중복X / 1이면 중복O)
        printf("ID : ");
        scanf("%s", new_user_info.new_id);
        for (int i = 0 ; i < count ; i++) // ID 중복 검사 코드
        {
            if (strcmp(new_user_info.new_id, users_info[i].u_ID) == 0)
            {
                printf("이미 사용중인 ID입니다.\n");
                same_switch = 1;
                break;
            }
        }
        if ((strlen(new_user_info.new_id) < 6 || strlen(new_user_info.new_id) > 20) && (same_switch == 0))
        {
            printf("아이디가 너무 짧습니다.\n");
        }
        else if ((strlen(new_user_info.new_id) >= 6 && strlen(new_user_info.new_id)) <= 20 && (same_switch == 0))
        {
            break;
        }
    }

    while(1) // 회원가입 PW 입력
    {
        int pw_switch = 1; // 회원가입 PW 스위치, 비밀번호 완벽=1
        printf("비밀번호는 6~20자리 | 한글X | 대소문자, 숫자, 특수문자 무조건 1개 이상\n");
        printf("PW :");
        scanf("%s", new_user_info.new_pw);
        if (strlen(new_user_info.new_pw) < 6 || strlen(new_user_info.new_pw) > 20)
        {
            printf("비밀번호가 너무 짧습니다.\n");
            continue;
        }
        for (int i = 0 ; i < strlen(new_user_info.new_pw) ; i++)
        {
            if ((new_user_info.new_pw[i]<33) || (new_user_info.new_pw[i]>126)) // 영어, 숫자, 특수문자가 아닌 경우
            {
                printf("대소문자, 숫자, 특수문자만 입력하세요.\n");
                pw_switch = 0;
                break;
            }
            if ((new_user_info.new_pw[i]>=48) && (new_user_info.new_pw[i]<=57)) // 숫자가 들어간 갯수
            {
                new_user_info.char_cnt[0] += 1;
            }
            else if ((new_user_info.new_pw[i]>=65) && (new_user_info.new_pw[i]<=90)) // 대문자가 들어간 갯수
            {
                new_user_info.char_cnt[1] += 1;
            }
            else if ((new_user_info.new_pw[i]>=97) && (new_user_info.new_pw[i]<=122)) // 소문자가 들어간 갯수
            {
                new_user_info.char_cnt[2] += 1;
            }
            else if (((new_user_info.new_pw[i]>=33) && (new_user_info.new_pw[i]<=47)) || ((new_user_info.new_pw[i]>=58) && (new_user_info.new_pw[i]<=64)) || ((new_user_info.new_pw[i]>=91) && (new_user_info.new_pw[i]<=96)) || ((new_user_info.new_pw[i]>=123) && (new_user_info.new_pw[i]<=126))) // 특수문자가 들어간 갯수
            {
                new_user_info.char_cnt[3] += 1;
            }
        }
        for (int i = 0 ; i < 4 ; i++)
        {
            if (new_user_info.char_cnt[i] == 0)
            {
                printf("대소문자, 숫자, 특수문자는 1개씩 꼭 써주세요.\n");
                pw_switch = 0;
                break;
            }
        }
        // 비밀번호 한번더 확인하기
        if (pw_switch)
        {
            printf("비밀번호 확인을 위해 한번 더 입력해주세요 : ");
            scanf("%s", new_user_info.new_pw_check);
            if (strcmp(new_user_info.new_pw, new_user_info.new_pw_check) == 0)
            {
                printf("비밀번호가 일치합니다.\n");
            }
            else
            {
                printf("비밀번호가 일치하지 않습니다. 처음부터 다시 입력해주세요.\n");
                continue;
            }
        }
        break;
    }

    // 회원가입 후 DB에 저장하는 코드
    fp = fopen(csv_path, "a+");
    if (fp == NULL) 
    {
        perror("파일을 열 수 없습니다");
        // return 1;
    }
    fprintf(fp, "%d,%s,%s,%s,%d,%s,%s,%s\n", count+1, new_user_info.new_id, new_user_info.new_pw, new_user_info.new_name, new_user_info.new_age, new_user_info.new_phone_1, new_user_info.new_phone_2, new_user_info.new_phone_3);
    fclose(fp);

    // 사용자 ID 디렉토리 생성

    char new_dir[SIZE] = "/home/iot122/Desktop/3H_mail/ID/";
    strcpy(new_dir,cwd_path);
    strcat(new_dir,"/ID/");
    strcat(new_dir, new_user_info.new_id); // strcat() : 문자열 이어붙이기
    mkdir(new_dir, S_IRWXU); // S_IRWXU : 소유자 읽기, 쓰기, 실행
    char tmp_dir[SIZE]; // 임시 디렉토리
    strcpy(tmp_dir, new_dir); // strcpy() : 문자열 복사
    strcat(new_dir, folder);
    mkdir(new_dir, S_IRWXU);
    mkdir(tmp_dir, S_IRWXU);
}

// 유저 찾기 함수
void find_user_func(users *users_info, int count,  char *cwd_path)
{
    int user_find_num;
    char find_id[SIZE];
    int user_find_switch = 1;
    while(user_find_switch)
    {
        system("clear");
        printf("---------------\n");
        printf("  [유저 찾기]\n");
        printf("---------------\n");
        printf("1. ID로 찾기\n2. 이름으로 찾기\n3. 뒤로가기\n");
        scanf("%d", &user_find_num);
        system("clear");
        switch (user_find_num)
        {
            case 1: // ID로 찾기
            {
                printf("찾으려는 유저 ID를 입력하세요 (q: 뒤로가기) : "); // ID로 찾기
                scanf("%s", find_id);
                while(getchar() != '\n')
                system("clear");
                if (strcmp(find_id, "q") == 0)
                {
                    break;
                }
                else
                {
                    printf("             [ 유저 목록 ] (q: 뒤로가기)\n");
                    printf("======================================\n");
                    printf("ID\t이름\t나이\t  전화번호\n");
                    printf("--------------------------------------\n");
                    for (int i = 0 ; i < count ; i++)
                    {
                        char *ptr_find = strstr(users_info[i].u_ID, find_id);
                        if (ptr_find != NULL) // ID가 있는 경우
                        {
                            printf("%s\t%s\t%d\t%s-****-%s\n", users_info[i].u_ID, users_info[i].u_name, users_info[i].u_age, users_info[i].u_phone_1, users_info[i].u_phone_3);
                        }
                    }
                    scanf("%s", find_id);
                    if (strcmp(find_id, "q") == 0)
                    {
                        continue;
                    }
                }
            }
            case 2: // 이름으로 찾기
            {
                printf("찾으려는 유저 이름을 입력하세요 (q: 뒤로가기) : "); // ID로 찾기
                while(getchar() != '\n');
                scanf("%s", find_id);
                while(getchar() != '\n');
                system("clear");
                if (strcmp(find_id, "q") == 0) // q 누르면 뒤로가기
                {
                    continue;
                }
                else
                {
                    printf("             [ 유저 목록 ] (q: 뒤로가기)\n");
                    printf("======================================\n");
                    printf("ID\t이름\t나이\t  전화번호\n");
                    printf("--------------------------------------\n");
                    for (int i = 0 ; i < count ; i++)
                    {
                        if (strcmp(find_id, users_info[i].u_name) == 0)
                        {
                            printf("%s\t%s\t%d\t%s-****-%s\n", users_info[i].u_ID, users_info[i].u_name, users_info[i].u_age, users_info[i].u_phone_1, users_info[i].u_phone_3);
                        }
                    }
                    scanf("%s", find_id);
                    if (strcmp(find_id, "q") == 0)
                    {
                        continue;
                    }
                }
            }
            case 3: // 뒤로가기
            {
                user_find_switch = 0;
                break;
            }
        }
    }
}

// .TXT 전송하기
void send_message(char* login_u_ID,  char *cwd_path) //메세지 보내기
{
    char receiver[50];  // 받는 사람 id저장
    char content[1024];  // 메일 본문 저장
    char receiver_mail_dir_path[256]; // 받은 사람의 메시지 저장 경로
    char fake_cwd_path[SIZE];
    time_t now = time(NULL);    // 현재기반 날짜 시간정보
    struct tm* t = localtime(&now);
    char file_name[100];     // 메시지를 저장할 파일 이름

    printf("받는 사람 아이디를 입력하세요: ");
    scanf("%s", receiver);
    if (!user_exists(receiver, cwd_path)) // user_exists(receiver) 함수로 사용자 존재 여부 확인
    {
        printf("❌ 존재하지 않는 아이디입니다.\n");
        return;
    }

    // getchar();  // 버퍼 정리
    // printf("보낼 메시지를 입력하세요: ");
    // fgets(content, sizeof(content), stdin);  // 한 줄 전체 입력
    // content[strcspn(content, "\n")] = 0;    // strcspn(content, "\n")는 \n이 있는 인덱스를 찾고// 끝에 붙는 개행 문자 제거 (0이 제거됨을 나타냄)

    // 파일 이름 생성(시간 기반)
    sprintf(file_name, "[NEW]%04d_%02d_%02d_%02d_%02d_%02d.txt",
        t -> tm_year + 1900, t -> tm_mon + 1 , t -> tm_mday,
        t -> tm_hour, t -> tm_min, t -> tm_sec);

      // 받은 사람 mail_receive 경로 지정
    strcpy(fake_cwd_path, cwd_path);
    strcat(fake_cwd_path,"/ID");
    sprintf(receiver_mail_dir_path, "%s/%s/%s",fake_cwd_path, receiver, file_name);//바꿔줘야함

    
  
        // 받은 사람 inbox에 저장
    FILE* fp = fopen(receiver_mail_dir_path, "w");
    if (fp) 
    {
        fprintf(fp,
            "📨 보낸 사람: %s\n📬 받은 시간: %04d_%02d_%02d_%02d_%02d_%02d\n\n%s",
            login_u_ID,
            t -> tm_year + 1900, t -> tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec,
            content);
        fclose(fp);
        printf("✅ 메시지가 전송되었습니다!\n");
    }
    else
    {
        printf("❌ 메시지 저장 실패 (경로 또는 권한 확인 필요)\n");
    }
}

// 존재하는 유저인지 확인
int user_exists(const char* save_user_id,  char *cwd_path) 
{
    char receiver_mail_dir_path[256];
    sprintf(receiver_mail_dir_path, "users/%s", save_user_id);
    DIR* dir = opendir(receiver_mail_dir_path);
    if (dir) 
    {
        closedir(dir);
        return 1;
    }
    return 0;
}

// 텍스트 파일 만들어서 해당유저 아이디 / 경로에 저장
void mk_txt(FILE *fp, char *login_u_ID, char *cwd_path, struct tm *tm)
{
    //내가 만든 파일 경로  ==  현재작업위치/ID/u_id/1_시간_제목.txt
    char tmp_cwd_path[SIZE];
    strcpy(tmp_cwd_path,cwd_path);  //현재 작업 위치를 tmp_cwd_path에 저장
    strcat(tmp_cwd_path,"/ID"); //현재 경로와 ID 디렉토리로 경로 재설정
    char u_id[SIZE];
    strcpy(u_id,login_u_ID);   //로그인된 아이디를 u_id배열에 저장
    strcat(tmp_cwd_path, "/");
    strcat(tmp_cwd_path,u_id); //현재작업위치/ID/u_id 문자열이 tmp_cwd_path 에 저장됨
    char text_inside[SIZE];

    char mailtype_time[SIZE];   //메일타입(받은메일, 저장된메일)과 시간정보 섞인 문자열
    char mon[SIZE];
    char mday[SIZE];
    char hour[SIZE];
    char min [SIZE];
    char sec [SIZE];
    char input_m_name[SIZE];  //유저가 입력한 메일 제목
    
    //시간정보에 일의 자리면 0붙이는 코드
    if ((tm->tm_mon+1)<10)
        sprintf(mon, "0%d", tm->tm_mon+1);
    else
        sprintf(mon, "%d", tm->tm_mon+1);  
    if ((tm->tm_mday)<10)
        sprintf(mday, "0%d", tm->tm_mday);
    else
        sprintf(mday, "%d", tm->tm_mday);
    if ((tm->tm_hour)<10)
        sprintf(hour, "0%d", tm->tm_hour);
    else
        sprintf(hour, "%d", tm->tm_hour);
    if ((tm->tm_min)<10)
        sprintf(min, "0%d", tm->tm_min);
    else
        sprintf(min, "%d", tm->tm_min);
    if ((tm->tm_sec)<10)
        sprintf(sec, "0%d", tm->tm_sec);
    else
        sprintf(sec, "%d", tm->tm_sec);

 
    sprintf (mailtype_time, "/1_%d_%s_%s_%s_%s_%s_", tm -> tm_year+1900, mon, mday, hour, min, sec);
    
    printf("메일 제목을 입력해 주세요.\n");
    scanf("%s",input_m_name);
    strcat(tmp_cwd_path, mailtype_time);
    strcat(tmp_cwd_path, input_m_name);
    strcat(tmp_cwd_path, ".txt");
    printf("%s\n",tmp_cwd_path);

    fp = fopen(tmp_cwd_path, "a");

    if (fp == NULL)
    {
        perror("파일을 열 수 없습니다");
        // return 1;
    }
    printf("문자열을 입력하시오. 입력을 끝내려면 'end'를 누르시오. \n");
    gets(text_inside); // 문자열 입력
    while(strcmp(text_inside, "end")) 
    { // 입력된 문자열이 end가 아니면 loop 반복
        strcat(text_inside, "\n"); // 문자열에 "\n" 추가
        fputs(text_inside, fp); // 3. 파일 입출력 : 문자열 fp가 가리키는 파일에 출력
        gets(text_inside);
    }

    fclose(fp);
}

// 텍스트 목록
void text_list_func(char *login_u_ID,  char *cwd_path)
{
    int start_page = 1; // 시작 페이지
    int end_page; // 끝 페이지
    int start_num = 0; // 각 페이지 시작 번호
    int end_num = PAGE; // 각 페이지 끝 번호
    
    char user_page_select[SIZE];
    while (1)
    {
        char new_delete_file_name[SIZE];

        char base_dir[SIZE] = "/home/boeun/Desktop/3H_mail_copy2/ID/";
        strcpy(base_dir,cwd_path);
        strcat(base_dir,"/ID/");
        char user_dir[SIZE] = "/home/boeun/Desktop/3H_mail_copy2/ID/";
        strcpy(user_dir,cwd_path);
        strcat(user_dir,"/ID/");
        strcat(user_dir, login_u_ID); // 해당 ID의 디렉토리
        DIR *dir; // DIR 타입은 디렉토리를 읽어 들이기 위한 스트림을 관리하는 구조체
        struct dirent *entry;
        char files[MAX_MAIL_NUM][SIZE];

        int mail_count = 0;
        dir = opendir(user_dir); // tmp_dir 디렉토리 열기, opendir(): path로 지정한 디렉토리를 읽기 위해 open하고 DIR 타입에 대한 포인터를 반환
        if (dir == NULL)
        {
            perror("디렉토리 열기 실패");
            // return 0;
        }
        while ((entry = readdir(dir)) != NULL) // 디렉토리 포인터(dir)를 인자로 받고, 디렉토리의 정보를 읽어와 dirent라는 구조체(entry)에 값을 저장 후, dirent 구조체 포인터를 반환
        {
    
            if ((!strcmp(".", entry->d_name)) || (!strcmp("..", entry->d_name)))
            {
                continue;
            }
            strcpy(files[mail_count], entry->d_name); // files에 파일 이름 복사
            mail_count++; // 메일 하나 읽고 메일 갯수 +1
    
        }
        closedir(dir); // 디렉토리 닫기
    
        qsort(files, mail_count, sizeof(files[0]), compare); // 내림차순 정렬하기
        if (mail_count % 10 != 0) // 총 메일 갯수가 10으로 나누어 떨어지지 않은 경우
        {
            end_page = (mail_count / 10) + 1;
        }
        else
        {
            end_page = mail_count / 10; // 총 메일 갯수가 10으로 나누어 떨어지는 경우
        }
        printf("   [📬 %s님의 메일함 📬]\n\n", login_u_ID);
        for (int j = start_num ; j < end_num ; j++)
        {
            if (j < mail_count) // 목록 갯수만큼만 출력
            {
                printf("(%3d) %s\n", j+1, files[j]);
            }
        }
        printf("\n             %d / %d\n\n", start_page, end_page);
        printf("[e. 다음 | q. 이전 | z. 돌아가기 ]\n");
        scanf("%s", user_page_select);
        system("clear");
        int str_len = strlen(user_page_select);

        int str_num[SIZE];
        char str_str[SIZE] = {};
        int str_switch = 1; // 숫자가 끝나면 0으로 바껴요
        for (int i = 0 ; i <str_len ; i++)
        {
            if ((isdigit(user_page_select[i]) != 0) && (str_switch == 1)) // 숫자인 경우
            {
                str_num[i] = atoi(&user_page_select[i]);
                str_switch == 0;
            }
            else // 문자인 경우
            {
                strcpy(str_str, &user_page_select[i]);
            }
        }
        if (strcmp(user_page_select, "e") == 0) // 다음 페이지를 눌렀을 경우
        {
            if (start_page == end_page) // 끝 페이지에서 다음 페이지 선택하는거 방지
            {
                continue;
            }
            else
            {
                start_page += 1;
                start_num += PAGE;
                end_num += PAGE;
            }
        }
        else if (strcmp(user_page_select, "q") == 0) // 이전 페이지를 눌렀을 경우
        {
            if (start_page == 1) // 시작 페이지에서 이전 페이지 선택하는거 방지
            {
                continue;
            }
            else
            {
                start_page -= 1;
                start_num -= PAGE;
                end_num -= PAGE;
            }
        }
        else if (strcmp(user_page_select, "z") == 0) // 돌아가기를 눌렀을 경우
        {
            break;
        }
        else if (str_num[0] != 0) // 숫자를 입력했을 때
        {
            if (strlen(str_str) == 0) // 숫자만 입력했을 때
            {

                printf("[r. 읽기 | d. 삭제하기]]\n");
                scanf("%s", user_page_select);
                strcpy(str_str, user_page_select);
            }
            if (strcmp(str_str, "r") == 0) // 읽기를 선택했을 때
            {
                system("clear");
                strcpy(user_dir, "/home/boeun/Desktop/3H_mail_copy2/ID/");
                strcpy(user_dir,cwd_path);
                strcat(user_dir,"/ID/");
                strcat(user_dir, login_u_ID);
                strcat(user_dir, "/");
                strcat(user_dir, files[str_num[0] - 1]);

                FILE *fp;
                char c;
                fp = fopen(user_dir, "r"); // 읽기모드로 파일 열기
                printf("[ 제목 : %s ]\n\n", files[str_num[0] - 1]);
                while((c = fgetc(fp)) != EOF)
                {
                    putchar(c);
                }
                fclose(fp);

                if (files[str_num[0] - 1][0] == '[') // 텍스트 제목에 [NEW]가 붙어있을 때
                {
                    for (int i = 5 ; i < strlen(files[str_num[0] - 1]) ; i++) // [NEW]없앤 파일이름 new_delete_file_name에 저장
                    {
                        new_delete_file_name[i-5] = files[str_num[0] - 1][i];
                    }
                    rename(user_dir, strcat(strcat(strcat(base_dir, login_u_ID), "/"), new_delete_file_name));
                }

                printf("\n\n(z. 돌아가기)"); // z누르면 페이지 화면으로 돌아가기
                scanf("%s", user_page_select);
                system("clear");
                continue;
            }
            else if (strcmp(str_str, "d") == 0) // 삭제를 선택했을 때
            {
                system("clear");
                printf("진짜 삭제하시나요?\n");
                printf("[ 1. 삭제 | 2. 삭제X ]");
                scanf("%s", user_page_select);
                system("clear");
                if (strcmp(user_page_select, "1") == 0)
                {
                    strcpy(user_dir, "/home/boeun/Desktop/3H_mail_copy2/ID/");
                    strcpy(user_dir,cwd_path);
                    strcat(user_dir,"/ID/");
                    strcat(user_dir, login_u_ID);
                    strcat(user_dir, "/");
                    strcat(user_dir, files[str_num[0] - 1]);
                    remove(user_dir);
                }
                else
                {
                    continue;
                }
                
            }
        }
    }
}

// 텍스트 목록에서 필요한 함수
int compare(const void *a, const void *b)
{
    return -(strcmp((char *)a, (char *)b));
}

// 내정보확인
void myinfo(users *users_info, int *count, char *login_u_ID,  char *cwd_path)
{
    for(int i = 0;i<count;i++)
    {
        if(strcmp(users_info[i].u_ID, login_u_ID)==0)
        {
            printf("M Y  I D: %s\n", users_info[i].u_ID);
            printf("이    름: %s\n", users_info[i].u_name);
            printf("나    이: %d\n", users_info[i].u_age);
            printf("전화번호: %s-%s-%s\n\n\n", users_info[i].u_phone_1,users_info[i].u_phone_2, users_info[i].u_phone_3);
            printf("돌아가시려면 아무키나 입력해 주세요.");
        }
    }
    getchar();
    while(getchar()!='\n');
}